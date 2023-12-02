#pragma once
#include<deque>
#include<assert.h>
#include<JuceHeader.h>
#include<chrono>
#include<cstdlib>
#include "receiver_transfer.h"
//#include "transmitter.h"

/////////////////////////////////
// set these macros properly!///
/////////////////////////////////
#define NUM_HEADER_BITS 8
#define NUM_DEST_BITS 3
#define NUM_SRC_BITS 3
#define NUM_TYPE_BITS 2
#define MY_MAC_ADDRESS 0b001
#define RECEND_THRESHOLD 8
// milisecond
#define ACK_TIME_OUT_THRESHOLD 10000
//structure  PREAMBLE+CRC_SYMBOLS+PACKET_NUM_SYMBOLS+DEST+SRC+TYPE
//enum class Rx_Frame_Received_Type {
//    still_receiving = -1,
//    error = 0,
//    valid_ack = 0b01,
//    valid_data = 0b10
//};

class MAC_Layer {
public:
    MAC_Layer() {

    }
    MAC_Layer(juce::Label *labels[], int num_labels) {
        if (num_labels > 5) {
            assert(0);
        }
        for (int i = 0; i < num_labels; ++i) {
            mes[i] = labels[i];
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
        TxPending = true;
        wait = false;
        backoff_exp = 0;
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

public:
    MAC_States_Set macState{MAC_States_Set::Idle};
    bool TxPending{ false };
    std::deque<int> received_data;
    bool wait = false;
    int start_for_wait_sample=0;

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
    Receiver receiver;
    Transfer transmitter;
};
void KeepSilence(const float* inBuffer, float* outBuffer, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        outBuffer[i] = 0;
    }
}
void MAC_Layer::refresh_MAC(const float *inBuffer, float *outBuffer, int num_samples) {
    // for debug
    if (macState == MAC_States_Set::debug_error) {
        assert(0);
    }
       // deal with every state
    if (transmitter.transmitted_packet >= maximum_packet) {
        macState = MAC_States_Set::LinkError;
    }
    /// Idle
    if (macState == MAC_States_Set::Idle) {

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
                macState = MAC_States_Set::LinkError;
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
            macState = MAC_States_Set::RxFrame;
            return;
        }
    }
    /// RxFrame
    else if (macState == MAC_States_Set::RxFrame) {
        Rx_Frame_Received_Type tmp = receiver.decode_one_packet(inBuffer, outBuffer, num_samples);
        switch (tmp) {
            case Rx_Frame_Received_Type::error:
                macState = MAC_States_Set::Idle;
                return;
            case Rx_Frame_Received_Type::still_receiving:
                return;
            case Rx_Frame_Received_Type::valid_ack:
                ackTimeOut_valid = false;
                transmitter.transmitted_packet += 1;//the next staus transmit the next packet
                macState = MAC_States_Set::Idle;
                if (transmitter.transmitted_packet >= 2) {
                    int xxxx = 1;
                    xxxx++;
                }
                wait = false;
                return;
            case Rx_Frame_Received_Type::valid_data:
                macState = MAC_States_Set::TxACK;
                receiver.received_packet += 1;
                bool feedback = transmitter.Add_one_packet(inBuffer, outBuffer, num_samples, Tx_frame_status::Tx_ack);
                mes[1]->setText("ACK received: " + std::to_string(receiver.received_packet), juce::dontSendNotification);
                return;             
        }
    }
    /// TxACK
    else if (macState == MAC_States_Set::TxACK) {
        bool finish = transmitter.Trans(inBuffer, outBuffer, num_samples);
        if (finish) {
            macState = MAC_States_Set::Idle;
       }
        return;
    }
    /// CarrierSense
    else if (macState == MAC_States_Set::CarrierSense) {
        //for (int i = 0; i < num_samples; i++) {
        //    outBuffer[i] = 0;
        //}
        if (receiver.if_channel_quiet(inBuffer, num_samples)) {
            macState = MAC_States_Set::TxFrame;
            if (transmitter.transmitted_packet >= 1) {
                int xxxx = 1;
                xxxx++;
            }
            bool feedback = transmitter.Add_one_packet(inBuffer, outBuffer, num_samples, Tx_frame_status::Tx_data);
            //ackTimeOut_valid = true;
            //beforeTime_ack = std::chrono::steady_clock::now();
            //backoff_exp = 0;
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
        if (transmitter.transmitted_packet >= 1) {
            int xxxx = 1;
            xxxx++;
        }

        bool finish= transmitter.Trans(inBuffer, outBuffer, num_samples);

         // transmition finishes
        if (finish) {
            beforeTime_ack = std::chrono::steady_clock::now();
            //backoff_exp = 9;
            ackTimeOut_valid = true;
            macState = MAC_States_Set::Idle;
            wait = true;
        }
    }
    /// ACKTimeout
    else if (macState == MAC_States_Set::ACKTimeout) {
        if (resend > RECEND_THRESHOLD) {
            macState = MAC_States_Set::LinkError;
            return;
        }
        else {
            ++resend;
            // set backoff after ack timeout
            // [1, 8]
            backoff_exp = rand() % 8 + 1;
            beforeTime_backoff = std::chrono::steady_clock::now();
            macState = MAC_States_Set::Idle;
            wait = false;
            return;
        }
    }
    /// LinkError
    else if (macState == MAC_States_Set::LinkError) {
       
    }
}