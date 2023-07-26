#include "ipmi/pef_policy.hpp"
#define TBLSIZE 		8
static struct pef_policy_table policy_table[TBLSIZE];
extern char *Alert_Strings[4];

/* now used to change status(enable/disable) only */
int set_policy_ipmi(uint8_t i, struct pef_policy_entry* entry) {
	policy_table[i-1].entry.policy = entry->policy;
    (save_policy_table());
    return 0;
}


int set_policy(uint8_t index, uint8_t policy_set, uint8_t rule, uint8_t alert_num) {
	int i = index - 1;
	if (i > TBLSIZE-1) {
		printf("The index is greater than policy table size\n");
		return -1;
	}
	else {
		policy_table[i].data1 = index;
	}
	//puts("set_policy");

#if BR2_PACKAGE_NETSNMP
	policy_table[i].entry.policy = 0x10 * policy_set + rule;
#else
	policy_table[i].entry.policy = 0x10 * policy_set + 0;
#endif
	policy_table[i].entry.chan_dest = 0x10 + alert_num; // 0x10 is LAN(destination channel)
	policy_table[i].entry.alert_string_key = 0x00;
	(save_policy_table());
}

// save the table changed to file
void save_policy_table() {
	printf("Saving Policy table\n");
	FILE * fp;
	if(access(POLICY_FILE, F_OK) != 0)
		system("touch /conf/ipmi/policy.bin");
	fp = fopen(POLICY_FILE, "w+");
    fwrite(&policy_table, sizeof(policy_table), 1, fp);
	fclose(fp);
}


uint8_t delete_policy_entry(uint8_t i) {
	printf("Delete Policy Entry\n");
	i -= 1;
	policy_table[i].entry.policy = 0x00;
	policy_table[i].entry.chan_dest = 0x00;
	policy_table[i].entry.alert_string_key = 0x00;
	(save_policy_table());
}


int get_entry_by_policy_set(uint8_t policy_set, struct pef_policy_entry* matching_entries) {
	//puts("get_entry_by_policy_set");
	int count = 0;
	for (int i=0; i<TBLSIZE; i++) {
		//printf("policy_table[%d].entry.policy: %x\n", policy_table[i].entry.policy);
		uint8_t policy_set0 = 0;
		(policy_set0 = parse_policy_set(policy_table[i].entry.policy));
		//printf("policy_set: %x\n", policy_set);
		//printf("policy_set0: %x\n", policy_set0);
		if (policy_set0 == 0 || policy_set == 0)
			continue;
		if (policy_set0 == policy_set) {
			//printf("[%d] %u : %u , same policy set", i, policy_set0, policy_set);
			memcpy(&matching_entries[count], &policy_table[i].entry, sizeof(struct pef_policy_entry));
			count++;
		}	
	}
	return count;
}

uint8_t get_data1(uint8_t i) {
	i -= 1;
	return policy_table[i].data1;
}
uint8_t get_policy(uint8_t i) {
	i -= 1;
	return policy_table[i].entry.policy;
}

uint8_t get_chan_dest(uint8_t i) {
	i -= 1;
	return policy_table[i].entry.chan_dest;
}

uint8_t get_alert_string_key(uint8_t i) {
	i -= 1;
	return policy_table[i].entry.alert_string_key;
}




uint8_t get_policy_table_size() {
	return TBLSIZE ;
}

void init_policy_table() {
	FILE* fp;
    
	int nResult = access(POLICY_FILE, 0);
	char cmd[40] = {0};

	memset(policy_table, 0, sizeof(struct pef_policy_table)*TBLSIZE);

	if (nResult != 0){
		sprintf(cmd, "touch %s", POLICY_FILE);
		system(cmd);
	}
	fp = fopen(POLICY_FILE, "r+");
	for (int i=0; i<TBLSIZE; i++) {
		fread(&policy_table[i], sizeof(policy_table[i]), 1, fp);
#if !BR2_PACKAGE_NETSNMP
		policy_table[i].entry.policy = 0;
#endif
		policy_table[i].data1 = i+1;
	}
	fclose(fp);
}

int rest_alert_make_json(unsigned char dest_addr[][18]) {
	//puts("rest_alert_make_json");
	FILE *fp_json;
	fp_json = fopen(ALERT_JSON, "w");
	fprintf(fp_json, "{\n");
    fprintf(fp_json, "   \"ALERT\": [\n");
	for (int i=0; i<TBLSIZE; i++) {
            char rule[20] = "\0";
            char severity[20] = "\0";
			
            char ip[4] = "\0";
            char alert_num[3] = "\0";
            char alert_msg[50] = "\0"; 
			char policy_set[2] = "\0";
			char tmp = policy_table[i].entry.policy % 0x10;
			if (tmp >= 0x08) {
				if (tmp == 0x08)
				    strcpy(rule, "Match-Always");
				else if (tmp == 0x09)
					strcpy(rule, "Try-on-next-poliy");
			
				sprintf(alert_num, "%x", policy_table[i].entry.chan_dest % 0x10);
				//puts("alert_num");
				//uint8_t* alert_num = policy_table[i].entry.chan_dest % 0x10;

				memset(&ip, 0, sizeof(ip));
				memcpy(ip, dest_addr[i+1]+1, sizeof(char)*4); // 인덱스에 주의

				strcpy(alert_msg, Alert_Strings[policy_table[i].entry.alert_string_key]);

				sprintf(policy_set, "%x", policy_table[i].entry.policy / 0x10);
			}
			else {
			     strcpy(rule, "disabled");
			     strcpy(severity, "\0");
			     strcpy(ip, "\0");
			     strcpy(alert_num, "\0");
			     strcpy(alert_msg, "\0");
			}
			
			fprintf(fp_json, "      {\n       \"INDEX\": \"%x\",\n", policy_table[i].data1);
			fprintf(fp_json, "       \"RULE\": \"%s\",\n", rule);
			fprintf(fp_json, "       \"POLICY_SET\": \"%s\",\n", policy_set);
			fprintf(fp_json, "       \"IP\": \"%d.%d.%d.%d\",\n", ip[0], ip[1], ip[2], ip[3]);
			fprintf(fp_json, "       \"ALERT_NUM\": \"%s\",\n", alert_num);
			fprintf(fp_json, "       \"ALERT_STRINGS\": \"%s\"}\n", alert_msg);
			if (i < TBLSIZE-1) {  
				fprintf(fp_json, "       ,\n");
			}
			else
				continue;
	}
	fprintf(fp_json, "      ]\n}\n");
	fclose(fp_json);
}
