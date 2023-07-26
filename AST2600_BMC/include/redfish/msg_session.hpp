#pragma once

#ifndef __MSG_SESSION_HPP__
#define __MSG_SESSION_HPP__

#include<ipmi/ipmi.hpp>
#include <redfish/stdafx.hpp>


// responder
#define IPMI_MSG_HEADER_RSSA				0x00	// rs_sa responder slave address
#define IPMI_MSG_HEADER_NETFN_RSLNUN		0x01	// 7:2 net_fn, 1:0 rs_lun
#define IPMI_MSG_HEADER_CHECKSUM_HEADER		0x02	// checksum header
#define IPMI_MSG_HEADER_LENGTH				0x06	// length of head, see struct ipmi_msg_out
// requester
#define IPMI_MSG_HEADER_RQSA				0x03	// rq_sa requester slave address
#define IPMI_MSG_HEADER_RQSEQ_RQLNUN		0x04	// 7:2 rq_seq, 1:0 rq_lun
#define IPMI_MSG_HEADER_CMD					0x05	// rq_cmd
#define IPMI_MSG_HEADER_CHECKSUM_DATA		0x06	// checksum data

#define IPMI_SES_HEADER_OFFSET_AUTH_TYPE		0x00	// 1 byte  0 (see below)
#define IPMI_SES_HEADER_OFFSET_SEQ_NUM			0x05	// 4 bytes 1 2 3 4
#define IPMI_SES_HEADER_OFFSET_SES_ID			0x01	// 4 bytes 5 6 7 8
#define IPMI_SES_HEADER_OFFSET_MSG_AUTH_CODE	0x09	// 16 bytes, field only present when auth type != none
#define IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_NONE 0x09	// 1 byte 9
#define IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_TRUE 0x19	// 1 byte 25

#define IPMI_SES_HEADER_LENGTH_NO_AUTH			10
#define IPMI_SES_HEADER_LENGTH_AUTH				26

#define IPMI_SES_HEADER_AUTH_TYPE_NONE			0x00
#define IPMI_SES_HEADER_AUTH_TYPE_MD2			0x01
#define IPMI_SES_HEADER_AUTH_TYPE_MD5			0x02
#define IPMI_SES_HEADER_AUTH_TYPE_PW			0x04

typedef struct {
    uint8_t auth_type;
    uint32_t seq_num;
    uint32_t ses_id;
    std::vector<uint8_t> auth_code;
	uint8_t ipmi_msg_len;
	uint8_t auth_valid;
} ipmi_session_auth_t;

typedef struct  {
	unsigned char rs_sa;
	union {
		unsigned char netfn_rslun_raw;
		struct {
			unsigned char rs_lun :2;
			unsigned char net_fn :6;
		} netfn_rslun;
	} uni_netfn_rslun;
	unsigned char checksum_header;
	unsigned char rq_sa;
	union {
		unsigned char rqseq_rqlun_raw;
		struct {
			unsigned char rq_lun :2;
			unsigned char rq_seq :6;
		} rqseq_rqlun;
	} uni_rqseq_rqlun;

	unsigned char cmd;
	unsigned char checksum_data; // whole packet
	unsigned char* data;		 // often unused
} ipmi_msg_in;

typedef struct  {
	unsigned char rq_sa;
	union {
		unsigned char netfn_rqlun_raw;
		struct {
			unsigned char rq_lun :2;
			unsigned char net_fn :6;
		} netfn_rqlun;
	} uni_netfn_rqlun;
	unsigned char checksum_header;
	unsigned char rs_sa;
	union {
		unsigned char rqseq_rslun_raw;
		struct {
			unsigned char rs_lun :2;
			unsigned char rq_seq :6;
		} rqseq_rslun;
	} uni_rqseq_rslun;
	unsigned char cmd;
	// till here all field can be copied direct to outgoing packet
	unsigned char checksum_data;	// whole packet
	unsigned char data_len;
	unsigned char* data;
} ipmi_msg_out;

std::vector<uint8_t> ipmi_session_process_packet(std::vector<uint8_t> packet_in);
uint8_t ipmi_checksum(const void *buf, unsigned int buflen);
#endif
