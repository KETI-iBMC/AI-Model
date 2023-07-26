#pragma once
#ifndef RMCP_HPP
#define RMCP_HPP

#include <redfish/stdafx.hpp>
// Header Offsets and Length
#define RMCP_HEADER_OFFSET_VERSION 		0x00
#define RMCP_HEADER_OFFSET_RESERVED 	0x01	 // not used
#define RMCP_HEADER_OFFSET_SEQ_NR 		0x02
#define RMCP_HEADER_OFFSET_CLASS 		0x03
#define RMCP_HEADER_LENGTH 				0x04
#define RMCP_HEADER_OFFSET_AUTH			0x04

// Header Masks
#define RMCP_HEADER_CLASS_MASK			0b00001111	// 6: ASF, 7: IPMI, etc

// Header Values
#define RMCP_HEADER_VERSION				0x06	//
#define RMCP_HEADER_RESERVED			0x00
#define PAYLOAD_TYPE_LENGTH				2

#define RMCP_HEADER_CLASS_ASF			0x06
#define RMCP_HEADER_CLASS_IPMI			0x07
#define RMCP_HEADER_SEQ_NOACK			0xff

#define AUTH_TYPE_RMCP_PLUS             0x06
#define RMCP_PACKET_OFFSET_TYPE         0x05

#define RMCP_OPENSESSION_REQUEST        0x10
#define RMCP_OPENSESSION_RESPONSE       0x11
#define RMCP_DATA_OFFSET_REQUEST        0x10

#define RAKP1_RMCP_PLUS					0x12
#define RAKP2_RMCP_PLUS					0x13
#define RAKP3_RMCP_PLUS					0x14
#define RAKP4_RMCP_PLUS					0x15

#define IPMI_RMCP_PLUS_NONE_ENCRYPT     0x00
#define IPMI_RMCP_PLUS_SHA1_NONE        0x40
#define IPMI_RMCP_PLUS					0xc0
#define IPMI_RMCP_PLUS_SOL              0xc1

#define INTF_NAME_ETH0                  "eth0"
#define INTF_NAME_ETH1                  "eth1"
#define INTF_NAME_ETH2                  "eth2"
#define INTF_NAME_ETH3                  "eth3"

class Rmcppacket
{
    private:
        struct sockaddr_in clientAddr;
        std::vector<uint8_t> sessionIdClient;
        std::vector<uint8_t> sessionIdServer;
        std::vector<uint8_t> sequenceNumber;
        uint8_t auth_type;
        uint8_t integrity_type;
        uint8_t confidential_type;
    public:
        std::vector<uint8_t> packet;
        Rmcppacket()
        {
        }
        Rmcppacket(struct sockaddr_in _cliAddress, std::vector<uint8_t> _packetIn);
        std::vector<uint8_t> getClientSessionId();
        std::vector<uint8_t> getMgmtSessionId();
        std::vector<uint8_t> getClientMacAddress();
        uint32_t getClientIpAddress();
        uint16_t getClientPort();
        bool setRmcpSessionId(std::vector<uint8_t> _clientId, std::vector<uint8_t> _managedId);
        uint8_t getRmcpAuthentication(uint8_t flag);
        bool setRmcpAuthentication(uint8_t _auth, uint8_t _integrity, uint8_t _confidential);
        bool setRmcpInformation(std::vector<uint8_t> _InPacket, struct sockaddr_in _cliAddress);
        std::vector<uint8_t> rmcp_process_packet();
        void getRmcpClientAddr();
        ~Rmcppacket()
        {
            sessionIdServer.clear();
            sessionIdClient.clear();
            sequenceNumber.clear();
            auth_type = 0;
            integrity_type = 0;
            confidential_type = 0;
        }
};
#endif