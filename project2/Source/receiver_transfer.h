#pragma once
#include<deque>
#include "modulate_and_demoudulate.h"
#include "macros.h"


/// //////////////////////////
///  set the macros appropriately!!!
#define QUIET_THRESHOLD 4

constexpr const int maximum_packet = 50000 / PACKET_DATA_SIZE / BITS_PER_SYMBOL;
constexpr const int CRC_SYMBOLS = NUM_CRC_BITS / BITS_PER_SYMBOL;//number of symbols in crc
std::vector<double > empty=std::vector<double>(0);
std::deque<double> empty_deque=std::deque<double>(0,0.0);
constexpr const int samples_per_symbol = samples_per_bit;
constexpr const int MAC_PACKET_SAMPLES = PREAMBLE_SIZE + (OVERHEAD_SYMBOLS + PACKET_DATA_SIZE) * samples_per_symbol;//how many samples in a mac packet
constexpr const int PHY_PACKET_SAMPLES = PREAMBLE_SIZE + (CRC_SYMBOLS+ PACKET_DATA_SIZE ) * samples_per_symbol;//how many samples in a phy packet

enum class  Frame_Type {
    ack = 0b01,
    data = 0b10
};
enum  Rx_Frame_Received_Type {
    still_receiving = -1,
    error = 0,
    valid_ack = 1,
    valid_data = 2
};

class Receiver {
public:
    Receiver() :receive_buffer(empty) {
        Initialize();
    }

    void Initialize() {
        sync_buffer = empty_deque;
        for (int i = 0; i < PREAMBLE_SIZE; i++) {
            sync_buffer.push_back(0);
        }
        receive_buffer.clear();
        receive_power = 0;
        symbol_code.clear();
        bits.clear();
        received_packet = 0;
        decode_buffer.clear();
        start_index = -1;
        demoudulator = new Demoudulator();

    }
    void Write_symbols()
    {

        Write("project2_bits_receiver.txt", symbol_code);
    }

    // decode a 4 samples
    // return a bit, 0 or 1.
    /// sample_buffer: The function gets samples from this buffer
    /// start_index: start at which position to decode
    /// after_end_index: position after the end position
    int decode_a_bit(std::vector<double> &sample_buffer, int start_index) {
        // 0
        if (sample_buffer[start_index] + sample_buffer[start_index + 1] < 0 &&
            sample_buffer[start_index + 2] + sample_buffer[start_index + 3] > 0) {
            return 0;
        }
        // 1
        else {
            return 1;
        }
    }

        // inBuffer: input ,outBuffer: not used ,num_samples: sample from input
        // try to read the num_samples sample from inBuffer,if the accumulated buffer still cannot achieve a packet,return still_receiving,if it satisfies a packet samples
        // return whether it is valid_ack,valid_data,or error,if valid_data,push the translated data symbols into symbol_code
        //-1  CRC_ERROR, 
        // 0 still RX_frame--still decoding,
        // 1 VALID_ACK,
        // 2 VALID_DATA
    Rx_Frame_Received_Type decode_one_packet(const float *inBuffer, float *outBuffer, int num_samples) {
        double scale = 1;
        double zero_detect_threashold = 0.2;
        // detech head, go over 0 value interval between preamble and mac header.
        bool data_synced = false;
        if (!decode_buffer.empty()) {
            bool if_need_this = false;
            for (size_t i = 3; i < decode_buffer.size(); ++i) {
                if (abs(decode_buffer[i]) + abs(decode_buffer[i - 1]) + abs(decode_buffer[i - 2]) + abs(decode_buffer[i - 3]) < zero_detect_threashold) {
                    if_need_this = true;
                    continue;
                }
                else if (if_need_this &&
                    abs(decode_buffer[i]) + abs(decode_buffer[i - 1]) + abs(decode_buffer[i - 2]) + abs(decode_buffer[i - 3]) > zero_detect_threashold) {
                    decode_buffer.erase(decode_buffer.begin(), decode_buffer.begin() + i);
                    break;
                }
            }
        }
        for (int i = 0; i < num_samples; i++) {
            if (!data_synced && i >= 3) {
                if (abs(inBuffer[i - 3]) + abs(inBuffer[i - 2]) + abs(inBuffer[i - 1]) + abs(inBuffer[i]) > zero_detect_threashold) {
                    data_synced = true;
                }
                else {
                    continue;
                }
            }
            decode_buffer.push_back(scale * inBuffer[i]);

            if (decode_buffer.size() >= (NUM_MAC_HEADER_BITS+ NUM_PACKET_DATA_BITS)  * NUM_SAMPLES_PER_BIT) {
                std::vector<int> header_vec;
                // j is the bit index
                for (int j = 0; j < NUM_MAC_HEADER_BITS; ++j)  {
                    header_vec.emplace_back(decode_a_bit(decode_buffer, j * NUM_SAMPLES_PER_BIT));
                }                
                int dest = (header_vec[0] << 2) + (header_vec[1] << 1) + header_vec[2];
                int src = (header_vec[3] << 2) + (header_vec[4] << 1) + header_vec[5];
                int type = (header_vec[6] << 1) + header_vec[7];
                std::cout << "dest, src, type: " << dest << src << type << std::endl;
                int packet_num = 0;
                for (int j = NUM_MAC_HEADER_BITS - PACKET_NUM_BITS, offset = PACKET_NUM_BITS - 1; 
                    j < NUM_MAC_HEADER_BITS; ++j, --offset) {
                    packet_num += header_vec[j] << offset;
                }
                std::cout << "packet num: " << packet_num << std::endl;
                std::cout << "== data? " << (Frame_Type(type) == Frame_Type::data) << std::endl;
                // packet error
                if (dest != MY_MAC_ADDRESS || (Frame_Type(type) != Frame_Type::ack && Frame_Type(type) != Frame_Type::data)) {
                    std::cout << "error packet" << std::endl;
                    std::cout << decode_buffer.size() << std::endl;
                    Write("decode_log.txt", decode_buffer);
                    decode_buffer.clear();
                    return error;
                }
                // ack
                if (Frame_Type(type) == Frame_Type::ack) {
                    std::cout << "exit after receiving ack" << std::endl;
                    Write("decode_log.txt", decode_buffer);
                    decode_buffer.clear();
                    return valid_ack;
                }
                // data
                else if (Frame_Type(type) == Frame_Type::data) {
                    // start_position: bit index, remember x4
                    int start_position = NUM_MAC_HEADER_BITS;
                    for (int bit_index = start_position; bit_index < start_position + NUM_PACKET_DATA_BITS; ++bit_index) {
                        symbol_code.emplace_back(decode_a_bit(decode_buffer, bit_index *4));
                    }
                    std::cout << "exit after receiving data" << std::endl;
                    Write("decode_log.txt", decode_buffer);
                    decode_buffer.clear();
                    return valid_data;
                }

            }
        }
        return still_receiving;

    }
    
    /// outBuffer is not used,inBuffer is the input ,read num_samples sample from input
    /// detect the preamble if  preamble detected return true,else false
    /// after detecting it auto enter the decode_state and initialize the receiver(decode_buffer will have 200samples)    
    bool detect_frame(const float *inBuffer, float *outBuffer, int num_samples)
    {
        bool detected = false;
        double scale = 1;
        for (int i = 0; i < num_samples; i++) {
            double current_sample;

            current_sample = scale*inBuffer[i];

            receive_power = (receive_power * 63 + current_sample * current_sample) / 64;
            receive_buffer.push_back(current_sample);
            sync_buffer.push_back(current_sample);
            sync_buffer.pop_front();

            int size = sync_buffer.size();
            double sum = 0;
            for (int j = 0; j < size; j++) {
                sum += preamble[j] * sync_buffer[j];
            }
            sum /= 200;

            if (sum > receive_power * 2 && sum > sync_max && sum > 0.05) {
                sync_max = sum;
                start_index = receive_buffer.size() - 1;
            }
            else {
                if (receive_buffer.size() - start_index > 200 && start_index > 0) {
                    // Copy samples from receiver_buffer to decode_buffer
                    // //start to decode
                    decode_buffer = vector_from_start_to_end(receive_buffer, start_index + 1, receive_buffer.size());
                    for (int j = i+1; j < num_samples; j++) {
                        decode_buffer.push_back(inBuffer[j]);
                    }//receive for
                    receive_buffer.clear();
                    sync_buffer.clear();
                    for (int i = 0; i < PREAMBLE_SIZE; i++) {
                        sync_buffer.push_back(0);
                    }
                    start_index = -1;
                    sync_max = 0;
                    receive_power = 0;
                    detected = true;
                    return detected;
                }
            }
        }
        return detected;
    }

    // deteck if the channel dose not has either jamming noise or data.
    bool if_channel_quiet(const float *inBuffer, int num_samples) {
        // compare the sum of power with the threshold
        float absolutePowerSum = 0;

        for (int i = 0; i < num_samples; ++i) {
            absolutePowerSum += abs(inBuffer[i]);
        }
        if (absolutePowerSum < QUIET_THRESHOLD)
            return true;
        else
            return false;
    }
public:
    std::vector<double> receive_buffer = empty;
    std::deque<double> sync_buffer = empty_deque;
    std::vector<double> decode_buffer = empty;
    std::vector<double> preamble = default_trans.preamble;
    std::vector<double> carrier_waves_1 = default_trans_wire.carrier_waves_1;
    std::vector<double> carrier_waves_0 = default_trans_wire.carrier_waves_0;
    std::vector<int>symbol_code;
    std::vector<bool> bits;
    int start_index = -1;
    unsigned int received_packet = 0;
    double sync_max = 0;
    double receive_power = 0;
    Demoudulator* demoudulator;
};

enum Tx_frame_status {
    Tx_ack = 0,
    Tx_data = 1
};


class Transfer {
public:
    Transfer() {
        generate_ack_packet();
        generate_packet_sequence();
    }
    void Initialize() { CRC_symbols.clear();
        ack_data_symbols.clear();
        transfer_num = 0;
        transmittion_buffer.clear();
        transmitted_packet = 0;
    }
    std::vector<double > transmittion_buffer;
    std::vector<bool>bits = default_trans_wire.bits;
    std::vector<unsigned int> symbols = default_trans_wire.symbols;
    std::vector<double>preamble = default_trans_wire.preamble;
    std::vector<double> carrier_waves_0 = default_trans_wire.carrier_waves_0;
    std::vector<double> carrier_waves_1 = default_trans_wire.carrier_waves_1;
    std::vector<double > packet_sequences;
    std::vector<double> ack_packet;
    std::vector<bool> CRC_bits;
    std::vector<unsigned> CRC_symbols;
    std::vector<bool> ack_data_symbols = std::vector<bool>(PACKET_DATA_SIZE * BITS_PER_SYMBOL, 0);
    int transfer_num = 0;
    int transmitted_packet = 0;
    Modulater *modulater = new Modulater;
    void generate_packet_sequence() {
        int size = symbols.size();
        unsigned int ask_num = 1 << (BITS_PER_SYMBOL - 1);// if 4 bits then 8 amplitude
        int bits_size = bits.size();
        unsigned int data_bits_per_packet = BITS_PER_SYMBOL * PACKET_DATA_SIZE;
        for (int i = 0; i < bits_size; i += data_bits_per_packet) {
            std::vector<bool>bits_slice;
            uint8_t crc_8;
            for (int j = i; j < i + data_bits_per_packet; j++) {
                bits_slice.push_back(bits[j]);
            }
            int data_size;
            unsigned char *data = unsigned_int_to_unsigned_char_star(from_bits_vector_to_unsigned_int(bits_slice), data_size);
            crc_8 = CRC::CalculateBits(data, data_bits_per_packet, CRC::CRC_8());
            std::vector<bool> crc_8_vector = from_uint8_t_to_bits_vector(crc_8);
            for (int i = 0; i <= 7; i++) {
                CRC_bits.push_back(crc_8_vector[i]);
            }


        }
        CRC_symbols = translate_from_bits_vector_to_unsigned_int_vector(CRC_bits, BITS_PER_SYMBOL);
        for (int i = 0; i < size; i += PACKET_DATA_SIZE) {

            int start = i;
            int end = i + PACKET_DATA_SIZE;
            if (end >= size) {
                end = size;
            }
            for (int j = 0; j < PREAMBLE_SIZE; j++) {
                packet_sequences.push_back(preamble[j]);
            }
            // push the preamble
          // then the crc
            int packet_num = i / PACKET_DATA_SIZE;
            for (int j = 0; j < CRC_SYMBOL_SIZE; j++) {
                int symbol_index_start = packet_num * CRC_SYMBOL_SIZE;
                unsigned int symbol = CRC_symbols[symbol_index_start + j];
                auto carrier = (symbol & 1) == 1 ? carrier_waves_1 : carrier_waves_0;//using the last bit to determine psk
                float amplitude = max_amplitude * float((symbol >> 1) + 1) / ask_num;
                for (int k = 0; k < samples_per_bit; k++) {
                    packet_sequences.push_back(amplitude * carrier[k]);
                }
            }

            for (int symbol_index = start; symbol_index < end; symbol_index++) {
                unsigned int symbol = symbols[symbol_index];
                auto carrier = (symbol & 1) == 1 ? carrier_waves_1 : carrier_waves_0;//using the last bit to determine psk
                float amplitude = max_amplitude * float((symbol >> 1) + 1) / ask_num;
                for (int j = 0; j < samples_per_bit; j++) {
                    packet_sequences.push_back(amplitude * carrier[j]);
                }
            }
            //modulation

        }
    }
    void generate_ack_packet() {
        for (int i = 0; i < PREAMBLE_SIZE; i++) {
            ack_packet.push_back(preamble[i]);
        }
        std::vector<bool> data_bits = ack_data_symbols;
        unsigned int data_bits_per_packet = BITS_PER_SYMBOL * PACKET_DATA_SIZE;
        uint8_t crc_8;
        int data_size;
        unsigned char *data = unsigned_int_to_unsigned_char_star(from_bits_vector_to_unsigned_int(data_bits), data_size);
        crc_8 = CRC::CalculateBits(data, data_bits_per_packet, CRC::CRC_8());
        std::vector<unsigned int> crc_8_symbol = translate_from_bits_vector_to_unsigned_int_vector(from_uint8_t_to_bits_vector(crc_8), BITS_PER_SYMBOL);
        auto crc_8_symbol_after_modulation = modulater->Modulate(crc_8_symbol, 0);
        auto data_symbol_after_modulation = modulater->Modulate(translate_from_bits_vector_to_unsigned_int_vector(ack_data_symbols, BITS_PER_SYMBOL), 0);
        for (int i = 0; i < crc_8_symbol_after_modulation.size(); i++) {
            ack_packet.push_back(crc_8_symbol_after_modulation[i]);
        }
        //also modulate for mac 
        for (int i = 0; i < data_symbol_after_modulation.size(); i++) {
            ack_packet.push_back(data_symbol_after_modulation[i]);
        }
    }

    // convert a bit to 4 samples and add the samples to the end fo dest.
    void add_samples_from_a_bit(std::vector<double> &dest, int bit) {
        if (bit == 0) {
            dest.emplace_back(-0.9);
            dest.emplace_back(-0.9);
            dest.emplace_back(0.9);
            dest.emplace_back(0.9);
        }
        else if (bit == 1) {
            dest.emplace_back(0.9);
            dest.emplace_back(0.9);
            dest.emplace_back(-0.9);
            dest.emplace_back(-0.9);
        }
    }

    /// inBuffer ,outBuffer and num_samples is not used,status indicate ack or data you want to add,it will add to the transmittion_buffer 
    /// The return value is not used.
    bool Add_one_packet(const float *inBuffer, float *outBuffer, int num_samples, Tx_frame_status status, 
        unsigned int received_packet = 1) {
    // useful variables:
    // 
        // set these constants properly
        constexpr int data_bits_in_a_packet = NUM_PACKET_DATA_BITS;
        // 50000 / bits_in_a_packet is the number of packet
        // 1 是从1 到-1
        // 0 是从-1 到 1
        // 4 个 sample 表示1 bit
        if (status == Tx_data) {
            if (transmitted_packet >= maximum_packet)
                return false;
        }
            // add preamble
            for (int i = 0; i < PREAMBLE_SIZE; ++i) {
                transmittion_buffer.emplace_back(preamble[i]);
            }
            // Add a gap between preamble and others.
            for (int i = 0; i < 10; ++i) {
                transmittion_buffer.emplace_back(0.0);
            }
            // No crc. Add destination, source, type, packet number(start from 0).
            int concatenate = (OTHER_MAC_ADDRESS << 5) + (MY_MAC_ADDRESS << 2)
                + (status == Tx_data ? (int)Frame_Type::data : (int)Frame_Type::ack);
            concatenate = (concatenate << PACKET_NUM_BITS)
                + (status == Tx_data ?  transmitted_packet : received_packet - 1);
            for (int i = NUM_MAC_HEADER_BITS - 1; i >= 0; --i) {
                int bit = concatenate >> i & 1;
                if (bit == 1) {
                    add_samples_from_a_bit(transmittion_buffer, bit);
                }
                // 0
                else {
                    add_samples_from_a_bit(transmittion_buffer, bit);
                }                
            }
            // data
            if (status == Tx_data) {
                for (int i = transmitted_packet * data_bits_in_a_packet; i < (transmitted_packet + 1) * data_bits_in_a_packet; ++i) {
                    add_samples_from_a_bit(transmittion_buffer, (int)bits[i]);
                }
            }
            else if (status == Tx_ack) {
                for (int i = 0; i < data_bits_in_a_packet; ++i) {
                    add_samples_from_a_bit(transmittion_buffer, 0);
                }
            }
        
        return true;
    }

    //inBuffer is not used,outBuffer is used to read from tranmittion_buffer,and read the num_samples sample
    //trans finished then return true
    bool Trans(const float *inBuffer, float *outBuffer, int num_samples)    
    {
        bool silence = false;

            for (int i = 0; i < num_samples; i++) {
                if (transfer_num >= transmittion_buffer.size()) {
                    silence = true;
                    outBuffer[i] = 0;
                }
                else {

                    outBuffer[i] = transmittion_buffer[transfer_num];
                    transfer_num++;
                }
            }
            if (transfer_num >= transmittion_buffer.size()) {
                transfer_num = 0;
                transmittion_buffer.clear();
            }
            return silence;
    }
};