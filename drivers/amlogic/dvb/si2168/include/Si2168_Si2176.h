#ifndef  TUNER_Si2176_SPECIFICS
#define  TUNER_Si2176_SPECIFICS

    #include "Si2176_L2_API.h"

#ifdef    SILABS_EVB
    #define Si2168_CLOCK_MODE_TER              Si2168_START_CLK_CMD_CLK_MODE_XTAL
    #define Si2168_REF_FREQUENCY_TER           16
#else  /* SILABS_EVB */
    #define Si2168_CLOCK_MODE_TER              Si2168_START_CLK_CMD_CLK_MODE_CLK_CLKIO
    #define Si2168_REF_FREQUENCY_TER           24
#endif /* SILABS_EVB */

    #define TUNERTER_API
    #define TER_TUNER_CONTEXT            L1_Si2176_Context
    #define TUNER_ADDRESS_TER            0xc0

    #define TER_TUNER_INIT(api)           Si2176_Init(api)
    #define TER_TUNER_WAKEUP(api)         Si2176_pollForCTS(api)
    /* Compatibility with Si2176 ROM12 & ROM13 APIs */
   #ifdef    Si2176_STANDBY_CMD_TYPE_MIN
     #define TER_TUNER_STANDBY(api)        Si2176_L1_STANDBY(api, Si2176_STANDBY_CMD_TYPE_MIN)
   #else  /* Si2176_STANDBY_CMD_TYPE_MIN */
    #define TER_TUNER_STANDBY(api)        Si2176_L1_STANDBY(api)
   #endif /* Si2176_STANDBY_CMD_TYPE_MIN */
    #define TER_TUNER_CLOCK_OFF(api)      Si2176_ClockOff(api)
    #define TER_TUNER_CLOCK_ON(api)       Si2176_ClockOn(api)
    #define TER_TUNER_ERROR_TEXT(res)     Si2176_L1_API_ERROR_TEXT(res)
#ifdef    Si2168_COMMAND_LINE_APPLICATION
    #define TER_TUNER_MENU(full)          Si2176_menu(full)
    #define TER_TUNER_LOOP(api,choice)    Si2176_demoLoop(api,choice)
#endif /* Si2168_COMMAND_LINE_APPLICATION */

    #define TER_TUNER_MODULATION_DVBT     Si2176_DTV_MODE_PROP_MODULATION_DVBT
    #define TER_TUNER_MODULATION_DVBC     Si2176_DTV_MODE_PROP_MODULATION_DVBC

    #define L1_RF_TER_TUNER_Init(api,add) Si2176_L1_API_Init(api, add);
    #define L1_RF_TER_TUNER_Tune(api,rf)  Si2176_DTVTune(api, rf, bw, modulation, Si2176_DTV_MODE_PROP_INVERT_SPECTRUM_NORMAL)

    #define L1_RF_TER_TUNER_Get_RF

    #define L1_RF_TER_TUNER_MODULATION_DVBC  Si2176_DTV_MODE_PROP_MODULATION_DVBC
    #define L1_RF_TER_TUNER_MODULATION_DVBT  Si2176_DTV_MODE_PROP_MODULATION_DVBT
    #define L1_RF_TER_TUNER_MODULATION_DVBT2 Si2176_DTV_MODE_PROP_MODULATION_DVBT

#ifdef    Si2176_DTV_AGC_FREEZE_INPUT_PROP_CODE
    #define   L1_RF_TER_TUNER_FEF_MODE_FREEZE_PIN_SETUP(api) \
    api->prop->dtv_initial_agc_speed.agc_decim     = Si2176_DTV_INITIAL_AGC_SPEED_PROP_AGC_DECIM_OFF;\
    api->prop->dtv_initial_agc_speed.if_agc_speed  = Si2176_DTV_INITIAL_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO;\
    api->prop->dtv_initial_agc_speed_period.period = 0;\
    api->prop->dtv_agc_speed.agc_decim             = Si2176_DTV_AGC_SPEED_PROP_AGC_DECIM_OFF;\
    api->prop->dtv_agc_speed.if_agc_speed          = Si2176_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO;\
    Si2176_L1_SetProperty2(api, Si2176_DTV_INITIAL_AGC_SPEED_PERIOD_PROP_CODE);\
    Si2176_L1_SetProperty2(api, Si2176_DTV_AGC_SPEED_PROP_CODE);

    #define   L1_RF_TER_TUNER_FEF_MODE_FREEZE_PIN(api,fef) \
    if (fef == 0) {\
      api->prop->dtv_agc_freeze_input.pin          = Si2176_DTV_AGC_FREEZE_INPUT_PROP_PIN_NONE;\
    } else {\
      api->prop->dtv_agc_freeze_input.pin          = Si2176_DTV_AGC_FREEZE_INPUT_PROP_PIN_GPIO1;\
    }\
    Si2176_L1_SetProperty2(api, Si2176_DTV_AGC_FREEZE_INPUT_PROP_CODE);
#endif /*   Si2176_DTV_AGC_FREEZE_INPUT_PROP_CODE */

    #define   L1_RF_TER_TUNER_FEF_MODE_SLOW_INITIAL_AGC_SETUP(api,fef) \
      if (fef == 0) {\
        api->prop->dtv_initial_agc_speed.agc_decim     = Si2176_DTV_INITIAL_AGC_SPEED_PROP_AGC_DECIM_OFF;\
        api->prop->dtv_initial_agc_speed.if_agc_speed  = Si2176_DTV_INITIAL_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO;\
        api->prop->dtv_initial_agc_speed_period.period = 0;\
        api->prop->dtv_agc_speed.agc_decim             = Si2176_DTV_AGC_SPEED_PROP_AGC_DECIM_OFF;\
        api->prop->dtv_agc_speed.if_agc_speed          = Si2176_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO;\
      } else {\
        api->prop->dtv_initial_agc_speed.agc_decim     = Si2176_DTV_INITIAL_AGC_SPEED_PROP_AGC_DECIM_OFF;\
        api->prop->dtv_initial_agc_speed.if_agc_speed  = Si2176_DTV_INITIAL_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO;\
        api->prop->dtv_initial_agc_speed_period.period = 310;\
        api->prop->dtv_agc_speed.agc_decim             = Si2176_DTV_AGC_SPEED_PROP_AGC_DECIM_4;\
        api->prop->dtv_agc_speed.if_agc_speed          = Si2176_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_39;\
      }\
      Si2176_L1_SetProperty2(api, Si2176_DTV_INITIAL_AGC_SPEED_PERIOD_PROP_CODE);\
      Si2176_L1_SetProperty2(api, Si2176_DTV_AGC_SPEED_PROP_CODE);


    #define   L1_RF_TER_TUNER_FEF_MODE_SLOW_NORMAL_AGC_SETUP(api,fef) \
      api->prop->dtv_initial_agc_speed_period.period = 0;\
      if (fef == 0) {\
        api->prop->dtv_agc_speed.agc_decim             = Si2176_DTV_AGC_SPEED_PROP_AGC_DECIM_OFF;\
        api->prop->dtv_agc_speed.if_agc_speed          = Si2176_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO;\
      } else {\
        api->prop->dtv_agc_speed.if_agc_speed          = Si2176_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_39;\
        api->prop->dtv_agc_speed.agc_decim             = Si2176_DTV_AGC_SPEED_PROP_AGC_DECIM_4;\
      }\
      Si2176_L1_SetProperty2(api, Si2176_DTV_INITIAL_AGC_SPEED_PERIOD_PROP_CODE);\
      Si2176_L1_SetProperty2(api, Si2176_DTV_AGC_SPEED_PROP_CODE);

    #define   L1_RF_TER_TUNER_FEF_MODE_SLOW_NORMAL_AGC(api,fef) \
      if (fef == 0) {\
        api->prop->dtv_agc_speed.agc_decim             = Si2176_DTV_AGC_SPEED_PROP_AGC_DECIM_OFF;\
        api->prop->dtv_agc_speed.if_agc_speed          = Si2176_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO;\
      } else {\
        api->prop->dtv_agc_speed.if_agc_speed          = Si2176_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_39;\
        api->prop->dtv_agc_speed.agc_decim             = Si2176_DTV_AGC_SPEED_PROP_AGC_DECIM_4;\
      }\
      Si2176_L1_SetProperty2(api, Si2176_DTV_AGC_SPEED_PROP_CODE);

    #define TER_TUNER_ATV_LO_INJECTION(api) \
      api->prop->tuner_lo_injection.band_1 = Si2176_TUNER_LO_INJECTION_PROP_BAND_1_HIGH_SIDE;\
      api->prop->tuner_lo_injection.band_2 = Si2176_TUNER_LO_INJECTION_PROP_BAND_2_HIGH_SIDE;\
      api->prop->tuner_lo_injection.band_3 = Si2176_TUNER_LO_INJECTION_PROP_BAND_3_HIGH_SIDE;\
      api->prop->tuner_lo_injection.band_4 = Si2176_TUNER_LO_INJECTION_PROP_BAND_4_HIGH_SIDE;\
      api->prop->tuner_lo_injection.band_5 = Si2176_TUNER_LO_INJECTION_PROP_BAND_5_HIGH_SIDE;\
      Si2176_L1_SetProperty2(api, Si2176_TUNER_LO_INJECTION_PROP_CODE);

    #define TER_TUNER_DTV_LO_INJECTION(api) \
      api->prop->tuner_lo_injection.band_1 = Si2176_TUNER_LO_INJECTION_PROP_BAND_1_HIGH_SIDE;\
      api->prop->tuner_lo_injection.band_2 = Si2176_TUNER_LO_INJECTION_PROP_BAND_2_LOW_SIDE;\
      api->prop->tuner_lo_injection.band_3 = Si2176_TUNER_LO_INJECTION_PROP_BAND_3_LOW_SIDE;\
      api->prop->tuner_lo_injection.band_4 = Si2176_TUNER_LO_INJECTION_PROP_BAND_4_LOW_SIDE;\
      api->prop->tuner_lo_injection.band_5 = Si2176_TUNER_LO_INJECTION_PROP_BAND_5_LOW_SIDE;\
      Si2176_L1_SetProperty2(api, Si2176_TUNER_LO_INJECTION_PROP_CODE);

#endif /* TUNER_Si2176_SPECIFICS */












