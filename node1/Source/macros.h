#pragma once

// This cpp header file contains the **public** macros used by more than ones files.

/////////////////////////////////
// set these macros properly!///
/////////////////////////////////

//#define NUM_CRC_BITS 8
#define NUM_DEST_BITS 3
#define NUM_SRC_BITS 3
#define NUM_TYPE_BITS 2
#define NUM_DATE_LEN_BITS 16
// how many bits in a packet
#define NUM_PACKET_DATA_BITS 5000
#define NUM_TOTAL_PACKETS 10
// packet index length
#define PACKET_NUM_BITS 8
// how many samples represent a bit
#define NUM_SAMPLES_PER_BIT 4
#define NUM_CRC_BITS_PER_PACKET 320

#define NUM_MAC_HEADER_BITS (NUM_DEST_BITS + NUM_SRC_BITS + NUM_TYPE_BITS + PACKET_NUM_BITS + NUM_DATE_LEN_BITS)

#define MY_MAC_ADDRESS 0b001
#define OTHER_MAC_ADDRESS 0b010

// record real time inBuffer. Stop in time. Otherwise, the vector bombs.
#define RECORD_IN_LIVE false
// We do not have to start the two computers simultaneously. The macro decides whether this
// computer start transmitting first.
#define START_TRANS_FIRST true
#define CSMA_ONLY_RECEIVE false

#define IS_ROUTER true
#define MY_IP 0x111
#define PING_MODE true

#define TEST_CRC false
#define CORNER_LOG false
#define STOP_THREASHOLD 93000

