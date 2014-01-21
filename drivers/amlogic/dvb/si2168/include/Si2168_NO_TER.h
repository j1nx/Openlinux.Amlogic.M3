#ifndef  TUNER_NO_TER_SPECIFICS
#define  TUNER_NO_TER_SPECIFICS

#ifdef    SILABS_EVB
    #define Si2168_CLOCK_MODE_TER              Si2168_START_CLK_CMD_CLK_MODE_CLK_CLKIO
    #define Si2168_REF_FREQUENCY_TER           24
#else  /* SILABS_EVB */
    #define Si2168_CLOCK_MODE_TER              Si2168_START_CLK_CMD_CLK_MODE_CLK_CLKIO
    #define Si2168_REF_FREQUENCY_TER           24
#endif /* SILABS_EVB */
    #define TUNER_ADDRESS_TER           0xc6
    #define TUNER_IF_MHZ_TER            5.000000
    #define LIF_MODE_TER
/*    #define SPECTRAL_INVERSION_TER */

    #define CONTROL_IF_AGC
    /* TER_IF_AGC_POLA : use 'mp_a_not_inverted' when 'gain vs voltage' curve slope is positive, 'mp_a_inverted' otherwise */
    #define TER_IF_AGC_POLA             mp_a_inv_not_inverted

    #define TER_TUNER_CONTEXT            void

    #define TER_TUNER_INIT                L1_RF_N0_TER_InitAfterReset
    #define L1_RF_TER_TUNER_Init(api,add) L1_RF_N0_TER_Init(api, add);
    #define L1_RF_TER_TUNER_Tune(api,rf)  L1_RF_N0_TER_Tune(api, rf)

    #define L1_RF_TER_TUNER_Get_RF        L1_RF_N0_TER_Get_RF

    #define L1_RF_TER_TUNER_MODULATION_DVBC  0
    #define L1_RF_TER_TUNER_MODULATION_DVBT  0
    #define L1_RF_TER_TUNER_MODULATION_DVBT2 0

#endif /* TUNER_NO_TER_SPECIFICS */












