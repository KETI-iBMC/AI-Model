#include "kvmrecv.h"
#include <sys/msg.h>
#include <sys/ipc.h>

char k_input[BUF_LEN];
int m_input_x;
int m_input_y;

int reset_cur;

int hold_b1;
int hold_b2;

int hold_shift;
int hold_ctrl;
int hold_alt;

int sock_1;

int k_fd;
int m_fd;

char* dummy_data="dummy";

pthread_mutex_t K_INPUT_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t M_INPUT_MUTEX = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t K_INPUT_SIGNAL = PTHREAD_COND_INITIALIZER;
pthread_cond_t M_INPUT_SIGNAL = PTHREAD_COND_INITIALIZER;

pthread_cond_t HAS_CLIENT_SIGNAL_2;
extern pthread_mutex_t HAS_CLIENT_MUTEX;

extern int has_client;

int keyboard_fill_report(char report[8], char buf[BUF_LEN], int* hold)
{       
        char *tok = strtok(buf, " ");
        int key = 0;
        int i = 0;
        
        for (; tok != NULL; tok = strtok(NULL, " ")) {

                if (strcmp(tok, "--quit") == 0)
                        return -1;

                if (strcmp(tok, "--hold") == 0) {
                        *hold = 1;
                        continue;
                }

                if (key < 6) {
                        for (i = 0; kval[i].opt != NULL; i++)
                                if (strcmp(tok, kval[i].opt) == 0) {
                                        report[2 + key++] = kval[i].val;
                                        break;
                                }
                        if (kval[i].opt != NULL)
                                continue;
                }

                if (key < 6){
                        if ( (tok[0] >= 0x41) && (tok[0] <= 0x5a) ) {
                                report[2 + key++] = tok[0] - 61;
                                continue;
                        } else if( (tok[0] >= 0x31) && (tok[0] <= 0x39) ) {
				report[2 + key++] = tok[0] - 19;
				continue;
			} else if( (tok[0] >= 0x61) && (tok[0] <= 0x69) ) {
                                report[2 + key++] = tok[0] - 8;
                                continue;
                        } else if( tok[0] == 0x30 ) {
				report[2 + key++] = 0x27;
                                continue;
			} else if( tok[0] == 0x60 ) {
                                report[2 + key++] = 0x62;
                                continue;
                        }
		}

                for (i = 0; kmod[i].opt != NULL; i++)
                        if (strcmp(tok, kmod[i].opt) == 0) {
                                report[0] = report[0] | kmod[i].val;
                                break;
                        }
                if (kmod[i].opt != NULL)
                        continue;

                if (key < 6)
                        fprintf(stderr, "unknown option: %s\n", tok);
        }
        return 8;
}

void keyboard_run()
{
        int fd = 0;
        char buf[BUF_LEN];
        int cmd_len;
        char report[8];
        int to_send = 8;
        int hold = 0;
        int retval, i;
        char filename[11] = "/dev/hidg0";

        if ((fd = open(filename, O_RDWR, 0666)) == -1) {
                perror(filename);
                return 3;
        }
	k_fd = fd;

	while(1) {
                if(!has_client) {
                        memset(buf, 0, sizeof(buf));
                        pthread_mutex_lock(&HAS_CLIENT_MUTEX);
                        pthread_cond_wait(&HAS_CLIENT_SIGNAL_2, &HAS_CLIENT_MUTEX);
                        pthread_mutex_unlock(&HAS_CLIENT_MUTEX);
                }
                pthread_mutex_lock(&K_INPUT_MUTEX);
		pthread_cond_wait(&K_INPUT_SIGNAL, &K_INPUT_MUTEX);
		memcpy(buf, k_input, BUF_LEN);
		pthread_mutex_unlock(&K_INPUT_MUTEX);
		//printf("key control : %s\n", buf);

		memset(report, 0x0, sizeof(report));
		hold = 0;

		to_send = keyboard_fill_report(report, buf, &hold);

		if (to_send == -1)
			break;

		if (write(fd, report, to_send) != to_send) {
			perror(filename);
			return 5;
		}
		if (!hold) {
			memset(report, 0x0, sizeof(report));
			if (write(fd, report, to_send) != to_send) {
				perror(filename);
				return 6;
			}
		}
	}
	close(fd);
        return 0;
}

void parse_k_input(char* input){
	char tmp_char[5];
	char result_char[BUF_LEN];
	memcpy(tmp_char, input, 5);
	
	char tmp_c;
	int length, i, j, ten;
	char ac_k_val=0;
	tmp_c = tmp_char[0];
	if(tmp_c == '1')
		length = 1;
	else if (tmp_c == '2')
		length = 2;
	else if (tmp_c == '3')
		length = 3;
	else {
		printf("length error at parse_k_input\n");
		return 0;
	}
	ten = 1;
	
	for(i=(5-length) ; i<5 ; i++){
		ac_k_val += (tmp_char[i]-48)*(ten);
		ten*=10;
	} 

	memset(k_input, 0, BUF_LEN);
	memset(result_char, 0, BUF_LEN);

	if(ac_k_val == 16){
		hold_shift = 1;
		return;
	} else if(ac_k_val == 17){
		hold_ctrl = 1;
		return;
	} else if(ac_k_val == 18){
		hold_alt = 1;
		return;
	}

	if(hold_shift)
                strcat(k_input, "--left-shift ");
        
        if(hold_ctrl)
                strcat(k_input, "--left-ctrl ");
        
        if(hold_alt)
                strcat(k_input, "--left-alt ");

	switch(ac_k_val){
		case 8 : 
			strcat(k_input, kval[2].opt);
			k_input[strlen(k_input)] = ' ';		
			break;
		case 9 : 
                        strcat(k_input, kval[3].opt);
                        k_input[strlen(k_input)] = ' ';		
			break;	
		case 13 :
                        strcat(k_input, kval[0].opt);
                        k_input[strlen(k_input)] = ' ';		
			break;
		case 20 : 
                        strcat(k_input, kval[5].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 27 : 
                        strcat(k_input, kval[1].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 32 : 
                        strcat(k_input, kval[4].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 33 : 
                        strcat(k_input, kval[20].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 34 : 
                        strcat(k_input, kval[23].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 35 : 
                        strcat(k_input, kval[22].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 36 : 
                        strcat(k_input, kval[19].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 37 : 
                        strcat(k_input, kval[25].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 38 : 
                        strcat(k_input, kval[28].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 39 : 
                        strcat(k_input, kval[24].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 40 :
                        strcat(k_input, kval[26].opt);
                        k_input[strlen(k_input)] = ' ';
                        break;
                case 45:
                        strcat(k_input, kval[18].opt);
                        k_input[strlen(k_input)] = ' ';
                        break;
                case 46:
                        strcat(k_input, kval[21].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 91:
                        strcat(k_input, kmod[6].opt);
                        k_input[strlen(k_input)] = ' ';
                        break;
                case 93:
                        strcat(k_input, kmod[7].opt);
                        k_input[strlen(k_input)] = ' ';
                        break;
                case 106 : 
                        strcat(k_input, kval[42].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 107 : 
                        strcat(k_input, kval[44].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 109 : 
                        strcat(k_input, kval[43].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 110 : 
                        strcat(k_input, kval[45].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 111 : 
                        strcat(k_input, kval[41].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 112 : 
                        strcat(k_input, kval[6].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 113 : 
                        strcat(k_input, kval[7].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 114 : 
                        strcat(k_input, kval[8].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 115 : 
                        strcat(k_input, kval[9].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 116 : 
                        strcat(k_input, kval[10].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 117 : 
                        strcat(k_input, kval[11].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 118 : 
                        strcat(k_input, kval[12].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 119 : 
                        strcat(k_input, kval[13].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 120 : 
                        strcat(k_input, kval[14].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 121 : 
                        strcat(k_input, kval[15].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 122 : 
                        strcat(k_input, kval[16].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 123 : 
                        strcat(k_input, kval[17].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 144 : 
                        strcat(k_input, kval[29].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;	
                case 186 : 
                        strcat(k_input, kval[33].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 187 : 
                        strcat(k_input, kval[30].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 188 : 
                        strcat(k_input, kval[36].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 189 :
                        strcat(k_input, kval[40].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 190 : 
                        strcat(k_input, kval[37].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 191 : 
                        strcat(k_input, kval[38].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 192 : 
                        strcat(k_input, kval[35].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 219 : 
                        strcat(k_input, kval[31].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 220 : 
                        strcat(k_input, kval[32].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 221 : 
                        strcat(k_input, kval[39].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
                case 222 : 
                        strcat(k_input, kval[34].opt);
                        k_input[strlen(k_input)] = ' ';		
                        break;
		default : 
			result_char[0] = ac_k_val;
			result_char[1] = ' ';

			strcat(k_input, result_char);
			break;
	}
}

void mouse_run()
{
        int fd = 0;
        char buf[BUF_LEN];
        int cmd_len;
        char report[8];
        int to_send = 8;
        int hold = 0;
        int retval, i;
        char filename[11] = "/dev/hidg1";

        if ((fd = open(filename, O_RDWR, 0666)) == -1) {
                perror(filename);
                return 3;
        }

	m_fd = fd;

	char go_zero[12] = "128 128";
	int ready=0;
	int idx;

        int left_val, clx, cly, dx, dy, mx, my=0;
	char cx = 0x08;
        while(1) {
                pthread_mutex_lock(&M_INPUT_MUTEX);
                pthread_cond_wait(&M_INPUT_SIGNAL, &M_INPUT_MUTEX);

                memset(report, 0x0, sizeof(report));

                to_send = 5;
		
		if(hold_b1)
			report[0] = 1;
		else if(hold_b2)
			report[0] = 2;
		else
			report[0] = 0;
 
                report[1] = (0xff & (m_input_x));
                report[2] = (0xff & (m_input_x >> 8));
                report[3] = (0xff & (m_input_y));
                report[4] = (0xff & (m_input_y >> 8));

                if (write(fd, report, to_send) != to_send)
                {
                        perror(filename);
//                        return 5;
                }

                pthread_mutex_unlock(&M_INPUT_MUTEX);
        }
        close(fd);
        return 0;
}

void parse_m_input(char* input) {
	char tmp_char[15];
        char result_char[BUF_LEN];
        memcpy(tmp_char, input, 15);

        int x_len, y_len;
        int length, i, j, ten, width, height = 0;

        int start_x=2;
        int start_y=8;

	x_len = tmp_char[start_x] - 48;
	y_len = tmp_char[start_y] - 48;

        if(tmp_char[0] == '1')
                hold_b1=1;
	else if(tmp_char[0] == '2')
		hold_b1=0;
        else if(tmp_char[0] == '3')
                hold_b2=1;
	else if(tmp_char[0] == '4')
		hold_b2=0;

        ten = 1;
	for(i=start_x+x_len ; i>start_x ; i--){
		width += (tmp_char[i]-48) * ten;
		ten *= 10;
	}
	ten = 1;
	for(j=start_y+y_len ; j>start_y ; j--){
		height += (tmp_char[j]-48) * ten;
		ten *= 10;
        }
	if(width >= 0)
	{
            m_input_x = width;
	}
	else
	{
	    printf("mouse input source is invalid\n");
	    m_input_x = 0;
	}

	if(height >= 0)
	{
            m_input_y = height;
	}
	else
	{
	    printf("mouse input source is invalid\n");
	    m_input_y = 0;
	}
}

void _run_km_server() {
	pthread_t keyboard_thread;
	pthread_t mouse_thread;
	pthread_t ending_thread;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

// 	pthread_create(&keyboard_thread, NULL, keyboard_run, NULL);
//     pthread_create(&mouse_thread, NULL, mouse_run, NULL);
	//pthread_create(&ending_thread, &attr, waiting_death, NULL);

        int SERVER_PORT = 8878;
        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(SERVER_PORT);

        // htonl: host to network long: same as htons but to long
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        // create a TCP socket, creation returns -1 on failure
        int listen_sock_1;
        if ((listen_sock_1 = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                printf("could not create listen socket\n");
                return 1;
        }
	int enable = 1;
	if (setsockopt(listen_sock_1, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		    error("setsockopt(SO_REUSEADDR) failed");
        // bind it to listen to the incoming connections on the created server
        // address, will return -1 on error
        if ((bind(listen_sock_1, (struct sockaddr *)&server_address, sizeof(server_address))) < 0) {
                printf("could not bind socket\n");
                return 1;
        }

        int wait_size = 16;  // maximum number of waiting clients, after which
        // dropping begins
        if (listen(listen_sock_1, wait_size) < 0) {
                printf("could not open socket for listening\n");
                return 1;
        }

        // socket address used to store client address
        struct sockaddr_in client_address;
        int client_address_len = 0;

	char *buf = malloc(25);
	char *close_test_buf = malloc(5);

        while (true) {
       //         int sock_1;
                if ((sock_1 = accept(listen_sock_1, (struct sockaddr *)&client_address,&client_address_len)) < 0) {
                        printf("could not open a socket to accept data\n");
                        return 1;
		}
                
		int a, b, c, d = 0;
		char sym_buf[15];
		unsigned char * buf;
                while(1){
			//mutex_lock
                        c = recv(sock_1, sym_buf, 15,  MSG_WAITALL);//keysym
                        if(sym_buf[1] == '0') {
				parse_k_input(sym_buf);
				pthread_cond_signal(&K_INPUT_SIGNAL);
			} else if(sym_buf[1] == '1') {
                                pthread_mutex_lock(&M_INPUT_MUTEX);
				m_input_x=0;
                                m_input_y=0;
				parse_m_input(sym_buf);
				pthread_cond_signal(&M_INPUT_SIGNAL);
                                pthread_mutex_unlock(&M_INPUT_MUTEX);
			} else if(sym_buf[1] == '2') {
				if(sym_buf[3] == '6')
					hold_shift = 0;
				else if(sym_buf[3] == '7')
					hold_ctrl = 0;
				else if(sym_buf[3] == '8')
					hold_alt = 0;
			} else if(sym_buf[1] == '3') {

                        } else if(sym_buf[0] == 'c' && sym_buf[1] == 'l'){
                                close(sock_1);
                                break;
                        } else {
				printf("error : invalid symcode type %s\n", sym_buf);
			}
//			printf("recv : %s\n", sym_buf);
                        if(!has_client) {
                                close(sock_1);
                                break;
                        }
                }
        }
        close(listen_sock_1);

	pthread_join(keyboard_thread, NULL);
	pthread_join(mouse_thread, NULL);
	return 0;
}


