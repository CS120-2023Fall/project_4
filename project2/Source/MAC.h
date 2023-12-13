#pragma once
#include<deque>
#include<list>
#include<assert.h>
#include<JuceHeader.h>
#include<chrono>
#include<cstdlib>
#include "receiver_transfer.h"
#include "macros.h"
//#include <pcap.h>

// milisecond
#define ACK_TIME_OUT_THRESHOLD 5000
#define RECEND_THRESHOLD 4

class MAC_Layer {
public:
    MAC_Layer() {}
    MAC_Layer(juce::Label *labels[], int num_labels) {
        if (num_labels > 5) {
            assert(0);
        }
        for (int i = 0; i < num_labels; ++i) {
            mes[i] = labels[i];
        }
        if (START_TRANS_FIRST) {
            startTransmitting = true;
        }
        else {
            startTransmitting = false;
        }
    };

    ~MAC_Layer() {
    }
    // update MAC states
    void refresh_MAC(const float *inBuffer, float *outBuffer, int num_samples);
    // prepare for next packet
    void Start() {
        macState = MAC_States_Set::Idle;
        receiver.Initialize();
        transmitter.Initialize();
        resend = 0;
        ackTimeOut_valid = false;
        //TxPending = true;
        wait = false;
        backoff_exp = 0;
        startTransmitting = START_TRANS_FIRST;
        RTT_log.clear();
    }
    
    //void reset_receiving_info();
    void STOP() {
        receiver.Write_symbols();
       
    }

public:
    enum class MAC_States_Set {
        Idle,
        CarrierSense,
        RxFrame,
        TxFrame,
        TxACK,
        ACKTimeout,
        LinkError,
        debug_error
    };

    MAC_States_Set macState{MAC_States_Set::Idle};
    bool TxPending{ false };
    std::deque<int> received_data;
    bool wait = false;
    int start_for_wait_sample=0;
    bool startTransmitting;

private:
    int mac_address{ MY_MAC_ADDRESS };
    // array of pointers to send message
    juce::Label *mes[5]{ nullptr };
    // the number of resending times
    int resend{ 0 };
    // ack time out detect
    // std::chrono::steady_clock::now() 
    std::chrono::time_point<std::chrono::steady_clock> beforeTime_ack;
    bool ackTimeOut_valid{ false };
    // exponent of the backoff. 2^m - 1, millisecond
    int backoff_exp{ 1 };
    std::chrono::time_point < std::chrono::steady_clock> beforeTime_backoff{ std::chrono::steady_clock::now() };
    std::chrono::time_point<std::chrono::steady_clock> send_Echo_time;
    std::list<double> RTT_log;
public:
    Receiver receiver;
    Transfer transmitter;
};
void KeepSilence(const float* inBuffer, float* outBuffer, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        outBuffer[i] = 0;
    }
    //pcap_handler *p;
}
void MAC_Layer::refresh_MAC(const float *inBuffer, float *outBuffer, int num_samples) {

    if (macState == MAC_States_Set::Idle) {
        if (RTT_log.size() >= 10) {
            Write("RTT_log.txt", RTT_log);
            macState = MAC_States_Set::LinkError;
            return;
        }
        // 2. ack time out
        // ///////////////////////
        // pass ackTimeout state, exit directly
        ///////////////////////////////
        if (ackTimeOut_valid) {
            auto currentTime = std::chrono::steady_clock::now();
            // milisecond
            double duration_millsecond = std::chrono::duration<double, std::milli>(currentTime - beforeTime_ack).count();
            if (duration_millsecond > ACK_TIME_OUT_THRESHOLD) {
                macState = MAC_States_Set::ACKTimeout;//resend the package
                ackTimeOut_valid = false;
                /////////////////////////////// watch out here!!!!!!!!!! ///////////////
                //macState = MAC_States_Set::LinkError;
                /////////////////////////
                return;
            }
        }
        // 3. send data
        auto currentTime = std::chrono::steady_clock::now();
        double duration_milisecond = std::chrono::duration<double, std::milli>(currentTime - beforeTime_backoff).count();
        // +, - first, then <<
        double backoff = (1 << backoff_exp) - 1;
        if (TxPending && (backoff == 0 || duration_milisecond > backoff)) {
            backoff_exp = 0;
            macState = MAC_States_Set::CarrierSense;
            return;
        }
        bool tmp = receiver.detect_frame(inBuffer, outBuffer, num_samples);
        // 1. detect preamble, invoke detect_frame()
        if (tmp) {
            mes[3]->setText("preamble detecked " + std::to_string(receiver.received_packet) + ", " + std::to_string(transmitter.transmitted_packet), 
                juce::NotificationType::dontSendNotification);
            macState = MAC_States_Set::RxFrame;
            std::cout << "detect_frame" << std::endl;
            std::cout <<"receiver_buffer:" <<receiver.receive_buffer.size() << std::endl;
            std::cout << "sync_buffer:" << receiver.sync_buffer.size() << std::endl;
            std::cout << "decode_buffer:" << receiver.decode_buffer.size() << std::endl;
            std::cout << "symbol_code:" << receiver.symbol_code.size() << std::endl;
            return;
        }
    }
    /// RxFrame
    else if (macState == MAC_States_Set::RxFrame) {
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
            transmitter.transmitted_packet += 1;//the next staus transmit the next packet
            macState = MAC_States_Set::Idle;
            mes[2]->setText("Received ack, transmitted packet: " + std::to_string(transmitter.transmitted_packet),
                juce::NotificationType::dontSendNotification);
            wait = false;
            backoff_exp = rand() % 5 + 4;

            // finish ping
            std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
            double RTT = std::chrono::duration<double, std::milli>(current - send_Echo_time).count();
            RTT_log.emplace_back(RTT);
            return;
        }
            case Rx_Frame_Received_Type::valid_data:
                std::cout << "receiver_buffer:" << receiver.receive_buffer.size() << std::endl;
                std::cout << "sync_buffer:" << receiver.sync_buffer.size() << std::endl;
                std::cout << "decode_buffer:" << receiver.decode_buffer.size() << std::endl;
                std::cout << "symbol_code:" << receiver.symbol_code.size() << std::endl;
                macState = MAC_States_Set::TxACK;
                receiver.received_packet += 1;
                bool feedback = transmitter.Add_one_packet(inBuffer, outBuffer, num_samples, 
                    Tx_frame_status::Tx_ack, receiver.received_packet);
                backoff_exp = rand() % 5 + 3;
                beforeTime_backoff = std::chrono::steady_clock::now();
                mes[1]->setText("Packet received: " + std::to_string(receiver.received_packet), juce::dontSendNotification);
                /////////////////////// delete me ��������������������������������
                if (receiver.received_packet * NUM_PACKET_DATA_BITS >= 50000 && 
                    transmitter.transmitted_packet * NUM_PACKET_DATA_BITS >= 50000 ) {
                    macState = MAC_States_Set::LinkError;
                    std::cout << "to click stop" << std::endl;
                }
                //////////////////////////////////////////////////////////
                return;
        }
    }
    /// TxACK
    else if (macState == MAC_States_Set::TxACK) {
        
        auto currentTime = std::chrono::steady_clock::now();
        double duration_milisecond = std::chrono::duration<double, std::milli>(currentTime - beforeTime_backoff).count();
        // +, - first, then <<�� so use ()
        double backoff = (1 << backoff_exp) - 1;
        if (duration_milisecond <= backoff) {
            return;
        }
        std::cout << "sending ack" << std::endl;
        //if (!receiver.if_channel_quiet(inBuffer, num_samples)) {
        //    return;
        //}
        bool finish = transmitter.Trans(inBuffer, outBuffer, num_samples);
        if (finish) {
            backoff_exp = rand() % 5 + 4;
            macState = MAC_States_Set::Idle;
        }
        // The computer has received a packet. It can start to transmit.
        startTransmitting = true;
        return;
    }
    /// CarrierSense
    else if (macState == MAC_States_Set::CarrierSense) {
        if (receiver.if_channel_quiet(inBuffer, num_samples)) {
            macState = MAC_States_Set::TxFrame;
            bool feedback = transmitter.Add_one_packet(inBuffer, outBuffer, num_samples, Tx_frame_status::Tx_data);
            return;
        }
        else {
            backoff_exp = rand() % 5 + 4;
            beforeTime_backoff = std::chrono::steady_clock::now();
            macState = MAC_States_Set::Idle;
            return;
        }
    }
    /// TxFrame
    else if (macState == MAC_States_Set::TxFrame) {

        bool finish= transmitter.Trans(inBuffer, outBuffer, num_samples);

         // transmition finishes
        if (finish) {
            beforeTime_ack = std::chrono::steady_clock::now();
            ackTimeOut_valid = true;
            macState = MAC_States_Set::Idle;
            wait = true;
        }
        // set start time stamp
        send_Echo_time = std::chrono::steady_clock::now();
    }
    /// ACKTimeout
    else if (macState == MAC_States_Set::ACKTimeout) {
        if (resend > RECEND_THRESHOLD) {
            macState = MAC_States_Set::LinkError;
            return;
        }
        // should not reach here
        else {
            ++resend;
            // set backoff after ack timeout
            // [3, 8]
            backoff_exp = rand() % 6 + 3;
            beforeTime_backoff = std::chrono::steady_clock::now();
            macState = MAC_States_Set::Idle;
            wait = false;
            return;
        }
    }
    /// LinkError
    else if (macState == MAC_States_Set::LinkError) {
        return;
    }
}