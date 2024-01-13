#pragma once
#include "transmitter.h"
#include "macros.h"

constexpr const int OVERHEAD_SYMBOLS = (NUM_CRC_BITS+ PACKET_NUM_BITS + NUM_DEST_BITS + NUM_SRC_BITS + NUM_TYPE_BITS) / BITS_PER_SYMBOL;//overhead of CRC +PACKET_NUM+DEST+SRC+TYPE

class Modulater {

public:
    std::vector<double> carrier_waves_0 = default_trans_wire.carrier_waves_0;
    std::vector<double> carrier_waves_1 = default_trans_wire.carrier_waves_1;
    std::vector<double> preamble = default_trans_wire.preamble;
    std::vector<double> Modulate(const std::vector<unsigned int > &symbols, int start_index = 0, int num_data = -1)
    {//modulate the data  and return the modulated data
        std::vector<double> buffer;
        int ask_num = 1 << (BITS_PER_SYMBOL - 1);
        if (num_data == -1)
        {//modulate them all
            for (auto& symbol : symbols)
            {
                auto carrier = (symbol & 1) == 1 ? carrier_waves_1 : carrier_waves_0;//using the last bit to determine psk
                float amplitude = max_amplitude * float((symbol >> 1) + 1) / ask_num;
                for (int j = 0; j < samples_per_bit; j++)
                {
                    buffer.push_back(amplitude * carrier[j]);
                }
            }

        }
        else {
            for (int i = start_index; i < start_index + num_data; i++) {
                auto symbol = symbols[i];
                auto carrier = (symbol & 1) == 1 ? carrier_waves_1 : carrier_waves_0;//using the last bit to determine psk
                float amplitude = max_amplitude * float((symbol >> 1) + 1) / ask_num;
                for (int j = 0; j < samples_per_bit; j++)
                {
                    buffer.push_back(amplitude * carrier[j]);
                }
            }
        }
        return buffer;
    }
};
class Demoudulator {
public:
    std::vector<double> carrier_waves_0 = default_trans_wire.carrier_waves_0;
    std::vector<double> carrier_waves_1 = default_trans_wire.carrier_waves_1;
    std::vector<double> max_vector;
    double amplitude_step = default_trans_wire.amplitude_step;
    void Write_max() {
        Write("max_vector.txt", max_vector);
    }
    std::vector<unsigned int > Demodulate(const std::vector<double >& decode_buffer,int start_index,int num_symbols=-1 ) {
        std::vector<unsigned int> symbols;
        if (num_symbols == -1) {//decode them all
            unsigned int size = decode_buffer.size();
            for (int i = start_index; i < size; i += samples_per_bit) {
                int _begin = i;
                unsigned int symbol = demodulate_one_symbol(decode_buffer, _begin);//read from begin to 
                symbols.push_back(symbol);
            }
        }
        else {
            unsigned int size = decode_buffer.size();
            unsigned int max_achieved_size = num_symbols * samples_per_bit + start_index > size ? size:num_symbols*samples_per_bit+start_index;
            for (int i = start_index; i < max_achieved_size; i += samples_per_bit) {
                int _begin = i;
                unsigned int symbol = demodulate_one_symbol(decode_buffer, _begin);
                symbols.push_back(symbol);
            }
        }
        return symbols;
    }

    bool check_crc_one_packet(const std::vector<unsigned int>& symbols,int start_index=0) {//check the packet if it satisfy crc_check then return true else return false
        unsigned int crc_offset = 8 / BITS_PER_SYMBOL;
        unsigned int data_offset = (NUM_CRC_BITS +PACKET_NUM_BITS+ NUM_DEST_BITS + NUM_SRC_BITS + NUM_TYPE_BITS) / BITS_PER_SYMBOL;
        std::vector<unsigned int > data_symbols=vector_from_start_to_end(symbols,start_index+data_offset,start_index+data_offset+PACKET_DATA_SIZE);//received data slice
        std::vector<unsigned int> crc_symbols = vector_from_start_to_end(symbols,start_index,start_index+crc_offset);//received symbols
        std::vector<bool> data_bits = from_symbols_to_bits(data_symbols,BITS_PER_SYMBOL);// translate the packet symbols to data
        int data_size,data_bits_per_packet=PACKET_DATA_SIZE*BITS_PER_SYMBOL;// total data_bits
        unsigned char* data = unsigned_int_to_unsigned_char_star(from_bits_vector_to_unsigned_int(data_bits), data_size);//translate the data_bits to unsigned char * and give the size to data_size
        uint8_t crc_8 = CRC::CalculateBits(data, data_bits_per_packet, CRC::CRC_8());//calculate the crc_8
        std::vector<bool> crc_8_vector = from_uint8_t_to_bits_vector(crc_8);//vectorize
        std::vector<bool> crc_bits = from_symbols_to_bits(crc_symbols, BITS_PER_SYMBOL);
        for (int i = 0; i <= 7; i++) {//check if they are the same
            if (crc_8_vector[i] != crc_bits[i]) {
                return false;
            }
        }
        return true;
    }
    unsigned int demodulate_one_symbol(const std::vector<double>& decode_buffer, int start_index) {
        double sum = 0;
        unsigned int number;
        float scale = 0.8;
        double max_amplitude = 0;
        double second_max_amplitude = 0;
        int sample_start = samples_per_bit / 8+start_index;
        int sample_end = samples_per_bit * 7 / 8+start_index;
        for (int i = sample_start; i <sample_end; ++i) {
            sum += decode_buffer[i] * carrier_waves_1[i];

        }
        int sample_find_max_start = start_index;
        int sample_find_max_end = start_index + samples_per_bit * 1 / 4;
        for (int i = sample_start; i < sample_end; ++i) {
            double abs = decode_buffer[i] > 0 ? decode_buffer[i] : -decode_buffer[i];
            max_amplitude = abs < max_amplitude ? max_amplitude : abs;
        }
        //double abs = decode_buffer[start_index] > 0 ? decode_buffer[start_index] : -decode_buffer[start_index];
     
        //for (int i = sample_start; i < sample_end; ++i) {
        //    double abs = decode_buffer[i] > 0 ? decode_buffer[i] : -decode_buffer[i];
        //    if (abs < max_amplitude) {
        //        second_max_amplitude = std::max(abs, second_max_amplitude);
        //    }
        //}
        max_amplitude /= scale;
        max_vector.push_back(max_amplitude);
        if (sum > 0) {
            number = default_trans_wire.demodulate(max_amplitude);
            number = (number << 1) + 1;

        }
        else {
            sum = 0;
            sum /= 14;
            sum *= scale;
            number = default_trans_wire.demodulate(max_amplitude);
            number = number << 1;

        }
        return number;

    }
};