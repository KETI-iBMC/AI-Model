#include "ipmi/channel.hpp"
/*
Ipmichannel ipmiChannel[9] = {Ipmichannel(0x1, 0x1, 0), Ipmichannel(0x4, 0x1, 1), Ipmichannel(0xC, 0x5, 2), Ipmichannel(3),
	Ipmichannel(4), Ipmichannel(5), Ipmichannel(0xC, 0x5, 6), Ipmichannel(0xC, 0x5, 7), Ipmichannel(0xC, 0x5, 8)};

*/
void Ipmichannel::getChannelInfo(ipmi_res_t *response, uint8_t *res_len, uint8_t _ch_num)
{
    uint8_t *data = &response->data[0];

    response->cc = CC_SUCCESS;
    *data++ = _ch_num;
    *data++ = this->medium_type;
    *data++ = this->protocol_type;
    *data++ = this->session_info;
    *data++ = 0xF2;
    *data++ = 0x1B;
    *data++ = 0x00;
    *data++ = 0x00;
    *data++ = 0x00;

    *res_len = data - &response->data[0];
}

void Ipmichannel::getChannelAccess(ipmi_res_t *response, uint8_t *res_len)
{
    response->cc = CC_SUCCESS;
    response->data[0] = this->alerting << 5;
    response->data[0] |= this->per_message_auth << 4;
    response->data[0] |= this->user_lvl_auth << 3;
    response->data[0] |= this->access_mode;
    response->data[1] = this->priv_limit;
    *res_len = 2;
}

void Ipmichannel::setChannelAccess(ipmi_req_t *request)
{
    this->alerting = (request->data[1] & 0x20) >> 6;
    this->per_message_auth = (request->data[1] & 0x10) >> 6;
    this->user_lvl_auth = (request->data[1] & 0x08);
    this->access_mode = (request->data[2] & 0x07);
    this->priv_limit = (request->data[2] >> 0x0F);   
}
void Ipmichannel::getChannelParams(uint8_t ch_num, int type_idx, uint8_t *data)
{
    switch(type_idx)
    {
        case 0:
            *data = this->medium_type;
        break;
        case 1:
            *data = this->protocol_type;
        break;
        case 2:
            *data = this->session_info;
        break;
    }
}

bool Ipmichannel::setChannelParams(uint8_t flag, uint8_t data)
{
    switch(flag)
    {
        case 1:
            this->medium_type = data;
        break;
        case 2:
            this->access_mode = data;
        break;
        case 3:
            this->alerting = data;
        break;
        case 4:
            this->alerting = data;
        break;
        case 5:
            this->priv_limit = data;
        break;
        case 6:
            this->user_lvl_auth = data;
        break;
        default:
            return false;
        break;

    }

    return true;
}
uint8_t Ipmichannel::getChannelMaxPriv(uint8_t chan)
{
    return this->priv_limit;
}
uint8_t Ipmichannel::getChannelAccessMode(uint8_t chan)
{
    return this->access_mode;
}
