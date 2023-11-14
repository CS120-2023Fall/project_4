#pragma once
#include<deque>
#include<assert.h>
#include<JuceHeader.h>
#include<chrono>

/////////////////////////////////
// set these macros properly!///
/////////////////////////////////
#define NUM_HEADER_BITS 8
#define NUM_DEST_BITS 3
#define NUM_SRC_BITS 3
#define NUM_TYPE_BITS 2
#define MY_MAC_ADDRESS 0b001
#define RECEND_THRESHOLD 5
#define TIME_OUT_THRESHOLD 1e-2

int transmit(float *outBuffer, const std::vector<int> &data_to_transmit, int data_index, int num_data);
int receive(const float *inBuffer, float *outBuffer, int num_samples, std::deque<int> &received_data,
    int behaviour);

enum class MAC_States_Set {
    Idle,
    CarrierSense,
    TxFrame_ACK,
    RxFrame,
    TxFrame,
    TxACK,
    ACKTimeout,
    LinkError
};

class MAC_Layer {
public:
    MAC_Layer() {};

    // update MAC states
    void refresh_MAC(const float *inBuffer, float *outBuffer, int num_samples);
    // prepare for next packet
    void reset_receiving_info();

public:
    MAC_States_Set macState{MAC_States_Set::Idle};
    bool TxPending{ false };
    std::deque<int> received_data;

private:
    enum class Frame_Type {
        unknown = 0 ,
        ack = 0b01,
        data = 0b10
    };
    // after detect preamble, note receiving information
    struct {
        bool header_removed{ false };
        bool ready_to_rm_header{ false };
        int dest{ -1 };
        int src{ -1 };
        Frame_Type frameType{ Frame_Type::unknown };
        // in one packet
        int num_received_data{ 0 };
    } receiving_info;
    int mac_address{ MY_MAC_ADDRESS };
    // array of pointers
    juce::Label *mes[5]{ nullptr };
    // recend the number of sending times
    int recend{ 0 };
    // ack time out detect
    std::chrono::time_point<std::chrono::steady_clock> beforeTime = std::chrono::steady_clock::now();
    bool timeOut_valid{ false };
};

void MAC_Layer::refresh_MAC(const float *inBuffer, float *outBuffer, int num_samples) {
    // deal with every state

    ///   This is unfinished.
    if (macState == MAC_States_Set::Idle) {
        // detect preamble, invoke receiver()
        if (receive(inBuffer, outBuffer, num_samples, received_data, 0) == 1) {
            macState = MAC_States_Set::RxFrame;
            return;
        }
        // ack time out
        if (timeOut_valid) {
            auto currentTime = std::chrono::steady_clock::now();
            double duration_millsecond = std::chrono::duration<double, std::milli>(currentTime - beforeTime).count();
            if (duration_millsecond > TIME_OUT_THRESHOLD) {
                macState = MAC_States_Set::ACKTimeout;
                return;
            }
        }
        // send data
        if (TxPending) {
            // TODO: invoke transmit()
        }
    }
    ///
    else if (macState == MAC_States_Set::RxFrame) {
        /////////////// TODO : fix this fuction call. /////////////////
        if (receiving_info.num_received_data > 9999)
            receive();
        else {

        }
        /////////////////////////////////
        // data is too few to process header
        if (received_data.size() < NUM_HEADER_BITS) {
            assert(0);
        }
        // destination
        for (int i = NUM_DEST_BITS - 1; i >= 0; --i) {
            receiving_info.dest += received_data[i] << i;
        }
        received_data.erase(received_data.begin(), received_data.begin() + NUM_DEST_BITS);
         //source
        for (int i = NUM_DEST_BITS - 1; i >= 0; --i) {
            receiving_info.src += received_data[i] << i;
        }
        received_data.erase(received_data.begin(), received_data.begin() + NUM_SRC_BITS);
        // type
        int type = 0;
        for (int i = NUM_TYPE_BITS - 1; i >= 0; --i) {
            type = received_data[i] << i;
        }
        // avoid conversion failure
        assert(type <= 2);
        received_data.erase(received_data.begin(), received_data.begin() + NUM_TYPE_BITS);
        receiving_info.frameType = Frame_Type(type);
        // process data
        if (receiving_info.frameType == Frame_Type::data) {
            
            // TODO: wirte to file buffer
        }
        else if (receiving_info.frameType == Frame_Type::ack) {
            //TODO : ckeck source and destination
            // define appropriate ack structure
        }
        // type error
        assert(receiving_info.frameType != Frame_Type::unknown);        
    } // end of RxFrame state
    /// send ack to other computers
    else if (macState == MAC_States_Set::TxFrame_ACK) {
        // TODO
    }
    ///
    else if (macState == MAC_States_Set::CarrierSense) {
        // TODO
    }
    /// send data to other computers
    else if (macState == MAC_States_Set::TxFrame) {
        // TODO
    }
    ///
    else if (macState == MAC_States_Set::ACKTimeout) {
        if (recend > RECEND_THRESHOLD) {
            macState = MAC_States_Set::LinkError;
            return;
        }
    }
    /// exit with error
    else if (macState == MAC_States_Set::LinkError) {
        assert(0);
    }

}