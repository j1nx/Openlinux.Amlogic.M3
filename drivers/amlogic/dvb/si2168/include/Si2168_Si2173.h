#ifndef  TUNER_Si2173_SPECIFICS
#define  TUNER_Si2173_SPECIFICS

    #include "Si2173_L2_API.h"

#ifdef    SILABS_EVB
    #define Si2168_CLOCK_MODE_TER              Si2168_START_CLK_CMD_CLK_MODE_CLK_CLKIO
    #define Si2168_REF_FREQUENCY_TER           24
#else  /* SILABS_EVB */
    #define Si2168_CLOCK_MODE_TER              Si2168_START_CLK_CMD_CLK_MODE_CLK_CLKIO
    #define Si2168_REF_FREQUENCY_TER           24
#endif /* SILABS_EVB */

    #define TUNERTER_API
    #define TER_TUNER_CONTEXT            L1_Si2173_Context
    #define TUNER_ADDRESS_TER            0xc0

    #define TER_TUNER_INIT(api)           Si2173_Init(api)
    #define TER_TUNER_WAKEUP(api)         Si2173_pollForCTS(api)
    #define TER_TUNER_STANDBY(api)        Si2173_L1_STANDBY(api)
    #define TER_TUNER_CLOCK_OFF(api)      Si2173_ClockOff(api)
    #define TER_TUNER_CLOCK_ON(api)       Si2173_ClockOn(api)
    #define TER_TUNER_ERROR_TEXT(res)     Si2173_L1_API_ERROR_TEXT(res)
#ifdef    Si2168_COMMAND_LINE_APPLICATION
    #define TER_TUNER_MENU(full)          Si2173_menu(full)
    #define TER_TUNER_LOOP(api,choice)    Si2173_demoLoop(api,choice)
#endif /* Si2168_COMMAND_LINE_APPLICATION */

    #define TER_TUNER_MODULATION_DVBT     Si2173_DTV_MODE_PROP_MODULATION_DVBT
    #define TER_TUNER_MODULATION_DVBC     Si2173_DTV_MODE_PROP_MODULATION_DVBC

    #define L1_RF_TER_TUNER_Init(api,add) Si2173_L1_API_Init(api, add);
    #define L1_RF_TER_TUNER_Tune(api,rf)  Si2173_DTVTune(api, rf, bw, modulation, Si2173_DTV_MODE_PROP_INVERT_SPECTRUM_NORMAL)

    #define L1_RF_TER_TUNER_Get_RF

    #define L1_RF_TER_TUNER_MODULATION_DVBC  Si2173_DTV_MODE_PROP_MODULATION_DVBC
    #define L1_RF_TER_TUNER_MODULATION_DVBT  Si2173_DTV_MODE_PROP_MODULATION_DVBT
    #define L1_RF_TER_TUNER_MODULATION_DVBT2 Si2173_DTV_MODE_PROP_MODULATION_DVBT

    #define   L1_RF_TER_TUNER_FEF_MODE_SLOW_NORMAL_AGC(api) \
      api->prop->dtv_agc_speed.agc_decim               = Si2173_DTV_AGC_SPEED_PROP_AGC_DECIM_2;\
      api->prop->dtv_agc_speed.if_agc_speed            = Si2173_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_39;\
      Si2173_L1_SetProperty2(api, Si2173_DTV_AGC_SPEED_PROP_CODE);

#endif /* TUNER_Si2173_SPECIFICS */
