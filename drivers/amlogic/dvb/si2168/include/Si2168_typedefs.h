/*************************************************************************************
                  Silicon Laboratories Broadcast Si2168 Layer 1 API
   API types used by commands and properties
   FILE: Si2168_typedefs.h
   Supported IC : Si2168
   Compiled for ROM 2 firmware 2_0_build_11
   Revision: REVISION_NAME
   Date: September 24 2012
  (C) Copyright 2012, Silicon Laboratories, Inc. All rights reserved.
**************************************************************************************/
/* Change log:

 As from 2.1.1:
  Defining Si2168_SAT_MAX_SEARCH_TIME and Si2168_TER_MAX_SEARCH_TIME for seekNext
  Defining Si2168_TERRESTRIAL and Si2168_SATELLITE to handle tuners in AUTODETECT mode
****************************************************************************************/

#ifndef   Si2168_TYPEDEFS_H
#define   Si2168_TYPEDEFS_H


#define L0_Context unsigned char
/*typedef struct{
	unsigned char	trackWrite;
	unsigned char	trackRead;
	unsigned char	trackWrite;
	unsigned char	trackWrite;
} L0_Context;*/
#define CONNECTION_TYPE int



#define MAX_LENGTH             80


  #define Si2168_DVBT_MIN_LOCK_TIME    100
  #define Si2168_DVBT_MAX_LOCK_TIME   5000

  #define Si2168_DVBT2_MIN_LOCK_TIME   100
  #define Si2168_DVBT2_MAX_LOCK_TIME  5000

  #define Si2168_DVBC_MIN_LOCK_TIME     80
  #define Si2168_DVBC_MAX_LOCK_TIME   2000
  #define Si2168_DVBC_MAX_SEARCH_TIME 5000

  #define Si2168_TER_MAX_SEARCH_TIME 10000

  #define Si2168_TERRESTRIAL 1


typedef struct L1_Si2168_Context {
  L0_Context                  i2c;//*i2c;
  L0_Context                  i2cObj;
  Si2168_CmdObj              *cmd;
  Si2168_CmdObj               cmdObj;
  Si2168_CmdReplyObj         *rsp;
  Si2168_CmdReplyObj          rspObj;
  Si2168_PropObj             *prop;
  Si2168_PropObj              propObj;
  Si2168_COMMON_REPLY_struct *status;
  Si2168_COMMON_REPLY_struct  statusObj;
  int                         standard;
  int                         media;
  unsigned int                tuner_ter_chip;
  unsigned int                tuner_ter_clock_freq;
  unsigned int                tuner_ter_clock_input;
  unsigned int  tuner_rssi;
} L1_Si2168_Context;
#endif /* Si2168_TYPEDEFS_H */

