#include <redfish/msg_session.hpp>
#include <ipmi/apps.hpp>
#include <ipmi/ipmi.hpp>
// #include "ipmi/apps.hpp"

uint8_t _checksum(void *buf, unsigned int buflen, uint8_t checksum_initial)
{
    register unsigned int i = 0;
    register int8_t checksum = checksum_initial;

    if (buf == NULL || buflen == 0)
        return (checksum);

    for (; i < buflen; i++)
        checksum = (checksum + ((uint8_t *)buf)[i]) % 256;

    return (checksum);
}

uint8_t ipmi_checksum(void *buf, unsigned int buflen)
{
    uint8_t checksum;
    checksum = _checksum(buf, buflen, 0);
    return (-checksum);
}

std::vector<uint8_t> ipmi_msg_process_packet(std::vector<uint8_t> packet_in)
{
    ipmi_msg_in msg_in;
    ipmi_msg_out *msg_out = new ipmi_msg_out;
    std::vector<uint8_t> packet_out;

    uint8_t res_buf[128] = {0,};
    uint8_t req_buf[128] = {0,};
    uint8_t res_len = 0;
    uint8_t res_cc;

    if(packet_in.size() == 7)
    {
        memcpy(&msg_in, packet_in.data(), sizeof(uint8_t)*7);
    }
    else if(packet_in.size() > 7)
    {
        memcpy(&msg_in, packet_in.data(), sizeof(uint8_t) * 6); // 초기 6바이트는 IPMI 메시지 구성요소임
        msg_in.data = new uint8_t[packet_in.size() - 7];
        memcpy(msg_in.data, packet_in.data() + (6 * sizeof(uint8_t)), packet_in.size() - 7);
    }
    else
    {
        memcpy(&msg_in, packet_in.data(), packet_in.size());
        msg_in.data = new uint8_t;
    }

    if(msg_in.checksum_header != ipmi_checksum(&msg_in, 2)){}
    if(msg_in.checksum_data != ipmi_checksum(packet_in.data(), packet_in.size() - 1))
    {}

    msg_out->rq_sa = msg_in.rq_sa;
    msg_out->uni_netfn_rqlun.netfn_rqlun.net_fn = msg_in.uni_netfn_rslun.netfn_rslun.net_fn+1;
    msg_out->uni_netfn_rqlun.netfn_rqlun.rq_lun = msg_in.uni_rqseq_rqlun.rqseq_rqlun.rq_lun;
    msg_out->checksum_header = ipmi_checksum(msg_out, 2);
    msg_out->rs_sa = msg_in.rs_sa;
    msg_out->uni_rqseq_rslun.rqseq_rslun.rq_seq = msg_in.uni_rqseq_rqlun.rqseq_rqlun.rq_seq;
    msg_out->uni_rqseq_rslun.rqseq_rslun.rs_lun = msg_in.uni_netfn_rslun.netfn_rslun.rs_lun;
    msg_out->cmd = msg_in.cmd;

    req_buf[0] = msg_in.uni_netfn_rslun.netfn_rslun.net_fn;
    req_buf[1] = msg_in.cmd;

    memcpy(req_buf + 2, packet_in.data() + 6, packet_in.size() - 7);
    //std::copy(packet_in.begin() + 6, packet_in.end()-7, req_buf + 2);
    //&req_buf[2] = packet_in.begin() + 6;

    res_len = 0;

    res_cc = ipmi_handle(1, req_buf, packet_in.size() - 5, res_buf, &res_len);
    
    if((res_cc != 0xff) && (res_cc != 0xfe))
    {
        msg_out->data = new uint8_t[res_len - 2];
        memcpy(msg_out->data, &(res_buf[2]), res_len - 2);
        msg_out->data_len = res_len - 2;
    }
    else {
        msg_out->data = new uint8_t;
        msg_out->data[0] = res_cc;
        msg_out->data_len = 1;
    }

    int pout_length = IPMI_MSG_HEADER_LENGTH + msg_out->data_len +1;
    uint8_t temp_pout[pout_length];
    std::fill_n(temp_pout, pout_length, 0);
    memcpy(temp_pout, msg_out, IPMI_MSG_HEADER_LENGTH);
    //memcpy(packet_out.data(), msg_out, IPMI_MSG_HEADER_LENGTH);

    if(msg_out->data_len != 0)
    {
        //packet_out.insert(packet_out.end(), msg_out->data, msg_out->data + msg_out->data_len);
        memcpy(temp_pout + IPMI_MSG_HEADER_LENGTH, msg_out->data, msg_out->data_len);
        delete msg_out->data;
    }
    else
        temp_pout[IPMI_MSG_HEADER_LENGTH] = msg_out->data[0];
  

    temp_pout[pout_length-1] = ipmi_checksum(temp_pout, pout_length - 1);
    //packet_out[packet_out.size() - 1] = ipmi_checksum(packet_out.data(), packet_out.size() - 1);  
    packet_out.insert(packet_out.end(), temp_pout, temp_pout+sizeof(temp_pout));
    //for(uint8_t &c : temp_pout)
	//    packet_out.push_back(static_cast<uint8_t> (c));

    delete msg_out;

    return packet_out;

}

std::vector<uint8_t> ipmi_session_process_packet(std::vector<uint8_t> packet_in)
{
    ipmi_session_auth_t ipmi_session_in;
    std::vector<uint8_t> packet_out;
    uint8_t ipmi_session_length;

    ipmi_session_in.auth_valid = 0;
    ipmi_session_in.auth_type = packet_in[IPMI_SES_HEADER_OFFSET_AUTH_TYPE];
    std::copy(packet_in.begin() + IPMI_SES_HEADER_OFFSET_SEQ_NUM, packet_in.begin() + IPMI_SES_HEADER_OFFSET_SEQ_NUM + 8, &ipmi_session_in.seq_num);

    if(ipmi_session_in.auth_type == IPMI_SES_HEADER_AUTH_TYPE_NONE)
    {
        ipmi_session_length = IPMI_SES_HEADER_LENGTH_NO_AUTH;
        ipmi_session_in.ipmi_msg_len = packet_in[IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_NONE];
    }
    else
    {
        ipmi_session_length = IPMI_SES_HEADER_LENGTH_AUTH;
        ipmi_session_in.ipmi_msg_len = packet_in[IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_TRUE];
        ipmi_session_in.auth_code.resize(16);
        ipmi_session_in.auth_code.assign(packet_in.begin() + 9, packet_in.begin() + 25);
    }

    std::vector<uint8_t> ipmi_msg_packet_in, ipmi_msg_packet_out;

    ipmi_msg_packet_in.assign(packet_in.begin() + ipmi_session_length, packet_in.end());

    if(ipmi_session_in.ipmi_msg_len != 0)
    {
        ipmi_msg_packet_out = ipmi_msg_process_packet(ipmi_msg_packet_in);
	//packet_out.resize(ipmi_session_length);
	
	//memcpy(packet_out.data(), &ipmi_session_in, ipmi_session_length);
	packet_out.insert(packet_out.begin(),ipmi_msg_packet_in.begin(), ipmi_msg_packet_in.begin() + ipmi_session_length);
	//packet_out.push_back(ipmi_msg_packet_out.begin());
        packet_out.insert(packet_out.end(), ipmi_msg_packet_out.begin(), ipmi_msg_packet_out.end());

        if(ipmi_session_in.auth_type == IPMI_SES_HEADER_AUTH_TYPE_NONE)
            packet_out[IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_NONE] = ipmi_msg_packet_out.size();
        else   
            packet_out[IPMI_SES_HEADER_OFFSET_MSG_LEN_AUTH_TRUE] = ipmi_msg_packet_out.size();
 
        return packet_out;
    }
    else
    {
        return packet_out;
    } 

}


