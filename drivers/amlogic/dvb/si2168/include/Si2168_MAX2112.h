#ifndef  TUNER_MAX2112_SPECIFICS
/* Change log: */
/*
 As from V2.5.1:
  Defining L1_RF_SAT_TUNER_LPF for MAX2112

****************************************************************************************/

    #define TUNER_MAX2112_SPECIFICS

    #include "Silabs_L1_RF_MAX2112_API.h"

#ifdef    SILABS_EVB
    #define Si2168_CLOCK_MODE_SAT              Si2168_START_CLK_CMD_CLK_MODE_XTAL
    #define Si2168_REF_FREQUENCY_SAT           16
    #define Si2168_SAT_IQ_SPECTRUM_INVERSION   Si2168_DD_MODE_PROP_INVERT_SPECTRUM_NORMAL
#else  /* SILABS_EVB */
    #define Si2168_CLOCK_MODE_SAT              Si2168_START_CLK_CMD_CLK_MODE_CLK_XTAL_IN
    #define Si2168_REF_FREQUENCY_SAT           16
    #define Si2168_SAT_IQ_SPECTRUM_INVERSION   Si2168_DD_MODE_PROP_INVERT_SPECTRUM_NORMAL
#endif /* SILABS_EVB */

    #define TUNER_ADDRESS_SAT           0xc2
    #define Si2168_SAT_AGC_INVERTED
    #define SAT_TUNER_CONTEXT           MAX2112_Context

/*    #define SAT_TUNER_WAKEUP(tuner)    */
/*    #define SAT_TUNER_STANDBY(tuner)   */
/*    #define SAT_TUNER_CLOCK_OFF(tuner) */
/*    #define SAT_TUNER_CLOCK_ON(tuner)  */

    /* Satellite tuner function names */
    #define L1_RF_SAT_TUNER_Process             L1_RF_MAX2112_Process
    #define L1_RF_SAT_TUNER_Init                L1_RF_MAX2112_Init
    #define SAT_TUNER_INIT                      L1_RF_MAX2112_InitAfterReset
    #define L1_RF_SAT_TUNER_Get_RF              L1_RF_MAX2112_Get_RF
    #define L1_RF_SAT_TUNER_Tune                L1_RF_MAX2112_Tune
    #define L1_RF_SAT_TUNER_LPF                 L1_RF_MAX2112_LPF

    #define SAT_TUNER_TAG_TEXT                  L1_RF_MAX2112_TAG_TEXT

#endif /* TUNER_MAX2112_SPECIFICS */
