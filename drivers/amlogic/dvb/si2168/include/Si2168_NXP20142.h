#ifndef  TUNER_NXP20142_SPECIFICS
    #define TUNER_NXP20142_SPECIFICS

    #include "Silabs_L1_RF_NXP20142_API.h"

    #define Si2168_CLOCK_MODE_SAT              Si2168_START_CLK_CMD_CLK_MODE_CLK_XTAL_IN
    #define Si2168_REF_FREQUENCY_SAT           27
    #define Si2168_SAT_IQ_SPECTRUM_INVERSION   Si2168_DD_MODE_PROP_INVERT_SPECTRUM_INVERTED

    #define TUNER_ADDRESS_SAT           0x1E
    #define TUNER_IF_MHZ_SAT            0.000000
    /*#define SPECTRAL_INVERSION_SAT*/ /* by default, no spectrum inversion */

    /* SAT_AGC_POLA :
      Assuming that SAT AGC is connected to MP_D:
      use 'mp_d_not_inverted' when 'gain vs voltage' curve slope is positive, 'mp_d_inverted' otherwise
      Do not define if not controlling the SAT AGC
    */
    /* SAT_AGC_POLARITY : define 'Si2168_SAT_AGC_INVERTED'  to invert the polarity */
/*    #define Si2168_SAT_AGC_INVERTED*/

    #define SAT_TUNER_CONTEXT           NXP20142_Context

    #define SAT_TUNER_WAKEUP(tuner)             L1_RF_NXP20142_Wakeup(tuner)
    #define SAT_TUNER_STANDBY(tuner)            L1_RF_NXP20142_Standby(tuner)
/*    #define SAT_TUNER_CLOCK_OFF(tuner) NXP20142_ClockOff */
/*    #define SAT_TUNER_CLOCK_ON(tuner)  NXP20142_ClockOn */

    /* Satellite tuner function names */
    #define L1_RF_SAT_TUNER_Process             L1_RF_NXP20142_Process
    #define L1_RF_SAT_TUNER_Init                L1_RF_NXP20142_Init
    #define SAT_TUNER_INIT                      L1_RF_NXP20142_InitAfterReset
    #define L1_RF_SAT_TUNER_Get_RF              L1_RF_NXP20142_Get_RF
    #define L1_RF_SAT_TUNER_Tune                L1_RF_NXP20142_Tune
    #define L1_RF_SAT_TUNER_LPF                 L1_RF_NXP20142_LPF
    #define SAT_TUNER_COMM_CHECK                NXP20142_Communication_Check

    #define SAT_TUNER_TAG_TEXT                  L1_RF_NXP20142_TAG_TEXT

#endif /* TUNER_NXP20142_SPECIFICS */
