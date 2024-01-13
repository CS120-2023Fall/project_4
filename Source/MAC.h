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
        macState = MAC_States_Set::ICMP_sniff;
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
        start_dns = false;
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
public:
    Receiver receiver;
    Transfer transmitter;
    double accumulate_buffer = 0;
    unsigned int sequence_num;
    std::vector<char> char_buffer;
    bool start_dns = false;
    bool is_host = true;
};
void KeepSilence(const float* inBuffer, float* outBuffer, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        outBuffer[i] = 0;
    }
    //pcap_handler *p;
}
void Get_Input_To_Buffer(std::vector<char>& char_buffer) {
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
    //KeepSilence(inBuffer, outBuffer, num_samples);
    if (is_host && !start_dns) { Get_Input(); }
    if (char_buffer.size() > 4) {
        std::string s;
        for (int i = 0; i < 4;i++) {
            s.push_back(char_buffer[i]);
        }
        if (s == "ping") {
            start_dns = true;
        }
        else {
            char_buffer.clear();
        }
        for (auto& i : char_buffer) {
            std::cout << i;
        }
        
    }
    if (macState == MAC_States_Set::ICMP_ping_wan)
    {

    }
    if (macState == MAC_States_Set::ICMP_send) {
        ////handler.Inverse_the_detected_packet_data();
        handler.set_the_detected_into_send_packet();
        handler.send_packet(1);

        macState = MAC_States_Set::ICMP_sniff;

        return;
    }
    else if (macState == MAC_States_Set::ICMP_sniff) {

        ans = -1;
        int tmp = 0;
        //tmp = ++tmp % 10;
        handler.run(ans, ip, icmp);
        //if you want to inverse and send,
        //handler.Inverse_the_detected_packet_data();
        //handler.set_the_detected_into_send_packet();
        //handler.send_packet(1);
        //if just forwarding
        //handler.set_the_detected_into_send_packet();
        //handler.send_packet(1);
        // request
        std::cout << "sniffing" << std::endl;
        if (ans == -1) {
            std::cout << "no packet " << std::flush;
        }
        if (ans == 5) {
            send_audio_data_ICMP = true;
            macState = MAC_States_Set::TxFrame;
            bool feedback = transmitter.Add_one_packet(inBuffer, outBuffer, num_samples,
                Tx_frame_status::Tx_data);
            std::cout << "go" << std::endl;
            ping_audio = std::chrono::steady_clock::now();
            return;
        }
        if (ans == 8) {
            handler.Inverse_the_detected_packet_data();
            handler.set_the_detected_into_send_packet();
            handler.send_packet(1);
            std::cout << "reply" << std::endl;
            macState == MAC_States_Set::Idle;
            return;
        }

        return;
    }
    else if (macState == MAC_States_Set::Idle) {
        //if (RTT_log.size() >= 10) {
        //    Write("RTT_log.txt", RTT_log);
        //    macState = MAC_States_Set::LinkError;
        //    return;
        //}
        // 2. ack time out
        // ///////////////////////
        // pass ackTimeout state, exit directly
        ///////////////////////////////
        //if (ackTimeOut_valid) {
        //    auto currentTime = std::chrono::steady_clock::now();
        //    // milisecond
        //    double duration_millsecond = std::chrono::duration<double, std::milli>(currentTime - beforeTime_ack).count();
        //    if (duration_millsecond > ACK_TIME_OUT_THRESHOLD) {
        //        macState = MAC_States_Set::ACKTimeout;//resend the package
        //        ackTimeOut_valid = false;
        //        /////////////////////////////// watch out here!!!!!!!!!! ///////////////
        //        //macState = MAC_States_Set::LinkError;
        //        /////////////////////////
        //        return;
        //    }
        //}
        // 3. send data
        //auto currentTime = std::chrono::steady_clock::now();
        //double duration_milisecond = std::chrono::duration<double, std::milli>(currentTime - beforeTime_backoff).count();
        //// +, - first, then <<
        //double backoff = (1 << backoff_exp) - 1;
        //if (TxPending && (backoff == 0 || duration_milisecond > backoff)) 
        //if (send_audio_data_ICMP)
        //{
        //    backoff_exp = 0;
        //    macState = MAC_States_Set::CarrierSense;
        //    return;
        //}
        bool tmp = receiver.detect_frame(inBuffer, outBuffer, num_samples);
        std::cout << "idle" << std::endl;
        // 1. detect preamble, invoke detect_frame()

        if (tmp) {
            mes[3]->setText("preamble detecked " + std::to_string(receiver.received_packet) + ", " + std::to_string(transmitter.transmitted_packet),
                juce::NotificationType::dontSendNotification);
            macState = MAC_States_Set::RxFrame;
            std::cout << "detect_frame" << std::endl;
            return;
        }
    }
    /// RxFrame
    if (macState == MAC_States_Set::RxFrame) {
        Rx_Frame_Received_Type tmp = receiver.decode_one_packet(inBuffer, outBuffer, num_samples);

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
            handler.send_packet(1);

            // finish ping (Task 1)
            //std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
            //double RTT = std::chrono::duration<double, std::milli>(current - ping_audio).count();
            //RTT_log.emplace_back(RTT);
            // TODO: tell router to reply to node 3
            //std::cout << "audio RTT: " << RTT << std::endl;

            macState = MAC_States_Set::ICMP_sniff;
            return;
        }
        case Rx_Frame_Received_Type::valid_data:
            macState = MAC_States_Set::TxACK;
            //receiver.received_packet += 1;
            bool feedback = transmitter.Add_one_packet(inBuffer, outBuffer, num_samples,
                Tx_frame_status::Tx_ack, receiver.received_packet);
            backoff_exp = rand() % 5 + 3;
            beforeTime_backoff = std::chrono::steady_clock::now();
            mes[1]->setText("Packet received: " + std::to_string(receiver.received_packet), juce::dontSendNotification);
            /////////////////////// delete me 


            std::cout << "send" << std::endl;
            handler.send_the_ping_to_wan_with_ip_and_sequence_num(*(Router_table::node_LAN), sequence_num);
            sequence_num++;
            std::chrono::time_point<std::chrono::steady_clock> tmp = std::chrono::steady_clock::now();
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
        bool finish = transmitter.Trans(inBuffer, outBuffer, num_samples);

        // transmit finishes
        if (finish) {
            std::cout << "finish" << std::endl;
            beforeTime_ack = std::chrono::steady_clock::now();
            ackTimeOut_valid = true;
            macState = MAC_States_Set::Idle;
            wait = true;
        }
        // set start time stamp
        //send_Echo_time = std::chrono::steady_clock::now();
        //double t = std::chrono::duration<double, std::milli>(send_Echo_time - ping_audio).count();
        //std::cout << "trans finish: " << t <<  std::endl;
        // ICMP Echo packet sending is finished.
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