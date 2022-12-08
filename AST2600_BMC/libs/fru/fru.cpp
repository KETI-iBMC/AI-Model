/** @file fru.hpp
* @author Park Ki Cheol
* @date 2021-04-13
* @brief 본 문서는 x86 서버보드 기술 개발의 KETI-IPMI 소프트웨어에 대한 설명입니다.\n
* FRU 헤더파일에 대한 설명이  포함되어있습니다.
* @version 0.0.1
*/
#include<ipmi/sensor_define.hpp>
#include <ipmi/fru.hpp>
std::map<uint8_t, std::map<uint8_t, Ipmifru>> fru_rec;
extern std::map<uint8_t, std::map<uint8_t, Ipmisdr>> sdr_rec;
extern int g_Tmax;

// string chassis_type_desc_fru[100]={
//     "Unspecified", "Other", "Unknown",
// 	"Desktop", "Low Profile Desktop", "Pizza Box",
// 	"Mini Tower", "Tower",
// 	"Portable", "LapTop", "Notebook", "Hand Held",
// 	"Docking Station", "All in One", "Sub Notebook",
// 	"Space-saving", "Lunch Box", "Main Server Chassis",
// 	"Expansion Chassis", "SubChassis", "Bus Expansion Chassis",
// 	"Peripheral Chassis", "RAID Chassis", "Rack Mount Chassis",
// 	"Sealed-case PC", "Multi-system Chassis", "CompactPCI",
// 	"AdvancedTCA", "Blade", "Blade Enclosure"
// };

// [테스트] Fru Chassis type string 초기화
vector<string> tmp{
	"Unspecified", "Other", "Unknown",
	"Desktop", "Low Profile Desktop", "Pizza Box",
	"Mini Tower", "Tower",
	"Portable", "LapTop", "Notebook", "Hand Held",
	"Docking Station", "All in One", "Sub Notebook",
	"Space-saving", "Lunch Box", "Main Server Chassis",
	"Expansion Chassis", "SubChassis", "Bus Expansion Chassis",
	"Peripheral Chassis", "RAID Chassis", "Rack Mount Chassis",
	"Sealed-case PC", "Multi-system Chassis", "CompactPCI",
	"AdvancedTCA", "Blade", "Blade Enclosure"
};
vector<string> Ipmifru::chassis_type_desc_fru = tmp;

// 확인
static int g_rsv_id = 0x1;
int plat_fru_num_entries(void);
int plat_fru_num_entries(void)
{
	return fru_rec.size();
}

// Special RecID value for first and last (IPMI/Section 31)
#define SDR_RECID_FIRST 0x0000
#define SDR_RECID_LAST 0xFFFF

#define FRU_INDEX_MIN 0x1d
#define FRU_INDEX_MAX 0x1d
int plat_fru_get_entry(int rsv_id, int read_rec_id, std::vector<uint8_t> *v_rec, int *next_rec_id)
{
	int index = 0;

	if (rsv_id != g_rsv_id)
		return -1;

	std::map<uint8_t, std::map<uint8_t, Ipmifru>>::iterator iter;

	if (read_rec_id == 0)
		index = fru_rec.begin()->first;
	else if (read_rec_id == 1)
		index = (fru_rec.end()--)->first;
	else
		index = read_rec_id - 1;

	if (plat_fru_num_entries() == 0)
		return -1;

	if ((index < FRU_INDEX_MIN) || (index > FRU_INDEX_MAX))
		return -1;

	if (index < fru_rec.begin()->first || index >= fru_rec.end()--->first)
		return -1;

	// *v_rec = fru_rec.find(1)->second.;
	// cout<<"fru_rec complete"<<endl;
	// for(auto ptr = fru_rec.begin() ; ptr != fru_rec.end() ; ptr++)
	// {
	//     if(ptr->first == index)
	//     {
	//         sensor_thresh_t *test = ptr->second.find(index)->second.sdr_get_entry();
	//         printf("ptr : %d / sensor name : %s / index : %d\n", ptr->first, test->str, index);
	//         *v_rec = ptr->second.find(index)->second.sdr_get_rec_to_vector();
	//         break;
	//     }
	// }

	if (read_rec_id == SDR_RECID_FIRST)
		*next_rec_id = ++read_rec_id + 1;
	else
		*next_rec_id = ++read_rec_id;

	if (*next_rec_id > (fru_rec.end()--)->first)
		*next_rec_id = SDR_RECID_LAST;

	return 0;
}
uint8_t keti_ipmi_checksum(unsigned char *d, int s)
{
	uint8_t c = 0;
	uint8_t x = 0;
	for (x = 0; x < s; x++)
	{
		c += d[x];
	}
	return (-c);
}

void storage_get_fru_info(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
	printf("Get FRU ID Information");

	uint8_t *data = &response->data[0];
	std::vector<uint8_t> entry;
	ipmi_req_t *req = request;
	ipmi_res_t *res = response;
	
	
	unsigned char id = req->data[0];
	res->cc = CC_SUCCESS;
	
	//plat_fru_device_init();
	Ipmifru fru_this = fru_rec.find(0)->second.find(0)->second;

	printf("\nname = %s\n\n", fru_this.product.name);
	printf("header id= %d\n\n", fru_this.fru_header.id);
	printf("header version= %d\n\n", fru_this.fru_header.version);
	printf("header internal= %d\n\n", fru_this.fru_header.internal);
	printf("header chassis= %d\n\n", fru_this.fru_header.chassis);
	printf("header board= %d\n\n", fru_this.fru_header.board);
	printf("chassis.part_number= %s\n\n", fru_this.fru_chassis.part_number);
	printf("fru_chassis.serial= %s\n\n", fru_this.fru_chassis.serial);
	printf("fru_chassis.type %d\n\n", fru_this.fru_chassis.type);
	
	*data++ = 0x00; //sizeof(g_fruid_info[id]);
	//data += 2;
	*data++ = 0x01;
	*data++ = 0x00; // word or byte
	fru_this.header_flag = 0;

	*res_len = data - &res->data[0];
	return;
}
/**
 * @brief fru 장치 추가
 * @details fru 개수만큼 반복해서 fru 디바이스 추가
 * @todo 현재는 KTNF-18A 장치만 추가하게 되어있음
 * info만 정상동작 확인
 * 
 */
//fru 추가 부분
//한번만 저장되도록 변경해야함
void plat_fru_device_init(void)
{
	cout << "fru_device_init" << endl;
	std::map<uint8_t, Ipmifru> inner;

	uint8_t rec_id = 0;
	fru_header_t g_fru_header;
	fru_chassis_info_t g_fru_chassis;
	fru_board_info_t g_fru_board;
	fru_product_info_t g_fru_product;
	g_fru_header.id = 0;
	g_fru_header.version = 1;
	g_fru_header.internal = 0;
	g_fru_header.board = 4;
	g_fru_header.product = 2;
	g_fru_header.chassis = 3;
	g_fru_header.multi = 0;
	strcpy(((char *)g_fru_chassis.part_number), "KTNF-18A");
	strcpy(((char *)g_fru_chassis.serial), "6CU252VN6P");
	//chassis_flag[i] = 1;
	
	g_fru_board.mfg_date[0] = 99;//0;
	g_fru_board.mfg_date[1] = 96;//0xC0;
	g_fru_board.mfg_date[2] = 97;//0x1B;
	g_fru_board.mfg_date[3] = 128;//0xBA;
	strcpy(((char *)g_fru_board.mfg), "KTNF.");
	strcpy(((char *)g_fru_board.product), "KM-H620");
	strcpy(((char *)g_fru_board.serial), "PTW1N931P0001");
	strcpy(((char *)g_fru_board.part_number), "BNX1A-PWO1A");
	strcpy(((char *)g_fru_product.mfg), "KTNF.");
	strcpy(((char *)g_fru_product.name), "KR422-SA1");
	strcpy(((char *)g_fru_product.part_number), "KXAP-B0001");
	strcpy(((char *)g_fru_product.version), "1.0");
	strcpy(((char *)g_fru_product.serial), "temp_KETI-215");
	strcpy(((char *)g_fru_product.asset_tag), "NR422-SA1-0001");

	g_fru_chassis.type = 23;
	strcpy(((char *)g_fru_chassis.part_number), "KTNF-18A");
	strcpy(((char *)g_fru_chassis.serial), "6CU252VN6P");

	//product_flag[i] = 1;
	Ipmifru infru(g_fru_header, g_fru_chassis, g_fru_board, g_fru_product);
	infru.header_flag = 1;
	infru.chassis_flag = 1;
	infru.board_flag = 1;
	infru.product_flag = 1;
	inner.insert(std::make_pair(rec_id, infru));
	cout << "fru_device_init success " << endl;
	fru_rec.insert(std::make_pair(rec_id, inner));
	rec_id++;

	plat_fru_data_init();
	printf("plat_fru_data_init\n");
}

void storage_get_fru(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
	uint8_t *data = &response->data[0];
	std::vector<uint8_t> entry;
	ipmi_req_t *req = request;
	ipmi_res_t *res = response;
	unsigned char id = req->data[0];
	unsigned char start = req->data[1];
	unsigned char offset = req->data[2];
	unsigned char start_offset = 0;
	unsigned char len = req->data[3];
	//unsigned char len =8;
	unsigned char read_length = MAX_READ_SIZE;
	std::vector<uint8_t> v;
	//현재 0만 존재
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	unsigned char checksum_data[7];
	unsigned char edit_offset = 0;
	//id
	static int test = 0;

	unsigned char checksum_value = 0;
	checksum_value = keti_ipmi_checksum(checksum_data, 7);
	test++;

	cout << "Get FRU ID Data" << endl;
	res->cc = CC_SUCCESS;

	if (start == 0 && offset == 0 && ((len == 8) || (len == MIN_READ_SIZE) || (len == MAX_READ_SIZE)))
	{ //for header
		if (len == MIN_READ_SIZE || len == MAX_READ_SIZE)
		{
			fru_this->edit_flag = 1;
		}
		else
			fru_this->edit_flag = 0;

		*data++ = 0x08;
		// none			//---------------------

		*data++ = (fru_this->fru_header.version);
		printf("header.version = %d\n", fru_this->fru_header.version);
		*data++ = (fru_this->fru_header.internal);
		printf("header.internal = %d\n", fru_this->fru_header.internal);
		printf("header.chassis = %d\n", fru_this->fru_header.chassis);
		*data++ = (fru_this->fru_header.chassis);
		*data++ = (fru_this->fru_header.board);
		*data++ = (fru_this->fru_header.product);
		*data++ = (fru_this->fru_header.multi);
		*data++ = 0x00; //NULL SECTION
		checksum_data[0] = (fru_this->fru_header.version);
		checksum_data[1] = (fru_this->fru_header.internal);
		checksum_data[2] = (fru_this->fru_header.chassis);
		checksum_data[3] = (fru_this->fru_header.board);
		checksum_data[4] = (fru_this->fru_header.product);
		checksum_data[5] = (fru_this->fru_header.multi);
		checksum_data[6] = 0x00;
		unsigned char checksum_value = 0;
		checksum_value = keti_ipmi_checksum(checksum_data, 7);
		*data++ = checksum_value;

		printf("FRU ID : 0x%02x\n", id);
		printf("FRU Version : 0x%02x\n", fru_this->fru_header.version);
		printf("FRU Internal : 0x%02x\n", fru_this->fru_header.internal);
		printf("FRU Chassis : 0x%02x\n", fru_this->fru_header.chassis);
		printf("FRU Board : 0x%02x\n", fru_this->fru_header.board);
		printf("FRU Product : 0x%02x\n", fru_this->fru_header.product);
		printf("FRU Multi : 0x%02x\n", fru_this->fru_header.multi);
	}
	else
	{
		printf("start ! error find\n");
		if (start == fru_this->fru_header.chassis * 8)
		{
			printf("info : head info \n");
			fru_this->header_flag = 1;
		}

		else if (start == fru_this->fru_header.board * 8)
		{
			printf("info : board info \n");
			fru_this->header_flag = 2;
		}

		else if (start == fru_this->fru_header.product * 8)
		{
			printf("info : product info \n");
			fru_this->header_flag = 3;
		}

		if (fru_this->header_flag == 1)
		{
			start_offset = fru_this->fru_header.chassis * 8;
			if (fru_this->fru_header.chassis > 3)
				read_length = MAX_READ_SIZE;
			else
				read_length = MIN_READ_SIZE;

			if (start == start_offset)
			{
				printf("Start to build FRU Chassis Information\n");
				if (len > (read_length))
				{
					res->cc = 0xca;
				}
				//*data++ = 0x10;
				if (len == 0x02 || len == 0x03)
				{ // FRU PRINT
					printf("FRU PRINT\n");
					if (len == 0x03)
						memset(fru_this->write_buffer, 0, 255);
					*data++ = len;
					*data++ = 0x01;
					//*data++ = sizeof(fru_this->fru_chassis_buf[id]) / 16;
					*data++ = sizeof(fru_this->fru_chassis_buf) / 16;
				}
				// 실질적으로 FRU 데이터를 읽는 사이즈임.

				else if (len == (read_length))
				{
					printf("FRU DATA READ!!");
					fru_this->temp_write = 0; // 현재 기록된 write length

					*data++ = len;
					*data++ = 0x01;									   // fru version
					*data++ = ((fru_this->chassis_total + 8) / 8) + 1; // fru area length
					memcpy(data, fru_this->fru_chassis_buf, sizeof(unsigned char) * (len));
					fru_this->temp_write += (len);
					data += (len - FRU_CHASSIS_HEADER_LENGTH);
				}
			}
			else if (start == 0x08)
			{
				fru_this->temp_write = 0; // 현재 기록된 write length
				//*data++ = len;
				*data++ = 0x01;
				if (read_length == MIN_READ_SIZE || read_length == MAX_READ_SIZE)
				{
					if (fru_this->edit_flag == 1)
					{
						edit_offset = ((fru_this->fru_header.chassis - 1) * 8);
						int e = 0;
						for (e = 0; e < edit_offset; e++)
							*data++ = 0x00; // offset data
					}
					//*data++ = ((fru_this->chassis_total + 8) / 8) + 1; // fru area length
					//기철 - 수정 샤시 길이 chassis legnth 수정
					*data++ = 126; // fru area length

					memcpy(data, fru_this->fru_chassis_buf, sizeof(unsigned char) * (len - edit_offset));
					fru_this->temp_write += (len - edit_offset);
					data += (len - FRU_CHASSIS_HEADER_LENGTH - edit_offset);
				}
			}
			else if ((start > start_offset) || (start > start_offset - ((fru_this->fru_header.chassis - 1) * 8)))
			{

				if ((start - (fru_this->fru_header.chassis * 8)) < fru_this->chassis_total)
				{
					*data++ = len;
					memcpy(data, fru_this->fru_chassis_buf + (start - (fru_this->fru_header.chassis * 8) - FRU_CHASSIS_HEADER_LENGTH), fru_this->chassis_total - fru_this->temp_write + FRU_CHASSIS_HEADER_LENGTH);
					printf("size  -2 =%d \n ", (start - (fru_this->fru_header.chassis * 8) - FRU_CHASSIS_HEADER_LENGTH), fru_this->chassis_total - fru_this->temp_write + FRU_CHASSIS_HEADER_LENGTH);
					data += (fru_this->chassis_total - fru_this->temp_write + FRU_CHASSIS_HEADER_LENGTH);
					fru_this->temp_write += len;
				}
				else
				{
					*data++ = 0x00;
				}
			}

			else
			{
				printf("is'n it ?\n\n\n");
				res->cc = CC_DISABLED;
			}
		}

		//3번째 fru info 수행시작

		else if (fru_this->header_flag == 2)
		{
			printf("board edata info\n");
			start_offset = fru_this->fru_header.board * 8;
			if (fru_this->fru_header.board > 3)
				read_length = MAX_READ_SIZE;
			else
				read_length = MIN_READ_SIZE;
			if (start == start_offset)
			{
				printf("Start to build FRU Board Information\n");
				if (len > (read_length))
				{
					res->cc = 0xca;
				}
				//*data++ = 0x10;
				if (len == 0x02 || len == 0x03)
				{ // FRU PRINT
					printf("INFO board PRINT\n");
					if (len == 0x03)
						memset(fru_this->write_buffer, 0, 255);
					*data++ = len;
					*data++ = 0x01;
					*data++ = sizeof(fru_this->fru_board) / 16;
					printf("Info :data size -1 = %d \n", sizeof(fru_this->fru_board) / 16);
				}
				// 실질적으로 FRU 데이터를 읽는 사이즈임.

				else if (len == (read_length))
				{
					fru_this->temp_write = 0; // 현재 기록된 write length
					*data++ = len;
					*data++ = 0x02;									 // fru area version
					*data++ = ((fru_this->board_total + 8) / 8) + 1; // fru area length
					*data++ = fru_this->fru_board.mfg_date[0];
					*data++ = fru_this->fru_board.mfg_date[1];
					*data++ = fru_this->fru_board.mfg_date[2];
					*data++ = fru_this->fru_board.mfg_date[3];
					memcpy(data, fru_this->fru_board_buf, sizeof(unsigned char) * (len));
					printf("data size 0 = %d \n", sizeof(unsigned char) * (len));
					fru_this->temp_write += (len);
					data += (len - FRU_BOARD_HEADER_LENGTH);
				}
			}
			else if (start == 0x08)
			{
				fru_this->temp_write = 0; // 현재 기록된 write length
				*data++ = len;
				*data++ = 0x02;
				if (read_length == MIN_READ_SIZE || read_length == MAX_READ_SIZE)
				{
					if (fru_this->edit_flag == 1)
					{
						edit_offset = ((fru_this->fru_header.board - 1) * 8);
						int e = 0;
						for (e = 0; e < edit_offset; e++)
							*data++ = 0x00; // offset data
					}
					*data++ = ((fru_this->board_total + 8) / 8) + 1; // fru area length
					*data++ = fru_this->fru_board.mfg_date[0];
					*data++ = fru_this->fru_board.mfg_date[1];
					*data++ = fru_this->fru_board.mfg_date[2];
					*data++ = fru_this->fru_board.mfg_date[3];
					memcpy(data, fru_this->fru_board_buf, sizeof(unsigned char) * (len - edit_offset));
					printf("data size 1 = %d \n", sizeof(unsigned char) * (len - edit_offset));
					//memcpy(data, fru_this->fru_board_buf, 224);
					fru_this->temp_write += (len - edit_offset);
					data += (len - FRU_BOARD_HEADER_LENGTH - edit_offset);
				}
			}
			else if ((start > start_offset) || (start > (fru_this->fru_header.board * 8)))
			{

				if ((start - (fru_this->fru_header.board * 8)) < fru_this->board_total)
				{
					*data++ = len;
					memcpy(data, fru_this->fru_board_buf + (start - (fru_this->fru_header.board * 8) - FRU_BOARD_HEADER_LENGTH), fru_this->board_total - fru_this->temp_write + FRU_BOARD_HEADER_LENGTH);
					//memcpy(data,224);
					printf("data size 2 = %d \n", fru_this->board_total - fru_this->temp_write + FRU_BOARD_HEADER_LENGTH);
					data += (fru_this->board_total - fru_this->temp_write + FRU_BOARD_HEADER_LENGTH);
					fru_this->temp_write += len;
				}
				else
				{
					*data++ = 0x00;
				}
			}
			else
			{
				res->cc = CC_DISABLED;
			}
		}
		// //4번째 부분리드오류

		//prduct info
		else if (fru_this->header_flag == 3)
		{
			printf("start pruduct info\n\n\n");
			start_offset = fru_this->fru_header.product * 8;
			if (fru_this->fru_header.product > 3)
				read_length = MAX_READ_SIZE;
			else
				read_length = MIN_READ_SIZE;
			if (start == start_offset)
			{
				printf("Start to build FRU Product Information\n");
				if (len > (read_length))
				{
					res->cc = 0xca;
				}
				//*data++ = 0x10;
				if (len == 0x02 || len == 0x03)
				{ // FRU PRINT
					if (len == 0x03)
						memset(fru_this->write_buffer, 0, 255);
					*data++ = len;
					*data++ = 0x01;
					*data++ = sizeof(fru_this->fru_product_buf) / 16;
				}
				// 실질적으로 FRU 데이터를 읽는 사이즈임.

				else if (len == (read_length))
				{
					fru_this->temp_write = 0; // 현재 기록된 write length
					*data++ = len;
					*data++ = 0x03;									   // fru area version
					*data++ = ((fru_this->product_total + 8) / 8) + 1; // fru area length
					*data++ = 0x00;									   // fru area version
					memcpy(data, fru_this->fru_product_buf, sizeof(unsigned char) * (len));
					fru_this->temp_write += (len);
					data += (len - FRU_PRODUCT_HEADER_LENGTH);
				}
			}
			else if (start == 0x08)
			{
				fru_this->temp_write = 0; // 현재 기록된 write length
				*data++ = len;
				*data++ = 0x03;
				if (read_length == MIN_READ_SIZE || read_length == MAX_READ_SIZE)
				{
					if (fru_this->edit_flag == 1)
					{
						edit_offset = ((fru_this->fru_header.product - 1) * 8);
						int e = 0;
						for (e = 0; e < edit_offset; e++)
							*data++ = 0x00; // offset data
					}

					*data++ = ((fru_this->product_total + 8) / 8) + 1; // fru area length
					*data++ = 0x00;
					memcpy(data, fru_this->fru_product_buf, sizeof(unsigned char) * (len - edit_offset));
					fru_this->temp_write += (len - edit_offset);
					data += (len - FRU_PRODUCT_HEADER_LENGTH - edit_offset);
				}
			}
			else if ((start > start_offset) || (start > (fru_this->fru_header.product * 8)))
			{

				if ((start - (fru_this->fru_header.product * 8)) < fru_this->product_total)
				{
					*data++ = len;
					memcpy(data, fru_this->fru_product_buf + (start - (fru_this->fru_header.product * 8) - FRU_PRODUCT_HEADER_LENGTH), fru_this->product_total - fru_this->temp_write + FRU_PRODUCT_HEADER_LENGTH);
					data += (fru_this->product_total - fru_this->temp_write + FRU_PRODUCT_HEADER_LENGTH);
					fru_this->temp_write += len;
				}
				else
				{
					*data++ = 0x00;
				}
			}
			else
			{
				printf("error : product error \n");
				res->cc = CC_DISABLED;
			}
		}
	}

	*res_len = data - &res->data[0];
	printf("res_len size ??? =%d \n", res_len);
	return;
}
/**
*@brief fru Init
*@details 
*@param args 콘솔 파라미터
*@return 반환값 설명
*
*@bug 메모리 누수 있음
*@todo 버그 해결 해야함
*@exception None
*
*@see KETI-APP
*/
void plat_fru_data_init()
{
	unsigned char type_len = 0;
	unsigned char i = 0;
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	/*

			FRU Type len Description

			7:6	00 = Unicode
				01 = BCD Plus
				10 = 6bit ASCII
				11 = 8bit ASCII

			5	Reserved
			4:0 Length of following Data (String)
				11111 = Reserved.

			Ex) 0xC8 : 8Bit ASCII / 8Byte Data
				0xD2 : 8Bit ASCII / 18Byte Data
		*/

	// ID = 0
	//센서가 1개인경우로 계산

	memset(&fru_this->fru_board_buf, 0, 256);
	memset(&fru_this->fru_chassis_buf, 0, 256);
	memset(&fru_this->fru_product_buf, 0, 256);
	fru_this->board_total = 0;
	fru_this->chassis_total = 0;
	fru_this->product_total = 0;

	if (fru_this->chassis_flag == 1)
	{
		memcpy(&(fru_this->fru_chassis_buf[fru_this->chassis_total]), &fru_this->fru_chassis.type, 1);
		fru_this->chassis_total++;

		type_len = 0xC0 | strlen((char *)(fru_this->fru_chassis.part_number));
		memcpy(&(fru_this->fru_chassis_buf[fru_this->chassis_total]), &type_len, 1);
		fru_this->chassis_total++;
		memcpy(&fru_this->fru_chassis_buf[fru_this->chassis_total], fru_this->fru_chassis.part_number, strlen(((char *)fru_this->fru_chassis.part_number)));
		fru_this->chassis_total += strlen(((char *)fru_this->fru_chassis.part_number));

		//chassis_total[i]++;
		type_len = 0xC0 | strlen(((char *)fru_this->fru_chassis.serial));

		memcpy(&(fru_this->fru_chassis_buf[fru_this->chassis_total]), &type_len, 1);
		fru_this->chassis_total++;
		memcpy(&(fru_this->fru_chassis_buf[fru_this->chassis_total]), fru_this->fru_chassis.serial, strlen(((char *)fru_this->fru_chassis.serial)));
		fru_this->chassis_total += strlen(((char *)fru_this->fru_chassis.serial));
		fru_this->fru_chassis_buf[fru_this->chassis_total] = 0xC1;
		fru_this->chassis_total++;
		printf("total chassis =%d \n", fru_this->chassis_total);
	}

	if (fru_this->board_flag == 1)
	{
		type_len = 0xC0 | strlen(((char *)fru_this->fru_board.mfg));
		memcpy(&(fru_this->fru_board_buf[fru_this->board_total]), &type_len, 1);
		fru_this->board_total++;
		memcpy(&(fru_this->fru_board_buf[fru_this->board_total]), fru_this->fru_board.mfg, strlen(((char *)fru_this->fru_board.mfg)));
		fru_this->board_total += strlen(((char *)fru_this->fru_board.mfg));

		type_len = 0xC0 | strlen(((char *)fru_this->fru_board.product));
		memcpy(&(fru_this->fru_board_buf[fru_this->board_total]), &type_len, 1);
		fru_this->board_total++;
		memcpy(&(fru_this->fru_board_buf[fru_this->board_total]), fru_this->fru_board.product, strlen(((char *)fru_this->fru_board.product)));
		fru_this->board_total += strlen(((char *)fru_this->fru_board.product));

		type_len = 0xC0 | strlen(((char *)fru_this->fru_board.serial));
		memcpy(&(fru_this->fru_board_buf[fru_this->board_total]), &type_len, 1);
		fru_this->board_total++;
		memcpy(&(fru_this->fru_board_buf[fru_this->board_total]), fru_this->fru_board.serial, strlen((char *)fru_this->fru_board.serial));
		fru_this->board_total += strlen(((char *)fru_this->fru_board.serial));

		type_len = 0xC0 | strlen(((char *)fru_this->fru_board.part_number));
		memcpy(&(fru_this->fru_board_buf[fru_this->board_total]), &type_len, 1);
		fru_this->board_total++;
		memcpy(&(fru_this->fru_board_buf[fru_this->board_total]), fru_this->fru_board.part_number, strlen(((char *)fru_this->fru_board.part_number)));
		fru_this->board_total += strlen(((char *)fru_this->fru_board.part_number));
		fru_this->fru_board_buf[fru_this->board_total] = 0xC1;
		fru_this->board_total++;
		fru_this->fru_board_buf[fru_this->board_total] = 0x31;
		fru_this->board_total++;
		fru_this->fru_board_buf[fru_this->board_total] = 0xC1;
		fru_this->board_total++;
	}

	if (fru_this->product_flag == 1)
	{
		type_len = 0xC0 | strlen(((char *)fru_this->product.mfg));
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), &type_len, 1);
		fru_this->product_total++;
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), fru_this->product.mfg, strlen(((char *)fru_this->product.mfg)));
		fru_this->product_total += strlen(((char *)fru_this->product.mfg));

		type_len = 0xC0 | strlen(((char *)fru_this->product.name));
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), &type_len, 1);
		fru_this->product_total++;
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), fru_this->product.name, strlen(((char *)fru_this->product.name)));
		fru_this->product_total += strlen(((char *)fru_this->product.name));

		type_len = 0xC0 | strlen(((char *)fru_this->product.part_number));
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), &type_len, 1);
		fru_this->product_total++;
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), fru_this->product.part_number, strlen(((char *)fru_this->product.part_number)));
		fru_this->product_total += strlen(((char *)fru_this->product.part_number));

		type_len = 0xC0 | strlen(((char *)fru_this->product.version));
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), &type_len, 1);
		fru_this->product_total++;
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), fru_this->product.version, strlen(((char *)fru_this->product.version)));
		fru_this->product_total += strlen(((char *)fru_this->product.version));

		type_len = 0xC0 | strlen(((char *)fru_this->product.serial));
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), &type_len, 1);
		fru_this->product_total++;
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), fru_this->product.serial, strlen(((char *)fru_this->product.serial)));
		fru_this->product_total += strlen(((char *)fru_this->product.serial));

		type_len = 0xC0 | strlen(((char *)fru_this->product.asset_tag));
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), &type_len, 1);
		fru_this->product_total++;
		memcpy(&(fru_this->fru_product_buf[fru_this->product_total]), fru_this->product.asset_tag, strlen(((char *)fru_this->product.asset_tag)));
		fru_this->product_total += strlen(((char *)fru_this->product.asset_tag));

		fru_this->fru_product_buf[fru_this->product_total] = 0xC1;
		fru_this->product_total++;
		fru_this->fru_product_buf[fru_this->product_total] = 0x31;
		fru_this->product_total++;
		fru_this->fru_product_buf[fru_this->product_total] = 0xC1;
		fru_this->product_total++;

		printf("info : device sucess !!\n");
	}
}

/**
 * @brief 현재 FRU configuration get
 * @date 21.05.20
 * @author doyoung
 */
int rest_get_fru_config (char* res)
{
	cout << "Enter rest_get_flu_config" << endl;

	char buf[1024];
	unsigned char i = 0;
	unsigned char temp_number = 0;
	struct tm *strtm;
	time_t tval;

	json::value obj = json::value::object();
		std::vector<json::value> fru_vec;
		json::value fru = json::value::object();
			json::value BOARD = json::value::object();
			json::value PRODUCT = json::value::object();
			json::value CHASSIS = json::value::object();

	// 현재 fru 0만 존재. fru 개수 늘어날 시, size만큼 반복해야 함.
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	Ipmisdr *sdr_this = &sdr_rec.find(0)->second.find(0)->second;
	sensor_thresh_t *c_sdr;
	
	c_sdr = sdr_this->sdr_get_entry();
	
	fru["ID"] = json::value::number(i);

	string fru_device_desc = "FRU Device Description : ";
	if (fru_this->fru_header.id == 0 && i == 0)
		fru_device_desc += "Builtin FRU Device";
	else if (fru_this->fru_header.id == 0 && i != 0)
		fru_device_desc += "Not Configured";
	else{
		string sdr_str(c_sdr->str);
		fru_device_desc += sdr_str;	
	}	
	fru["DEVICE"] = json::value::string(U(fru_device_desc));

	if (fru_this->fru_header.board != 0){
		// convert char [4] -> time_t (ex. 5f ee 66 00 -> 1609459200)
		tval = (((unsigned char *)fru_this->fru_board.mfg_date)[0] << 24) | (((unsigned char *)fru_this->fru_board.mfg_date)[1] << 16) | (((unsigned char *)fru_this->fru_board.mfg_date)[2] << 8) | ((unsigned char *)fru_this->fru_board.mfg_date)[3]; 
		
		strtm = localtime(&tval);
		unsigned char Dates[100];
		strftime(Dates, 100, "%Y-%m-%d %H:%M:%S", strtm);
	
		Dates[strlen(asctime(strtm)) - 1] = '\0';
		BOARD["MFG_DATE"] = json::value::string(U((char *)Dates));
		BOARD["MFG"] = json::value::string(U((char *)fru_this->fru_board.mfg));
		BOARD["PRODUCT"] = json::value::string(U((char *)fru_this->fru_board.product));
		BOARD["SERIAL"] = json::value::string(U((char *)fru_this->fru_board.serial));
		BOARD["PART_NUM"] = json::value::string(U((char *)fru_this->fru_board.part_number));	
	} else {
		BOARD["MFG_DATE"] = json::value::string(U(""));
		BOARD["MFG"] = json::value::string(U(""));
		BOARD["PRODUCT"] = json::value::string(U(""));
		BOARD["SERIAL"] = json::value::string(U(""));
		BOARD["PART_NUM"] = json::value::string(U(""));		
	}
	
	if (fru_this->fru_header.product != 0){
		PRODUCT["NAME"] = json::value::string(U((char *)fru_this->product.name));
		PRODUCT["MFG"] = json::value::string(U((char *)fru_this->product.mfg));
		PRODUCT["VERSION"] = json::value::string(U((char *)fru_this->product.version));
		PRODUCT["SERIAL"] = json::value::string(U((char *)fru_this->product.serial));
		PRODUCT["PART_NUM"] = json::value::string(U((char *)fru_this->product.part_number));
	}else {
		PRODUCT["NAME"] = json::value::string(U(""));
		PRODUCT["MFG"] = json::value::string(U(""));
		PRODUCT["VERSION"] = json::value::string(U(""));
		PRODUCT["SERIAL"] = json::value::string(U(""));
		PRODUCT["PART_NUM"] = json::value::string(U(""));
	}

	if (fru_this->fru_header.chassis != 0){
		CHASSIS["TYPE"] = json::value::string(Ipmifru::chassis_type_desc_fru[fru_this->fru_chassis.type]);
		// CHASSIS["TYPE"] = json::value::string(U(chassis_type_desc_fru[fru_this->fru_chassis.type]));
		CHASSIS["SERIAL"] = json::value::string(U((char *)fru_this->fru_chassis.serial));
		CHASSIS["PART_NUM"] = json::value::string(U((char *)fru_this->fru_chassis.part_number));	
	}else {
		CHASSIS["TYPE"] = json::value::string(U(""));
		CHASSIS["SERIAL"] = json::value::string(U(""));
		CHASSIS["PART_NUM"] = json::value::string(U(""));
	}

	fru["BOARD"] = BOARD;
	fru["PRODUCT"] = PRODUCT;
	fru["CHASSIS"] = CHASSIS;
	fru_vec.push_back(fru);
	obj["FRU_JSON"] = json::value::array(fru_vec);

	strcpy(res, obj.serialize().c_str());
	return obj.serialize().length();
}

/**
 * @brief 현재 FRU header 정보 set
 * @date 21.05.20
 * @author doyoung
 */
void rest_set_fru_header(int id, char h_board, char h_chassis, char h_product){
	cout << "Enter rest_set_fru_header" << endl;
	
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	
	fru_this->fru_header.id = (uint8_t)id;
	fru_this->fru_header.board = h_board;
	fru_this->fru_header.chassis = h_chassis;
	fru_this->fru_header.product = h_product;

	return;
}

/**
 * @brief 현재 FRU board 정보 입력받아 변경사항 있을 때만 set.
 * @date 21.05.20
 * @author doyoung
 */
void rest_set_fru_board(int id, char f_mfg_date[4], char *f_mfg, char *f_product,
	char *f_serial, char *f_part_num){
	
	cout << "Enter rest_set_fru_board" << endl;
	
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	uint8_t update_flag = 0;
	
	cout << "t0"<<endl;
	memcpy((char *)fru_this->fru_board.mfg_date, f_mfg_date, 4);
	cout << "t1" << endl;
	cout << " fru_this " << (char *)fru_this->fru_board.mfg <<endl;
	cout << " fru mfg " << f_mfg<<endl;
	if(strcmp((char *)fru_this->fru_board.mfg, f_mfg) != 0){
		cout<<"t2"<<endl;
		update_flag++;
		memcpy((char *)fru_this->fru_board.mfg, f_mfg, LEN_MFG);
	}
	cout << " fru_this " << (char *)fru_this->fru_board.product <<endl;
	cout << " fru pro " << f_product<<endl;
	if(strcmp((char *)fru_this->fru_board.product, f_product) != 0){
		cout<<"t3"<<endl;
		update_flag++;
		memcpy((char *)fru_this->fru_board.product, f_product, LEN_PRODUCT);
	}
	cout << " fru_this " << (char *)fru_this->fru_board.serial <<endl;
	cout << " fru serial " << f_serial<<endl;
	if(strcmp((char *)fru_this->fru_board.serial, f_serial) != 0){
		cout<<"t4"<<endl;
		update_flag++;
		memcpy((char *)fru_this->fru_board.serial, f_serial, LEN_SERIAL);
	}
	cout << " fru_this " << (char *)fru_this->fru_board.part_number <<endl;
	cout << " fru f_part_num " << f_part_num<<endl;
	if(strcmp((char *)fru_this->fru_board.part_number, f_part_num) != 0){
		update_flag++;
		memcpy((char *)fru_this->fru_board.part_number, f_part_num, LEN_PART_NUM);
	}
	if(update_flag != 0)
		plat_fru_device_save();
	return;
}

/**
 * @brief 현재 FRU product 정보 입력받아 변경사항 있을 때만 set
 * @date 21.05.20
 * @author doyoung
 */
void rest_set_fru_product(int id, char* f_name, char* f_mfg, char* f_version,
	char *f_serial, char *f_part_num){
	cout << "Enter rest_set_fru_product" << endl;
	
	uint8_t update_flag = 0;
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;

	cout << "F_name" << f_name<<endl;
	cout << "f_mfg" << f_mfg<<endl;
	cout << "f_version" << f_version<<endl;
	cout << "f_serial" << f_serial<<endl;
	cout << "f_part_num" << f_part_num<<endl;

	
	if(strcmp((char *)fru_this->product.name, f_name) != 0){
		update_flag++;
		memcpy((char *)fru_this->product.name, f_name, LEN_NAME);
	}
	if(strcmp((char *)fru_this->product.mfg, f_mfg) != 0){
		update_flag++;
		memcpy((char *)fru_this->product.mfg, f_mfg, LEN_MFG);
	}
	if(strcmp((char *)fru_this->product.version, f_version) != 0){
		update_flag++;	
		memcpy((char *)fru_this->product.version, f_version, LEN_VERSION);
	}
	if(strcmp((char *)fru_this->product.part_number, f_part_num) != 0){
		update_flag++;	
		memcpy((char *)fru_this->product.part_number, f_part_num, LEN_PART_NUM);
	}
	if(strcmp((char *)fru_this->product.serial, f_serial) != 0){
		update_flag++;
		memcpy((char *)fru_this->product.serial, f_serial, LEN_SERIAL);
	}

	if(update_flag != 0)
		plat_fru_device_save();

	return;
}

/**
 * @brief 현재 FRU chassis 정보 입력받아 변경사항 있을 때만 set
 * @date 21.05.20
 * @author doyoung
 */
void rest_set_fru_chassis(int id, char* f_type, char* f_serial, char* f_part_num){
	cout << "Enter rest_set_fru_chassis" << endl;
	
	uint8_t update_flag = 0;
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	
	// [테스트] Fru Chassis 3가지 전부 변경안되는 사항 수정
	// cout << "Origin fru chassis type " << (int)fru_this->fru_chassis.type <<endl;
	cout << "Origin fru chassis type " << Ipmifru::chassis_type_desc_fru[fru_this->fru_chassis.type] <<endl;
	cout << "New fru chassis type " << f_type <<endl;
	string c_type = f_type;
	int index;
	auto pos = find(Ipmifru::chassis_type_desc_fru.begin(), Ipmifru::chassis_type_desc_fru.end(), c_type);
	if(pos != Ipmifru::chassis_type_desc_fru.end())
	{
		index = std::distance(Ipmifru::chassis_type_desc_fru.begin(), pos);
		cout << "INDEX! : " << index << endl;
		if(index != fru_this->fru_chassis.type)
		{
			cout << " Different! : " << (int)fru_this->fru_chassis.type << endl;
			fru_this->fru_chassis.type = index;
		}

	}
	else
		cout << "NO Chassis Type ! " << endl;

	// if(fru_this->fru_chassis.type != f_type){
	// 	update_flag++;
	// 	cout << "COME? 1" << endl;
	// 	fru_this->fru_chassis.type = f_type;
	// }
	cout << "After modify chassis type : " << (int)fru_this->fru_chassis.type << endl;

	cout << "Origin fru chassis serial  " << (char *)fru_this->fru_chassis.serial <<endl;
	cout << "New fru chassis serial " << f_serial <<endl;
	if(strcmp((char *)fru_this->fru_chassis.serial, f_serial) != 0){
		update_flag++;
		memcpy((char *)fru_this->fru_chassis.serial, f_serial, LEN_SERIAL);
	}
	cout << "After modify chassis serial : " << (char *)fru_this->fru_chassis.serial << endl;

	cout << "Origin fru chassis part num " << (char *)fru_this->fru_chassis.part_number <<endl;
	cout << "New fru chassis part num " << f_part_num <<endl;
	if(strcmp((char *)fru_this->fru_chassis.part_number, f_part_num) != 0){
		update_flag++;
		memcpy((char *)fru_this->fru_chassis.part_number, f_part_num, LEN_PART_NUM);
	}
	cout << "After modify chassis part num : " << (char *)fru_this->fru_chassis.part_number << endl;
	if(update_flag != 0)
		plat_fru_device_save();
	
	cout << "End fru chassis " <<endl;
	return ;
}

void plat_fru_device_save(){
	cout << "fru device _ save"<<endl;
	FILE *fru_fp;
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	unsigned char save[500] = {0,};
	
	fru_fp = fopen(IPMI_FRU_HEADER_PATH, "w");
	if(fru_fp==NULL)
	{
		cout<<"error not open :"<<IPMI_FRU_HEADER_PATH<<endl;
	}
	fwrite(&fru_this->fru_header, sizeof(fru_header_t), SENSOR_FRU_MAX, fru_fp);
	fclose(fru_fp);
	
	fru_fp = fopen(IPMI_FRU_BOARD_PATH, "w");
	fwrite(&fru_this->fru_board, sizeof(fru_board_info_t), SENSOR_FRU_MAX, fru_fp);
	fclose(fru_fp);

	fru_fp = fopen(IPMI_FRU_PRODUCT_PATH, "w");
	fwrite(&fru_this->product, sizeof(fru_product_info_t), SENSOR_FRU_MAX, fru_fp);
	fclose(fru_fp);

	fru_fp = fopen(IPMI_FRU_CHASSIS_PATH, "w");
	fwrite(&fru_this->fru_chassis, sizeof(fru_chassis_info_t), SENSOR_FRU_MAX, fru_fp);
	fclose(fru_fp);

	sprintf(save, "cp %s /conf/backup/ipmi/", IPMI_FRU_HEADER_PATH);
	system(save);
	memset(save, 0, sizeof(unsigned char) * 500);
	sprintf(save, "cp %s /conf/backup/ipmi/", IPMI_FRU_BOARD_PATH);
	system(save);
	memset(save, 0, sizeof(unsigned char) * 500);
	sprintf(save, "cp %s /conf/backup/ipmi/", IPMI_FRU_PRODUCT_PATH);
	system(save);
	memset(save, 0, sizeof(unsigned char) * 500);
	sprintf(save, "cp %s /conf/backup/ipmi/", IPMI_FRU_CHASSIS_PATH);
	system(save);
	memset(save, 0, sizeof(unsigned char) * 500);
}

