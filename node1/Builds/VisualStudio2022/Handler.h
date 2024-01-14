#ifndef HANDLER_H
#define HANDLER_H
#include <stdio.h>
#include <pcap.h>
#define WIN32
#pragma warning(disable : 4995)
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"ws2_32.lib")

#define HAVE_REMOTE
#define ETHERTYPE_IP            0x0800          /* IP */  

typedef struct eth_header
{
	unsigned char DestMac[6];
	unsigned char SrcMac[6];
	unsigned short Etype;
}eth_header;
struct Packet_handler {
	Packet_handler() {
		Initialize();
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
			if (count == 6) {
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
		packet[22] = 0x80;//ttl
		packet[23] = 0x01;
		packet[24] = 0x00;//ip checksum
		packet[25] = 0x00;
		packet[26] = 0x0b;//src.ip
		packet[27] = 0x14;
		packet[28] = 0xa3;
		packet[29] = 0x73;
		packet[30] = 0x10;
		packet[31] = 0x20;
		packet[32] = 0x16;
		packet[33] = 0x07;//dest.ip
		packet[34] = 0x01;//icmp type
		packet[35] = 0x03;
		packet[36] = 0xbb;
		packet[37] = 0x6d;
		if ((fp = pcap_open_live(device->name,		// name of the device
			65536,			// portion of the packet to capture. It doesn't matter in this case 
			1,				// promiscuous mode (nonzero means promiscuous)
			1000,			// read timeout
			errbuf			// error buffer
		)) == NULL)
		{
			fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n");
		}
		if (NULL == (handler = pcap_open_live(device->name, 65535, 		// portion of the packet to capture. It doesn't matter in this case 
			1,				// promiscuous mode (nonzero means promiscuous)
			1000,			// read timeout
			errbuf))) {
			//设置接受的包大小为65535，即可以接受所有大小的包
			printf("err in pcap_open : %s", errbuf);
		}
		//open the pcap


	}

	void send_packet(int num) {
		for (int i = 0; i < num; i++) {
			if (pcap_sendpacket(fp,	// Adapter
				packet,				// buffer with the packet
				35					// size
			) != 0)
			{
				fprintf(stderr, "\nError sending the packet: %s\n", pcap_geterr(fp));
				//return 3;
			}
			printf("send_success\n");
			Sleep(10);
		}

	}
	void detect_packet()
	{


		pcap_pkthdr* pkt_header;
		const u_char* pkt_data;
		for (int j = 0; j < 5; j++) {
			printf("%d --------\n", j);
			if (1 == pcap_next_ex(handler, &pkt_header, &pkt_data)) {
				for (int k = 0; k < 66; k++) {//输出每个包的前66个byte数据
					if (k % 15 == 0 && k != 0)//输出美观
						printf("\n");
					printf("%02x ", *(pkt_data + k));
				}
			}


		}
	}
	~Packet_handler() {
		pcap_close(fp);
	}
	void set_packet(u_char* p, int num) {
		for (int i = 0; i < num; i++) {
			packet[i] = p[i];
		}
	}
	pcap_if_t* device;
	pcap_t* handler;
	pcap_t* fp;
	u_char packet[100];
	pcap_if_t* alldevs;
	char errbuf[PCAP_ERRBUF_SIZE];

};

#endif