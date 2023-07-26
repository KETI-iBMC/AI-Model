#include "ipmi/setting_service.hpp"

/**
 * @brief Alert Service 구조체
 * 
 */
alert_servce_t g_alert_service;
/**
 * @brief SSH Service 구조체
 * 
 */
ssh_service_t g_ssh_service;
/**
 * @brief BMC WEB Setting 구조체
 * 
 */

ss_t g_setting;

/**
 * @brief WEB 정보를 담고있는 구조체
 * 
 */
web_service_t g_web_service;
/**
 * @brief KVM 정보를 담고있는 구조체
 * 
 */
kvm_service_t g_kvm_service;
char web_port[6] = {0};

int get_nginx_port() {
	char str_port[10] = {0};
	FILE *fp = popen(GET_WEB_PORT, "r");

	while(fgets(str_port, 10, fp) != NULL){}

	return atoi(str_port);
}
int init_setting_service(){
	int init_flag = 0;

	if (init_ssh_service() != 0){
		init_flag = -1;
	}
	if(init_alert_service() != 0){
		init_flag += -1;
	}
	if(init_kvm_service() != 0){
		init_flag += -1;
	}
	if(init_web_service() != 0){
		init_flag += -1;
	}
	return init_flag;
}

int load_g_setting() {
	
	int rc = init_setting_service();
	if( rc == 0)
	{
		g_setting.kvm_enables = g_kvm_service.kvm_enables;
		g_setting.ssh_enables = g_ssh_service.ssh_enables;
		g_setting.alert_enables = g_alert_service.alert_enables;
		g_setting.web_enables = g_web_service.web_enables;
		strcpy(g_setting.web_port, g_web_service.web_port);
		strcpy(g_setting.kvm_port, g_kvm_service.kvm_port);
		strcpy(g_setting.kvm_proxy_port, g_kvm_service.kvm_proxy_port);
		strcpy(g_setting.ssh_port, g_ssh_service.ssh_port);
		strcpy(g_setting.alert_port, g_alert_service.alert_port);
	}
	else
		return -1;
	
	return 0;
}

int init_ssh_service(){
	if(access(SSH_SERVICE_BIN, "r") != 0){
		g_ssh_service.ssh_enables = 1;
		strcpy(g_ssh_service.ssh_port, "22");
		return 0;
	}
	else{
		FILE *fp = fopen(SSH_SERVICE_BIN, "r");
		if(fread(&g_ssh_service, sizeof(ssh_service_t), 1, fp) < 1){
			perror("fread in load g_ssh_service failed");
			return -1;
		}
		fclose(fp);
		return 0;
	}
}

int init_kvm_service(){
	if(access(KVM_PORT_BIN, "r") != 0){
		g_kvm_service.kvm_enables = 1;
		strcpy(g_kvm_service.kvm_port, "7887");
		strcpy(g_kvm_service.kvm_proxy_port, "8877");
		return 0;
	}
	else{
		FILE *fp = fopen(KVM_PORT_BIN, "r");
		if(fread(&g_kvm_service, sizeof(kvm_service_t), 1, fp) < 1){
			perror("fread in load g_kvm_service failed");
			return -1;
		}
		fclose(fp);
		return 0;
	}
}

int init_alert_service(){
	if(access(ALERT_PORT_BIN, "r") != 0){
		g_alert_service.alert_enables = 1;
		strcpy(g_alert_service.alert_port, "25");
		return 0;
	}
	else{
		FILE *fp = fopen(ALERT_PORT_BIN, "r");
		if(fread(&g_alert_service, sizeof(alert_servce_t), 1, fp) < 1){
			perror("fread in load g_alert_service failed");
			return -1;
		}
		fclose(fp);
		return 0;
	}
}

int init_web_service(){
	if(access(WEB_PORT_BIN, "r") != 0){
		g_web_service.web_enables = 1;
		strcpy(g_web_service.web_port, "8000");
		return 0;
	}
	else{
		FILE *fp = fopen(WEB_PORT_BIN, "r");
		if(fread(&g_web_service, sizeof(web_service_t), 1, fp) < 1){
			perror("fread in load g_alert_service failed");
			return -1;
		}
		fclose(fp);
		return 0;
	}
}


int set_smtp_port(char* port) {
	if (access("/conf/.msmtprc", F_OK) != 0) {
		perror("No file");
		return -1;
	}
	char buf[128];
	sprintf(buf, "sed -i '5s/port.*/port %s/' /conf/.msmtprc", port);
	if (system(buf) == 0)
		return 0;
	else
		return -1;
}

int get_alert_status() {
	return g_setting.alert_enables;
}

#define WORK_JS_WEBPORT "sed -i -e \'112s/.*/var rest_port \\= %s/g\' /wev/www/html/work.js"
#define KVM_PROXY		"/usr/sbin/websockify %d :%d &"
#define KILL_PROXY		"ps -ef | grep websockify | grep -v grep | awk \'{print $1}\' | xargs kill -9"
#define PROXY_ADDRESS	"sed -i -e \'%ds/.*/		websockify %d :%d \\&/g\' /etc/init.d/S52ketiapps"
#define WORK_JS_PROXY	"sed -i -e \'%ds/.*/proxy_socket_%d \\= %d/g\' /web/www/html/work.js"

/**
 * @brief setting 정보 set
 * @date 21.05.20
 * @author doyoung
 */
int set_setting_service(char flag, char* str) {
	int kvm_port, proxy_port;
	char* stringend;
	char buf[128];
	FILE *fp_write;

	switch (flag) { 
		case 0:  // rest port

			strcpy(g_setting.web_port, str);
			strcpy(g_web_service.web_port, str);
			fp_write = fopen(WEB_PORT_BIN, "w");
			if(fwrite(&g_web_service, sizeof(web_service_t), 1, fp_write) < 1){
				perror("fwrite in set_setting_service failed");
				return -1;
			}
			fclose(fp_write);
            sprintf(buf, "sed -i \'150s/.*/    var port = %s;/g\' /web/www/html/js/app.js", str);
			system(buf);
			sprintf(buf, WORK_JS_WEBPORT, str);
			system(buf);

			break;
		case 1:  // ssh
			if (str[0] == '0') { // disable
				g_setting.ssh_enables = 0;
			}
			else {
				g_setting.ssh_enables = 1;
				strcpy(g_setting.ssh_port, str);
                sprintf(buf, "sed -i \'13s/.*/Port %s/g\' /etc/ssh/sshd_config", str);
				system(buf);
				system("/etc/init.d/S50sshd restart");
			}
			g_ssh_service.ssh_enables = g_setting.ssh_enables;
			strcpy(g_ssh_service.ssh_port, g_setting.ssh_port);

			fp_write = fopen(SSH_SERVICE_BIN, "w");
			if(fwrite(&g_ssh_service, sizeof(ssh_service_t), 1, fp_write) < 1){
				perror("fwrite in set_setting_service failed");
				return -1;
			}
			fclose(fp_write);

			break;
		case 2: // alert
			if (str[0] == '0') {
				g_setting.alert_enables = 0;
			}
			else {
				if (set_smtp_port(str) == 0) {
					g_setting.alert_enables = 1;
					strcpy(g_setting.alert_port, str);
				}			
				// TODO: set port
			}

			g_alert_service.alert_enables = g_setting.alert_enables;
			strcpy(g_alert_service.alert_port, g_setting.alert_port);

			fp_write = fopen(ALERT_PORT_BIN, "w");
			if(fwrite(&g_ssh_service, sizeof(ssh_service_t), 1, fp_write) < 1){
				perror("fwrite in set_setting_service failed");
				return -1;
			}
			fclose(fp_write);
			break;
		case 3: // kvm
			strcpy(g_setting.kvm_port, str);

			kvm_port = strtol(g_setting.kvm_port, &stringend, 10);
			proxy_port = strtol(g_setting.kvm_proxy_port, &stringend, 10);

			strcpy(buf, KILL_PROXY);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, KVM_PROXY, kvm_port, proxy_port);
			system(buf);
			memset(buf, 0, sizeof(buf));
			
			sprintf(buf, KVM_PROXY, kvm_port-100, proxy_port+1);
			system(buf);
			memset(buf, 0, sizeof(buf));

			//sed..
			sprintf(buf, PROXY_ADDRESS, 16, kvm_port, proxy_port);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, PROXY_ADDRESS, 17, kvm_port-100, proxy_port+1);
			system(buf);
			memset(buf, 0, sizeof(buf));

			strcpy(g_kvm_service.kvm_port, g_setting.kvm_port);
			fp_write = fopen(KVM_PORT_BIN, "w");
			if(fwrite(&g_kvm_service, sizeof(kvm_service_t), 1, fp_write) < 1){
				perror("fwrite in set_setting_service failed");
				return -1;
			}
			fclose(fp_write);

			// TODO
			break;
		case 4: // proxy
			strcpy(g_setting.kvm_proxy_port, str);

			kvm_port = strtol(g_setting.kvm_port, &stringend, 10);
			proxy_port = strtol(g_setting.kvm_proxy_port, &stringend, 10);

			strcpy(buf, KILL_PROXY);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, KVM_PROXY, kvm_port, proxy_port);
			system(buf);
			memset(buf, 0, sizeof(buf));
			
			sprintf(buf, KVM_PROXY, kvm_port-100, proxy_port+1);
			system(buf);
			memset(buf, 0, sizeof(buf));

			//sed..
			sprintf(buf, PROXY_ADDRESS, 16, kvm_port, proxy_port);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, PROXY_ADDRESS, 17, kvm_port-100, proxy_port+1);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, WORK_JS_PROXY, 110, 1, proxy_port);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, WORK_JS_PROXY, 111, 2, proxy_port+1);
			system(buf);
			memset(buf, 0, sizeof(buf));

			// TODO
			strcpy(g_kvm_service.kvm_proxy_port, g_setting.kvm_proxy_port);
			fp_write = fopen(KVM_PORT_BIN, "w");
			if(fwrite(&g_kvm_service, sizeof(kvm_service_t), 1, fp_write) < 1){
				perror("fwrite in set_setting_service failed");
				return -1;
			}
			fclose(fp_write);

			break;
		case 5: {// kvm + proxy
			int len_kvm = str[0] - '0';
			int len_proxy = str[1] - '0';

			strncpy(g_setting.kvm_port, str+2, len_kvm);
			strncpy(g_setting.kvm_proxy_port, str+2+len_kvm, len_proxy);

			kvm_port = strtol(g_setting.kvm_port, &stringend, 10);
			proxy_port = strtol(g_setting.kvm_proxy_port, &stringend, 10);

			strcpy(buf, KILL_PROXY);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, KVM_PROXY, kvm_port, proxy_port);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, KVM_PROXY, kvm_port-100, proxy_port+1);
			system(buf);
			memset(buf, 0, sizeof(buf));

			//sed..
			sprintf(buf, PROXY_ADDRESS, 16, kvm_port, proxy_port);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, PROXY_ADDRESS, 17, kvm_port-100, proxy_port+1);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, WORK_JS_PROXY, 110, 1, proxy_port);
			system(buf);
			memset(buf, 0, sizeof(buf));

			sprintf(buf, WORK_JS_PROXY, 111, 2, proxy_port+1);
			system(buf);
			memset(buf, 0, sizeof(buf));
			
			strcpy(g_kvm_service.kvm_port, g_setting.kvm_port);
			strcpy(g_kvm_service.kvm_proxy_port, g_setting.kvm_proxy_port);
			fp_write = fopen(KVM_PORT_BIN, "w");
			if(fwrite(&g_kvm_service, sizeof(kvm_service_t), 1, fp_write) < 1){
				perror("fwrite in set_setting_service failed");
				return -1;
			}
			fclose(fp_write);

			break;
				}
		default:
			puts("Unknown flag in set_setting_service");
			break;
	}
/*
	FILE* fp_write = fopen(SET_FILE, "w");

	if (fwrite(&g_setting, sizeof(ss_t), 1, fp_write) < 1) {
		perror("fwrite in set_setting_service failed");
		fclose(fp_write);
		return -1;
	}

	fclose(fp_write);
	*/
	return 0;
}

/**
 * @brief setting 정보 get
 * @date 21.05.20
 * @author doyoung
 */
int get_setting_service(char* res) {
	json::value obj = json::value::object();
	json::value SETTING_SERVICE = json::value::object();

	if ((access(SSH_SERVICE_BIN, F_OK) != 0) || (access(KVM_PORT_BIN, F_OK) != 0) || (access(ALERT_PORT_BIN, F_OK) != 0) || (access(WEB_PORT_BIN, F_OK) != 0)) {	
		if (init_setting_service() != 0)
			return FAIL;
	}

	SETTING_SERVICE["WEB_PORT"] = json::value::string(U(g_setting.web_port));
	SETTING_SERVICE["SSH_ENABLES"] = json::value::number(g_setting.ssh_enables);
	SETTING_SERVICE["SSH_PORT"] = json::value::string(U(g_setting.ssh_port));
	SETTING_SERVICE["ALERT_ENABLES"] = json::value::number(g_setting.alert_enables);
	SETTING_SERVICE["ALERT_PORT"] = json::value::string(U(g_setting.alert_port));
	SETTING_SERVICE["KVM_ENABLES"] = json::value::number(g_setting.kvm_enables);
	SETTING_SERVICE["KVM_PORT"] = json::value::string(U(g_setting.kvm_port));
	SETTING_SERVICE["KVM_PROXY_PORT"] = json::value::string(U(g_setting.kvm_proxy_port));
	obj["SETTING_SERVICE"] = SETTING_SERVICE;

	strcpy(res, obj.serialize().c_str());
	return obj.serialize().length();
}
