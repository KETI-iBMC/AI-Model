#include "vsend.h"

pthread_mutex_t HAS_CLIENT_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t TRIGGER_MUTEX = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t HAS_CLIENT_SIGNAL = PTHREAD_COND_INITIALIZER;
pthread_cond_t CLIENT_LEFT_SIGNAL = PTHREAD_COND_INITIALIZER;

extern pthread_cond_t HAS_CLIENT_SIGNAL_2;
extern int video_fd;

int fps = 0;
int client_sock;
int has_client;

/// 기철 수정 
extern unsigned long Video_Status; //VGA mode change
extern unsigned long framecount ;
 extern unsigned char firstframe;
///

unsigned char *jpeg_virt_addr;
void *stream_virt_addr;
u8 jpeg_encode;

static int GetINFData(PVIDEO_ENGINE_INFO VideoEngineInfo)
{
	u8 string[81], name[80], StringToken[256];
	u32 i;
	FILE *fp;

	if (!system("grep -r AST-G5 /proc/cpuinfo"))
	{
		VideoEngineInfo->INFData.AST2500 = 1;
	}
	else
	{
		VideoEngineInfo->INFData.AST2500 = 0;
	}

	fp = fopen("/usr/sbin/video.inf", "rb");

	if (fp == NULL)
	{
		//printf("video.inf not find...");
		return 1;
	}

	while (fgets(string, 80, fp) != NULL)
	{
		sscanf(string, "%[^=] = %s", name, StringToken);
		////printf("%s ", string);
		////////////////////////////////////////////////////////////////////////////////////////////////////
		//New Configuration
		if (strcmp(name, "INPUT_SOURCE") == 0)
		{
			VideoEngineInfo->INFData.Input_Signale = (u8)(atoi(StringToken));
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
		if (strcmp(name, "COMPRESS_MODE") == 0)
		{
			VideoEngineInfo->FrameHeader.CompressionMode = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "Y_JPEG_TABLESELECTION") == 0)
		{
			VideoEngineInfo->FrameHeader.Y_JPEGTableSelector = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "JPEG_YUVTABLE_MAPPING") == 0)
		{
			VideoEngineInfo->FrameHeader.JPEGYUVTableMapping = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "JPEG_SCALE_FACTOR") == 0)
		{
			VideoEngineInfo->FrameHeader.JPEGScaleFactor = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "DOWN_SCALING_ENABLE") == 0)
		{
			VideoEngineInfo->INFData.DownScalingEnable = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "DIFFERENTIAL_SETTING") == 0)
		{
			VideoEngineInfo->INFData.DifferentialSetting = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "DESTINATION_X") == 0)
		{
			VideoEngineInfo->DestinationModeInfo.X = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "DESTINATION_Y") == 0)
		{
			VideoEngineInfo->DestinationModeInfo.Y = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "RC4_ENABLE") == 0)
		{
			VideoEngineInfo->FrameHeader.RC4Enable = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "RC4_KEYS") == 0)
		{
			for (i = 0; i < strlen(StringToken); i++)
			{
				EncodeKeys[i] = StringToken[i];
			}
		}
		if (strcmp(name, "ANALOG_DIFFERENTIAL_THRESHOLD") == 0)
		{
			VideoEngineInfo->INFData.AnalogDifferentialThreshold = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "DIGITAL_DIFFERENTIAL_THRESHOLD") == 0)
		{
			VideoEngineInfo->INFData.DigitalDifferentialThreshold = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "JPEG_ADVANCE_TABLESELECTION") == 0)
		{
			VideoEngineInfo->FrameHeader.AdvanceTableSelector = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "JPEG_ADVANCE_SCALE_FACTOR") == 0)
		{
			VideoEngineInfo->FrameHeader.AdvanceScaleFactor = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "JPEG_VISUAL_LOSSLESS") == 0)
		{
			VideoEngineInfo->FrameHeader.Visual_Lossless = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "SHARP_MODE_SELECTION") == 0)
		{
			VideoEngineInfo->FrameHeader.SharpModeSelection = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "AUTO_MODE") == 0)
		{
			VideoEngineInfo->INFData.AutoMode = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "DIRECT_MODE") == 0)
		{
			VideoEngineInfo->INFData.DirectMode = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "DELAY_CONTROL") == 0)
		{
			VideoEngineInfo->INFData.DelayControl = (USHORT)(atoi(StringToken));
		}
		if (strcmp(name, "MODE_420") == 0)
		{
			VideoEngineInfo->FrameHeader.Mode420 = (u8)(atoi(StringToken));
			if (VideoEngineInfo->FrameHeader.Visual_Lossless == 1)
			{
				VideoEngineInfo->FrameHeader.Mode420 = 0;
			}
		}
		if (strcmp(name, "VQ_MODE") == 0)
		{
			VideoEngineInfo->INFData.VQMode = (u8)(atoi(StringToken));
		}
		if (strcmp(name, "JPEG_FILE") == 0)
		{

			VideoEngineInfo->INFData.JPEG_FILE = (u8)(atoi(StringToken));
		//	//printf("JPEG = %d \n", VideoEngineInfo->INFData.JPEG_FILE);
		}
	}

	fclose(fp);
	return 0;
}

int int_width(int num)
{
	int ret;
	if (num < 0)
		num = -num;
	for (ret = 0; num > 0; num /= 10, ret++) /* NULL */
		;
	return ret;
}

int set_resolution_buffer(unsigned char* resolutionBuffer, int width, int height){
    unsigned char width_buffer[5]; 
    unsigned char height_buffer[5];

    int width_of_width = int_width(width);
    int width_of_height = int_width(height);
	
    unsigned char *tmp_buffer;
    tmp_buffer = malloc(5);

	int i;

    for(i=0 ; i<(5-width_of_width) ; i++)
        width_buffer[i] = 0;
    for(i=0 ; i<(5-width_of_height) ; i++)
        height_buffer[i] = 0;
    
    sprintf(tmp_buffer, "%d", width);
    memmove(resolutionBuffer + (9-width_of_width), tmp_buffer, strlen(tmp_buffer));

    sprintf(tmp_buffer, "%d", height);
    memmove(resolutionBuffer + (14-width_of_width), tmp_buffer, strlen(tmp_buffer));
}

//extern int errno;

// void *VideoStream(void *data)
// {
// 	PVIDEO_ENGINE_INFO VideoEngineInfo = (PVIDEO_ENGINE_INFO)data;
// 	while (1)
// 	{
// 		int host_width, host_height;
// 		int fileLen;
// 		int n = 0;
// 		int len = 0;
// 		int send_result;
// 		char *lengthBuffer;
// 		unsigned char *resolutionBuffer = malloc(sizeof(unsigned char) * 14);
// 		resolutionBuffer[0]=11; resolutionBuffer[1]=3; resolutionBuffer[2]=8; resolutionBuffer[3]=181;

// 		TRANSFER_HEADER Transfer_Header;
// 		struct ast_mode_detection mode_detection;
// 		struct ast_scaling set_scaling;
// 		struct ast_video_config video_config;

// 		char encrypt_key_cmd[512];
// 		struct ast_auto_mode auto_mode;

// 		set_scaling.enable = VideoEngineInfo->INFData.DownScalingEnable;
// 		set_scaling.x = VideoEngineInfo->DestinationModeInfo.X;
// 		set_scaling.y = VideoEngineInfo->DestinationModeInfo.Y;

// 		sprintf(encrypt_key_cmd, "echo %s > /sys/devices/platform/ast-video.0/compress0_encrypt_key", EncodeKeys);
// 				system("echo 1 > /sys/devices/platform/ast-video.0/video_reset");

// 				ast_video_vga_mode_detection(&mode_detection);


// 				video_config.engine = 0;
// 				video_config.rc4_enable = VideoEngineInfo->FrameHeader.RC4Enable;
// 				video_config.compression_mode = VideoEngineInfo->FrameHeader.CompressionMode;
// 				video_config.AutoMode = 1;
// 				video_config.compression_format = 0;
// 				video_config.capture_format = 2;
// 				video_config.YUV420_mode = VideoEngineInfo->FrameHeader.Mode420;
// 				video_config.Visual_Lossless = VideoEngineInfo->FrameHeader.Visual_Lossless;
// 				video_config.Y_JPEGTableSelector = VideoEngineInfo->FrameHeader.Y_JPEGTableSelector;
// 				ast_video_eng_config(&video_config);
// 				set_scaling.engine = 0;
// 				ast_video_set_scaling(&set_scaling);


// 				video_config.engine = 1;
// 				video_config.rc4_enable = 0;
// 				video_config.compression_mode = VideoEngineInfo->FrameHeader.CompressionMode;
// 				video_config.AutoMode = 1;
// 				video_config.compression_format = 1;
// 				video_config.Visual_Lossless = 0;
// 				video_config.YUV420_mode = VideoEngineInfo->FrameHeader.Mode420;
// 				video_config.Y_JPEGTableSelector = 4; //Fix JPEG encode in 4:20
// 				ast_video_eng_config(&video_config);
// 				set_scaling.engine = 1;
// 				ast_video_set_scaling(&set_scaling);

// 		while (1)
// 		{
// 		init:
// 			if(!has_client)
// 			{
// 				pthread_mutex_lock(&HAS_CLIENT_MUTEX);
// 				pthread_cond_wait(&HAS_CLIENT_SIGNAL, &HAS_CLIENT_MUTEX);
// 				pthread_mutex_unlock(&HAS_CLIENT_MUTEX);
// 				Video_Status = 1;
// 			}

// 			if (Video_Status == 1)
// 			{
// 				firstframe = 1;
// 				system("echo 1 > /sys/devices/platform/ast-video.0/video_reset");

// 				ast_video_vga_mode_detection(&mode_detection);

// 				VideoEngineInfo->SourceModeInfo.X = mode_detection.src_x;
// 				VideoEngineInfo->SourceModeInfo.Y = mode_detection.src_y;

// 				//printf("mode detection : %d x %d \n", mode_detection.src_x, mode_detection.src_y);
// 				host_width = mode_detection.src_x;
// 				host_height = mode_detection.src_y;
	
// 				{
// 					int idx;
// 					for(idx=4 ; idx<14 ; idx++)
// 						resolutionBuffer[idx]=0;

// 					set_resolution_buffer(resolutionBuffer, host_width, host_height);

// 					fileLen = 14;
// 					n = int_width(fileLen);
// 					len = n;
// 					lengthBuffer = malloc((len + 1) * (sizeof(unsigned char)));

// 					for (; n > 0; n--)
// 						len *= 10;
// 					len += fileLen;

// 					sprintf(lengthBuffer, "%d", len);

// 					pthread_mutex_lock(&HAS_CLIENT_MUTEX);
// 					if (has_client)
// 						send_result = send(client_sock, lengthBuffer, strlen(lengthBuffer), 0);
// 					pthread_mutex_unlock(&HAS_CLIENT_MUTEX);

// 					pthread_mutex_lock(&HAS_CLIENT_MUTEX);
// 					if (has_client)
// 						send_result = send(client_sock, resolutionBuffer, fileLen, 0);
// 					pthread_mutex_unlock(&HAS_CLIENT_MUTEX);
					
// 				}

// 				video_config.engine = 0;
// 				video_config.rc4_enable = VideoEngineInfo->FrameHeader.RC4Enable;
// 				video_config.compression_mode = VideoEngineInfo->FrameHeader.CompressionMode;
// 				video_config.AutoMode = 1;
// 				video_config.compression_format = 0;
// 				video_config.capture_format = 2;
// 				video_config.YUV420_mode = VideoEngineInfo->FrameHeader.Mode420;
// 				video_config.Visual_Lossless = VideoEngineInfo->FrameHeader.Visual_Lossless;
// 				video_config.Y_JPEGTableSelector = VideoEngineInfo->FrameHeader.Y_JPEGTableSelector;
// 				ast_video_eng_config(&video_config);
// 				set_scaling.engine = 0;
// 				ast_video_set_scaling(&set_scaling);


// 				video_config.engine = 1;
// 				video_config.rc4_enable = 0;
// 				video_config.compression_mode = VideoEngineInfo->FrameHeader.CompressionMode;
// 				video_config.AutoMode = 1;
// 				video_config.compression_format = 1;
// 				video_config.Visual_Lossless = 0;
// 				video_config.YUV420_mode = VideoEngineInfo->FrameHeader.Mode420;
// 				video_config.Y_JPEGTableSelector = 4; //Fix JPEG encode in 4:20
// 				ast_video_eng_config(&video_config);
// 				set_scaling.engine = 1;
// 				ast_video_set_scaling(&set_scaling);


// 				if(has_client)
// 					Video_Status = 0;
// 			}
// 			else
// 			{
// 				if (has_client)
// 				{
// 					pthread_mutex_lock(&TRIGGER_MUTEX);
// 					auto_mode.engine_idx = 0;
// 					if (firstframe == 1)
// 					{
// 						auto_mode.differential = 0;
// 					}
// 					else
// 					{
// 						auto_mode.differential = 1;
// 					}
// 					auto_mode.mode_change = 0;
// 					ast_video_auto_mode_trigger(&auto_mode);
// 					Video_Status = auto_mode.mode_change;
// 					Transfer_Header.Data_Length = auto_mode.total_size; 
// 					Transfer_Header.Blocks_Changed = auto_mode.block_count;
// 					pthread_mutex_unlock(&TRIGGER_MUTEX);

// 					if (Video_Status)
// 					{
// 						goto init;
// 					}

// 					if(Transfer_Header.Data_Length != 0)
// 					{
// 						fileLen = Transfer_Header.Data_Length * 4;
// 						n = int_width(fileLen);
// 						len = n;
// 						lengthBuffer = malloc((len + 1) * (sizeof(unsigned char)));

// 						for (; n > 0; n--)
// 							len *= 10;
// 						len += fileLen;

// 						sprintf(lengthBuffer, "%d", len);

// 						pthread_mutex_lock(&HAS_CLIENT_MUTEX);
// 						if (has_client)
// 							send_result = send(client_sock, lengthBuffer, strlen(lengthBuffer), 0);
// 						pthread_mutex_unlock(&HAS_CLIENT_MUTEX);

// 						pthread_mutex_lock(&HAS_CLIENT_MUTEX);
// 						if (has_client)
// 							send_result = send(client_sock, (unsigned char *)stream_virt_addr, fileLen, 0);
// 						pthread_mutex_unlock(&HAS_CLIENT_MUTEX);

// 						firstframe = 0;
// 						fps++;
// 					}
// 				}
// 			} //else
// 		}	 //while(1)
// 	}		  //while(1)
// }

// void *TcpRun(void *data)
// {
// 	int SERVER_PORT = 8877;
// 	struct sockaddr_in server_address;
// 	struct sockaddr_in client_address;
// 	int client_address_len = 0;
// 	memset(&server_address, 0, sizeof(server_address));
// 	server_address.sin_family = AF_INET;
// 	server_address.sin_port = htons(SERVER_PORT);
// 	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
// 	int listen_sock;

// 	float gap;
// 	time_t startTime = 0, endTime = 0;

// 	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
// 	{
// 		return 1;
// 	}
// 	int enable = 1;
// 	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
// 	{
//     		error("setsockopt(SO_REUSEADDR) failed");
// 	}
// 	if ((bind(listen_sock, (struct sockaddr *)&server_address,
// 			  sizeof(server_address))) < 0)
// 	{
// 		return 1;
// 	}
// 	int wait_size = 1;
// 	if (listen(listen_sock, wait_size) < 0)
// 	{
// 		return 1;
// 	}

// 	while (1)
// 	{
// 		if ((client_sock = accept(listen_sock, (struct sockaddr *)&client_address, &client_address_len)) < 0)
// 		{
// 			return 1;
// 		}
// //		system("req kvm set 1");
// 		struct sockaddr_in *tmpsock = (struct sockaddr_in *)&client_address;
// 		char *s = malloc(INET6_ADDRSTRLEN);
// 		inet_ntop(AF_INET, &(tmpsock->sin_addr), s, INET_ADDRSTRLEN); 
// 		startTime = clock();
// 		pthread_mutex_lock(&HAS_CLIENT_MUTEX);
// 		has_client = 1;
// 		pthread_cond_signal(&HAS_CLIENT_SIGNAL);
// 		pthread_cond_signal(&HAS_CLIENT_SIGNAL_2);
// 		pthread_mutex_unlock(&HAS_CLIENT_MUTEX);

// 		char *recvbuf = malloc(3);
// 		recv(client_sock, recvbuf, 3, MSG_WAITALL);
// 		pthread_mutex_lock(&HAS_CLIENT_MUTEX);
// //		system("req kvm set 0");
// 		has_client = 0;
// 		pthread_mutex_unlock(&HAS_CLIENT_MUTEX);
// 		endTime = clock();
// 		gap = (float)(endTime - startTime) / (CLOCKS_PER_SEC);

// 		close(client_sock);
// 	}
// 	close(listen_sock);
// } //tcprun



// void *JpegSnapshot(void *data)
// {
// 	float gap;
// 	time_t startTime = 0, endTime = 0;
// 	int jpegLen;

// 	struct ast_auto_mode auto_mode;
// 	int Video_Status_JPEG = 1;
	
// 	FILE *outFile;

// 	while (1)
// 	{
// 	init:
// 		if (Video_Status_JPEG == 1)
// 		{
// 			sleep(3);
// 			Video_Status_JPEG = 0;
// 		}
// 		else
// 		{
// 			sleep(JPEG_UPLOAD_TERM);

// 			pthread_mutex_lock(&TRIGGER_MUTEX);
// 			auto_mode.engine_idx = 1;
// 			auto_mode.differential = 0;
// 			auto_mode.mode_change = 0;
// 			ast_video_auto_mode_trigger(&auto_mode);
// 			jpegLen = auto_mode.total_size;
// 			Video_Status_JPEG = auto_mode.mode_change;
// 			pthread_mutex_unlock(&TRIGGER_MUTEX);
// 			if (Video_Status_JPEG)
// 			{
// 				goto init;
// 			}
// 			if (jpegLen != 0)
// 			{
// 				startTime = clock();
// 				outFile = fopen("/web/www/html/images/keti.jpeg", "wb");
// 				fwrite(jpeg_virt_addr, sizeof(unsigned char), jpegLen,  outFile);
// 				fclose(outFile); 
// 			}
// 		}
// 	} //while(1)
// }

void _run_v_server(){
        PVIDEO_ENGINE_INFO VideoEngineInfo;
        memset(&VideoEngineInfo, 0, sizeof(VideoEngineInfo));

        if (ast_video_open() < 0)
                exit(1);

        stream_virt_addr = ast_video_mmap_stream_addr();
        jpeg_virt_addr = ast_video_mmap_jpeg_addr();
        Video_Status = 1;
        has_client = 0;

        if (GetINFData(&VideoEngineInfo))
        {
                ast_video_close();
                exit(1);
        }

		pthread_t tcp_thread;
        pthread_t jpeg_thread;
        pthread_t stream_thread;

        pthread_create(&tcp_thread, NULL, TcpRun, NULL);
        // pthread_create(&jpeg_thread, NULL, JpegSnapshot, NULL);
        // pthread_create(&stream_thread, NULL, VideoStream, &VideoEngineInfo);

        pthread_join(tcp_thread, NULL);
        // pthread_join(jpeg_thread, NULL);
        // pthread_join(stream_thread, NULL);
}