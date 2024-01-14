#ifndef TRANSMITTER_H
#define TRANSMITTER_H
#include <vector>
#include <string>
#include <deque>
#include <iostream>
#include <cmath>
#include <assert.h>
#include <JuceHeader.h>
#include "CRC.h"
#include <iomanip>  // Includes ::std::hex
#include <iostream> // Includes ::std::cout
#include <cstdint>  // Includes ::std::uint32_t
#include "utility.h"
#include "macros.h"

class Transmitter_with_wire {
public:
    Transmitter_with_wire() = default;
    Transmitter_with_wire(const std::string &path, int _sample_rate) :sample_rate(_sample_rate) {
        bits = Read_bits_from_bin(path);//read the bits from a bin file
        //every 504 bits (63 Bytes) have a crc calculation. A packet has 10 calculation, the last
        // calculation is not full 63 Bytes.
        for (int packet = 0; packet < NUM_TOTAL_PACKETS; ++packet) {
            char oneByte = 0;
            // first 9 calculation, 504 * 9 bits
            for (int one_calculation_start = 0; one_calculation_start + 504 < NUM_PACKET_DATA_BITS; 
                one_calculation_start += 504) {
                for (int j = 0; j < 504; ++j) {
                    oneByte = (oneByte << 1) + (char)bits[j + one_calculation_start + packet * NUM_PACKET_DATA_BITS];
                    if ((j + 1) % 8 == 0) {
                        bytes_for_crc_calculation[j / 8] = oneByte;
                        oneByte = 0;
                    }
                }
                std::uint32_t crc = CRC::CalculateBits(bytes_for_crc_calculation, sizeof(bytes_for_crc_calculation) * 8, CRC::CRC_32());
                crc_32_t.emplace_back((int)crc);
            } // end of first 9 calculation
            // The last 464 bits at the end of a packet
            oneByte = 0;
            for (int i = 504 * 9; i < NUM_PACKET_DATA_BITS; ++i) {
                oneByte = (oneByte << 1) + (char)bits[i + packet * NUM_PACKET_DATA_BITS];
                if ((i + 1) % 8 == 0) {
                    bytes_for_crc_calculation[(i - 504 * 9) / 8] = oneByte;
                    oneByte = 0;
                }
            }
            std::uint32_t crc = CRC::CalculateBits(bytes_for_crc_calculation, 464, CRC::CRC_32());
            crc_32_t.emplace_back((int)crc);
        } // end of all crc calculation

        FILE *file = fopen("crc.txt", "w");
        for (int i = 0; i < crc_32_t.size(); ++i) {
            fprintf(file, "%x, ", crc_32_t[i]);
            if ((i + 1) % 10 == 0) {
                fprintf(file, "\n");
            }
        }
        fclose(file);

        generate_preamble();
    }

    void generate_preamble() {
        preamble = std::vector<int>(64, 0);
        // 10101010 * 7 + 10101011
        for (int i = 0; i <= 62; ++i) {
            preamble[i] = (i + 1) % 2;
        }
        preamble[63] = 1;
    }

    std::vector<bool> bits;
    std::vector<int>preamble;
    int transmitted_packet;
    std::deque<double> transmittion_buffer;
    int sample_rate;

    // crc result, 32 bit width data
    std::vector<int> crc_32_t;
    char bytes_for_crc_calculation[63]{ 0 };
};
Transmitter_with_wire default_trans_wire("INPUT1to2.bin", 48000);

//Transmitter default_trans("INPUT.txt", default_sample_rate);


#endif
