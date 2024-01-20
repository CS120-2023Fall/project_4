#pragma once
#include<deque>
#include<list>
#include<assert.h>
#include<JuceHeader.h>
#include<chrono>
#include<cstdlib>
#include "receiver_transfer.h"
#include "macros.h"
#include "ip.h"
#include <vector>
#include <thread>
// millisecond
#define ACK_TIME_OUT_THRESHOLD 500
#define RESEND_THRESHOLD 100
double RTT = 70;

Packet_handler handler;
ip_header* ip;
icmp_header* icmp;
int ans;
class MAC_Layer {
public:
    MAC_Layer() { handler.Initialize(); }
    MAC_Layer(juce::Label* labels[], int num_labels) {
        handler.Initialize();
        if (num_labels > 5) {
            assert(0);
        }
        for (int i = 0; i < num_labels; ++i) {
            mes[i] = labels[i];
        }
    }

    ~MAC_Layer() {
    }
    // update MAC states
    void refresh_MAC(const float* inBuffer, float* outBuffer, int num_samples);
    // prepare for next packet
    void Start() {
        macState = MAC_States_Set::ICMP_VIRTUAL ;
        receiver.Initialize();
        transmitter.Initialize();
        resend = 0;
        ackTimeOut_valid = false;
        //TxPending = true;
        wait = false;
        backoff_exp = 0;
        startTransmitting = START_TRANS_FIRST;
        RTT_log.clear();
        send_audio_data_ICMP = false;
        sequence_num = 15;
        is_host = true;
        start_ping = false;
        ping_ip = 0;
        trans_id = 0;
        ans = -1;
    }

    //void reset_receiving_info();
    void STOP() {
        receiver.Write_symbols();

    }
    void Get_Input() {
        std::string s;
        std::cin >> s;
        if (!s.empty()) {
            printf("loading\n");
            for (int i = 0; i < s.size(); i++) {
                char_buffer.push_back(s[i]);

            }

        }
    }
public:
    enum class MAC_States_Set {
        Idle,
        RxFrame,
        TxFrame,
        TxACK,
        ACKTimeout,
        LinkError,
        debug_error,
        ICMP_sniff,
        ICMP_send,
        ICMP_ping_wan,
        ICMP_VIRTUAL,
    };

    MAC_States_Set macState{ MAC_States_Set::Idle };
    bool TxPending{ false };
    std::deque<int> received_data;
    bool wait = false; // wait for ack
    int start_for_wait_sample = 0;
    bool startTransmitting{ START_TRANS_FIRST }; // for csma
    // for ICMP
    bool send_audio_data_ICMP{ false };
    // Router variables
    std::chrono::time_point<std::chrono::steady_clock> ping_audio;


private:
    int mac_address{ MY_MAC_ADDRESS };
    // array of pointers to send message
    juce::Label* mes[5]{ nullptr };
    // the number of resending times
    int resend{ 0 };
    // ack time out detect
    // std::chrono::steady_clock::now()
    std::chrono::time_point<std::chrono::steady_clock> beforeTime_ack;
    bool ackTimeOut_valid{ false };
    int backoff_exp{ 0 }; // exponent of the backoff. 2^m - 1, millisecond
    std::chrono::time_point < std::chrono::steady_clock> beforeTime_backoff{ std::chrono::steady_clock::now() };
    std::chrono::time_point<std::chrono::steady_clock> send_Echo_time;
    std::list<double> RTT_log;
    std::chrono::time_point<std::chrono::steady_clock> beforeTime_carrier_sense{ std::chrono::steady_clock::now() };

public:
    Receiver receiver;
    Transfer transmitter;
    double accumulate_buffer = 0;
    unsigned int sequence_num;
    std::vector<unsigned char> char_buffer;
    bool start_ping = false;
    bool is_host = true;
    unsigned int query_ID = 69;
    uint32_t ping_ip;
    uint16_t trans_id;
    bool start_dns = false;
    u_char ip_4[4];
    u_char transcation_id[2];
};
void KeepSilence(const float* inBuffer, float* outBuffer, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        outBuffer[i] = 0;
    }
    //pcap_handler *p;
}
void Get_Input_To_Buffer(std::vector<unsigned char>& char_buffer) {
    std::string s;
    std::cin >> s;
    if (!s.empty()) {
        printf("loading\n");
        for (int i = 0; i < s.size(); i++) {
            char_buffer.push_back(s[i]);

        }

    }
}
void MAC_Layer::refresh_MAC(const float* inBuffer, float* outBuffer, int num_samples)
{
<<<<<<< HEAD
    if (macState == MAC_States_Set::ICMP_VIRTUAL) {
        handler.detect_for_virtual();
  }
=======
    KeepSilence(inBuffer, outBuffer, num_samples);
    //if (is_host && !start_dns) { Get_Input(); }
    //if (char_buffer.size() > 4) {
    //    std::string s;
    //    for (int i = 0; i < 4;i++) {
    //        s.push_back(char_buffer[i]);
    //    }
    //    if (s == "ping") {
    //        start_dns = true;
    //    }
    //    else {
    //        char_buffer.clear();
    //    }
    //    for (auto& i : char_buffer) {
    //        std::cout << i;
    //    }
    //    
    //}
  
>>>>>>> 92a084e082fc655983fed9dcf4783d9e8743dd54
    if (macState == MAC_States_Set::ICMP_ping_wan)
    {

            ans = handler.detect_for_icmp();
            if (ans == 2) {
               std::cout << "i find a  lan packet " << std::endl;

                handler.set_the_detected_into_send_packet();
                handler.set_the_wan();
                u_char dst[] = { handler.detected_data[42],handler.detected_data[43],handler.detected_data[44],handler.detected_data[45] };
                packet[22] = 0x20;
                packet[30] = dst[0];
                packet[31] = dst[1];
                packet[32] = dst[2];
                packet[33] = dst[3];
                handler.recalculate_check_sum_length_icmp();

                handler.send_wifi_packet();
            }
            else if (ans == 1) {
                auto* data = handler.detected_data;
                std::cout << "i find a WiFi packet " << std::endl;
                ans = -1;
                //data from node 4 if to node 1 then turn to asio and wait for ack,if to node 3 then 
                unsigned int local_address = *(unsigned int*)(handler.detected_data + 42);
               std::cout << local_address << "local" << *node1_ip << "node1" << std::endl;
                if (local_address == *node1_ip) {
                    //asio set the transmit_bit TODO 
                  std::cout << "transmit a packet" << std::endl;
                    macState = MAC_States_Set::TxFrame;
                    transmitter.add_flag = true;
                    return;
                }

                else if (local_address == *node3_ip) {
                  //  std::cout << "send_lan" << std::endl;
                    u_char dst[] = { handler.detected_data[42],handler.detected_data[43],handler.detected_data[44],handler.detected_data[45] };
                    u_char source_lan[] = { handler.detected_data[26],handler.detected_data[27],handler.detected_data[28],handler.detected_data[29] };
                    handler.detected_data[42] = source_lan[0];
                    handler.detected_data[43] = source_lan[1];
                    handler.detected_data[44] = source_lan[2];


                    handler.detected_data[45] = source_lan[3];//source as data
                    //set the 
                    handler.set_the_detected_into_send_packet();
                    handler.set_the_lan();
                    handler.recalculate_check_sum_length_icmp();
                    handler.send_lan_packet();
                    std::cout << "send_lan" << std::endl;
                    ans = -1;
                    //wait for response

                }
            }
    }
    else if (macState == MAC_States_Set::ICMP_sniff) {

        ans = -1;
        ans = handler.detect_response();
        while (ans == -1) {
            ans = handler.detect_response();
            if (ans == 1) {
                ENTERING;
                uint32_t response_ip = *(uint32_t*)(&(handler.detected_data[26]));
                for (int i = 0; i < 4; i++) {
                    std::cout << (uint8_t)((response_ip >> (8 * (4 - i))) && (0xff)) << " ";
                }
                std::cout << "check" << std::endl;
                if (response_ip == ping_ip) {
                    macState = MAC_States_Set::TxACK;
                    //receiver.received_packet += 1;
                    bool feedback = transmitter.Add_one_packet(inBuffer, outBuffer, num_samples,
                        Tx_frame_status::Tx_ack, std::deque<unsigned>(), receiver.received_packet + 1);
                    response_ip = 0;
                    trans_id = 0;

                }
            }
            else if (ans == 2) {
                uint16_t response_id = *(uint16_t*)(&(handler.detected_data[42]));
                if (response_id == trans_id) {
                    //response the ip
                    u_char* data = handler.detected_data;
                    int index = 54;
                    while (data[index] != 0x00) {
                        uint8_t count = data[index];
                        index += count + 1;
                    }
                    ping_ip = *(uint32_t*)(handler.detected_data + index + 17);
                    for (int i = 0; i < 4; i++) {
                        std::cout << ((ping_ip >> (8 * (4 - i))) && (0xff)) << " ";
                    }
                }
                handler.send_the_ping_to_wan_with_ip_and_sequence_num(ping_ip, get_Rand(0, 100));
            }
        }
    }
    else if (macState == MAC_States_Set::Idle) {
        //std::cout << "idle" << std::endl;
        do {
            /// Detect preamble, invoke detect_frame()
     
            bool tmp = receiver.detect_frame(inBuffer, outBuffer, num_samples);
            if (tmp) {
                mes[3]->setText("preamble detected " + std::to_string(receiver.received_packet) + ", " + std::to_string(transmitter.transmitted_packet),
                    juce::NotificationType::dontSendNotification);
                macState = MAC_States_Set::RxFrame;
                std::cout << "detect_frame" << std::endl;
                // The computer has received a packet. It can start to transmit.
                startTransmitting = true;

                // It must return due to the implementation of detect_frame().
                return;
            }

            ///  Ack time
            //auto currentTime = std::chrono::steady_clock::now();
            //if (ackTimeOut_valid) {
            //    // millisecond
            //    double duration_millisecond = std::chrono::duration<double, std::milli>(currentTime - beforeTime_ack).count();
            //    if (duration_millisecond > ACK_TIME_OUT_THRESHOLD) {
            //        macState = MAC_States_Set::ACKTimeout;//resend the package
            //        ackTimeOut_valid = false;
            //        break;
            //    }
            //}

            ///  Send data
            //if (TxPending) {
            //    double duration_millisecond = DBL_MAX;
            //    //double duration_millisecond = std::chrono::duration<double, std::milli>(currentTime - beforeTime_backoff).count();
            //    // +, - are prior to <<
            //    double backoff = (1 << backoff_exp) - 1;
            //    if (!CSMA_ONLY_RECEIVE && (backoff == 0 || duration_millisecond >= backoff)) {
            //        backoff_exp = 0;
            //        macState = MAC_States_Set::TxFrame;
            //        beforeTime_carrier_sense = std::chrono::steady_clock::now();
            //    }
            //    break;
            //}
        } while (0); // end of do while 0
    }
    /// RxFrame
    if (macState == MAC_States_Set::RxFrame) {
        Rx_Frame_Received_Type tmp;
        tmp = Rx_Frame_Received_Type::valid_ack;
        std::cout << "received packet type: " << (int)tmp << std::endl;
        switch (tmp) {
        case Rx_Frame_Received_Type::error:
            macState = MAC_States_Set::Idle;
            return;
        case Rx_Frame_Received_Type::still_receiving:
            return;
        case Rx_Frame_Received_Type::valid_ack:
        {
            ackTimeOut_valid = false;
            //transmitter.transmitted_packet += 1;//the next staus transmit the next packet
            // macState = MAC_States_Set::Idle;
             mes[2]->setText("Received ack, transmitted packet: " + std::to_string(transmitter.transmitted_packet),
                 juce::NotificationType::dontSendNotification);
             wait = false;
             //backoff_exp = rand() % 5 + 4;
            handler.Inverse_the_detected_packet_data();
            handler.set_the_detected_into_send_packet();
            handler.send_wifi_packet();

            // finish ping (Task 1)
            //std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
            //double RTT = std::chrono::duration<double, std::milli>(current - ping_audio).count();
            //RTT_log.emplace_back(RTT);
            // TODO: tell router to reply to node 3
            //std::cout << "audio RTT: " << RTT << std::endl;

            macState = MAC_States_Set::ICMP_ping_wan;
            return;
        }
        case Rx_Frame_Received_Type::valid_data:
            macState = MAC_States_Set::ICMP_sniff;
            //receiver.received_packet += 1;
            
            //if receive the data then start to ping

            backoff_exp = rand() % 5 + 3;
            beforeTime_backoff = std::chrono::steady_clock::now();
            mes[1]->setText("Packet received: " + std::to_string(receiver.received_packet), juce::dontSendNotification);
            /////////////////////// delete me 

            start_dns = true;
            std::cout << "send" << std::endl;
        
            //while (1) {
            //    std::chrono::time_point<std::chrono::steady_clock> t1 = std::chrono::steady_clock::now();
            //    double duration = std::chrono::duration<double, std::milli>(t1 - tmp).count();
            //    if (duration > 50) {
            //        break;
            //    }
            //}

            //if (receiver.received_packet * NUM_PACKET_DATA_BITS >= 50000 &&
            //    transmitter.transmitted_packet * NUM_PACKET_DATA_BITS >= 50000) {
            //    macState = MAC_States_Set::LinkError;
            //    std::cout << "to click stop" << std::endl;
            //}
            //////////////////////////////////////////////////////////
            return;
        }
    }
    /// TxACK
    if (macState == MAC_States_Set::TxACK) {
        std::cout << "sending ack" << std::endl;
        bool finish = transmitter.Trans(inBuffer, outBuffer, num_samples);
        if (finish) {
            backoff_exp = rand() % 5 + 4;
            macState = MAC_States_Set::Idle;
        }
        // The computer has received a packet. It can start to transmit.
        startTransmitting = true;
        return;
    }
    /// TxFrame
    if (macState == MAC_States_Set::TxFrame) {
        if (transmitter.add_flag) {
            transmitter.Add_one_packet(inBuffer, outBuffer, num_samples, Tx_frame_status::Tx_data, default_trans_wire.random_message_bits, receiver.received_packet + 1);
            transmitter.add_flag = false;
        }
        bool finish = transmitter.Trans(inBuffer, outBuffer, num_samples);

        // transmit finishes
        if (finish) {
            std::cout << "finish" << std::endl;
            beforeTime_ack = std::chrono::steady_clock::now();
            ackTimeOut_valid = true;
            macState = MAC_States_Set::RxFrame;
            wait = true;

        }

        send_audio_data_ICMP = false;
        return;
    }
    /// ACKTimeout
    if (macState == MAC_States_Set::ACKTimeout) {
        if (resend > RESEND_THRESHOLD) {
            macState = MAC_States_Set::LinkError;
        }
        else {
            ++resend;
            // set backoff after ack timeout
            // [3, 8]
            backoff_exp = rand() % 5 + 3;
            beforeTime_backoff = std::chrono::steady_clock::now();
            macState = MAC_States_Set::Idle;
            wait = false;
            return;
        }
    }
    /// LinkError
    if (macState == MAC_States_Set::LinkError) {
        std::cout << "link error" << std::endl;
        return;
    }
}