#include <pcap.h>
#include <stdio.h>

//#define WIN32
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#pragma warning(disable : 4995)
#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "ws2_32.lib")
// reference https://cloud.tencent.com/developer/article/2351541
#include <stdint.h>
#define HAVE_REMOTE
#define ETHERTYPE_IP 0x0800 /* IP */
#include <tchar.h>
#include<thread>
#include<ctime>
#include <cstdlib>
int get_Rand(int min, int max) {
    return (rand() % (max - min + 1));
}
uint16_t TOTAL_PACKET_LEN = 74;
typedef struct ip_header {
    u_char version : 4;
    u_char headerlength : 4;
    u_char cTOS;
    unsigned short totla_length;
    unsigned short identification;
    unsigned short flags_offset;
    u_char time_to_live;
    u_char Protocol;
    unsigned short check_sum;
    unsigned int SrcAddr;
    unsigned int DstAddr;
} ip_header;
typedef struct icmp_header {
    uint8_t type;            // ICMP����
    uint8_t code;            // ����
    uint16_t checksum;       // У���
    uint16_t identification; // ��ʶ
    uint16_t sequence;       // ���к�
    uint32_t init_time;      // ����ʱ���
    uint16_t recv_time;      // ����ʱ���
    uint16_t send_time;      // ����ʱ���
} icmp_header;

void from_char_vector_data_to_dns_format(const std::vector<char>& origin_data, std::vector< char>& output_data) {
    //eg www.baidu.com
    std::vector<int> dot_index;
    for (int i = 0; i < origin_data.size(); i++) {
        
        if (origin_data[i] == '.') {
            int index = i;
            int prev_index = -1;
            if (!dot_index.empty()) {
                prev_index = dot_index[dot_index.size()-1];

            }
           int count = index - prev_index-1;
             char byte0 = count & (0xff);
            output_data.push_back(byte0);
            for (int j = prev_index+1; j < index; j++) {
                output_data.push_back(origin_data[j]);
            }
            dot_index.push_back(index);
        }
        //also deal with the last
        else if (i == origin_data.size() - 1) {
            int index = i+1;
            int prev_index = -1;
            if (!dot_index.empty()) {
                prev_index = dot_index[dot_index.size() - 1];

            }
             int count = index - prev_index - 1;
             char byte0 = count & (0xff);
 
            output_data.push_back(byte0);
            for (int j = prev_index + 1; j < index; j++) {
                output_data.push_back(origin_data[j]);
            }
            dot_index.push_back(index);
        }
    }

}
void from_string_to_dns_format(const std::string& s, std::vector< char>& output_data) {
    std::vector<char> origin_data = from_string_to_char_vector(s);
    from_char_vector_data_to_dns_format(origin_data, output_data);

}
namespace Router_table {
    u_char node_2[4] = { 33, 22, 44, 22 }; // the router's ip
    unsigned int* node2_ip = (unsigned int*)node_2;
    u_char node_LAN[4] = { 10,20,99,95 };
    
    u_char node_3[4] = { 66,55,44,33 };
    u_char node_4[4] = { 1,1,1,1 };//node 4 is wan
    u_char node_1[4] = { 11, 44, 22, 33 };
  
    unsigned int* node3_ip = (unsigned int*)node_3;
    unsigned int* node4_ip = (unsigned int*)node_4;

    unsigned int* node_lan = (unsigned int*)node_LAN;
    unsigned int* node1_ip = (unsigned int*)node_1;
}
using namespace Router_table;
void calculate_total_length(const u_char* packetData) {
    unsigned int byte0;
    unsigned int byte1;
    byte1 = (unsigned int)packetData[16];
    byte0 = (unsigned int)packetData[17];
    TOTAL_PACKET_LEN = byte1 * 256 + byte0 + 14;//14 is mac
}
void get_the_information_of_the_packet(u_char* packetData, ip_header* ip_layer_head, icmp_header* icmp_layer_head) {
    ip_layer_head = (struct ip_header*)(packetData + 14);
    icmp_layer_head = (struct icmp_header*)(packetData + 14 + 20);

}
void calculate_check_sum_ip(u_char* packetData, unsigned int packet_len) {
    uint32_t cksum = 0;
    *(uint16_t*)(packetData + 14 + 10) = cksum;
    for (int i = 14; i < 14 + 4 * (packetData[14] & 0xf); i += 2) {
        cksum += *(uint16_t*)(packetData + i);
    }
    while (cksum >> 16) {
        cksum = (cksum & 0xffff) + (cksum >> 16);
    }
    cksum = ~cksum;
    *(uint16_t*)(packetData + 14 + 10) = cksum;
}
void calculate_check_sum_ICMP(u_char* packetData, unsigned int packet_len) {
    uint32_t cksum = 0;
    *(uint16_t*)(packetData + 36) = cksum; // first turn to zero
    for (int i = 34; i < packet_len; i += 2) {
        cksum += *(uint16_t*)(packetData + i);
    }
    while (cksum >> 16) {
        cksum = (cksum & 0xffff) + (cksum >> 16);
    }
    cksum = ~cksum;
    *(uint16_t*)(packetData + 36) = cksum;
}
void calculate_check_sum_DNS(u_char* packetData, unsigned int packet_len) 
{
    uint32_t cksum = 0;
    *(uint16_t*)(packetData + 40) = cksum;
    u_char udp_fake_head[] = { packetData[26],packetData[27],packetData[28],packetData[29],packetData[30],packetData[31],packetData[32],packetData[33]
    ,0x00,packetData[23],packetData[38],packetData[39] };
    for (int i = 0; i < 12; i += 2) {
        cksum += *(uint16_t *)(udp_fake_head + i);

    }

    for (int i = 34; i < packet_len; i += 2) {
        cksum += *(uint16_t*)(packetData + i);
    }
    while (cksum >> 16) {
        cksum = (cksum & 0xffff) + (cksum >> 16);
    }
    cksum = ~cksum;
    *(uint16_t*)(packetData + 40) = cksum;
}
int PrintIPHeader(const u_char* packetData);
// ����ICMP���ݰ����ڽ������Ҫͬ����Ҫ����������·���IP��, Ȼ���ٸ���ICMP���ͺŽ���,
// ���õ����ͺ�Ϊ`type 8`�������ŷ��ͺͽ������ݰ���ʱ�����
int PrintICMPHeader(const u_char* packetData) {

    struct icmp_header* icmp_protocol;

    // +14 ����������·�� +20 ����IP��
    icmp_protocol = (struct icmp_header*)(packetData + 14 + 20);

    int type = icmp_protocol->type;
    int init_time = icmp_protocol->init_time;
    int send_time = icmp_protocol->send_time;
    int recv_time = icmp_protocol->recv_time;
    printf("this is:%c,%d\n", icmp_protocol->type, icmp_protocol->type);
    if (type == 0 || type == 8) {
        // printf("����ʱ���: %d --> ����ʱ���: %d --> ����ʱ���: %d ����: ",
        // init_time, send_time, recv_time);

        switch (type) {
        case 0: {

            calculate_total_length(packetData);
            return 0;

            break;
        }
        case 8: {
            calculate_total_length(packetData);
            return 8;
            break;
        }
        default:
            break;
        }
    }
    return -1;
}
    u_char packet[10000];
struct Packet_handler 
{
    Packet_handler() { Initialize()  ;
    srand(time(0));
    }
    void Initialize() {
        if (pcap_findalldevs(&alldevs, errbuf) == -1) {

        }
        else {
        }
        int count = 0;
        device = alldevs;
        for (auto d = alldevs; d != NULL; d = d->next) {
            printf("%s %s\n", d->name, d->description);
            count++;
            if (count == 5) {
                device = d;
                break;
            }
        }
        printf("my_dev:%s %s\n", device->name, device->description);
        packet[0] = 0x00;
        packet[1] = 0x00;
        packet[2] = 0x5e;
        packet[3] = 0x00;
        packet[4] = 0x01;
        packet[5] = 0x01;

        /* set mac source*/
        packet[6] = 0x4C;
        packet[7] = 0x79;
        packet[8] = 0x6E;
        packet[9] = 0xBF;
        packet[10] = 0xA1;
        packet[11] = 0x5D;
        packet[12] = 0x08;
        packet[13] = 0x00;
        packet[14] = 0x45;
        packet[15] = 0x00;
        packet[16] = 0x00;
        packet[17] = 0xf8;
        packet[18] = 0x6e;
        packet[19] = 0x47;
        packet[20] = 0x00;
        packet[21] = 0x00;
        packet[22] = 0x80; // ttl
        packet[23] = 0x01;
        packet[24] = 0x00; // ip checksum
        packet[25] = 0x00;
        packet[26] = 0x0a; // src.ip
        packet[27] = 0x14;
        packet[28] = 0xa3;
        packet[29] = 0x73;
        packet[30] = 0x10;
        packet[31] = 0x20;
        packet[32] = 0x16;
        packet[33] = 0x07; // dest.ip
        packet[34] = 0x08; // icmp type
        packet[35] = 0x03;
        packet[36] = 0xbb;
        packet[37] = 0x6d;
        for (int i = 38; i < 1000; i++) {
            packet[i] = 0;
        }
        char s[] = "\\Device\\NPF_{E31332DC-ED9C-4ED7-A908-F4C348DAC4E8}";//THE LOCAL
        char wifi[] = "\\Device\\NPF_{5E6AAA06-F372-40E1-AFEB-34E48B6F4B92}";
        // local #3 \\Device\\NPF_{E31332DC-ED9C-4ED7-A908-F4C348DAC4E8}
        // \Device\NPF_{5E6AAA06-F372-40E1-AFEB-34E48B6F4B92}
        if ((fp = pcap_open_live(wifi, // name of the device
            65536, // portion of the packet to capture. It
            // doesn't matter in this case
            1, // promiscuous mode (nonzero means promiscuous)
            100,   // read timeout
            errbuf // error buffer
        )) == NULL) {
            fprintf(stderr,
                "\nUnable to open the adapter. %s is not supported by WinPcap\n");
        }
        if (NULL == (handler = pcap_open_live(
            wifi, 65536, // portion of the packet to capture.
            // It doesn't matter in this case
            1,   // promiscuous mode (nonzero means promiscuous)
            100, // read timeout
            errbuf))) {
            // ���ý��ܵİ���СΪ65535�������Խ������д�С�İ�
            printf("err in pcap_open : %s", errbuf);
        }
        // open the pcap
    }

    void send_the_ping_to_wan_with_ip_and_sequence_num(unsigned int source_IP, unsigned int sequence_num) {
        packet[0] = 0x00;
        packet[1] = 0x00;
        packet[2] = 0x5e;
        packet[3] = 0x00;
        packet[4] = 0x01;
        packet[5] = 0x01;

        /* set mac source*/
        packet[6] = 0x4C;
        packet[7] = 0x79;
        packet[8] = 0x6E;
        packet[9] = 0xBF;
        packet[10] = 0xA1;
        packet[11] = 0x5D;
        packet[12] = 0x08;
        packet[13] = 0x00;
        packet[14] = 0x45;
        packet[15] = 0x00;
        packet[16] = 0x00;
        packet[17] = 0x3c;
        packet[18] = 0x6f;
        packet[19] = 0x6f;
        packet[20] = 0x00;
        packet[21] = 0x00;
        packet[22] = 0x80; // ttl
        packet[23] = 0x01;
        packet[24] = 0x00; // ip checksum
        packet[25] = 0x00;
        packet[26] = 0x0a; // src.ip
        packet[27] = 0x14;
        packet[28] = 0x63;
        packet[29] = 0x5f;
        packet[30] = 0x1;
        packet[31] = 0x1;
        packet[32] = 0x1;
        packet[33] = 0x1; // dest.ip
        packet[34] = 0x08; // icmp type
        packet[35] = 0x00;
        packet[36] = 0xbb;
        packet[37] = 0x6d;
        packet[38] = 0;
        packet[39] = 1;
        TOTAL_PACKET_LEN = 74;

        for (int i = 42; i < TOTAL_PACKET_LEN; i++) {
            packet[i] = i - 41 + 6 * 16;
        }
        for (int i = 65; i < TOTAL_PACKET_LEN; i++) {
            packet[i] = i - 64 + 6 * 16;
        }
        for (int i = 0; i < 4; i++) {
            packet[26 + i] = (source_IP >> (8 * i)) & 0xff;
        }
        packet[26] = 10;
        packet[27] = 20;
        packet[28] = 255;
        packet[29] = 79;
        for (int i = 0; i < 2; i++) {
            packet[40 + i] = (sequence_num >> (8 * (1 - i))) & 0xff;
        }
        calculate_total_length(packet);
        calculate_check_sum_ICMP(packet, TOTAL_PACKET_LEN);
        calculate_check_sum_ip(packet, TOTAL_PACKET_LEN);
        send_packet(1);

    }
    void calculate_udp_length(u_char *packet) {
        unsigned int byte0;
        unsigned int byte1;
        byte1 = (unsigned int)packet[16];
        byte0 = (unsigned int)packet[17];

        unsigned int total_length_without_mac = byte1 * 256 + byte0;
        unsigned int byte = (unsigned int)packet[14]&(0xff);
        unsigned int total_length_ip = (byte & (0xf)) * 4;
        unsigned int udp_length = total_length_without_mac - total_length_ip;
        packet[38] = (udp_length >> 8) & (0xff);
        packet[39] = udp_length & (0xff);
        
    }
    void send_the_dns_request(unsigned int source_IP, unsigned int sequence_num) {
        //mac dst
        packet[0] = 0x00;
        packet[1] = 0x00;
        packet[2] = 0x5e;
        packet[3] = 0x00;
        packet[4] = 0x01;
        packet[5] = 0x01;

        /* set mac source*/
        packet[6] = 0x14;
        packet[7] = 0xac;
        packet[8] = 0x60;
        packet[9] = 0x85;
        packet[10] = 0xc6;
        packet[11] = 0xf1;
        //TYPE
        packet[12] = 0x08;
        packet[13] = 0x00;
        //header_length
        packet[14] = 0x45;
        packet[15] = 0x00;
        //total_length
        packet[16] = 0x00;
        packet[17] = 0x3b;
        //ip_id
        packet[18] = 0xad;
        packet[19] = 0xf2;
        //fragment_offset
        packet[20] = 0x00;
        packet[21] = 0x00;
        //ttl
        packet[22] = 0x80;
       //ip.protol
        
        packet[23] = 0x11;
        //ip_check_sum
        packet[24] = 0x00;
        packet[25] = 0x00;
        //ip.src
        packet[26] = 0x0a;
        packet[27] = 0x14;
        packet[28] = 0xd6;
        packet[29] = 0x4c;

        //ip.dst
        packet[30] = 0x0a;
        packet[31] = 0x0f;
        packet[32] = 0x2c;
        packet[33] = 0x0b;
        //src port
        packet[34] = 0xc0;
        packet[35] = 0xad;
        //dst port 
        packet[36] = 0x00;
        packet[37] = 0x35;
        //udp_length
        packet[38] = 0x00;
        packet[39] = 0x00;
        //udp check sum
        packet[40] = 0x00;
        packet[41] = 0x00;
        //transcation_id
        packet[42] = 0xf2;
        packet[43] = 0xc7;
        //you can set the transcation_id as a random_variable
        packet[42] = get_Rand(10, 255);
        packet[43] = get_Rand(10, 255);
        //
        packet[44] = 0x01;
        packet[45] = 0x00;
        packet[46] = 0x00;
        packet[47] = 0x01;
        packet[48] = 0x00;
        packet[49] = 0x00;
        packet[50] = 0x00;
        packet[51] = 0x00;
        packet[52] = 0x00;
        packet[53] = 0x00;
        //dns.name
        packet[54] = 0x03;
        packet[55] = 0x77;
        packet[56] = 0x77;
        packet[57] = 0x77;
        packet[58] = 0x05;
        packet[59] = 0x62;
        packet[60] = 0x61;
        packet[61] = 0x69;
        packet[62] = 0x64;
        packet[63] = 0x75;
        packet[64] = 0x03;
        packet[65] = 0x63;
        packet[66] = 0x6f;
        packet[67] = 0x6d;
        packet[68] = 0x00;
        //dns type and class
        packet[69] = 0x00;
        packet[70] = 0x01;
        packet[71] = 0x00;
        packet[72] = 0x01;
        //



        calculate_total_length(packet);
        calculate_udp_length(packet);

        calculate_check_sum_ip(packet, TOTAL_PACKET_LEN);
        calculate_check_sum_DNS(packet, TOTAL_PACKET_LEN);
        //packet[24] = 0x00;
        //packet[25] = 0x00;
        //packet[40] = 0x15;
        //packet[41] = 0xa4;
        for (int i = 0; i < TOTAL_PACKET_LEN; i++) {
            std::cout << " :" << (unsigned int)packet[i] <<" ";
        }
        send_packet(1);
        //packet[66] = 0x1c;
        //packet[42] = 0x8a;
        //packet[43] = 0x2d;
        //calculate_total_length(packet);
        //calculate_check_sum_DNS(packet, TOTAL_PACKET_LEN);
        //calculate_check_sum_ip(packet, TOTAL_PACKET_LEN);
        //send_packet(1);

    }
    void send_the_dns_request(std::string s) {
        //you need to reconsider the total_length
        std::vector< char> char_buffer;

      from_string_to_dns_format(s,char_buffer);
        //mac dst
        packet[0] = 0x00;
        packet[1] = 0x00;
        packet[2] = 0x5e;
        packet[3] = 0x00;
        packet[4] = 0x01;
        packet[5] = 0x01;

        /* set mac source*/
        packet[6] = 0x14;
        packet[7] = 0xac;
        packet[8] = 0x60;
        packet[9] = 0x85;
        packet[10] = 0xc6;
        packet[11] = 0xf1;
        //TYPE
        packet[12] = 0x08;
        packet[13] = 0x00;
        //header_length
        packet[14] = 0x45;
        packet[15] = 0x00;
        //total_length
        packet[16] = 0x00;
        packet[17] = 0x3b;
        //ip_id
        packet[18] = 0xad;
        packet[19] = 0xf2;
        //fragment_offset
        packet[20] = 0x00;
        packet[21] = 0x00;
        //ttl
        packet[22] = 0x80;
        //ip.protol

        packet[23] = 0x11;
        //ip_check_sum
        packet[24] = 0x00;
        packet[25] = 0x00;
        //ip.src
        packet[26] = 0x0a;
        packet[27] = 0x14;
        packet[28] = 0xd6;
        packet[29] = 0x4c;

        //ip.dst
        packet[30] = 0x0a;
        packet[31] = 0x0f;
        packet[32] = 0x2c;
        packet[33] = 0x0b;
        //src port
        packet[34] = 0xc0;
        packet[35] = 0xad;
        //dst port 
        packet[36] = 0x00;
        packet[37] = 0x35;
        //udp_length
        packet[38] = 0x00;
        packet[39] = 0x00;
        //udp check sum
        packet[40] = 0x00;
        packet[41] = 0x00;
        //transcation_id
        packet[42] = 0xf2;
        packet[43] = 0xc7;
        //you can set the transcation_id as a random_variable
        packet[42] = get_Rand(10, 255);
        packet[43] = get_Rand(10, 255);
        //
        packet[44] = 0x01;
        packet[45] = 0x00;
        packet[46] = 0x00;
        packet[47] = 0x01;
        packet[48] = 0x00;
        packet[49] = 0x00;
        packet[50] = 0x00;
        packet[51] = 0x00;
        packet[52] = 0x00;
        packet[53] = 0x00;
        //dns.name
        packet[54] = 0x03;
        packet[55] = 0x77;
        packet[56] = 0x77;
        packet[57] = 0x77;
        packet[58] = 0x05;
        packet[59] = 0x62;
        packet[60] = 0x61;
        packet[61] = 0x69;
        packet[62] = 0x64;
        packet[63] = 0x75;
        packet[64] = 0x03;
        packet[65] = 0x63;
        packet[66] = 0x6f;
        packet[67] = 0x6d;
        packet[68] = 0x00;
        //dns type and class
        packet[69] = 0x00;
        packet[70] = 0x01;
        packet[71] = 0x00;
        packet[72] = 0x01;
        //
        int dns_end_index = 54;
        for (int index = 0; index < char_buffer.size(); index++) {
            packet[dns_end_index] = char_buffer[index];
            dns_end_index++;
        }
        packet[dns_end_index++] = 0x00;
        packet[dns_end_index++] = 0x00;
        packet[dns_end_index++] = 0x01;
        packet[dns_end_index++] = 0x00;
        packet[dns_end_index++] = 0x01;

        unsigned int total_length_without_mac = 44+char_buffer.size()+1;
        packet[16] = (total_length_without_mac >> 8) & (0xff);
        packet[17] = (total_length_without_mac) & (0xff);
        calculate_total_length(packet);
        calculate_udp_length(packet);

        calculate_check_sum_ip(packet, TOTAL_PACKET_LEN);
        calculate_check_sum_DNS(packet, TOTAL_PACKET_LEN);
        //packet[24] = 0x00;
        //packet[25] = 0x00;
        //packet[40] = 0x15;
        //packet[41] = 0xa4;
        for (int i = 0; i < TOTAL_PACKET_LEN; i++) {
            std::cout << " :" << (unsigned int)packet[i] << " ";
        }
        send_packet(1);
        //packet[66] = 0x1c;
        //packet[42] = 0x8a;
        //packet[43] = 0x2d;
        //calculate_total_length(packet);
        //calculate_check_sum_DNS(packet, TOTAL_PACKET_LEN);
        //calculate_check_sum_ip(packet, TOTAL_PACKET_LEN);
        //send_packet(1);

    }
    void send_packet(int num) {
        ////printf("%d", TOTAL_PACKET_LEN)
        //std::thread t1(pcap_sendpacket, fp, tmp_packet, TOTAL_PACKET_LEN);
        //Sleep(100);


        //t1.join();
        for (int i = 0; i < num; i++) {
            
            if (pcap_sendpacket(fp,              // Adapter
                packet,          // buffer with the packet
                TOTAL_PACKET_LEN // size
            ) != 0) {
                fprintf(stderr, "\nError sending the packet: %s\n", pcap_geterr(fp));
            }
                // return 3;
            }
            // printf("send_success\n");
        
    }
    int detect_packet(u_char* detected_data = nullptr) {

        pcap_pkthdr* pkt_header;
        const u_char* pkt_data;
        for (int j = 0; j < 5; j++) {
            // printf(" --------\n");
            if (1 == pcap_next_ex(handler, &pkt_header, &pkt_data)) {
                int ans = PrintIPHeader(pkt_data);

                if (ans != -1) {
                    for (int i = 0; i < TOTAL_PACKET_LEN; i++) {
                        detected_data[i] = pkt_data[i];
                    }
                    return ans; // try to send_packet;
                }
            }
        }
        return -1;
    }
    void Inverse_the_detected_packet_data()
        //change the reply to request or request to reply
    {
        for (int i = 0; i < 6; i++) {
            u_char byte_prev = detected_data[i];
            u_char byte_after = detected_data[i + 6];
            detected_data[i] = byte_after;
            detected_data[i + 6] = byte_prev;
        }
        for (int i = 26; i < 26 + 4; i++) {
            u_char byte_prev = detected_data[i];
            u_char byte_after = detected_data[i + 4];
            detected_data[i] = byte_after;
            detected_data[i + 4] = byte_prev;
        }
        uint8_t icmp_type = detected_data[34];
        if (icmp_type == 0) {
            //icmp type =0 you need send a pacekt type =8
            detected_data[34] = 8;
        }
        if (icmp_type == 8) {
            //icmp type =8 you need send a packet type =0
            detected_data[34] = 0;
        }
        calculate_check_sum_ICMP(detected_data, TOTAL_PACKET_LEN);
        calculate_check_sum_ip(detected_data, TOTAL_PACKET_LEN);
    }
    ~Packet_handler() { pcap_close(fp); }
    // ans value=0 receive a icmp type=0,value =8 receive a icmp type=8,return
    // value =4,just do the forward,-1 none of my businnes,5 is just forwarding,6 is ping wan
    void run(int& a, ip_header* ip = nullptr, icmp_header* icmp = nullptr) {

        int ans = detect_packet(detected_data);
        get_the_information_of_the_packet(detected_data, ip, icmp);

        //if (ans != -1) {
        //    if (ans == 4) {
        //        //ans=4 just do the forwarding

        //    
        //    }
        //    else {
        //        Inverse_the_detected_packet_data();
        //    }
        //    set_packet(detected_data, TOTAL_PACKET_LEN);
        //    send_packet(1);
        //}
        a = ans;


    }
    void set_the_detected_into_send_packet() {
        for (int i = 0; i < TOTAL_PACKET_LEN; i++) {
            packet[i] = detected_data[i];
        }
    }
    void set_packet(const std::vector<u_char> &data) {
        for (int i = 0; i < data.size(); i++) {
            packet[i] = data[i];

        }
        TOTAL_PACKET_LEN = data.size();
    }
    void set_packet(u_char* p, int num) {
        for (int i = 0; i < num; i++) {
            packet[i] = p[i];
        }
    }
    u_char detected_data[100000];
    pcap_if_t* device;
    pcap_t* handler;
    pcap_t* fp;

    pcap_if_t* alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];
    bool send = false;
};
// return value=0 receive a icmp type=0,value =8 receive a icmp type=8,return
// value =4,just do the forward,else none of my businnes,5 is just forwarding
int PrintIPHeader(const u_char* packetData) {
    // it is a router function to detect the data

    // create a router table
    struct ip_header* ip_protocol;

    // +14 ����������·��
    ip_protocol = (struct ip_header*)(packetData + 14);
    SOCKADDR_IN Src_Addr, Dst_Addr = { 0 };

    u_short check_sum = ntohs(ip_protocol->check_sum);
    int ttl = ip_protocol->time_to_live;
    int proto = ip_protocol->Protocol;

    Src_Addr.sin_addr.s_addr = ip_protocol->SrcAddr;
    Dst_Addr.sin_addr.s_addr = ip_protocol->DstAddr;

    if (ip_protocol->Protocol != 1) {
        return -1; // I only care about ICMP
    }

    if (ip_protocol->DstAddr != *node2_ip)
    {
        
        if (ip_protocol->SrcAddr == *node2_ip) {
            return -1; // it is none of my business,包是我发的
        }
        else {
            // check the router
            if (ip_protocol->DstAddr == *node1_ip ||
                ip_protocol->DstAddr == *node3_ip ||
                ip_protocol->DstAddr == *node4_ip) {

                calculate_total_length(packetData);
                if (ip_protocol->DstAddr == *node1_ip) {
                    return 5;// 5 is just forwarding
                }
             
                // just do the forward and not change the data;
                return 4;
            }
            else {
                // not in the router table ,none of my business
                return -1;
            }
        }
    }
    printf("源地址: %15s --> ", inet_ntoa(Src_Addr.sin_addr));
    printf("目标地址: %15s --> ", inet_ntoa(Dst_Addr.sin_addr));
    printf("\n");
    int ans = -1;
    switch (ip_protocol->Protocol) {
    case 1:
        ans = PrintICMPHeader(packetData);
        return ans;
        break;
        // case 2: printf("IGMP \n"); break;
        // case 6: printf("TCP \n");  break;
        // case 17: printf("UDP \n"); break;
        // case 89: printf("OSPF \n"); break;
    default:
        return -1;
        break;
    }
    return -1;
}

