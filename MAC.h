#pragma once
#include<deque>
#include<assert.h>
#include<JuceHeader.h>

// set these macros properly!
#define NUM_HEADER_BITS 8
#define NUM_DEST_BITS 3
#define NUM_SRC_BITS 3
#define NUM_TYPE_BITS 2
#define MY_MAC_ADDRESS 0b001
#define RECEND_THRESHOLD 5

int transmit(float *outBuffer, const std::vector<int> &data_to_transmit, int data_index, int num_data);
int receive(const float *inBuffer, float *outBuffer, int num_samples, std::deque<int> &received_data,
    int behaviour);

enum class MAC_States_Set {
    Idle,
    CarrierSense,
    RxFrame,
    TxFrame,
    TxACK,
    ACKTimeout,
    LinkError
};
class Receiver {
public:
    std::vector<double> receive_buffer;
    std::vector<double> sync_buffer;
    std::vector<double> decode_buffer;
    std::vector<double> preamble = default_trans.preamble;
    std::vector<double> carrier_1 = default_trans_wire.carreir_waves_1;
        int start_index = -1;
    unsigned int received_packet = 0;
    double sync_max=0;
    double receive_power = 0;
    void Initialize() {
        receive_buffer.clear();
        sync_buffer.clear();
        for (int i = 0; i < PREAMBLE_SIZE; i++) {
            sync_buffer.push_back(0);
        }
        start_index = -1;

    }
    bool detect_frame(const float* inBuffer, float* outBuffer, int num_samples)


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
    // recend times
    int recend{ 0 };
    

};

void MAC_Layer::refresh_MAC(const float *inBuffer, float *outBuffer, int num_samples) {
    // deal with every state
    if (macState == MAC_States_Set::Idle) {
        // detect preamble
        if (receive(inBuffer, outBuffer, num_samples, received_data, 0) == 1) {
            macState = MAC_States_Set::RxFrame;
            return;
        }
    }
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
    // send data to other computers
    else if (macState == MAC_States_Set::TxFrame) {

    }
    else if (macState == MAC_States_Set::CarrierSense) {

    }
    else if (macState == MAC_States_Set::ACKTimeout) {
        if (recend > RECEND_THRESHOLD) {
            macState = MAC_States_Set::LinkError;
            return;
        }
    }
    else if (macState == MAC_States_Set::LinkError) {
        assert(0);
    }

}