#pragma once

// This cpp header file contains the **public** macros used by more than ones files.

/////////////////////////////////
// set these macros properly!///
/////////////////////////////////
//
// This is not used.
#define NUM_HEADER_BITS 8

#define NUM_CRC_BITS 8
#define NUM_DEST_BITS 3
#define NUM_SRC_BITS 3
#define NUM_TYPE_BITS 2
#define NUM_MAC_HEADER_BITS (NUM_DEST_BITS + NUM_SRC_BITS + NUM_TYPE_BITS)
#define NUM_PACKET_DATA_BITS 500
// packet index
#define PACKET_NUM_BITS 8
// how many samples represent a bit
#define NUM_SAMPLES_PER_BIT 4

#define MY_MAC_ADDRESS 0b010
#define OTHER_MAC_ADDRESS 0b001

