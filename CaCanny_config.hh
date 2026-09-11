/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_config_hh
#define CaCanny_config_hh

#define ENABLE_MCP2515   1 // include in build

/* Uncomment one of the following if building on Teensy
 */
//#define TEST_CAN1_MCP2515_250k  1
//#define TEST_CAN2_MCP2515_1000k 1
#define TEST_CAN1_CAN2_250k     1
//#define TEST_CAN1_CAN2_1000k    1

#endif /* ! CaCanny_config_hh */
