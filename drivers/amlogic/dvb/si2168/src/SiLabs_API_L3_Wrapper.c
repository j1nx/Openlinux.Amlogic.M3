/*************************************************************************************************************/
/*                                  Silicon Laboratories                                                     */
/*                                  Broadcast Video Group                                                    */
/*                                  SiLabs API Functions                                                     */
/*-----------------------------------------------------------------------------------------------------------*/
/*   This source code contains all API functions for a SiLabs digital TV demodulator Evaluation board        */
/*-----------------------------------------------------------------------------------------------------------*/
/* Change log: */
/* Last  changes:

  As from V2.0.6:
   In SiLabs_API_Demod_status:
    For Si2165D:        
     status->IFagc = Si2165_L1_Demod_get_IFagc        (front_end->Si2165_FE->demod);
     (previously returning aci_agc_cmd)
    For Si2168:
     Calling Si2168_L1_DVBT2_TX_ID
     status->cell_id            = front_end->Si2168_FE->demod->rsp->dvbt2_tx_id.cell_id;

*/
/* Older changes:
  As from V2.0.5:
   LNBH init correction:
   In SiLabs_API_SW_Init, front_end->lnb_chip_init_done = 0; to force the flag at '0'.
    (Some compilers may set it randomly, so it needs to be forced to '0' for compatibility reasons.)

  As from V2.0.4:
   ADDED FEATURE: Added SiLabs_API_Channel_Lock_Abort function, to allow aborting a call to SiLabs_API_lock_to_carrier.
   In SiLabs_API_Demod_status:
    Added Si2167 compatibility with TER tuners not from SiLabs

  As from V2.0.3:
   In SiLabs_API_TS_Mode:
    For Si2167B and Si2168: settings ts parallel clock and data shape to 7 for GPIF mode, and back to 2 for parallel mode

  As from V2.0.2:
   In SiLabs_API_Select_PLP:
    For Si2168: if (plp_mode == Si2168_DVBT2_PLP_SELECT_CMD_PLP_ID_SEL_MODE_AUTO) { Si2168_L1_DD_RESTART(front_end->Si2168_FE->demod); system_wait(300); }

  As from V2.0.1:
   Added lnb_chip_init_done in SILABS_FE_Context
   In SiLabs_API_SAT_voltage_and_tone:
    if (front_end->lnb_chip_init_done == 0) { front_end->lnb_chip_init_done = L1_LNBH25_InitAfterReset(front_end->lnbh25); }
    This is because the LNBH25 requires an init of all registers

  As from V2.0.0:
   In SiLabs_API_Text_status:
    sprintf(formatted_status, "%s TS bitrate       %d kbps\n", formatted_status, status->TS_bitrate_kHz);
    sprintf(formatted_status, "%s TS clock         %d kHz\n" , formatted_status, status->TS_clock_kHz  );
   SiLabs_API_Reset_Uncorrs compatibility with Si2165
   SiLabs_API_Demod_reset   compatibility with Si2165 and Si2167

  As from V1.9.9:
    Adding NO_SAT tags to allow using Si2168 code without SAT features

  As from V1.9.8:
    Compatibility with TER_TUNER_Si2190
    Compatibility with TER_TUNER_CUSTOMTER
    In SiLabs_API_SAT_AutoDetectCheck:
     Adapting Si2168 code to return the current SAT standard when locked, 0 otherwise.
    In Silabs_API_Test:
    adding wrapper/sat_auto_detect option
    Compatibility with SAT_TUNER_RDA5816S

  As from V1.9.7:
    Adding LNB control in the API, to allow driving LNBH25 or LNBH21 easily
    In SiLabs_API_Demod_status: If Si2168, setting plp_id based on rsp.dvbt2_status.plp_id

  As from V1.9.6:
    Passing pointer to LNB function when calling SiLabs_Unicable_API_Init, following the new definition of SiLabs_Unicable_API_Init

  As from V1.9.5:
    Adding SILABS_MCNS, SILABS_DVB-C2 and SILABS_SLEEP possibilities
    Adding MCNS statusing
    In SiLabs_API_Demod_status: no demod status in SLEEP mode

  As from V1.9.4:
    In SiLabs_API_SAT_Get_AGC:
      Corrected value returned for Si2168 SAT AGC
    In Silabs_API_Test:
      Added Unicable test pipe access

  As from V1.9.3:
    adding TS_bitrate_kHz and TS_clock_kHz in demod status

  As from V1.9.2:
    Si2146 ATV and DTV STATUS removed (not in the Si2146 API anymore)

  As from V1.9.1:
    added/moved tags to allow Si2166B export
   In SiLabs_API_Demod_status:
    setting SSi and SQI at 0 by default.
    compatibility with rssi from CUSTOMTER and CUSTOMSAT tuners

  As from V1.9.0:
   In SiLabs_API_SAT_Tuner_status: moving tags to allow export for Si2168
   Wrapper code compatible with Si2167B: checked to be able to lock a Si2168 board when using the Si2167B code with the proper FW.

  As from V1.8.9:
   Tracing Wrapper source code info during init and in SiLabs_API_Infos
   SiLabs_API_SatAutoDetectCheck renamed as SiLabs_API_SAT_AutoDetectCheck for consistency
   Adding Test Pipe feature (only if SILABS_API_TEST_PIPE is defined at project level), using new Silabs_API_Test function
   Adding PLP management (for DVB_T2 only).
   In SiLabs_API_Demod_status:
    updating spectral_inversion for Si2168 in DVB-T and DBVB-T2
   In SiLabs_API_TS_Mode:
    Stopping GPIF clock if using the Cypress USB interface and not using GPIF mode
   In SiLabs_API_Demod_status and SiLabs_API_Text_status:
    Not storing current standard as front_end->standard, to avoid creating problems with standard switching.
    Using status->standard in all switches.
   In SiLabs_API_TER_Tuner_status and SiLabs_API_SAT_Tuner_status:
    Enabling i2c passthru before statusing tuners

  As from V1.8.8:
   In SiLabs_API_Demod_status:
    setting more statuses by default to indicate a no-lock:
      status->c_n                = 0;
      status->freq_offset        = 0;
      status->timing_offset      = 0;
      status->code_rate          = -1;
      status->SSI                = 0;
      status->SQI                = 0;
    For Si2168: returning '0' immediately in case a standard-specific status returns with an error.

  As from V1.8.7:
   Compatibility with Si2167B (coming soon)

  As from V1.8.6:
   Compatibility with Si2148/Si2158

  As from V1.8.5:
   Adding auto-detect functions:
   int   SiLabs_API_SAT_AutoDetect           (SILABS_FE_Context *front_end, int  on_off);
   int   SiLabs_API_TER_AutoDetect           (SILABS_FE_Context *front_end, int  on_off);

  As from V1.8.4:
   In SiLabs_API_SAT_voltage_and_tone: disegBuffer value correction
   Adding UNICABLE functions (compiled if #define UNICABLE_COMPATIBLE).

  As from V1.8.3:
   Compatibility with Si2178

  As from V1.8.2:
   In SiLabs_API_Demod_status:
    setting ber, fer and per by default at '-1' to indicate unavailability if not set later on.
   In SiLabs_API_Text_status:
    ber and per displayed as '--------' when not available

  As from V1.8.1:
   In SiLabs_API_Demod_status:
    (Si2168) Comments correction indicating that the rate checks are done on the exponent
    Comments correction indicating that the rate checks are done on the exponent
   In SiLabs_API_Text_status:
    spectral inversion added to text status

  As from V1.8.0:
   Compatibility with DTT759x (Terrestrial can tuner)
   Checking exponent for rate in SiLabs_API_Demod_status, to return -1 if not available

  As from V1.7.9:
   voltage_and_tone working with Si2167

  As from V1.7.8:
   voltage_and_tone working with LNBH21
   SAT and DVB-C blindscan working for Si2168
   compatibility with NO_TER 'dummy' TER tuner (for lab use)

  As from V1.7.7:
   BER monitored for Si2168 in DVB-T2 and DVB-S2 as well as for all DTV standards (previously not in FW so it was skipped)

  As from V1.7.6:
   2 lines added to allow exporting for demods with no 'STANDBY' or 'CLOCK_ON' feature

  As from 1.7.3:
   some lines moved for greater compatibility with Visual Studio

  As from 1.7.2:
   Si2168 agc values retrieved in SiLabs_API_Demod_status

  As from 1.7.0:
   adding WrapperI2C context to allow easy i2c read/write
   added SiLabs_API_ReadString/SiLabs_API_WriteString functions
   For SAT: added voltage/tone and DiSEqC functions

  As from 1.6.9:
   For Si2168: status->stream based on demod->prop->dvbt_hierarchy.stream;

  As from 1.6.7:
   Compatibility with NXP20142 SAT tuner
  API change: using Si2168 DD_SSI_SQI instead of Si2168_DVBT_SSI_SQI (also available in DVB-T2)

  As from 1.6.6:
   Adding missing BER status for Si2168

  As from 1.6.5:
   as it makes it easier to handle C-only or T-only exports

  As from 1.6.1:
   Compatibility with TER tuner cans (not using API mode)
   SiLabs_API_TER_Tuner_ATV_Tune compatible with Si2165

  As from 1.6.0:
   Added Si2185 support
   In   SiLabs_API_Demod_status:
     Set to 0 all info used to relock (bandwidth_Hz, symbol_rate, stream, constellation)
   In SiLabs_API_switch_to_standard:
     For Si2168: Checking dd_status.modulation if switch_to_standard fails
   In SiLabs_API_lock_to_carrier:
     Returning 0 if switch_to_standard fails

  As from 1.5.1:
        power_of_n corrected to return the proper value

 *************************************************************************************************************/
/* TAG V2.0.6 */

#include "SiLabs_API_L3_Wrapper.h"

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

int   Silabs_init_done = 0;
/* Also init a simple i2c context to allow i2c communication checking */
L0_Context* WrapperI2C;
L0_Context  WrapperI2C_context;

/* Translation functions from 'Custom' values to 'SiLabs' values */
int   power_of_n (int n, int m) {
  int i;
  int p;
  p = 1;
  for (i=1; i<= m; i++) {
    p = p*n;
  }
  return p;
}
/************************************************************************************************************************
  Silabs_standardCode function
  Use:        standard code function
              Used to retrieve the standard value used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  standard, the value used by the top-level application (configurable in CUSTOM_Standard_Enum)
************************************************************************************************************************/
int   Silabs_standardCode                 (SILABS_FE_Context* front_end, CUSTOM_Standard_Enum          standard)
{
  front_end = front_end; /* to avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (standard) {
      case SILABS_DVB_T : return Si2168_DD_MODE_PROP_MODULATION_DVBT;
      case SILABS_DVB_T2: return Si2168_DD_MODE_PROP_MODULATION_DVBT2;
      case SILABS_DVB_C : return Si2168_DD_MODE_PROP_MODULATION_DVBC;
      default           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_constelCode function
  Use:        constel code function
              Used to retrieve the constel value used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  constel, the value used by the top-level application (configurable in CUSTOM_Constel_Enum)
************************************************************************************************************************/
int   Silabs_constelCode                  (SILABS_FE_Context* front_end, CUSTOM_Constel_Enum           constel)
{
  front_end = front_end; /* to avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (constel) {
      case SILABS_QAMAUTO : return Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_AUTO ;
      case SILABS_QAM16   : return Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM16 ;
      case SILABS_QAM32   : return Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM32 ;
      case SILABS_QAM64   : return Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM64 ;
      case SILABS_QAM128  : return Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM128;
      case SILABS_QAM256  : return Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM256;
      default             : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_streamCode function
  Use:        stream code function
              Used to retrieve the stream value used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  stream, the value used by the top-level application (configurable in CUSTOM_Stream_Enum)
************************************************************************************************************************/
int   Silabs_streamCode                   (SILABS_FE_Context* front_end, CUSTOM_Stream_Enum            stream)
{
  front_end = front_end; /* to avoid compiler warning if not used */
  stream    = stream;    /* to avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (stream) {
      case SILABS_HP : return Si2168_DVBT_HIERARCHY_PROP_STREAM_HP   ;
      case SILABS_LP : return Si2168_DVBT_HIERARCHY_PROP_STREAM_LP   ;
      default           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_plptypeCode function
  Use:        plp type code function
              Used to retrieve the plp type value used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  plp_type, the value used by the top-level application (configurable in CUSTOM_T2_PLP_TYPE)
************************************************************************************************************************/
int   Silabs_plptypeCode                  (SILABS_FE_Context* front_end, CUSTOM_T2_PLP_TYPE            plp_type)
{
  front_end = front_end; /* to avoid compiler warning if not used */
  plp_type  = plp_type ; /* To avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (plp_type) {
      case SILABS_PLP_TYPE_COMMON     : return Si2168_DVBT2_PLP_INFO_RESPONSE_PLP_TYPE_COMMON    ;
      case SILABS_PLP_TYPE_DATA_TYPE1 : return Si2168_DVBT2_PLP_INFO_RESPONSE_PLP_TYPE_DATA_TYPE1;
      case SILABS_PLP_TYPE_DATA_TYPE2 : return Si2168_DVBT2_PLP_INFO_RESPONSE_PLP_TYPE_DATA_TYPE2;
      default           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_fftCode function
  Use:        fft code function
              Used to retrieve the fft value used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  fft, the value used by the top-level application (configurable in CUSTOM_FFT_Mode_Enum)
************************************************************************************************************************/
int   Silabs_fftCode                      (SILABS_FE_Context* front_end, CUSTOM_FFT_Mode_Enum          fft)
{
  front_end = front_end; /* to avoid compiler warning if not used */
  fft       = fft;       /* to avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (fft) {
      case SILABS_FFT_MODE_2K : return Si2168_DVBT_STATUS_RESPONSE_FFT_MODE_2K;
      case SILABS_FFT_MODE_4K : return Si2168_DVBT_STATUS_RESPONSE_FFT_MODE_4K;
      case SILABS_FFT_MODE_8K : return Si2168_DVBT_STATUS_RESPONSE_FFT_MODE_8K;
      case SILABS_FFT_MODE_16K: return Si2168_DVBT2_STATUS_RESPONSE_FFT_MODE_16K;
      case SILABS_FFT_MODE_32K: return Si2168_DVBT2_STATUS_RESPONSE_FFT_MODE_32K;
      default           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_giCode function
  Use:        gi code function
              Used to retrieve the gi value used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  gi, the value used by the top-level application (configurable in CUSTOM_gi_Mode_Enum)
************************************************************************************************************************/
int   Silabs_giCode                       (SILABS_FE_Context* front_end, CUSTOM_GI_Enum                gi)
{
  front_end = front_end; /* to avoid compiler warning if not used */
  gi        = gi;        /* to avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (gi) {
      case SILABS_GUARD_INTERVAL_1_32  : return Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_32;
      case SILABS_GUARD_INTERVAL_1_16  : return Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_16;
      case SILABS_GUARD_INTERVAL_1_8   : return Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_8 ;
      case SILABS_GUARD_INTERVAL_1_4   : return Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_4 ;
      case SILABS_GUARD_INTERVAL_1_128 : return Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_128 ;
      case SILABS_GUARD_INTERVAL_19_128: return Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_19_128 ;
      case SILABS_GUARD_INTERVAL_19_256: return Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_19_256 ;
      default           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_coderateCode function
  Use:        coderate code function
              Used to retrieve the coderate value used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  coderate, the value used by the top-level application (configurable in CUSTOM_Coderate_Enum)
************************************************************************************************************************/
int   Silabs_coderateCode                 (SILABS_FE_Context* front_end, CUSTOM_Coderate_Enum          coderate)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (coderate) {
      case SILABS_CODERATE_1_2 : return Si2168_DVBT_STATUS_RESPONSE_RATE_HP_1_2;
      case SILABS_CODERATE_2_3 : return Si2168_DVBT_STATUS_RESPONSE_RATE_HP_2_3;
      case SILABS_CODERATE_3_4 : return Si2168_DVBT_STATUS_RESPONSE_RATE_HP_3_4;
      case SILABS_CODERATE_5_6 : return Si2168_DVBT_STATUS_RESPONSE_RATE_HP_5_6;
      case SILABS_CODERATE_7_8 : return Si2168_DVBT_STATUS_RESPONSE_RATE_HP_7_8;
      default           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_hierarchyCode function
  Use:        hierarchy code function
              Used to retrieve the hierarchy value used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  hierarchy, the value used by the top-level application (configurable in CUSTOM_Hierarchy_Enum)
************************************************************************************************************************/
int   Silabs_hierarchyCode                (SILABS_FE_Context* front_end, CUSTOM_Hierarchy_Enum         hierarchy)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (hierarchy) {
      case SILABS_HIERARCHY_NONE : return Si2168_DVBT_STATUS_RESPONSE_HIERARCHY_NONE;
      case SILABS_HIERARCHY_ALFA1: return Si2168_DVBT_STATUS_RESPONSE_HIERARCHY_ALFA1;
      case SILABS_HIERARCHY_ALFA2: return Si2168_DVBT_STATUS_RESPONSE_HIERARCHY_ALFA2;
      case SILABS_HIERARCHY_ALFA4: return Si2168_DVBT_STATUS_RESPONSE_HIERARCHY_ALFA4;
      default           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_pilotPatternCode function
  Use:        pilot pattern code function
              Used to retrieve the pilot pattern value used by DVB-T2 demodulators
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  pilot_pattern, the value used by the top-level application (configurable in CUSTOM_Pilot_Pattern_Enum)
************************************************************************************************************************/
int   Silabs_pilotPatternCode             (SILABS_FE_Context* front_end, CUSTOM_Pilot_Pattern_Enum     pilot_pattern)
{
  front_end->chip = front_end->chip; /* To avoid compiler warning */
  pilot_pattern   = pilot_pattern  ; /* To avoid compiler warning */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (pilot_pattern) {
      case SILABS_PILOT_PATTERN_PP1 : return Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP1;
      case SILABS_PILOT_PATTERN_PP2 : return Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP2;
      case SILABS_PILOT_PATTERN_PP3 : return Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP3;
      case SILABS_PILOT_PATTERN_PP4 : return Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP4;
      case SILABS_PILOT_PATTERN_PP5 : return Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP5;
      case SILABS_PILOT_PATTERN_PP6 : return Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP6;
      case SILABS_PILOT_PATTERN_PP7 : return Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP7;
      case SILABS_PILOT_PATTERN_PP8 : return Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP8;
      default           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_videoSysCode function
  Use:        analog video system code function
              Used to retrieve the analog video system value used by the tuner in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  video_sys, the value used by the top-level application (as returned by the tuner)
************************************************************************************************************************/
int   Silabs_videoSysCode                 (SILABS_FE_Context* front_end, CUSTOM_Video_Sys_Enum         video_sys)
{
  front_end = front_end; /* To avoid compiler warning */
  SiTRACE("Silabs_videoSysCode %d\n", video_sys);
#ifdef    Si2173_COMPATIBLE
  switch (video_sys) {
    case SILABS_VIDEO_SYS_B  : return Si2173_ATV_VIDEO_MODE_PROP_VIDEO_SYS_B;
    case SILABS_VIDEO_SYS_GH : return Si2173_ATV_VIDEO_MODE_PROP_VIDEO_SYS_GH;
    case SILABS_VIDEO_SYS_M  : return Si2173_ATV_VIDEO_MODE_PROP_VIDEO_SYS_M;
    case SILABS_VIDEO_SYS_N  : return Si2173_ATV_VIDEO_MODE_PROP_VIDEO_SYS_N;
    case SILABS_VIDEO_SYS_I  : return Si2173_ATV_VIDEO_MODE_PROP_VIDEO_SYS_I;
    case SILABS_VIDEO_SYS_DK : return Si2173_ATV_VIDEO_MODE_PROP_VIDEO_SYS_DK;
    case SILABS_VIDEO_SYS_L  : return Si2173_ATV_VIDEO_MODE_PROP_VIDEO_SYS_L;
    case SILABS_VIDEO_SYS_LP : return Si2173_ATV_VIDEO_MODE_PROP_VIDEO_SYS_LP;
    default           : return -1;
  }
#endif /* Si2173_COMPATIBLE */
#ifdef    Si2176_COMPATIBLE
  switch (video_sys) {
    case SILABS_VIDEO_SYS_B  : return Si2176_ATV_VIDEO_MODE_PROP_VIDEO_SYS_B;
    case SILABS_VIDEO_SYS_GH : return Si2176_ATV_VIDEO_MODE_PROP_VIDEO_SYS_GH;
    case SILABS_VIDEO_SYS_M  : return Si2176_ATV_VIDEO_MODE_PROP_VIDEO_SYS_M;
    case SILABS_VIDEO_SYS_N  : return Si2176_ATV_VIDEO_MODE_PROP_VIDEO_SYS_N;
    case SILABS_VIDEO_SYS_I  : return Si2176_ATV_VIDEO_MODE_PROP_VIDEO_SYS_I;
    case SILABS_VIDEO_SYS_DK : return Si2176_ATV_VIDEO_MODE_PROP_VIDEO_SYS_DK;
    case SILABS_VIDEO_SYS_L  : return Si2176_ATV_VIDEO_MODE_PROP_VIDEO_SYS_L;
    case SILABS_VIDEO_SYS_LP : return Si2176_ATV_VIDEO_MODE_PROP_VIDEO_SYS_LP;
    default           : return -1;
  }
#endif /* Si2176_COMPATIBLE */
#ifdef    Si2178_COMPATIBLE
  switch (video_sys) {
    case SILABS_VIDEO_SYS_B  : return Si2178_ATV_VIDEO_MODE_PROP_VIDEO_SYS_B;
    case SILABS_VIDEO_SYS_GH : return Si2178_ATV_VIDEO_MODE_PROP_VIDEO_SYS_GH;
    case SILABS_VIDEO_SYS_M  : return Si2178_ATV_VIDEO_MODE_PROP_VIDEO_SYS_M;
    case SILABS_VIDEO_SYS_N  : return Si2178_ATV_VIDEO_MODE_PROP_VIDEO_SYS_N;
    case SILABS_VIDEO_SYS_I  : return Si2178_ATV_VIDEO_MODE_PROP_VIDEO_SYS_I;
    case SILABS_VIDEO_SYS_DK : return Si2178_ATV_VIDEO_MODE_PROP_VIDEO_SYS_DK;
    case SILABS_VIDEO_SYS_L  : return Si2178_ATV_VIDEO_MODE_PROP_VIDEO_SYS_L;
    case SILABS_VIDEO_SYS_LP : return Si2178_ATV_VIDEO_MODE_PROP_VIDEO_SYS_LP;
    default           : return -1;
  }
#endif /* Si2178_COMPATIBLE */
#ifdef    Si2196_COMPATIBLE
  switch (video_sys) {
    case SILABS_VIDEO_SYS_B  : return Si2196_ATV_VIDEO_MODE_PROP_VIDEO_SYS_B;
    case SILABS_VIDEO_SYS_GH : return Si2196_ATV_VIDEO_MODE_PROP_VIDEO_SYS_GH;
    case SILABS_VIDEO_SYS_M  : return Si2196_ATV_VIDEO_MODE_PROP_VIDEO_SYS_M;
    case SILABS_VIDEO_SYS_N  : return Si2196_ATV_VIDEO_MODE_PROP_VIDEO_SYS_N;
    case SILABS_VIDEO_SYS_I  : return Si2196_ATV_VIDEO_MODE_PROP_VIDEO_SYS_I;
    case SILABS_VIDEO_SYS_DK : return Si2196_ATV_VIDEO_MODE_PROP_VIDEO_SYS_DK;
    case SILABS_VIDEO_SYS_L  : return Si2196_ATV_VIDEO_MODE_PROP_VIDEO_SYS_L;
    case SILABS_VIDEO_SYS_LP : return Si2196_ATV_VIDEO_MODE_PROP_VIDEO_SYS_LP;
    default           : return -1;
  }
#endif /* Si2196_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_colorCode function
  Use:        analog video color code function
              Used to retrieve the analog video color value used by the tuner in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  color, the value used by the top-level application (as returned by the tuner)
************************************************************************************************************************/
int   Silabs_colorCode                    (SILABS_FE_Context* front_end, CUSTOM_Color_Enum             color)
{
  front_end = front_end; /* To avoid compiler warning */
  SiTRACE("Silabs_colorCode %d\n", color);
#ifdef    Si2173_COMPATIBLE
  switch (color) {
    case SILABS_COLOR_PAL_NTSC : return Si2173_ATV_VIDEO_MODE_PROP_COLOR_PAL_NTSC;
    case SILABS_COLOR_SECAM    : return Si2173_ATV_VIDEO_MODE_PROP_COLOR_SECAM;
    default           : return -1;
  }
#endif /* Si2173_COMPATIBLE */
#ifdef    Si2176_COMPATIBLE
  switch (color) {
    case SILABS_COLOR_PAL_NTSC : return Si2176_ATV_VIDEO_MODE_PROP_COLOR_PAL_NTSC;
    case SILABS_COLOR_SECAM    : return Si2176_ATV_VIDEO_MODE_PROP_COLOR_SECAM;
    default           : return -1;
  }
#endif /* Si2176_COMPATIBLE */
#ifdef    Si2178_COMPATIBLE
  switch (color) {
    case SILABS_COLOR_PAL_NTSC : return Si2178_ATV_VIDEO_MODE_PROP_COLOR_PAL_NTSC;
    case SILABS_COLOR_SECAM    : return Si2178_ATV_VIDEO_MODE_PROP_COLOR_SECAM;
    default           : return -1;
  }
#endif /* Si2178_COMPATIBLE */
#ifdef    Si2196_COMPATIBLE
  switch (color) {
    case SILABS_COLOR_PAL_NTSC : return Si2196_ATV_VIDEO_MODE_PROP_COLOR_PAL_NTSC;
    case SILABS_COLOR_SECAM    : return Si2196_ATV_VIDEO_MODE_PROP_COLOR_SECAM;
    default           : return -1;
  }
#endif /* Si2196_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Silabs_transmissionCode function
  Use:        analog video transmission code function
              Used to retrieve the analog video transmission value used by the tuner in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  trans, the value used by the top-level application (as returned by the tuner)
************************************************************************************************************************/
int   Silabs_transmissionCode             (SILABS_FE_Context* front_end, CUSTOM_Transmission_Mode_Enum trans)
{
  front_end = front_end; /* To avoid compiler warning if not supported */
  trans     = trans;     /* To avoid compiler warning if not supported */
#ifdef    Si2173_COMPATIBLE
  switch (trans) {
    case SILABS_TRANSMISSION_MODE_TERRESTRIAL : return Si2173_ATV_VIDEO_MODE_PROP_TRANS_TERRESTRIAL;
    case SILABS_TRANSMISSION_MODE_CABLE       : return Si2173_ATV_VIDEO_MODE_PROP_TRANS_CABLE;
    default           : return -1;
  }
#endif /* Si2173_COMPATIBLE */
#ifdef    Si2176_COMPATIBLE
  switch (trans) {
    case SILABS_TRANSMISSION_MODE_TERRESTRIAL : return Si2176_ATV_VIDEO_MODE_PROP_TRANS_TERRESTRIAL;
    case SILABS_TRANSMISSION_MODE_CABLE       : return Si2176_ATV_VIDEO_MODE_PROP_TRANS_CABLE;
    default           : return -1;
  }
#endif /* Si2176_COMPATIBLE */
#ifdef    Si2178_COMPATIBLE
  switch (trans) {
 #ifdef   Si2178_ATV_VIDEO_MODE_PROP_TRANS_TERRESTRIAL
    case SILABS_TRANSMISSION_MODE_TERRESTRIAL : return Si2178_ATV_VIDEO_MODE_PROP_TRANS_TERRESTRIAL;
    case SILABS_TRANSMISSION_MODE_CABLE       : return Si2178_ATV_VIDEO_MODE_PROP_TRANS_CABLE;
 #endif /* Si2178_ATV_VIDEO_MODE_PROP_TRANS_TERRESTRIAL */
    default           : return -1;
  }
#endif /* Si2178_COMPATIBLE */
#ifdef    Si2196_COMPATIBLE
  switch (trans) {
    case SILABS_TRANSMISSION_MODE_TERRESTRIAL : return Si2196_ATV_VIDEO_MODE_PROP_TRANS_TERRESTRIAL;
    case SILABS_TRANSMISSION_MODE_CABLE       : return Si2196_ATV_VIDEO_MODE_PROP_TRANS_CABLE;
    default           : return -1;
  }
#endif /* Si2196_COMPATIBLE */
  return -1;
}

/* Translation functions from 'SiLabs' values to 'Custom' values */

/************************************************************************************************************************
  Custom_standardCode function
  Use:        standard code function
              Used to retrieve the standard value used by the DTV demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  standard, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_standardCode                 (SILABS_FE_Context* front_end, int standard)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (standard) {
      case Si2168_DD_MODE_PROP_MODULATION_DVBT : return SILABS_DVB_T ;
      case Si2168_DD_MODE_PROP_MODULATION_DVBT2: return SILABS_DVB_T2;
      case Si2168_DD_MODE_PROP_MODULATION_DVBC : return SILABS_DVB_C ;
      default                                  : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Custom_constelCode function
  Use:        constel code function
              Used to retrieve the constel value  used by the DTV demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  constel, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_constelCode                  (SILABS_FE_Context* front_end, int constel)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (constel) {
      case Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_AUTO  : return SILABS_QAMAUTO ;
      case Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM16 : return SILABS_QAM16   ;
      case Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM32 : return SILABS_QAM32   ;
      case Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM64 : return SILABS_QAM64   ;
      case Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM128: return SILABS_QAM128  ;
      case Si2168_DVBC_CONSTELLATION_PROP_CONSTELLATION_QAM256: return SILABS_QAM256  ;
      default                                                 : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Custom_streamCode function
  Use:        stream code function
              Used to retrieve the stream value used by the DTV demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  stream, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_streamCode                   (SILABS_FE_Context* front_end, int stream)
{
  front_end = front_end; /* To avoid compiler warning if not supported */
  stream    = stream;    /* To avoid compiler warning if not supported */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (stream) {
      case Si2168_DVBT_HIERARCHY_PROP_STREAM_HP: return SILABS_HP;
      case Si2168_DVBT_HIERARCHY_PROP_STREAM_LP: return SILABS_LP;
      default                                  : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Custom_plptypeCode function
  Use:        plp type code function
              Used to retrieve the plp type value used by the DTV demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  plp_type, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_plptypeCode                  (SILABS_FE_Context* front_end, int plp_type)
{
  front_end = front_end; /* To avoid compiler warning if not supported */
  plp_type  = plp_type;    /* To avoid compiler warning if not supported */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (plp_type) {
      case Si2168_DVBT2_PLP_INFO_RESPONSE_PLP_TYPE_COMMON    : return SILABS_PLP_TYPE_COMMON;
      case Si2168_DVBT2_PLP_INFO_RESPONSE_PLP_TYPE_DATA_TYPE1: return SILABS_PLP_TYPE_DATA_TYPE1;
      case Si2168_DVBT2_PLP_INFO_RESPONSE_PLP_TYPE_DATA_TYPE2: return SILABS_PLP_TYPE_DATA_TYPE2;
      default                                  : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Custom_fftCode function
  Use:        fft code function
              Used to retrieve the fft value used by the DTV demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  fft, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_fftCode                      (SILABS_FE_Context* front_end, int fft)
{
  front_end = front_end; /* To avoid compiler warning if not supported */
  fft       = fft;       /* To avoid compiler warning if not supported */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (fft) {
      case Si2168_DVBT_STATUS_RESPONSE_FFT_MODE_2K  : return SILABS_FFT_MODE_2K ;
      case Si2168_DVBT_STATUS_RESPONSE_FFT_MODE_4K  : return SILABS_FFT_MODE_4K ;
      case Si2168_DVBT_STATUS_RESPONSE_FFT_MODE_8K  : return SILABS_FFT_MODE_8K ;
      case Si2168_DVBT2_STATUS_RESPONSE_FFT_MODE_16K: return SILABS_FFT_MODE_16K;
      case Si2168_DVBT2_STATUS_RESPONSE_FFT_MODE_32K: return SILABS_FFT_MODE_32K;
      default                                       : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Custom_giCode function
  Use:        gi code function
              Used to retrieve the gi value used by the DTV demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  gi, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_giCode                       (SILABS_FE_Context* front_end, int gi)
{
  front_end = front_end; /* To avoid compiler warning if not supported */
  gi        = gi;        /* To avoid compiler warning if not supported */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (gi) {
      case Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_32  : return SILABS_GUARD_INTERVAL_1_32  ;
      case Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_16  : return SILABS_GUARD_INTERVAL_1_16  ;
      case Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_8   : return SILABS_GUARD_INTERVAL_1_8   ;
      case Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_4   : return SILABS_GUARD_INTERVAL_1_4   ;
      case Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_1_128 : return SILABS_GUARD_INTERVAL_1_128 ;
      case Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_19_128: return SILABS_GUARD_INTERVAL_19_128;
      case Si2168_DVBT2_STATUS_RESPONSE_GUARD_INT_19_256: return SILABS_GUARD_INTERVAL_19_256;
      default                                           : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Custom_coderateCode function
  Use:        coderate code function
              Used to retrieve the coderate value used by the DTV demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  coderate, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_coderateCode                 (SILABS_FE_Context* front_end, int coderate)
{
  front_end = front_end; /* To avoid compiler warning if not supported */
  coderate  = coderate;  /* To avoid compiler warning if not supported */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (coderate) {
      case Si2168_DVBT_STATUS_RESPONSE_RATE_HP_1_2    : return SILABS_CODERATE_1_2;
      case Si2168_DVBT_STATUS_RESPONSE_RATE_HP_2_3    : return SILABS_CODERATE_2_3;
      case Si2168_DVBT_STATUS_RESPONSE_RATE_HP_3_4    : return SILABS_CODERATE_3_4;
      case Si2168_DVBT_STATUS_RESPONSE_RATE_HP_5_6    : return SILABS_CODERATE_5_6;
      case Si2168_DVBT_STATUS_RESPONSE_RATE_HP_7_8    : return SILABS_CODERATE_7_8;
      default                                         : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Custom_hierarchyCode function
  Use:        hierarchy code function
              Used to retrieve the hierarchy value used by the DTV demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  hierarchy, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_hierarchyCode                (SILABS_FE_Context* front_end, int hierarchy)
{
  front_end = front_end; /* To avoid compiler warning if not supported */
  hierarchy = hierarchy; /* To avoid compiler warning if not supported */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (hierarchy) {
      case Si2168_DVBT_STATUS_RESPONSE_HIERARCHY_NONE : return SILABS_HIERARCHY_NONE;
      case Si2168_DVBT_STATUS_RESPONSE_HIERARCHY_ALFA1: return SILABS_HIERARCHY_ALFA1;
      case Si2168_DVBT_STATUS_RESPONSE_HIERARCHY_ALFA2: return SILABS_HIERARCHY_ALFA2;
      case Si2168_DVBT_STATUS_RESPONSE_HIERARCHY_ALFA4: return SILABS_HIERARCHY_ALFA4;
      default                                         : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  Custom_pilotPatternCode function
  Use:        pilot pattern code function
              Used to retrieve the pilot pattern value used by the DVB-T2 demodulator in custom format
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  pilot_pattern, the value used by the top-level application (as returned by the demod)
************************************************************************************************************************/
int   Custom_pilotPatternCode             (SILABS_FE_Context* front_end, int pilot_pattern)
{
  front_end->chip = front_end->chip; /* To avoid compiler warning if not supported */
  pilot_pattern   = pilot_pattern  ; /* To avoid compiler warning if not supported */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (pilot_pattern) {
      case Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP1: return SILABS_PILOT_PATTERN_PP1;
      case Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP2: return SILABS_PILOT_PATTERN_PP2;
      case Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP3: return SILABS_PILOT_PATTERN_PP3;
      case Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP4: return SILABS_PILOT_PATTERN_PP4;
      case Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP5: return SILABS_PILOT_PATTERN_PP5;
      case Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP6: return SILABS_PILOT_PATTERN_PP6;
      case Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP7: return SILABS_PILOT_PATTERN_PP7;
      case Si2168_DVBT2_STATUS_RESPONSE_PILOT_PATTERN_PP8: return SILABS_PILOT_PATTERN_PP8;
      default                                            : return -1;
    }
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}

/* Text functions returning strings based on 'Custom' values. */

/************************************************************************************************************************
  Silabs_Standard_Text function
  Use:        standard text retrieval function
              Used to retrieve the standard text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  standard, the value used by the top-level application (configurable in CUSTOM_Standard_Enum)
************************************************************************************************************************/
char *Silabs_Standard_Text                (CUSTOM_Standard_Enum  standard)
{
  switch (standard) {
    case SILABS_ANALOG: {return (char *)"ANALOG" ;}
    case SILABS_DVB_T : {return (char *)"DVB-T"  ;}
    case SILABS_DVB_T2: {return (char *)"DVB-T2" ;}
    case SILABS_DVB_C : {return (char *)"DVB-C"  ;}
    case SILABS_DVB_C2: {return (char *)"DVB-C2" ;}
    case SILABS_MCNS  : {return (char *)"MCNS"   ;}
    case SILABS_DVB_S : {return (char *)"DVB-S"  ;}
    case SILABS_DVB_S2: {return (char *)"DVB-S2" ;}
    case SILABS_DSS   : {return (char *)"DSS"    ;}
    case SILABS_SLEEP : {return (char *)"SLEEP"  ;}
    default           : {return (char *)"UNKNOWN";}
  }
}
/************************************************************************************************************************
  Silabs_Standard_Capability function
  Use:        standard capability retrieval function
              Used to know whether the front-end can handle a given standard
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  standard, the value used by the top-level application (configurable in CUSTOM_Standard_Enum)
  Returns  :  1 if the front-end can demodulate the standard
************************************************************************************************************************/
int   Silabs_Standard_Capability          (CUSTOM_Standard_Enum  standard)
{
  switch (standard) {
    case SILABS_ANALOG: {return 1;}
    case SILABS_DVB_T : {return 1;}
    case SILABS_DVB_T2: {return 1;}
    case SILABS_DVB_C : {return 1;}
    default           : {return 0;}
  }
  return 0;
}
/************************************************************************************************************************
  Silabs_Constel_Text function
  Use:        constel text retrieval function
              Used to retrieve the constel text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  constel, the value used by the top-level application (configurable in CUSTOM_Constel_Enum)
************************************************************************************************************************/
char *Silabs_Constel_Text                 (CUSTOM_Constel_Enum   constel)
{
  switch (constel) {
    case SILABS_QAMAUTO : { return (char *)"QAMAUTO"; break;}
    case SILABS_QAM16   : { return (char *)"QAM16"  ; break;}
    case SILABS_QAM32   : { return (char *)"QAM32"  ; break;}
    case SILABS_QAM64   : { return (char *)"QAM64"  ; break;}
    case SILABS_QAM128  : { return (char *)"QAM128" ; break;}
    case SILABS_QAM256  : { return (char *)"QAM256" ; break;}
    case SILABS_QPSK    : { return (char *)"QPSK"   ; break;}
    case SILABS_8PSK    : { return (char *)"8PSK"   ; break;}
    default             : { return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_Polarization_Text function
  Use:        polarization text retrieval function
              Used to retrieve the polarization text used by the front-end
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  polarization, the value used by the top-level application (configurable in CUSTOM_Polarization_Enum)
************************************************************************************************************************/
char *Silabs_Polarization_Text            (CUSTOM_Polarization_Enum   polarization)
{
  switch (polarization) {
    case SILABS_POLARIZATION_HORIZONTAL : { return (char *)"Horizontal"; break;}
    case SILABS_POLARIZATION_VERTICAL   : { return (char *)"Vertical"  ; break;}
    default             : { return (char *)"UNKNOWN"   ; break;}
  }
}
/************************************************************************************************************************
  Silabs_Band_Text function
  Use:        polarization text retrieval function
              Used to retrieve the polarization text used by the front-end
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  polarization, the value used by the top-level application (configurable in CUSTOM_Band_Enum)
************************************************************************************************************************/
char *Silabs_Band_Text                    (CUSTOM_Band_Enum      band)
{
  switch (band) {
    case SILABS_BAND_LOW : { return (char *)"Low "   ; break;}
    case SILABS_BAND_HIGH: { return (char *)"High"   ; break;}
    default              : { return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_Stream_Text function
  Use:        stream text retrieval function
              Used to retrieve the stream text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  stream, the value used by the top-level application (configurable in CUSTOM_Stream_Enum)
************************************************************************************************************************/
char *Silabs_Stream_Text                  (CUSTOM_Stream_Enum    stream)
{
  switch (stream) {
    case SILABS_HP    : { return (char *)"HP"     ; break;}
    case SILABS_LP    : { return (char *)"LP"     ; break;}
    default           : { return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_PLPType_Text function
  Use:        plp type text retrieval function
              Used to retrieve the plp type text string
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  plp_type, the value used by the top-level application (configurable in CUSTOM_T2_PLP_TYPE)
************************************************************************************************************************/
char *Silabs_PLPType_Text                 (CUSTOM_T2_PLP_TYPE    plp_type)
{
  switch (plp_type) {
    case SILABS_PLP_TYPE_COMMON     : { return (char *)"COMMON"     ; break;}
    case SILABS_PLP_TYPE_DATA_TYPE1 : { return (char *)"DATA_TYPE1" ; break;}
    case SILABS_PLP_TYPE_DATA_TYPE2 : { return (char *)"DATA_TYPE2" ; break;}
    default           : { return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_FFT_Text function
  Use:        fft text retrieval function
              Used to retrieve the fft text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  fft, the value used by the top-level application (configurable in CUSTOM_FFT_Mode_Enum)
************************************************************************************************************************/
char *Silabs_FFT_Text                     (CUSTOM_FFT_Mode_Enum  fft)
{
  switch (fft) {
    case SILABS_FFT_MODE_2K   : { return (char *)" 2K"    ; break;}
    case SILABS_FFT_MODE_4K   : { return (char *)" 4K"    ; break;}
    case SILABS_FFT_MODE_8K   : { return (char *)" 8K"    ; break;}
    case SILABS_FFT_MODE_16K  : { return (char *)"16K"    ; break;}
    case SILABS_FFT_MODE_32K  : { return (char *)"32K"    ; break;}
    default                   : { SiTRACE("UNKNOWN FFT mode value %d\n", fft); return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_GI_Text function
  Use:        gi text retrieval function
              Used to retrieve the gi text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  gi, the value used by the top-level application (configurable in CUSTOM_GI_Enum)
************************************************************************************************************************/
char *Silabs_GI_Text                      (CUSTOM_GI_Enum        gi)
{
  switch (gi) {
    case SILABS_GUARD_INTERVAL_1_32     : { return (char *)"1/32"    ; break;}
    case SILABS_GUARD_INTERVAL_1_16     : { return (char *)"1/16"    ; break;}
    case SILABS_GUARD_INTERVAL_1_8      : { return (char *)"1/8"     ; break;}
    case SILABS_GUARD_INTERVAL_1_4      : { return (char *)"1/4"     ; break;}
    case SILABS_GUARD_INTERVAL_1_128    : { return (char *)"1/128"   ; break;}
    case SILABS_GUARD_INTERVAL_19_128   : { return (char *)"19/128"  ; break;}
    case SILABS_GUARD_INTERVAL_19_256   : { return (char *)"19/256"  ; break;}
    default                             : { SiTRACE("UNKNOWN GI value %d\n", gi); return (char *)"UNKNOWN" ; break;}
  }
}
/************************************************************************************************************************
  Silabs_Coderate_Text function
  Use:        coderate text retrieval function
              Used to retrieve the coderate text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  coderate, the value used by the top-level application (configurable in CUSTOM_Coderate_Enum)
************************************************************************************************************************/
char *Silabs_Coderate_Text                (CUSTOM_Coderate_Enum  coderate)
{
  switch (coderate) {
    case SILABS_CODERATE_1_2    : { return (char *)"1/2"    ; break;}
    case SILABS_CODERATE_2_3    : { return (char *)"2/3"    ; break;}
    case SILABS_CODERATE_3_4    : { return (char *)"3/4"    ; break;}
    case SILABS_CODERATE_4_5    : { return (char *)"4/5"    ; break;}
    case SILABS_CODERATE_5_6    : { return (char *)"5/6"    ; break;}
    case SILABS_CODERATE_7_8    : { return (char *)"7/8"    ; break;}
    case SILABS_CODERATE_8_9    : { return (char *)"8/9"    ; break;}
    case SILABS_CODERATE_9_10   : { return (char *)"9/10"   ; break;}
    case SILABS_CODERATE_1_3    : { return (char *)"1/3"    ; break;}
    case SILABS_CODERATE_1_4    : { return (char *)"1/4"    ; break;}
    case SILABS_CODERATE_2_5    : { return (char *)"2/5"    ; break;}
    case SILABS_CODERATE_3_5    : { return (char *)"3/5"    ; break;}
    default                     : { SiTRACE("UNKNOWN code rate value %d\n", coderate); return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_Extended_BW_Text function
  Use:        bw_ext text retrieval function
              Used to retrieve the bw_ext text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  bw_extended, value used by the top-level application (configurable in CUSTOM_T2_BwExtended_Enum)
************************************************************************************************************************/
char *Silabs_Extended_BW_Text             (int bw_extended)
{
  switch (bw_extended) {
    case 0  : { return (char *)"normal  " ; break;}
    case 1  : { return (char *)"extended" ; break;}
    default : { SiTRACE("UNKNOWN bw_extended value %d\n",bw_extended); return (char *)"UNKNOWN" ; break;}
  }
}
/************************************************************************************************************************
  Silabs_Rotated_QAM_Text function
  Use:        rotated text retrieval function
              Used to retrieve the rotated text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  rotated, a 0/1 flag indicating the QAM rotation (0 = 'normal', 1 = 'rotated')
************************************************************************************************************************/
char *Silabs_Rotated_QAM_Text             (int rotated)
{
  switch (rotated) {
    case 0  : { return (char *)"normal "     ; break;}
    case 1  : { return (char *)"rotated"     ; break;}
    default : { SiTRACE("UNKNOWN QAM rotation value %d\n",rotated); return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_FEF_Text function
  Use:        fef text retrieval function
              Used to retrieve the fef text used by the demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  fef, a 0/1 flag indicating the fef presence (0 = 'no fef', 1 = 'fef')
************************************************************************************************************************/
char *Silabs_FEF_Text                     (int fef)
{
  switch (fef) {
    case 0   : { return (char *)"  no"   ; break;}
    case 1   : { return (char *)"with"   ; break;}
    default  : { SiTRACE("UNKNOWN FEF value %d\n",fef); return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_MISO_Text function
  Use:        tx text retrieval function
              Used to retrieve the tx text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  tx, the value used by the top-level application (1 for MISO, 0 otherwise)
************************************************************************************************************************/
char *Silabs_MISO_Text                    (int siso_miso)
{
  switch (siso_miso) {
    case 0   : { return (char *)"SISO"  ; break;}
    case 1   : { return (char *)"MISO"  ; break;}
    default  : { SiTRACE("UNKNOWN tx_mode %d\n", siso_miso); return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_Pilot_Pattern_Text function
  Use:        pilot_pattern text retrieval function
              Used to retrieve the pilot_pattern text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  pilot_pattern, the value used by the top-level application (configurable in CUSTOM_Pilot_Pattern_Enum)
************************************************************************************************************************/
char *Silabs_Pilot_Pattern_Text           (CUSTOM_Pilot_Pattern_Enum pilot_pattern)
{
  switch (pilot_pattern) {
    case SILABS_PILOT_PATTERN_PP1   : { return (char *)"PP1"  ; break;}
    case SILABS_PILOT_PATTERN_PP2   : { return (char *)"PP2"  ; break;}
    case SILABS_PILOT_PATTERN_PP3   : { return (char *)"PP3"  ; break;}
    case SILABS_PILOT_PATTERN_PP4   : { return (char *)"PP4"  ; break;}
    case SILABS_PILOT_PATTERN_PP5   : { return (char *)"PP5"  ; break;}
    case SILABS_PILOT_PATTERN_PP6   : { return (char *)"PP6"  ; break;}
    case SILABS_PILOT_PATTERN_PP7   : { return (char *)"PP7"  ; break;}
    case SILABS_PILOT_PATTERN_PP8   : { return (char *)"PP8"  ; break;}
    default                         : { SiTRACE("UNKNOWN Pilot Pattern %d\n", pilot_pattern); return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_No_Short_Frame_Text function
  Use:        frame text retrieval function
              Used to retrieve the frame text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  no_short_frame, the value used by the top-level application (0 for 'short frame', 1 for 'normal frame')
************************************************************************************************************************/
char *Silabs_No_Short_Frame_Text          (int no_short_frame)
{
  switch (no_short_frame) {
    case 0   : { return (char *)"short  "; break;}
    case 1   : { return (char *)"normal "; break;}
    default  : { SiTRACE("UNKNOWN short frame value %d\n", no_short_frame); return (char *)"UNKNOWN"; break;}
  }
}
/************************************************************************************************************************
  Silabs_Pilots_Text function
  Use:        frame text retrieval function
              Used to retrieve the frame text used by the DTV demodulator
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  pilots, the value used by the top-level application (0 for 'off', 1 for 'on')
************************************************************************************************************************/
char *Silabs_Pilots_Text                  (int pilots)
{
  switch (pilots) {
    case 0   : { return (char *)"OFF"    ; break;}
    case 1   : { return (char *)"ON "    ; break;}
    default  : { SiTRACE("UNKNOWN pilots flag %d\n",pilots); return (char *)"UNKNOWN"; break;}
  }
}

/* Chip detection function (To Be Defined) */

/************************************************************************************************************************
  SiLabs_chip_detect function
  Use:        chip detection function
              Used to detect whether the demodulator is a DTV demodulator
  Behavior:   This function uses raw i2c reads to check the presence of either a Si2167 or a Si2168
  Parameter:  demodAdd, the I2C address of the demod
  Returns:    2167 if there is a Si2167, 2168 if there is a 2168, 0 otherwise
************************************************************************************************************************/
int   SiLabs_chip_detect                  (int demodAdd)
{
  int chip;
  chip = 0;
  SiTRACE("Detecting chip at address 0x%02x\n", demodAdd);
  demodAdd = demodAdd;
#ifdef    Si2168_COMPATIBLE
/* TODO (mdorval#2#): Find a way to detect the presence of a Si2168 */
  chip = 2168;
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Chip  %d   (%X)\n", chip, chip);
  return chip;
}

/************************************************************************************************************************
  SiLabs_API_Demod_status function
  Use:        stream code function
              Used to retrieve the status of the demod in a structure
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  status, a pointer to the status structure (configurable in CUSTOM_Status_Struct)
  Return:     1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Demod_status             (SILABS_FE_Context* front_end, CUSTOM_Status_Struct *status)
{
  SiTRACE("SiLabs_API_Demod_status %d\n", front_end->chip);
  /* Set to 0 all info used to relock */
  status->demod_lock         =  0;
  status->fec_lock           =  0;
  status->bandwidth_Hz       =  0;
  status->symbol_rate        =  0;
  status->stream             =  0;
  status->constellation      =  0;
  status->c_n                =  0;
  status->freq_offset        =  0;
  status->timing_offset      =  0;
  status->code_rate          = -1;
  status->ber                = -1; /* Set to '-1' to signal unavailability if not set later on */
  status->per                = -1; /* Set to '-1' to signal unavailability if not set later on */
  status->fer                = -1; /* Set to '-1' to signal unavailability if not set later on */
  status->uncorrs            = -1; /* Set to '-1' to signal unavailability if not set later on */
  status->num_plp            = -1; /* Set to '-1' to allow 'auto' plp mode selection           */
  status->plp_id             =  0;
  status->RSSI               =  0;
  status->SSI                =  0;
  status->SQI                =  0;
  status->TS_bitrate_kHz     =  0;
  status->TS_clock_kHz       =  0;

  if (front_end->standard == SILABS_SLEEP) {status->standard = SILABS_SLEEP; return 0;}

#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    if (Si2168_L1_DD_STATUS (front_end->Si2168_FE->demod, Si2168_DD_STATUS_CMD_INTACK_OK) != NO_Si2168_ERROR) {
      SiERROR("Si2168_L1_DD_STATUS ERROR\n");
      return 0;
    }
    status->standard = Custom_standardCode(front_end, front_end->Si2168_FE->demod->rsp->dd_status.modulation);
    /* Retrieving TS  values */
    status->TS_bitrate_kHz  = front_end->Si2168_FE->demod->rsp->dd_status.ts_bit_rate*10;
    status->TS_clock_kHz    = front_end->Si2168_FE->demod->rsp->dd_status.ts_clk_freq*10;
    /* Retrieving AGC values */
    switch (status->standard) {
      case SILABS_DVB_T :
      case SILABS_DVB_C :
      case SILABS_DVB_T2:
      {
        front_end->Si2168_FE->demod->cmd->dd_ext_agc_ter.agc_1_mode  = Si2168_DD_EXT_AGC_TER_CMD_AGC_1_MODE_NO_CHANGE;
        front_end->Si2168_FE->demod->cmd->dd_ext_agc_ter.agc_2_mode  = Si2168_DD_EXT_AGC_TER_CMD_AGC_2_MODE_NO_CHANGE;
        SiTRACE("Si2168_L1_SendCommand2(front_end->Si2168_FE->demod, Si2168_DD_EXT_AGC_TER_CMD_CODE)\n");
        Si2168_L1_SendCommand2(front_end->Si2168_FE->demod, Si2168_DD_EXT_AGC_TER_CMD_CODE);
        status->RFagc              = 0;
        status->IFagc              = front_end->Si2168_FE->demod->rsp->dd_ext_agc_ter.agc_2_level;
        break;
      }
      default           : {
        status->RFagc              = 0;
        status->IFagc              = 0;
        break;
      }
    }
    switch (status->standard) {
      case SILABS_ANALOG: {
        return 1;
        break;
      }
      case SILABS_DVB_T :
      case SILABS_DVB_C :
      {
        front_end->Si2168_FE->demod->tuner_rssi = status->rssi;
        if ( Si2168_L1_DD_BER    (front_end->Si2168_FE->demod, Si2168_DD_BER_CMD_RST_RUN  ) != NO_Si2168_ERROR ) return 0;
        /* Mimick Si2167 clock_mode register values */
        status->clock_mode =  33;
        /* CHECK the exponent value to know if the BER is available or not */
        if(front_end->Si2168_FE->demod->rsp->dd_ber.exp!=0) {
          status->ber                = (front_end->Si2168_FE->demod->rsp->dd_ber.mant/10) / power_of_n(10, front_end->Si2168_FE->demod->rsp->dd_ber.exp);
        }
        if ( Si2168_L1_DD_PER    (front_end->Si2168_FE->demod, Si2168_DD_PER_CMD_RST_RUN  ) != NO_Si2168_ERROR ) return 0;
        /* CHECK the exponent value to know if the PER is available or not */
        if (front_end->Si2168_FE->demod->rsp->dd_per.exp!=0) {
          status->per                = (front_end->Si2168_FE->demod->rsp->dd_per.mant/10) / power_of_n(10, front_end->Si2168_FE->demod->rsp->dd_per.exp);
        }
        if ( Si2168_L1_DD_UNCOR  (front_end->Si2168_FE->demod, Si2168_DD_UNCOR_CMD_RST_RUN) != NO_Si2168_ERROR ) return 0;
        status->uncorrs            = (front_end->Si2168_FE->demod->rsp->dd_uncor.uncor_msb<<16) + front_end->Si2168_FE->demod->rsp->dd_uncor.uncor_lsb;
        break;
      }
      case SILABS_DVB_T2:
       {
        /* Mimick Si2167 clock_mode register values */
        switch (front_end->Si2168_FE->demod->cmd->start_clk.clk_mode) {
          case Si2168_START_CLK_CMD_CLK_MODE_CLK_CLKIO   : status->clock_mode =  32; break;
          case Si2168_START_CLK_CMD_CLK_MODE_CLK_XTAL_IN : status->clock_mode =  34; break;
          case Si2168_START_CLK_CMD_CLK_MODE_XTAL        : status->clock_mode =  33; break;
          default                                        : status->clock_mode =   0; break;
        }
        if ( Si2168_L1_DD_BER    (front_end->Si2168_FE->demod, Si2168_DD_BER_CMD_RST_RUN  ) != NO_Si2168_ERROR ) return 0;
        /* CHECK the exponent value to know if the BER is available or not*/
        if(front_end->Si2168_FE->demod->rsp->dd_ber.exp!=0) {
          status->ber                = (front_end->Si2168_FE->demod->rsp->dd_ber.mant/10) / power_of_n(10, front_end->Si2168_FE->demod->rsp->dd_ber.exp);
        }
        if ( Si2168_L1_DD_FER    (front_end->Si2168_FE->demod, Si2168_DD_FER_CMD_RST_RUN  ) != NO_Si2168_ERROR ) return 0;
        /* CHECK the exponent value to know if the FER is available or not*/
        if(front_end->Si2168_FE->demod->rsp->dd_fer.exp!=0) {
          status->fer                = (front_end->Si2168_FE->demod->rsp->dd_fer.mant/10) / power_of_n(10, front_end->Si2168_FE->demod->rsp->dd_fer.exp);
        }
        if ( Si2168_L1_DD_PER    (front_end->Si2168_FE->demod, Si2168_DD_PER_CMD_RST_RUN  ) != NO_Si2168_ERROR ) return 0;
        /* CHECK the exponent value to know if the PER is available or not*/
        if(front_end->Si2168_FE->demod->rsp->dd_per.exp!=0) {
          status->per                = (front_end->Si2168_FE->demod->rsp->dd_per.mant/10) / power_of_n(10, front_end->Si2168_FE->demod->rsp->dd_per.exp);
        }
        if ( Si2168_L1_DD_UNCOR  (front_end->Si2168_FE->demod, Si2168_DD_UNCOR_CMD_RST_RUN) != NO_Si2168_ERROR ) return 0;
        status->uncorrs            = (front_end->Si2168_FE->demod->rsp->dd_uncor.uncor_msb<<16) + front_end->Si2168_FE->demod->rsp->dd_uncor.uncor_lsb;
        break;
      }
      default           : { return 0; break; }
    }
    switch (status->standard) {
      case SILABS_DVB_T : {
        if (Si2168_L1_DVBT_STATUS    (front_end->Si2168_FE->demod, Si2168_DVBT_STATUS_CMD_INTACK_OK)                        != NO_Si2168_ERROR) return 0;
        if (Si2168_L1_DVBT_TPS_EXTRA (front_end->Si2168_FE->demod)                                                          != NO_Si2168_ERROR) return 0;
#ifdef    TUNERTER_API
    //    status->RSSI               = front_end->Si2168_FE->tuner_ter->rsp->tuner_status.rssi;
#else  /* TUNERTER_API */
    //    status->RSSI               = front_end->Si2168_FE->tuner_ter->rssi;
#endif /* TUNERTER_API */
        if (Si2168_L1_DD_SSI_SQI     (front_end->Si2168_FE->demod, status->RSSI) != NO_Si2168_ERROR) return 0;
        status->demod_lock         = front_end->Si2168_FE->demod->rsp->dvbt_status.pcl;
        status->fec_lock           = front_end->Si2168_FE->demod->rsp->dvbt_status.dl;
        status->spectral_inversion = front_end->Si2168_FE->demod->rsp->dvbt_status.sp_inv;
        status->c_n                = front_end->Si2168_FE->demod->rsp->dvbt_status.cnr/4;
        status->freq_offset        = front_end->Si2168_FE->demod->rsp->dvbt_status.afc_freq;
        status->timing_offset      = front_end->Si2168_FE->demod->rsp->dvbt_status.timing_offset;
        status->bandwidth_Hz       = front_end->Si2168_FE->demod->prop->dd_mode.bw*1000000;
        status->stream             = Custom_streamCode   (front_end, front_end->Si2168_FE->demod->prop->dvbt_hierarchy.stream);
        status->fft_mode           = Custom_fftCode      (front_end, front_end->Si2168_FE->demod->rsp->dvbt_status.fft_mode);
        status->guard_interval     = Custom_giCode       (front_end, front_end->Si2168_FE->demod->rsp->dvbt_status.guard_int);
        status->constellation      = Custom_constelCode  (front_end, front_end->Si2168_FE->demod->rsp->dvbt_status.constellation);
        status->hierarchy          = Custom_hierarchyCode(front_end, front_end->Si2168_FE->demod->rsp->dvbt_status.hierarchy);
        status->code_rate_hp       = Custom_coderateCode (front_end, front_end->Si2168_FE->demod->rsp->dvbt_status.rate_hp);
        status->code_rate_lp       = Custom_coderateCode (front_end, front_end->Si2168_FE->demod->rsp->dvbt_status.rate_lp);
        status->symbol_rate        = 0;
        status->SSI                = front_end->Si2168_FE->demod->rsp->dd_ssi_sqi.ssi;
        status->SQI                = front_end->Si2168_FE->demod->rsp->dd_ssi_sqi.sqi;
        break;
      }
      case SILABS_DVB_T2 : {
        if (Si2168_L1_DVBT2_STATUS   (front_end->Si2168_FE->demod, Si2168_DVBT2_STATUS_CMD_INTACK_OK)                       != NO_Si2168_ERROR) return 0;
        status->demod_lock         = front_end->Si2168_FE->demod->rsp->dvbt2_status.pcl;
        status->fec_lock           = front_end->Si2168_FE->demod->rsp->dvbt2_status.dl;
        status->spectral_inversion = front_end->Si2168_FE->demod->rsp->dvbt2_status.sp_inv;
        status->c_n                = front_end->Si2168_FE->demod->rsp->dvbt2_status.cnr/4;
        status->freq_offset        = front_end->Si2168_FE->demod->rsp->dvbt2_status.afc_freq;
        status->timing_offset      = front_end->Si2168_FE->demod->rsp->dvbt2_status.timing_offset;
        status->bandwidth_Hz       = front_end->Si2168_FE->demod->prop->dd_mode.bw*1000000;
        status->stream             = Custom_streamCode      (front_end, 0);
        status->fft_mode           = Custom_fftCode         (front_end, front_end->Si2168_FE->demod->rsp->dvbt2_status.fft_mode);
        status->guard_interval     = Custom_giCode          (front_end, front_end->Si2168_FE->demod->rsp->dvbt2_status.guard_int);
        status->constellation      = Custom_constelCode     (front_end, front_end->Si2168_FE->demod->rsp->dvbt2_status.constellation);
        status->code_rate          = Custom_coderateCode    (front_end, front_end->Si2168_FE->demod->rsp->dvbt2_status.code_rate);
        status->num_plp            = front_end->Si2168_FE->demod->rsp->dvbt2_status.num_plp;
        status->rotated            = front_end->Si2168_FE->demod->rsp->dvbt2_status.rotated;
        status->pilot_pattern      = Custom_pilotPatternCode(front_end, front_end->Si2168_FE->demod->rsp->dvbt2_status.pilot_pattern);
        status->bw_ext             = front_end->Si2168_FE->demod->rsp->dvbt2_status.bw_ext;
        status->num_plp            = front_end->Si2168_FE->demod->rsp->dvbt2_status.num_plp;
        status->plp_id             = front_end->Si2168_FE->demod->rsp->dvbt2_status.plp_id;
        status->tx_mode            = front_end->Si2168_FE->demod->rsp->dvbt2_status.tx_mode;
        status->short_frame        = front_end->Si2168_FE->demod->rsp->dvbt2_status.short_frame;
        status->fef                = front_end->Si2168_FE->demod->rsp->dvbt2_status.fef;
        if (Si2168_L1_DVBT2_TX_ID   (front_end->Si2168_FE->demod)                       != NO_Si2168_ERROR) return 0;
        status->cell_id            = front_end->Si2168_FE->demod->rsp->dvbt2_tx_id.cell_id;
#ifdef    TUNERTER_API
    //    status->RSSI               = front_end->Si2168_FE->tuner_ter->rsp->tuner_status.rssi;
#else  /* TUNERTER_API */
    //    status->RSSI               = front_end->Si2168_FE->tuner_ter->rssi;
#endif /* TUNERTER_API */
        if (Si2168_L1_DD_SSI_SQI     (front_end->Si2168_FE->demod, status->RSSI) != NO_Si2168_ERROR) return 0;
        status->SSI                = front_end->Si2168_FE->demod->rsp->dd_ssi_sqi.ssi;
        status->SQI                = front_end->Si2168_FE->demod->rsp->dd_ssi_sqi.sqi;
        status->symbol_rate        = 0;
        break;
      }
      case SILABS_DVB_C : {
        if (Si2168_L1_DVBC_STATUS    (front_end->Si2168_FE->demod, Si2168_DVBC_STATUS_CMD_INTACK_OK) != NO_Si2168_ERROR) return 0;
        status->demod_lock         = front_end->Si2168_FE->demod->rsp->dvbc_status.pcl;
        status->fec_lock           = front_end->Si2168_FE->demod->rsp->dvbc_status.dl;
        status->symbol_rate        = front_end->Si2168_FE->demod->prop->dvbc_symbol_rate.rate*1000;
        status->constellation      = Custom_constelCode (front_end, front_end->Si2168_FE->demod->rsp->dvbc_status.constellation);
        status->spectral_inversion = front_end->Si2168_FE->demod->rsp->dvbc_status.sp_inv;
        status->c_n                = front_end->Si2168_FE->demod->rsp->dvbc_status.cnr/4;
        status->freq_offset        = front_end->Si2168_FE->demod->rsp->dvbc_status.afc_freq;
        status->timing_offset      = front_end->Si2168_FE->demod->rsp->dvbc_status.timing_offset;
#ifdef    TUNERTER_API
   //     status->RSSI               = front_end->Si2168_FE->tuner_ter->rsp->tuner_status.rssi;
#else  /* TUNERTER_API */
   //     status->RSSI               = front_end->Si2168_FE->tuner_ter->rssi;
#endif /* TUNERTER_API */
        status->SSI                = 0;
        status->SQI                = 0;
        break;
      }
      default           : {
        return 0;
        break;
      }
    }
  }
#endif /* Si2168_COMPATIBLE */
  return 0;
}
/************************************************************************************************************************
  SiLabs_API_TER_Tuner_status function
  Use:        Terrestrial tuner status function
              Used to retrieve the status of the TER tuner in a structure
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  status, a pointer to the status structure (configurable in CUSTOM_Status_Struct)
  Return:     1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_TER_Tuner_status         (SILABS_FE_Context* front_end, CUSTOM_Status_Struct *status)
{
  SiLabs_API_Tuner_I2C_Enable  (front_end);
#ifdef    TER_TUNER_CUSTOMTER
  CUSTOMTER_Context *tuner_ter;
#endif /* TER_TUNER_CUSTOMTER */
#ifdef    TER_TUNER_DTT759x
  DTT759x_Context *tuner_ter;
#endif /* TER_TUNER_DTT759x */
#ifdef    TER_TUNER_Si2146
  L1_Si2146_Context *tuner_ter;
#endif /* TER_TUNER_Si2146 */
#ifdef    TER_TUNER_Si2148
  L1_Si2148_Context *tuner_ter;
#endif /* TER_TUNER_Si2148 */
#ifdef    TER_TUNER_Si2156
  L1_Si2156_Context *tuner_ter;
#endif /* TER_TUNER_Si2156 */
#ifdef    TER_TUNER_Si2158
  L1_Si2158_Context *tuner_ter;
#endif /* TER_TUNER_Si2158 */
#ifdef    TER_TUNER_Si2173
  L1_Si2173_Context *tuner_ter;
#endif /* TER_TUNER_Si2173 */
#ifdef    TER_TUNER_Si2176
  L1_Si2176_Context *tuner_ter;
#endif /* TER_TUNER_Si2176 */
#ifdef    TER_TUNER_Si2178
  L1_Si2178_Context *tuner_ter;
#endif /* TER_TUNER_Si2178 */
#ifdef    TER_TUNER_Si2185
  L1_Si2185_Context *tuner_ter;
#endif /* TER_TUNER_Si2185 */
#ifdef    TER_TUNER_Si2190
  L1_Si2190_Context *tuner_ter;
#endif /* TER_TUNER_Si2190 */
#ifdef    TER_TUNER_Si2196
  L1_Si2196_Context *tuner_ter;
#endif /* TER_TUNER_Si2196 */
#ifdef    TER_TUNER_NO_TER
  TER_TUNER_CONTEXT *tuner_ter;
#endif /* TER_TUNER_NO_TER */
  tuner_ter = NULL;
  SiTRACE("SiLabs_API_TER_Tuner_status\n");
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { tuner_ter = front_end->Si2168_FE->tuner_ter; }
#endif /* Si2168_COMPATIBLE */
  if (tuner_ter == NULL) {
      SiTRACE("No tuner_ter defined, SiLabs_API_TER_Tuner_status can't be executed!\n");
    return 0;
  }
#ifdef    TER_TUNER_DTT759x
  status->freq     =  tuner_ter->RF;
#endif /* TER_TUNER_DTT759x */
#ifdef    TER_TUNER_Si2146
  Si2146_L1_TUNER_STATUS            (tuner_ter, Si2146_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  if (front_end->standard == SILABS_ANALOG) {
    Si2146_L1_ATV_STATUS            (tuner_ter, Si2146_ATV_STATUS_CMD_INTACK_OK );
  } else {
    Si2146_L1_DTV_STATUS            (tuner_ter, Si2146_DTV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->dtv_status.chl;
    status->bw                 =  tuner_ter->rsp->dtv_status.bw;
    status->modulation         =  tuner_ter->rsp->dtv_status.modulation;
  }
  status->RSSI = tuner_ter->rsp->tuner_status.rssi;
#endif /* TER_TUNER_Si2146 */
#ifdef    TER_TUNER_Si2148
  Si2148_L1_TUNER_STATUS            (tuner_ter, Si2148_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  status->RSSI = tuner_ter->rsp->tuner_status.rssi;
#endif /* TER_TUNER_Si2148 */
#ifdef    TER_TUNER_Si2156
  Si2156_L1_TUNER_STATUS            (tuner_ter, Si2156_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  if (front_end->standard == SILABS_ANALOG) {
    Si2156_L1_ATV_STATUS            (tuner_ter, Si2156_ATV_STATUS_CMD_INTACK_OK );
  } else {
    Si2156_L1_DTV_STATUS            (tuner_ter, Si2156_DTV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->dtv_status.chl;
    status->bw                 =  tuner_ter->rsp->dtv_status.bw;
    status->modulation         =  tuner_ter->rsp->dtv_status.modulation;
  }
  status->RSSI = tuner_ter->rsp->tuner_status.rssi;
#endif /* TER_TUNER_Si2156 */
#ifdef    TER_TUNER_Si2158
  Si2158_L1_TUNER_STATUS            (tuner_ter, Si2158_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  if (front_end->standard == SILABS_ANALOG) {
    Si2158_L1_ATV_STATUS            (tuner_ter, Si2158_ATV_STATUS_CMD_INTACK_OK );
  } else {
    Si2158_L1_DTV_STATUS            (tuner_ter, Si2158_DTV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->dtv_status.chl;
    status->bw                 =  tuner_ter->rsp->dtv_status.bw;
    status->modulation         =  tuner_ter->rsp->dtv_status.modulation;
  }
  status->RSSI = tuner_ter->rsp->tuner_status.rssi;
#endif /* TER_TUNER_Si2158 */
#ifdef    TER_TUNER_Si2173
  Si2173_L1_TUNER_STATUS            (tuner_ter, Si2173_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  if (front_end->standard == SILABS_ANALOG) {
    Si2173_L1_ATV_STATUS            (tuner_ter, Si2173_ATV_STATUS_CMD_INTACK_OK );
    status->audio_chan_filt_bw =  tuner_ter->rsp->atv_status.audio_chan_bw;
    status->chl                =  tuner_ter->rsp->atv_status.chl;
    status->ATV_Sync_Lock      =  tuner_ter->rsp->atv_status.pcl;
    status->ATV_Master_Lock    =  tuner_ter->rsp->atv_status.dl;
    status->snrl               =  tuner_ter->rsp->atv_status.snrl;
    status->snrh               =  tuner_ter->rsp->atv_status.snrh;
    status->video_snr          =  tuner_ter->rsp->atv_status.video_snr;
    status->afc_freq           =  tuner_ter->rsp->atv_status.afc_freq;
    status->video_sc_spacing   =  tuner_ter->rsp->atv_status.video_sc_spacing;
    status->video_sys          =  tuner_ter->rsp->atv_status.video_sys;
    status->color              =  tuner_ter->rsp->atv_status.color;
    status->trans              =  tuner_ter->rsp->atv_status.trans;
    status->audio_sys          =  tuner_ter->rsp->atv_status.audio_sys;
  } else {
    Si2173_L1_DTV_STATUS            (tuner_ter, Si2173_DTV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->dtv_status.chl;
    status->bw                 =  tuner_ter->rsp->dtv_status.bw;
    status->modulation         =  tuner_ter->rsp->dtv_status.modulation;
  }
#endif /* TER_TUNER_Si2173 */
#ifdef    TER_TUNER_Si2176
  Si2176_L1_TUNER_STATUS            (tuner_ter, Si2176_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  if (front_end->standard == SILABS_ANALOG) {
    Si2176_L1_ATV_STATUS            (tuner_ter, Si2176_ATV_STATUS_CMD_INTACK_OK );
    status->audio_chan_filt_bw =  tuner_ter->rsp->atv_status.audio_chan_bw;
    status->chl                =  tuner_ter->rsp->atv_status.chl;
    status->ATV_Sync_Lock      =  tuner_ter->rsp->atv_status.pcl;
    status->ATV_Master_Lock    =  tuner_ter->rsp->atv_status.dl;
    status->snrl               =  tuner_ter->rsp->atv_status.snrl;
    status->snrh               =  tuner_ter->rsp->atv_status.snrh;
    status->video_snr          =  tuner_ter->rsp->atv_status.video_snr;
    status->afc_freq           =  tuner_ter->rsp->atv_status.afc_freq;
    status->video_sc_spacing   =  tuner_ter->rsp->atv_status.video_sc_spacing;
    status->video_sys          =  tuner_ter->rsp->atv_status.video_sys;
    status->color              =  tuner_ter->rsp->atv_status.color;
    status->trans              =  tuner_ter->rsp->atv_status.trans;
    status->audio_sys          =  tuner_ter->rsp->atv_status.audio_sys;
    status->audio_demod_mode   =  tuner_ter->rsp->atv_status.audio_demod_mode;
  } else {
    Si2176_L1_DTV_STATUS            (tuner_ter, Si2176_DTV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->dtv_status.chl;
    status->bw                 =  tuner_ter->rsp->dtv_status.bw;
    status->modulation         =  tuner_ter->rsp->dtv_status.modulation;
  }
#endif /* TER_TUNER_Si2176 */
#ifdef    TER_TUNER_Si2178
  Si2178_L1_TUNER_STATUS            (tuner_ter, Si2178_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  if (front_end->standard == SILABS_ANALOG) {
    Si2178_L1_ATV_STATUS            (tuner_ter, Si2178_ATV_STATUS_CMD_INTACK_OK );
    status->audio_chan_filt_bw =  tuner_ter->rsp->atv_status.audio_chan_bw;
    status->chl                =  tuner_ter->rsp->atv_status.chl;
    status->ATV_Sync_Lock      =  tuner_ter->rsp->atv_status.pcl;
    status->ATV_Master_Lock    =  tuner_ter->rsp->atv_status.dl;
    status->snrl               =  tuner_ter->rsp->atv_status.snrl;
    status->snrh               =  tuner_ter->rsp->atv_status.snrh;
    status->video_snr          =  tuner_ter->rsp->atv_status.video_snr;
    status->afc_freq           =  tuner_ter->rsp->atv_status.afc_freq;
    status->video_sc_spacing   =  tuner_ter->rsp->atv_status.video_sc_spacing;
    status->video_sys          =  tuner_ter->rsp->atv_status.video_sys;
    status->color              =  tuner_ter->rsp->atv_status.color;
    status->audio_sys          =  tuner_ter->rsp->atv_status.audio_sys;
  } else {
    Si2178_L1_DTV_STATUS            (tuner_ter, Si2178_DTV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->dtv_status.chl;
    status->bw                 =  tuner_ter->rsp->dtv_status.bw;
    status->modulation         =  tuner_ter->rsp->dtv_status.modulation;
  }
#endif /* TER_TUNER_Si2178 */
#ifdef    TER_TUNER_Si2190
  Si2190_L1_TUNER_STATUS            (tuner_ter, Si2190_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  if (front_end->standard == SILABS_ANALOG) {
    Si2190_L1_ATV_STATUS            (tuner_ter, Si2190_ATV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->atv_status.chl;
    status->ATV_Sync_Lock      =  tuner_ter->rsp->atv_status.pcl;
    status->ATV_Master_Lock    =  tuner_ter->rsp->atv_status.dl;
    status->snrl               =  tuner_ter->rsp->atv_status.snrl;
    status->snrh               =  tuner_ter->rsp->atv_status.snrh;
    status->video_snr          =  tuner_ter->rsp->atv_status.video_snr;
    status->afc_freq           =  tuner_ter->rsp->atv_status.afc_freq;
    status->video_sc_spacing   =  tuner_ter->rsp->atv_status.video_sc_spacing;
    status->video_sys          =  tuner_ter->rsp->atv_status.video_sys;
    status->color              =  tuner_ter->rsp->atv_status.color;
    status->audio_sys          =  tuner_ter->rsp->atv_status.audio_sys;
  } else {
    Si2190_L1_DTV_STATUS            (tuner_ter, Si2190_DTV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->dtv_status.chl;
    status->bw                 =  tuner_ter->rsp->dtv_status.bw;
    status->modulation         =  tuner_ter->rsp->dtv_status.modulation;
  }
#endif /* TER_TUNER_Si2190 */
#ifdef    TER_TUNER_Si2196
  Si2196_L1_TUNER_STATUS            (tuner_ter, Si2196_TUNER_STATUS_CMD_INTACK_OK );
  status->vco_code =  tuner_ter->rsp->tuner_status.vco_code;
  status->tc       =  tuner_ter->rsp->tuner_status.tc;
  status->rssil    =  tuner_ter->rsp->tuner_status.rssil;
  status->rssih    =  tuner_ter->rsp->tuner_status.rssih;
  status->rssi     =  tuner_ter->rsp->tuner_status.rssi;
  status->freq     =  tuner_ter->rsp->tuner_status.freq;
  status->mode     =  tuner_ter->rsp->tuner_status.mode;
  if (front_end->standard == SILABS_ANALOG) {
    Si2196_L1_ATV_STATUS            (tuner_ter, Si2196_ATV_STATUS_CMD_INTACK_OK );
    status->audio_chan_filt_bw =  tuner_ter->rsp->atv_status.audio_chan_filt_bw;
    status->chl                =  tuner_ter->rsp->atv_status.chl;
    status->ATV_Sync_Lock      =  tuner_ter->rsp->atv_status.pcl;
    status->ATV_Master_Lock    =  tuner_ter->rsp->atv_status.dl;
    status->snrl               =  tuner_ter->rsp->atv_status.snrl;
    status->snrh               =  tuner_ter->rsp->atv_status.snrh;
    status->video_snr          =  tuner_ter->rsp->atv_status.video_snr;
    status->afc_freq           =  tuner_ter->rsp->atv_status.afc_freq;
    status->video_sc_spacing   =  tuner_ter->rsp->atv_status.video_sc_spacing;
    status->video_sys          =  tuner_ter->rsp->atv_status.video_sys;
    status->color              =  tuner_ter->rsp->atv_status.color;
    status->trans              =  tuner_ter->rsp->atv_status.trans;
    status->audio_sys          =  tuner_ter->rsp->atv_status.audio_sys;
  } else {
    Si2196_L1_DTV_STATUS            (tuner_ter, Si2196_DTV_STATUS_CMD_INTACK_OK );
    status->chl                =  tuner_ter->rsp->dtv_status.chl;
    status->bw                 =  tuner_ter->rsp->dtv_status.bw;
    status->modulation         =  tuner_ter->rsp->dtv_status.modulation;
  }
#endif /* TER_TUNER_Si2196 */
  SiLabs_API_Tuner_I2C_Disable (front_end);
  front_end = front_end; /* To avoid compiler warning */
  status    = status;    /* To avoid compiler warning */
  return 1;
}
/************************************************************************************************************************
  SiLabs_API_SAT_Tuner_status function
  Use:        Satellite tuner status function
              Used to retrieve the status of the SAT tuner in a structure
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  status, a pointer to the status structure (configurable in CUSTOM_Status_Struct)
  Return:     1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_SAT_Tuner_status         (SILABS_FE_Context* front_end, CUSTOM_Status_Struct *status)
{
  int ref_level;
  front_end = front_end; /* To avoid compiler warning */
  status    = status;    /* To avoid compiler warning */
  ref_level = 0;
  return 1;
}
/************************************************************************************************************************
  SiLabs_API_FE_status function
  Use:        Front-End status function
              Used to retrieve the status of the front-end in a structure
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  status, a pointer to the status structure (configurable in CUSTOM_Status_Struct)
  Return:     1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_FE_status                (SILABS_FE_Context* front_end, CUSTOM_Status_Struct *status)
{

  SiTRACE("SiLabs_API_FE_status in %s\n", Silabs_Standard_Text(front_end->standard));
  switch (front_end->standard) {
    case SILABS_ANALOG: {
      SiLabs_API_TER_Tuner_status   (front_end, status);
      return 1;
      break;
    }
    case SILABS_DVB_T :
    case SILABS_DVB_T2:
    case SILABS_DVB_C :
    case SILABS_DVB_C2:
    case SILABS_MCNS  : {
      SiLabs_API_TER_Tuner_status   (front_end, status);
      break;
    }
    case SILABS_DVB_S :
    case SILABS_DVB_S2:
    case SILABS_DSS   : {
      SiLabs_API_SAT_Tuner_status   (front_end, status);
      break;
    }
    case SILABS_SLEEP : {
      status->standard = SILABS_SLEEP;
      return 0; break;
    }
    default           : { return 0; break; }
  }
  SiLabs_API_Demod_status       (front_end, status);
  return 1;
}
/************************************************************************************************************************
  SiLabs_API_Text_status function
  Use:        Front-End status function
              Used to retrieve the status of the front-end in a text
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  status, a pointer to the status structure (configurable in CUSTOM_Status_Struct)
  Parameter:  formatted_string, a text buffer to store the result
  Return:     1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Text_status              (SILABS_FE_Context* front_end, CUSTOM_Status_Struct *status, char *formatted_status)
{
  SiTRACE("SiLabs_API_Text_status in %s\n", Silabs_Standard_Text(status->standard));
    sprintf(formatted_status, "\nFront_end status:\n\n");
    switch (status->standard)
    {
        case SILABS_ANALOG: { sprintf(formatted_status, "%s standard       ANALOG\n", formatted_status); break; }
        case SILABS_DVB_T : { sprintf(formatted_status, "%s standard       DVB-T\n" , formatted_status); break; }
        case SILABS_DVB_T2: { sprintf(formatted_status, "%s standard       DVB-T2\n", formatted_status); break; }
        case SILABS_DVB_C : { sprintf(formatted_status, "%s standard       DVB-C\n" , formatted_status); break; }
        case SILABS_DVB_C2: { sprintf(formatted_status, "%s standard       DVB-C2\n", formatted_status); break; }
        case SILABS_MCNS  : { sprintf(formatted_status, "%s standard       MCNS\n"  , formatted_status); break; }
        case SILABS_DVB_S : { sprintf(formatted_status, "%s standard       DVB-S\n" , formatted_status); break; }
        case SILABS_DVB_S2: { sprintf(formatted_status, "%s standard       DVB-S2\n", formatted_status); break; }
        case SILABS_DSS   : { sprintf(formatted_status, "%s standard       DSS\n"   , formatted_status); break; }
        case SILABS_SLEEP : { sprintf(formatted_status, "%s SLEEP MODE\n"           , formatted_status); return 0 ; break; }
        default           : { sprintf(formatted_status, "%s INVALID standard (%d)!\n",formatted_status, front_end->standard); return 0; break; }
    }
    switch (status->standard)
    {
        case SILABS_ANALOG: { sprintf(formatted_status, "%s frequency         %ld  Hz\n", formatted_status, status->freq); break; }
        case SILABS_DVB_T : { sprintf(formatted_status, "%s frequency         %ld  Hz\n", formatted_status, status->freq); break; }
        case SILABS_DVB_T2: { sprintf(formatted_status, "%s frequency         %ld  Hz\n", formatted_status, status->freq); break; }
        case SILABS_DVB_C : { sprintf(formatted_status, "%s frequency         %ld  Hz\n", formatted_status, status->freq); break; }
        case SILABS_DVB_C2: { sprintf(formatted_status, "%s frequency         %ld  Hz\n", formatted_status, status->freq); break; }
        case SILABS_DVB_S : { sprintf(formatted_status, "%s frequency         %ld kHz\n", formatted_status, status->freq); break; }
        case SILABS_DVB_S2: { sprintf(formatted_status, "%s frequency         %ld kHz\n", formatted_status, status->freq); break; }
        case SILABS_DSS   : { sprintf(formatted_status, "%s frequency         %ld kHz\n", formatted_status, status->freq); break; }
        default           : { sprintf(formatted_status, "%s INVALID standard (%d)!\n",    formatted_status, front_end->standard); return 0; break; }
    }

    sprintf(formatted_status, "%s demod_lock        %d", formatted_status, status->demod_lock);
    if (status->demod_lock) { sprintf(formatted_status, "%s : locked\n"  , formatted_status);
    } else                  { sprintf(formatted_status, "%s : unlocked\n", formatted_status);
    }
    sprintf(formatted_status, "%s fec_lock          %d", formatted_status, status->fec_lock);
    if (status->fec_lock)   { sprintf(formatted_status, "%s : locked\n"  , formatted_status);
    } else {                  sprintf(formatted_status, "%s : unlocked\n", formatted_status);
    }

    switch (status->standard)
    {
        case SILABS_ANALOG: {
          sprintf(formatted_status, "%s standard       ANALOG\n", formatted_status);
          break;
        }
        case SILABS_DVB_T : {
          sprintf(formatted_status, "%s characteristics     %s ", formatted_status, Silabs_FFT_Text(status->fft_mode));
          sprintf(formatted_status, "%s %s "                , formatted_status, Silabs_Constel_Text(status->constellation));
          sprintf(formatted_status, "%s %s "                , formatted_status, Silabs_Coderate_Text(status->code_rate_hp));
          sprintf(formatted_status, "%s %s "                , formatted_status, Silabs_GI_Text(status->guard_interval));
          sprintf(formatted_status, "%s %3.1f MHz \n"       , formatted_status, status->bandwidth_Hz/1000000);
          break;
        }
        case SILABS_DVB_C : {
          sprintf(formatted_status, "%s characteristics     %s ", formatted_status, Silabs_Constel_Text(status->constellation));
          sprintf(formatted_status, "%s %1.3f Mbps \n"          , formatted_status, status->symbol_rate/1000000);
          break;
        }
        case SILABS_DVB_C2 : {
          sprintf(formatted_status, "%s characteristics     %s ", formatted_status, (char*)"To be defined for C2...");
          break;
        }
        case SILABS_MCNS  : {
          sprintf(formatted_status, "%s characteristics     %s ", formatted_status, Silabs_Constel_Text(status->constellation));
          sprintf(formatted_status, "%s %1.3f Mbps \n"          , formatted_status, status->symbol_rate/1000000);
          break;
        }
        case SILABS_DVB_S : {
          sprintf(formatted_status, "%s characteristics     %s ", formatted_status, Silabs_Constel_Text(status->constellation));
          sprintf(formatted_status, "%s %1.3f Mbps "           , formatted_status, status->symbol_rate/1000000);
          sprintf(formatted_status, "%s %s \n"                 , formatted_status, Silabs_Coderate_Text(status->code_rate));
          break;
        }
        case SILABS_DSS   : {
          sprintf(formatted_status, "%s characteristics     %s ", formatted_status, Silabs_Constel_Text(status->constellation));
//          sprintf(formatted_status, "%s %1.3f Mbps "           , formatted_status, status->symbol_rate/1000000.0);
          sprintf(formatted_status, "%s %s \n"                 , formatted_status, Silabs_Coderate_Text(status->code_rate));
          break;
        }
        case SILABS_DVB_S2: {
          sprintf(formatted_status, "%s characteristics     %s ", formatted_status, Silabs_Constel_Text(status->constellation));
          sprintf(formatted_status, "%s %1.3f Mbps "            , formatted_status, status->symbol_rate/1000000);
          sprintf(formatted_status, "%s %s "                    , formatted_status, Silabs_Coderate_Text(status->code_rate));
          sprintf(formatted_status, "%s %s \n"          , formatted_status, Silabs_Pilots_Text(status->pilots));
          if (status->fer == -1) {
          sprintf(formatted_status, "%s fer              --------\n", formatted_status);
          } else {
          sprintf(formatted_status, "%s fer              %8.2e\n", formatted_status, status->fer);
          }
          break;
        }
        case SILABS_DVB_T2: {
          sprintf(formatted_status, "%s characteristics     %s", formatted_status, Silabs_FFT_Text(status->fft_mode));
          sprintf(formatted_status, "%s %s "                   , formatted_status, Silabs_Extended_BW_Text(status->bw_ext));
          sprintf(formatted_status, "%s %s"                    , formatted_status, Silabs_Constel_Text(status->constellation));
          sprintf(formatted_status, "%s %s "                   , formatted_status, Silabs_Rotated_QAM_Text(status->rotated));
          /*sprintf(formatted_status, "%s %s "                 , formatted_status, Silabs_Coderate_Text(status->code_rate));*/
          sprintf(formatted_status, "%s %s "                   , formatted_status, Silabs_GI_Text(status->guard_interval));
          sprintf(formatted_status, "%s %3.1f MHz "            , formatted_status, status->bandwidth_Hz/1000000);
          sprintf(formatted_status, "%s %s "                   , formatted_status, Silabs_FEF_Text(status->fef));
          sprintf(formatted_status, "%s %s "                   , formatted_status, Silabs_MISO_Text(status->tx_mode));
          sprintf(formatted_status, "%s %s "                   , formatted_status, Silabs_Pilot_Pattern_Text(status->pilot_pattern));
          sprintf(formatted_status, "%s num_plp:%d "           , formatted_status, status->num_plp);
          sprintf(formatted_status, "%s plp_id:%d "            , formatted_status, status->plp_id);
          sprintf(formatted_status, "%s %s \n"                 , formatted_status, Silabs_No_Short_Frame_Text(status->short_frame));
          if (status->fer == -1) {
          sprintf(formatted_status, "%s fer              --------\n", formatted_status);
          } else {
          sprintf(formatted_status, "%s fer              %8.2e\n", formatted_status, status->fer);
          }
          break;
        }
        default           : {
          sprintf(formatted_status, "%s INVALID standard (%d)!!!!\n"  , formatted_status, front_end->standard);
          break;
        }
    }

    if (status->ber     == -1) {
    sprintf(formatted_status, "%s ber              --------\n", formatted_status);
    } else {
    sprintf(formatted_status, "%s ber              %8.2e\n"   , formatted_status, status->ber);
    }
    if (status->per     == -1) {
    sprintf(formatted_status, "%s per              --------\n", formatted_status);
    } else {
    sprintf(formatted_status, "%s per              %8.2e\n"   , formatted_status, status->per);
    }
    sprintf(formatted_status, "%s c_n              %8.2f\n"   , formatted_status, status->c_n);
    if (status->uncorrs == -1) {
    sprintf(formatted_status, "%s uncorrs          --------\n", formatted_status);
    } else {
    sprintf(formatted_status, "%s uncorrs          %8d\n"     , formatted_status, status->uncorrs);
    }
    sprintf(formatted_status, "%s spectral_invers. %8d\n"    , formatted_status, status->spectral_inversion);
    sprintf(formatted_status, "%s RFagc            %8d\n"    , formatted_status, status->RFagc);
    sprintf(formatted_status, "%s IFagc            %8d\n"    , formatted_status, status->IFagc);
    sprintf(formatted_status, "%s freq_offset      %8ld\n"   , formatted_status, status->freq_offset);
    sprintf(formatted_status, "%s timing_offset    %8ld\n"   , formatted_status, status->timing_offset);
    sprintf(formatted_status, "%s RSSI             %f\n"     , formatted_status, status->RSSI);
    sprintf(formatted_status, "%s SSI              %d\n"     , formatted_status, status->SSI);
    sprintf(formatted_status, "%s SQI              %d\n"     , formatted_status, status->SQI);
    sprintf(formatted_status, "%s TS bitrate       %d kbps\n", formatted_status, status->TS_bitrate_kHz);
    sprintf(formatted_status, "%s TS clock         %d kHz\n" , formatted_status, status->TS_clock_kHz  );

    return status->demod_lock;

}
/*****************************************************************************************/
/*               SiLabs demodulator API functions (demod and tuner)                      */
/*****************************************************************************************/

/************************************************************************************************************************
  SiLabs_API_SW_Init function
  Use:        software initialization function
              Used to initialize the DTV demodulator and tuner structures
  Behavior:   This function performs all the steps necessary to initialize the demod and tuner instances
  Parameter:  front_end, a pointer to the SILABS_FE_Context context to be initialized
  Parameter:  demodAdd, the I2C address of the demod
  Parameter:  tunerAdd, the I2C address of the tuner
  Comments:     It MUST be called first and once before using any other function.
                It can be used to build a multi-demod/multi-tuner application, if called several times from the upper
                  layer with different pointers and addresses.
                After execution, all demod and tuner functions are accessible.
************************************************************************************************************************/
char  SiLabs_API_SW_Init                  (SILABS_FE_Context *front_end, int demodAdd, int tunerAdd_Ter, int tunerAdd_Sat)
 {
  int chip;

  tunerAdd_Ter = tunerAdd_Ter; /* To avoid compiler warning */
  tunerAdd_Sat = tunerAdd_Sat; /* To avoid compiler warning */

  SiTRACE("Wrapper              Source code %s\n", SiLabs_API_TAG_TEXT() );
  SiTRACE("tunerAdd_Ter 0x%02x\n", tunerAdd_Ter);
  SiTRACE("tunerAdd_Sat 0x%02x\n", tunerAdd_Sat);
  /* Start by detecting the chip type */
  chip = SiLabs_chip_detect(demodAdd);
  SiTRACE("chip '%d' ('%X')\n", chip, chip);
  front_end->standard = -1;

//  WrapperI2C = &WrapperI2C_context;
//  WrapperI2C->indexSize = 0;
//  WrapperI2C->mustReadWithoutStop = 0;

#ifdef    Si2168_COMPATIBLE
  if (chip == 2168) {
    front_end->Si2168_FE = &(front_end->Si2168_FE_Obj);
    if (Si2168_L2_SW_Init   (front_end->Si2168_FE
                             , demodAdd
                             , tunerAdd_Ter
                             ) ) {
      front_end->chip = chip;
      Silabs_init_done = 1;
      return 1;
    } else {
      SiTRACE("ERROR initializing Si2168 context\n");
      return 0;
    }
  }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Unknown chip '%d'\n", front_end->chip);
  SiERROR("SiLabs_API_SW_Init: Unknown chip !\n");
  return 0;
}
/************************************************************************************************************************
  SiLabs_API_Infos  function
  Use:        software information function
              Used to retrieve various information about the front-end code
  Parameter:  front_end, a pointer to the SILABS_FE_Context context to be initialized
  Parameter:  infoString, a text buffer to be filled with teh information. It must be initialized by the caller.
  Return:     the length of the information string
************************************************************************************************************************/
int   SiLabs_API_Infos                     (SILABS_FE_Context *front_end, char *infoString)
 {
  front_end = front_end; /* To avoid compiler warning */
  if (infoString       == NULL) return 0;
  if (Silabs_init_done ==    0) {
    sprintf(infoString, "Call SiLabs_API_SW_Init first!\n");
  } else {
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    Si2168_L2_Infos  (front_end->Si2168_FE, infoString);
  }
#endif /* Si2168_COMPATIBLE */
  }
  sprintf(infoString, "%sWrapper              Source code %s\n"   , infoString, SiLabs_API_TAG_TEXT() );
  sprintf(infoString, "%s--------------------------------------\n", infoString);

  return strlen(infoString);
}
/************************************************************************************************************************
  SiLabs_API_HW_Connect function
  Use:        hardware connection function
              Used to connect the HW in the selected mode
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  connection_mode, the required mode
  Porting:    This function may be removed in the final application. It is useful during development to allow connection in various modes
************************************************************************************************************************/
char  SiLabs_API_HW_Connect               (SILABS_FE_Context *front_end, CONNECTION_TYPE connection_mode)
{
#ifdef    Si2168_COMPATIBLE
 // if (front_end->chip ==   2168 ) { Si2168_L2_HW_Connect (front_end->Si2168_FE, connection_mode); }
#endif /* Si2168_COMPATIBLE */
//  L0_Connect(WrapperI2C,    connection_mode);
  SiTRACE("SiLabs_API_HW_Connect in mode %d\n", connection_mode);
  return 1;
}
/************************************************************************************************************************
  SiLabs_API_ReadString function
  Use:        low-level i2c read function
              Used to easily read i2c bytes from the wrapper
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  readString, the string to be used
  Parameter:  pbtDataBuffer, a buffer to stor ethe read bytes
  Behavior:   Split the input string in tokens (separated by 'space') and read the corresponding bytes
              The chip address is the first byte
              The number of bytes to read is equal to the last token
              The index size if the number of tokens - 2
  Example:    'SiLabs_API_ReadString (front_end, "0xC8 0x12 0x34 5", pbtDataBuffer);' will read 5 bytes from the chip at address 0xc8 starting from index 0x1234
               0xC8 0x12 0x34  5
                |   ---------  |
                |       |      ----> 5 bytes to read
                |       -----------> index 0x1234 (on 2 bytes)
                -------------------> address = 0xc8
  Example:    'SiLabs_API_ReadString (front_end, "0xA0 0x00 5"     , pbtDataBuffer);' will read 5 bytes from the chip at address 0xA0 starting from index 0x00 (This will return the content of the Cypress eeprom)
  Example:    'SiLabs_API_ReadString (front_end, "0xC0 1"          , pbtDataBuffer);' will read 1 byte  from the chip at address 0xC0 (This will return the status byte for a command-mode chip)
  Returns:    The number of bytes written
************************************************************************************************************************/
int   SiLabs_API_ReadString               (SILABS_FE_Context *front_end, char *readString, unsigned char *pbtDataBuffer)
{
  front_end = front_end; /* To avoid compiler warning */
  return ;//L0_ReadString (WrapperI2C, readString, pbtDataBuffer);
}
/************************************************************************************************************************
  SiLabs_API_WriteString function
  Use:        low_level i2c write function
              Used to easily write bytes from the wrapper, based on a string input
  Behavior:   Split the input string in tokens (separated by 'space') and write the corresponding bytes
              The chip address is the first byte
              The number of bytes to write is equal to the number of tokens -1
  Example:    'SiLabs_API_WriteString (front_end, "0xC8 0x12 0x34 0x37");' will write '0x37' at index 0x1234 in the chip at address 0xc8
  Returns:    The number of bytes written
************************************************************************************************************************/
char  SiLabs_API_WriteString              (SILABS_FE_Context *front_end, char *writeString)
{
  front_end = front_end; /* To avoid compiler warning */
  return ;//L0_WriteString(WrapperI2C, writeString);
}
/************************************************************************************************************************
  SiLabs_API_bytes_trace function
  Use:        Byte-level tracing function
              Used to toggle the L0 traces
  Parameter:  front_end, a pointer to the SILABS_FE_Context context
  Parameter:  track_mode, the required trace mode
  Porting:    This function may be removed in the final application. It is useful during development to check i2c communication
  Returns:    The final track_mode
************************************************************************************************************************/
char  SiLabs_API_bytes_trace              (SILABS_FE_Context *front_end, unsigned char track_mode)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
 /*   front_end->Si2168_FE->demod->i2c->trackWrite     = track_mode;
    front_end->Si2168_FE->demod->i2c->trackRead      = track_mode;
    front_end->Si2168_FE->tuner_ter->i2c->trackWrite = track_mode;
    front_end->Si2168_FE->tuner_ter->i2c->trackRead  = track_mode;*///rsj
  }
#endif /* Si2168_COMPATIBLE */
//  WrapperI2C->trackWrite     = track_mode;
 // WrapperI2C->trackRead      = track_mode;
  return track_mode;
}
/************************************************************************************************************************
  SiLabs_API_communication_check function
  Use:      Communication check function
            Used to make sure all chips are connected
  Parameter: front_end a pointer to the front-end structure
  Return:    1 if sucessful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_communication_check      (SILABS_FE_Context *front_end)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { return Si2168_L2_communication_check (front_end->Si2168_FE); }
#endif /* Si2168_COMPATIBLE */
  return 0;
}
/************************************************************************************************************************
  SiLabs_API_switch_to_standard function
  Use:      Standard switching function selection
            Used to switch nicely to the wanted standard, taking into account the previous state
  Parameter: new_standard the wanted standard to switch to
  Return:    1 if sucessful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_switch_to_standard       (SILABS_FE_Context *front_end, int standard, unsigned char force_full_init)
{
  front_end->init_ok = 0;
  SiTRACE("Wrapper switching to %s\n", Silabs_Standard_Text(standard) );
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { front_end->init_ok = Si2168_L2_switch_to_standard (front_end->Si2168_FE , Silabs_standardCode(front_end, standard), force_full_init);}
#endif /* Si2168_COMPATIBLE */
  if (front_end->init_ok == 0) {
    SiTRACE("Problem switching to %s\n", Silabs_Standard_Text(standard));
  } else {
    front_end->standard = standard;
  }
  return front_end->init_ok;
}
/************************************************************************************************************************
  SiLabs_API_set_standard function
  Use:      Standard switching function selection
            Used to change the current standard only
  Behavior:  WARNING: use with caution, thsi is only used for SAT, where the demod can lock in S2 when set to S (and vice-versa)!
  Parameter: new_standard the wanted standard to switch to
  Return:    1 if sucessful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_set_standard             (SILABS_FE_Context *front_end, int standard)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { if (Si2168_L2_switch_to_standard (front_end->Si2168_FE,        Silabs_standardCode(front_end, (CUSTOM_Standard_Enum)standard), 0)) { front_end->standard = standard; return 1;} }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Problem switching to %s\n", Silabs_Standard_Text(standard));
  return 0;
}
/************************************************************************************************************************
  SiLabs_API_lock_to_carrier function
  Use:      relocking function
            Used to relock on a channel for the required standard
  Parameter: standard the standard to lock to
  Parameter: freq                the frequency to lock to (in Hz for TER, in kHz for SAT)
  Parameter: bandwidth_Hz        the channel bandwidth in Hz (only for DVB-T and DVB-T2)
  Parameter: dvb_t_stream        the HP/LP stream  (only for DVB-T)
  Parameter: symbol_rate_bps     the symbol rate in baud/s  (for DVB-C and SAT)
  Parameter: dvb_c_constellation the DVB-C constellation (only for DVB-C)
  Parameter: plp_id              the PLP Id (only for DVB-T2 and DVB-C2 when num_plp > 1)
  Return:    1 if locked, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_lock_to_carrier          (SILABS_FE_Context *front_end, int standard, int freq, int bandwidth_Hz, int stream, unsigned int symbol_rate_bps, int constellation, int polarization, int band, int plp_id)
{
  int standard_code;
  int constel_code;
  int stream_code;
  plp_id = plp_id; /* to avoid compiler warining if not used */

  SiTRACE("SiLabs_API_lock_to_carrier (front_end, %8d, %d, %d, %2d, %d, %d, %d, %d, %d)\n", standard, freq, bandwidth_Hz, stream, symbol_rate_bps, constellation, polarization, band, plp_id);

  standard_code = Silabs_standardCode(front_end, (CUSTOM_Standard_Enum)standard);
  constel_code  = Silabs_constelCode (front_end, (CUSTOM_Constel_Enum)constellation);
  stream_code   = Silabs_streamCode  (front_end, (CUSTOM_Stream_Enum)stream);

  SiTRACE("SiLabs_API_lock_to_carrier (front_end, %8s, %d, %d, %2s, %d, %s, %s, %s, %d)\n", Silabs_Standard_Text(standard), freq, bandwidth_Hz, Silabs_Stream_Text(stream), symbol_rate_bps, Silabs_Constel_Text(constellation), Silabs_Polarization_Text(polarization), Silabs_Band_Text(band) , plp_id );

  /* Use the API wrapper function to switch standard if required. */
  if (standard != front_end->standard) {
    if (SiLabs_API_switch_to_standard(front_end, standard, 0) == 0) {
      return 0;
    }
  }

#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    SiTRACE("Si2168_lock_to_carrier     (front_end->Si2168_FE, %d, %d, %d, %d, %d, %d, %d)\n", standard_code, freq, bandwidth_Hz, stream_code, symbol_rate_bps, constel_code, plp_id);
    return   Si2168_L2_lock_to_carrier  (front_end->Si2168_FE,                                 standard_code, freq, bandwidth_Hz, stream_code, symbol_rate_bps, constel_code, plp_id);
  }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Unknown chip '%d'\n", front_end->chip);
  return 0;
}
/************************************************************************************************************************
  SiLabs_API_Tune function
  Use:      tuning function
            Used to tune on a channel for the current standard
  Parameter: freq                the frequency to lock to (in Hz for TER, in kHz for SAT)
  Return:    the tuned freq, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Tune                     (SILABS_FE_Context *front_end, int freq)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    return   Si2168_L2_Tune  (front_end->Si2168_FE, freq );
  }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Unknown chip '%d'\n", front_end->chip);
  return 0;
}
/************************************************************************************************************************
  SiLabs_API_Demod_Standby function
  Use:      Demodulator standby function
            Used to switch the demodulator in standby mode
  Parameter: front_end, a pointer to the front End
  Returns:    1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Demod_Standby            (SILABS_FE_Context *front_end)
{
  front_end = front_end; /* To avoid compiler warning */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { Si2168_STANDBY       (front_end->Si2168_FE->demod); return 1; }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Problem switching %d in standby mode\n", front_end->chip);
  return 0;
}
/************************************************************************************************************************
  SiLabs_API_Demod_ClockOn function
  Use:      Demodulator clock activation function
            Used to switch the demodulator clock on
  Parameter: front_end, a pointer to the front End
  Returns:    1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Demod_ClockOn            (SILABS_FE_Context *front_end)
{
  front_end = front_end; /* To avoid compiler warning */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { return 1; }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Problem switching %d in standby mode\n", front_end->chip);
  return 0;
}
/************************************************************************************************************************
  SiLabs_API_Reset_Uncorrs function
  Use:      uncorrectable packets counter reset function
            Used to reset the uncor counter
  Parameter: front_end, a pointer to the front End
  Returns:    1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Reset_Uncorrs            (SILABS_FE_Context *front_end)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { Si2168_L1_DD_UNCOR (front_end->Si2168_FE->demod, Si2168_DD_UNCOR_CMD_RST_CLEAR); return 1;}
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Problem resetting %d uncorrs\n", front_end->chip);
  return -1;
}
/************************************************************************************************************************
  SiLabs_API_TS_Mode function
  Use:      Transport Stream control function
            Used to switch the TS output in the desired mode
  Parameter: mode the mode to switch to
************************************************************************************************************************/
int   SiLabs_API_TS_Mode                  (SILABS_FE_Context *front_end, int ts_mode)
{
  int valid_mode;
#ifdef    USB_Capability
  int gpif_on;
  int        retdval;
  char rettxtBuffer[256];
  char *rettxt;
  rettxt = rettxtBuffer;
  gpif_on = 1;
#endif /* USB_Capability */
  valid_mode = 0;
#ifdef    USB_Capability
  if (ts_mode != SILABS_TS_GPIF) { L0_Cypress_Configure("-gpif"     ,"off", 0   , &retdval, &rettxt);}
#endif /* USB_Capability */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    switch (ts_mode) {
      case SILABS_TS_SERIAL  : { front_end->Si2168_FE->demod->prop->dd_ts_mode.mode = Si2168_DD_TS_MODE_PROP_MODE_SERIAL  ; break; }
      case SILABS_TS_PARALLEL: { front_end->Si2168_FE->demod->prop->dd_ts_setup_par.ts_data_shape = 2;
                                 front_end->Si2168_FE->demod->prop->dd_ts_setup_par.ts_clk_shape  = 2;
                                 Si2168_L1_SetProperty2(front_end->Si2168_FE->demod, Si2168_DD_TS_SETUP_PAR_PROP_CODE);
                                 front_end->Si2168_FE->demod->prop->dd_ts_mode.mode = Si2168_DD_TS_MODE_PROP_MODE_PARALLEL; break; }
      case SILABS_TS_TRISTATE: { front_end->Si2168_FE->demod->prop->dd_ts_mode.mode = Si2168_DD_TS_MODE_PROP_MODE_OFF     ; break; }
      case SILABS_TS_GPIF    : { front_end->Si2168_FE->demod->prop->dd_ts_setup_par.ts_data_shape = 7;
                                 front_end->Si2168_FE->demod->prop->dd_ts_setup_par.ts_clk_shape  = 7;
                                 Si2168_L1_SetProperty2(front_end->Si2168_FE->demod, Si2168_DD_TS_SETUP_PAR_PROP_CODE);
                                 front_end->Si2168_FE->demod->prop->dd_ts_mode.mode = Si2168_DD_TS_MODE_PROP_MODE_GPIF    ; break; }
      default                : { return SiLabs_API_TS_Mode(front_end, SILABS_TS_TRISTATE)                                 ; break; }
    }
    Si2168_L1_SetProperty2(front_end->Si2168_FE->demod, Si2168_DD_TS_MODE_PROP_CODE);
    valid_mode = 1;
  }
#endif /* Si2168_COMPATIBLE */
  if (valid_mode) {
#ifdef    USB_Capability
    if (ts_mode == SILABS_TS_GPIF ) {
      L0_Cypress_Configure("-gpif"     ,"on"      , 0, &retdval, &rettxt);  /* Starting Cypress gpif state machine   */
      L0_Cypress_Configure("-gpif_clk" ,"on"      , 0, &retdval, &rettxt);  /* Starting Cypress gpif clock           */
      L0_Cypress_Process  ("ts"        ,"start"   , 0, &retdval, &rettxt);  /* Starting Cypress TS transfer over USB */
    } else {
      if (gpif_on) {
      L0_Cypress_Process  ("ts"        ,"stop"    , 0, &retdval, &rettxt);  /* Stopping Cypress TS transfer over USB */
      L0_Cypress_Configure("-gpif"     ,"off"     , 0, &retdval, &rettxt);  /* Stopping Cypress gpif state machine   */
      L0_Cypress_Configure("-gpif_clk" ,"tristate", 0, &retdval, &rettxt);  /* Stopping Cypress gpif clock           */
      }
    }
#endif /* USB_Capability */
    return ts_mode;
  } else {
    return -1;
  }
}
/************************************************************************************************************************
  SiLabs_API_Tuner_I2C_Enable function
  Use:      Demdo Loop through control function
            Used to switch the I2C loopthrough on, allowing commnication with the tuners
  Return:    the final mode (-1 if not known)
************************************************************************************************************************/
int   SiLabs_API_Tuner_I2C_Enable         (SILABS_FE_Context *front_end)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { Si2168_L2_Tuner_I2C_Enable (front_end->Si2168_FE); return 1;}
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  SiLabs_API_Tuner_I2C_Disable function
  Use:      Demdo Loop through control function
            Used to switch the I2C loopthrough off, stopping commnication with the tuners
  Return:    the final mode (-1 if not known)
************************************************************************************************************************/
int   SiLabs_API_Tuner_I2C_Disable        (SILABS_FE_Context *front_end)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { Si2168_L2_Tuner_I2C_Disable (front_end->Si2168_FE); return 0;}
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  NAME: SiLabs_API_Channel_Seek_Init
  DESCRIPTION: logs the seek parameters in the context structure

  Scan algorithm based on the seek feature:

    SiLabs_API_switch_to_standard(front_end, standard, 0);

    SiLabs_API_Channel_Seek_Init (front_end, rangeMin, rangeMax,...);

    While ( SiLabs_API_Channel_Seek_Init (front_end, rangeMin, rangeMax,...) !=0 ) {
      SiLabs_API_TS_Mode (front_end, SILABS_TS_SERIAL/SILABS_TS_PARALLEL);

      the new carrier is at front_end->detected_rf with front_end->detected_sr
      look for PSI/SI information

      SiLabs_API_TS_Mode (front_end, SILABS_TS_TRISTATE);
    }

    SiLabs_API_Channel_Seek_End (front_end);

  Parameter:  Pointer to SILABS_FE_Context
  Parameter:  starting Frequency (in Hz for TER, in kHz for SAT)
  Parameter:  ending   Frequency (in Hz for TER, in kHz for SAT)
  Parameter:  min RSSI dBm
  Parameter:  max RSSI dBm
  Parameter:  min SNR 1/2 dB
  Parameter:  max SNR 1/2 dB
  Returns:    0 if successful, otherwise an error.
************************************************************************************************************************/
int   SiLabs_API_Channel_Seek_Init        (SILABS_FE_Context *front_end,
                                            int rangeMin,     int rangeMax,
                                            int seekBWHz,     int seekStepHz,
                                            int minSRbps,     int maxSRbps,
                                            int minRSSIdBm,   int maxRSSIdBm,
                                            int minSNRHalfdB, int maxSNRHalfdB)
{
  SiTRACE("Seek Init values: freq <%d %d>, BW %d step %d, SR <%d %d>, RSSI <%d %d>, SNR <%d %d>\n", rangeMin, rangeMax, seekBWHz, seekStepHz, minSRbps, maxSRbps, minRSSIdBm, maxRSSIdBm, minSNRHalfdB, maxSNRHalfdB);
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { return Si2168_L2_Channel_Seek_Init (front_end->Si2168_FE,  rangeMin, rangeMax, seekBWHz, seekStepHz, minSRbps, maxSRbps, minRSSIdBm, maxRSSIdBm, minSNRHalfdB, maxSNRHalfdB); }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Unknown chip '%d'\n", front_end->chip);
  return 1;
}
/************************************************************************************************************************
  NAME: SiLabs_API_Channel_Seek_Next
  DESCRIPTION: Looks for the next channel, starting from the last detected channel
  Parameter:  Pointer to SILABS_FE_Context
  Returns:    1 if channel is found, 0 otherwise (either abort or end of range)
************************************************************************************************************************/
int   SiLabs_API_Channel_Seek_Next        (SILABS_FE_Context *front_end, int *standard, int *freq, int *bandwidth_Hz, int *stream, unsigned int *symbol_rate_bps, int *constellation, int *polarization, int *band, int *num_plp)
{
  int seek_result;
  num_plp      = num_plp;      /* to avoid compiler warning if not used */
  bandwidth_Hz = bandwidth_Hz; /* to avoid compiler warning if not used */
  polarization = polarization; /* to avoid compiler warning if not used */
  band         = band;         /* to avoid compiler warning if not used */
  num_plp      = num_plp;      /* to avoid compiler warning if not used */
  seek_result = -1;

#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { seek_result = Si2168_L2_Channel_Seek_Next (front_end->Si2168_FE, standard, freq, bandwidth_Hz, stream, symbol_rate_bps, constellation, num_plp); }
#endif /* Si2168_COMPATIBLE */
  if (seek_result ==  1) {
    *standard = Custom_standardCode(front_end, *standard);
    /* Translate demod-specific values to CUSTOM values */
    switch (*standard)
    {
      case SILABS_DVB_T : {
        *stream        = Custom_streamCode (front_end, *stream);
        break;
      }
      case SILABS_DVB_C : {
        *constellation = Custom_constelCode(front_end, *constellation);
        break;
      }
      default           : {
        break;
      }
    }
    return 1;
  }
  if (seek_result == -1) {
    SiTRACE("Chip '%d' not handled by SiLabs_API_Channel_Seek_Next\n", front_end->chip);
  }
  return 0;
}
/************************************************************************************************************************
  NAME: SiLabs_API_Channel_Seek_Abort
  DESCRIPTION: aborts the channel seek for the next channel
  Parameter:  Pointer to SILABS_FE_Context
  Returns:    1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Channel_Seek_Abort       (SILABS_FE_Context *front_end)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { return Si2168_L2_Channel_Seek_Abort (front_end->Si2168_FE); }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Unknown chip '%d'\n", front_end->chip);
  return 0;
}
/************************************************************************************************************************
  NAME: SiLabs_API_Channel_Lock_Abort
  DESCRIPTION: aborts the channel lock for the current channel
  Parameter:  Pointer to SILABS_FE_Context
  Returns:    1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Channel_Lock_Abort       (SILABS_FE_Context *front_end)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { return Si2168_L2_Channel_Lock_Abort (front_end->Si2168_FE); }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Unknown chip '%d'\n", front_end->chip);
  return 0;
}
/************************************************************************************************************************
  NAME: SiLabs_API_Channel_Seek_End
  DESCRIPTION: returns the chip back to normal following a seek
  Parameter:  Pointer to SILABS_FE_Context
  Returns:    1 if successful, 0 otherwise
************************************************************************************************************************/
int   SiLabs_API_Channel_Seek_End         (SILABS_FE_Context *front_end)
{
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { return Si2168_L2_Channel_Seek_End (front_end->Si2168_FE); }
#endif /* Si2168_COMPATIBLE */
  SiTRACE("Unknown chip '%d'\n", front_end->chip);
  return 0;
}
/************************************************************************************************************************
  NAME: SiLabs_API_T2_Get_PLP_ID
  DESCRIPTION: PLP ID and TYPE info function for DVB-T2
  Parameter: front_end, Pointer to SILABS_FE_Context
  Parameter:  plp_index, the index of the required PLP (must be lower than num_plp for the selected carrier),
  Parameter: *plp_id, a pointer to the plp id,
  Parameter: *plp_type, a pointer to the plp type,
  Returns:    0 if no error value, -1 in case of an error
************************************************************************************************************************/
int   SiLabs_API_Get_PLP_ID_and_TYPE      (SILABS_FE_Context *front_end, int plp_index, int *plp_id, int *plp_type)
{
  front_end = front_end;        /* To avoid compiler warning if not used */
  plp_index = plp_index; /* To avoid compiler warning if not used */
  plp_id    = plp_id   ; /* To avoid compiler warning if not used */
  plp_type  = plp_type ; /* To avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    if (Si2168_L1_DVBT2_PLP_INFO(front_end->Si2168_FE->demod, plp_index) != NO_Si2168_ERROR) {
      SiTRACE("Si2168_L1_DVBT2_PLP_INFO error when checking PLP index %d!\n", plp_index);
      SiERROR("Si2168_L1_DVBT2_PLP_INFO error!\n");
      return -1;
    }
    *plp_id   = front_end->Si2168_FE->demod->rsp->dvbt2_plp_info.plp_id;
    *plp_type = front_end->Si2168_FE->demod->rsp->dvbt2_plp_info.plp_type;
    SiTRACE("PLP index %3d: PLP ID %3d, PLP TYPE %d\n", plp_index, *plp_id, *plp_type);
    return 0;
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  NAME: SiLabs_API_Select_PLP
  DESCRIPTION: PLP selection function for DVB-T2
  Parameter: front_end, Pointer to SILABS_FE_Context
  Parameter:  plp_id, the PLP id of the required PLP (must be lower than num_plp for the selected carrier)
                Use '-1' for auto selection,
  Returns:    0 if no error value, -1 in case of an error
************************************************************************************************************************/
int   SiLabs_API_Select_PLP               (SILABS_FE_Context *front_end, int plp_id)
{
  int plp_mode;
  front_end = front_end;        /* To avoid compiler warning if not used */
  plp_id    = plp_id   ; /* To avoid compiler warning if not used */
  plp_mode  = 0;
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    if (plp_id < 0) {
      plp_mode = Si2168_DVBT2_PLP_SELECT_CMD_PLP_ID_SEL_MODE_AUTO;
    } else {
      plp_mode = Si2168_DVBT2_PLP_SELECT_CMD_PLP_ID_SEL_MODE_MANUAL;
    }
    if (Si2168_L1_DVBT2_PLP_SELECT(front_end->Si2168_FE->demod, plp_id, plp_mode) != NO_Si2168_ERROR) {
      SiTRACE("Si2168_L1_DVBT2_PLP_SELECT error when selecting PLP %d!\n", plp_id);
      SiERROR("Si2168_L1_DVBT2_PLP_SELECT error!\n");
      return -1;
    }
    if (plp_mode == Si2168_DVBT2_PLP_SELECT_CMD_PLP_ID_SEL_MODE_AUTO) { Si2168_L1_DD_RESTART(front_end->Si2168_FE->demod); system_wait(300); }
    return 0;
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
/************************************************************************************************************************
  NAME: SiLabs_API_TER_AutoDetect
  DESCRIPTION: activation function for the TER auto detect mode
  Parameter: front_end, Pointer to SILABS_FE_Context
  Parameter: on_off, which is set to '0' to de-activate the TER auto-detect feature,
              set to '1' to activate it and to any other value to retrieve the current status
  Returns:    the current state of auto_detect_TER
************************************************************************************************************************/
int   SiLabs_API_TER_AutoDetect           (SILABS_FE_Context *front_end, int on_off)
{
  front_end = front_end; /* To avoid compiler warning if not used */
  on_off    = on_off;    /* To avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    if (on_off == 0) { Si2168_TerAutoDetectOff(front_end->Si2168_FE); }
    if (on_off == 1) { Si2168_TerAutoDetect   (front_end->Si2168_FE); }
    return front_end->Si2168_FE->auto_detect_TER;
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
int   SiLabs_API_TER_Tuner_Fine_Tune      (SILABS_FE_Context *front_end, int offset_500hz)
{
  front_end    = front_end;    /* To avoid compiler warning if not supported */
  offset_500hz = offset_500hz; /* To avoid compiler warning if not supported */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
   #ifdef    Si2173_L1_FINE_TUNE
    Si2173_L1_FINE_TUNE(front_end->Si2168_FE->tuner_ter, 0, offset_500hz);
   #endif /* Si2173_L1_FINE_TUNE */
   #ifdef    Si2176_L1_FINE_TUNE
    Si2176_L1_FINE_TUNE(front_end->Si2168_FE->tuner_ter, 0, offset_500hz);
   #endif /* Si2176_L1_FINE_TUNE */
   #ifdef    Si2196_L1_FINE_TUNE
    Si2196_L1_FINE_TUNE(front_end->Si2168_FE->tuner_ter, 0, offset_500hz);
   #endif /* Si2196_L1_FINE_TUNE */
    return 1;
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
int   SiLabs_API_TER_Tuner_Init           (SILABS_FE_Context *front_end)
{
  front_end = front_end; /* To avoid compiler warning if not supported */
#ifdef    TER_TUNER_INIT
  SiLabs_API_Tuner_I2C_Enable (front_end);
#ifdef    Si2168_COMPATIBLE
//rsj // if (front_end->chip ==   2168 ) { TER_TUNER_INIT(front_end->Si2168_FE->tuner_ter); return 1; }
#endif /* Si2168_COMPATIBLE */
  SiLabs_API_Tuner_I2C_Disable(front_end);
#endif /* TER_TUNER_INIT */
  return -1;
}
int   SiLabs_API_TER_Tuner_SetMFT         (SILABS_FE_Context *front_end, int nStep)
{
  int           nCriteriaStep;
  int           nLeftMax;
  int           nRightMax;
  int           nReal_Step;
  int           nBeforeStep;
  front_end = front_end; /* To avoid compiler warning if not supported */
  nStep     = nStep;     /* To avoid compiler warning if not supported */

  nCriteriaStep = front_end->TER_Tuner_Cfg.nCriteriaStep;
  nLeftMax      = front_end->TER_Tuner_Cfg.nLeftMax;
  nRightMax     = front_end->TER_Tuner_Cfg.nReal_Step;
  nReal_Step    = front_end->TER_Tuner_Cfg.nReal_Step;
  nBeforeStep   = front_end->TER_Tuner_Cfg.nBeforeStep;

  if      (nStep == 32 && nBeforeStep == -32){
    SiLabs_API_TER_Tuner_Fine_Tune (front_end, -4000);
    nReal_Step = nRightMax - nCriteriaStep;
  }else if(nStep == -32 && nBeforeStep == 32){
    SiLabs_API_TER_Tuner_Fine_Tune (front_end,  4000);
    nReal_Step = nLeftMax - nCriteriaStep;
  }else{
    if (nCriteriaStep<0) nCriteriaStep = -nCriteriaStep;
    if(nStep > nRightMax)
      nReal_Step = nRightMax + nCriteriaStep;
    else if(nStep < nLeftMax)
      nReal_Step = nLeftMax  - nCriteriaStep;
    else
      nReal_Step = nStep - nCriteriaStep;

    if(nReal_Step > 32)
      nReal_Step = 32;
    else if(nReal_Step < -32)
      nReal_Step = -32;
  }

  SiLabs_API_TER_Tuner_Fine_Tune (front_end, -(125*nReal_Step));

  nBeforeStep = nStep;

  return 1;
}
int   SiLabs_API_TER_Tuner_Text_status    (SILABS_FE_Context *front_end, char *separator, char *msg)
{
  front_end = front_end; /* To avoid compiler warning */
  separator = separator; /* To avoid compiler warning */
  msg       = msg;       /* To avoid compiler warning */
#ifdef    TUNERTER_API
  SiLabs_API_Tuner_I2C_Enable (front_end);
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
   #ifdef    Si2146_COMPATIBLE
    Si2146_L1_TUNER_STATUS            (front_end->Si2168_FE->tuner_ter, Si2146_TUNER_STATUS_CMD_INTACK_OK);
    Si2146_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2146_TUNER_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2146_COMPATIBLE */
   #ifdef    Si2148_COMPATIBLE
    Si2148_L1_TUNER_STATUS            (front_end->Si2168_FE->tuner_ter, Si2148_TUNER_STATUS_CMD_INTACK_OK);
    Si2148_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2148_TUNER_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2148_COMPATIBLE */
   #ifdef    Si2156_COMPATIBLE
    Si2156_L1_TUNER_STATUS            (front_end->Si2168_FE->tuner_ter, Si2156_TUNER_STATUS_CMD_INTACK_OK);
    Si2156_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2156_TUNER_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2156_COMPATIBLE */
   #ifdef    Si2158_COMPATIBLE
    Si2158_L1_TUNER_STATUS            (front_end->Si2168_FE->tuner_ter, Si2158_TUNER_STATUS_CMD_INTACK_OK);
    Si2158_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2158_TUNER_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2158_COMPATIBLE */
   #ifdef    Si2173_COMPATIBLE
    Si2173_L1_TUNER_STATUS            (front_end->Si2168_FE->tuner_ter, Si2173_TUNER_STATUS_CMD_INTACK_OK);
    Si2173_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2173_TUNER_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2173_COMPATIBLE */
   #ifdef    Si2176_COMPATIBLE
    Si2176_L1_TUNER_STATUS            (front_end->Si2168_FE->tuner_ter, Si2176_TUNER_STATUS_CMD_INTACK_OK);
    Si2176_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2176_TUNER_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2176_COMPATIBLE */
   #ifdef    Si2178_COMPATIBLE
    Si2178_L1_TUNER_STATUS            (front_end->Si2168_FE->tuner_ter, Si2178_TUNER_STATUS_CMD_INTACK_OK);
    Si2178_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2178_TUNER_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2178_COMPATIBLE */
   #ifdef    Si2196_COMPATIBLE
    Si2196_L1_TUNER_STATUS            (front_end->Si2168_FE->tuner_ter, Si2196_TUNER_STATUS_CMD_INTACK_OK);
    Si2196_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2196_TUNER_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2196_COMPATIBLE */
    return 1;
  }
#endif /* Si2168_COMPATIBLE */
  SiLabs_API_Tuner_I2C_Disable(front_end);
#endif /* TUNERTER_API */
  return -1;
}
int   SiLabs_API_TER_Tuner_ATV_Text_status(SILABS_FE_Context *front_end, char *separator, char *msg)
{
  separator = separator; /* To avoid compiler warning if not used */
  msg       = msg      ; /* To avoid compiler warning if not used */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
   #ifdef    Si2146_COMPATIBLE
    Si2146_L1_ATV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2146_ATV_STATUS_CMD_INTACK_OK );
    Si2146_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2146_ATV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2146_COMPATIBLE */
   #ifdef    Si2156_COMPATIBLE
    Si2156_L1_ATV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2156_ATV_STATUS_CMD_INTACK_OK );
    Si2156_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2156_ATV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2156_COMPATIBLE */
   #ifdef    Si2158_COMPATIBLE
    Si2158_L1_ATV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2158_ATV_STATUS_CMD_INTACK_OK );
    Si2158_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2158_ATV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2158_COMPATIBLE */
   #ifdef    Si2173_COMPATIBLE
    Si2173_L1_ATV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2173_ATV_STATUS_CMD_INTACK_OK );
    Si2173_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2173_ATV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2173_COMPATIBLE */
   #ifdef    Si2176_COMPATIBLE
    Si2176_L1_ATV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2176_ATV_STATUS_CMD_INTACK_OK );
    Si2176_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2176_ATV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2176_COMPATIBLE */
   #ifdef    Si2178_COMPATIBLE
    Si2178_L1_ATV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2178_ATV_STATUS_CMD_INTACK_OK );
    Si2178_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2178_ATV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2178_COMPATIBLE */
   #ifdef    Si2196_COMPATIBLE
    Si2196_L1_ATV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2196_ATV_STATUS_CMD_INTACK_OK );
    Si2196_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2196_ATV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2196_COMPATIBLE */
    return 1;
  }
#endif /* Si2168_COMPATIBLE */
  return -1;
}
int   SiLabs_API_TER_Tuner_DTV_Text_status(SILABS_FE_Context *front_end, char *separator, char *msg)
{
  front_end = front_end; /* To avoid compiler warning */
  separator = separator; /* To avoid compiler warning */
  msg       = msg;       /* To avoid compiler warning */
#ifdef    TUNERTER_API
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
   #ifdef    Si2146_COMPATIBLE
    Si2146_L1_DTV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2146_DTV_STATUS_CMD_INTACK_OK );
    Si2146_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2146_DTV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2146_COMPATIBLE */
   #ifdef    Si2148_COMPATIBLE
    Si2148_L1_DTV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2148_DTV_STATUS_CMD_INTACK_OK );
    Si2148_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2148_DTV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2148_COMPATIBLE */
   #ifdef    Si2156_COMPATIBLE
    Si2156_L1_DTV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2156_DTV_STATUS_CMD_INTACK_OK );
    Si2156_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2156_DTV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2156_COMPATIBLE */
   #ifdef    Si2158_COMPATIBLE
    Si2158_L1_DTV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2158_DTV_STATUS_CMD_INTACK_OK );
    Si2158_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2158_DTV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2158_COMPATIBLE */
   #ifdef    Si2173_COMPATIBLE
    Si2173_L1_DTV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2173_DTV_STATUS_CMD_INTACK_OK );
    Si2173_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2173_DTV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2173_COMPATIBLE */
   #ifdef    Si2176_COMPATIBLE
    Si2176_L1_DTV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2176_DTV_STATUS_CMD_INTACK_OK );
    Si2176_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2176_DTV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2176_COMPATIBLE */
   #ifdef    Si2178_COMPATIBLE
    Si2178_L1_DTV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2178_DTV_STATUS_CMD_INTACK_OK );
    Si2178_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2178_DTV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2178_COMPATIBLE */
   #ifdef    Si2196_COMPATIBLE
    Si2196_L1_DTV_STATUS              (front_end->Si2168_FE->tuner_ter, Si2196_DTV_STATUS_CMD_INTACK_OK );
    Si2196_L1_GetCommandResponseString(front_end->Si2168_FE->tuner_ter, Si2196_DTV_STATUS_CMD_CODE, separator, msg);
   #endif /* Si2196_COMPATIBLE */
    return 1;
  }
#endif /* Si2168_COMPATIBLE */
#endif /* TUNERTER_API */
  return -1;
}
int   SiLabs_API_TER_Tuner_ATV_Tune       (SILABS_FE_Context *front_end)
{
  unsigned long freq;
  unsigned char video_sys;
  unsigned char trans;
  unsigned char color;
  unsigned char invert_signal;
  unsigned char invert_spectrum;

  freq            = front_end->Carrier_Cfg.freq;
  video_sys       = Silabs_videoSysCode    (front_end, front_end->Analog_Cfg.video_sys);
  trans           = Silabs_transmissionCode(front_end, front_end->Analog_Cfg.trans);
  color           = Silabs_colorCode       (front_end, front_end->Analog_Cfg.color);
  invert_signal   = front_end->Analog_Cfg.invert_signal;
  invert_spectrum = front_end->Analog_Cfg.invert_spectrum;

  /* Using Center frequency for ATV */
  switch (video_sys) {
    case SILABS_VIDEO_SYS_B  :
      freq += 2250000;
      break;
    case SILABS_VIDEO_SYS_M  :
    case SILABS_VIDEO_SYS_N  :
      freq += 1750000;
      break;
    case SILABS_VIDEO_SYS_GH :
    case SILABS_VIDEO_SYS_I  :
    case SILABS_VIDEO_SYS_DK :
    case SILABS_VIDEO_SYS_L  :
      freq += 2750000;
      break;
    case SILABS_VIDEO_SYS_LP :
      freq -= 2750000;
      break;
    default: return -1;
  }
  /* Center frequency end */

  SiTRACE("freq %ld, video_sys %d, trans %d\n", freq, video_sys, trans);
  SiLabs_API_Tuner_I2C_Enable (front_end);
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
   #ifdef    Si2176_COMPATIBLE
    Si2176_ATVTune  (front_end->Si2168_FE->tuner_ter, freq, video_sys, trans, color, invert_signal);
   #endif /* Si2176_COMPATIBLE */
   #ifdef    Si2178_COMPATIBLE
    Si2178_ATVTune  (front_end->Si2168_FE->tuner_ter, freq, video_sys, color, invert_signal);
   #endif /* Si2178_COMPATIBLE */
   #ifdef    Si2196_COMPATIBLE
    Si2196_ATVTune  (front_end->Si2168_FE->tuner_ter, freq, video_sys, trans, color, invert_signal);
   #endif /* Si2196_COMPATIBLE */
    return 1;
  }
#endif /* Si2168_COMPATIBLE */
  SiLabs_API_Tuner_I2C_Disable(front_end);
  return -1;
}
int   SiLabs_API_TER_Tuner_Block_VCO      (SILABS_FE_Context *front_end, int vco_code)
{
#ifndef   TUNERTER_API
  front_end = front_end; /* To avoid compiler warning */
  vco_code  = vco_code;  /* To avoid compiler warning */
#else  /* TUNERTER_API */
  SiLabs_API_Tuner_I2C_Enable (front_end);
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
   front_end->Si2168_FE->tuner_ter->prop->tuner_blocked_vco.vco_code = vco_code;
   #ifdef    Si2146_COMPATIBLE
    Si2146_L1_SetProperty2(front_end->Si2168_FE->tuner_ter, Si2146_TUNER_BLOCKED_VCO_PROP);
   #endif /* Si2146_COMPATIBLE */
   #ifdef    Si2148_COMPATIBLE
    Si2148_L1_SetProperty2(front_end->Si2168_FE->tuner_ter, Si2148_TUNER_BLOCKED_VCO_PROP);
   #endif /* Si2148_COMPATIBLE */
   #ifdef    Si2156_COMPATIBLE
    Si2156_L1_SetProperty2(front_end->Si2168_FE->tuner_ter, Si2156_TUNER_BLOCKED_VCO_PROP);
   #endif /* Si2156_COMPATIBLE */
   #ifdef    Si2158_COMPATIBLE
    Si2158_L1_SetProperty2(front_end->Si2168_FE->tuner_ter, Si2158_TUNER_BLOCKED_VCO_PROP);
   #endif /* Si2158_COMPATIBLE */
   #ifdef    Si2173_COMPATIBLE
    Si2173_L1_SetProperty2(front_end->Si2168_FE->tuner_ter, Si2173_TUNER_BLOCKED_VCO_PROP);
   #endif /* Si2173_COMPATIBLE */
   #ifdef    Si2176_COMPATIBLE
    Si2176_L1_SetProperty2(front_end->Si2168_FE->tuner_ter, Si2176_TUNER_BLOCKED_VCO_PROP);
   #endif /* Si2176_COMPATIBLE */
   #ifdef    Si2178_COMPATIBLE
    Si2178_L1_SetProperty2(front_end->Si2168_FE->tuner_ter, Si2178_TUNER_BLOCKED_VCO_PROP);
   #endif /* Si2178_COMPATIBLE */
   #ifdef    Si2196_COMPATIBLE
    Si2196_L1_SetProperty2(front_end->Si2168_FE->tuner_ter, Si2196_TUNER_BLOCKED_VCO_PROP);
   #endif /* Si2196_COMPATIBLE */
  }
#endif /* Si2168_COMPATIBLE */
  SiLabs_API_Tuner_I2C_Disable(front_end);
#endif /* TUNERTER_API */
  return 1;
}
/************************************************************************************************************************
  NAME: SiLabs_API_Demod_reset
  DESCRIPTION: demodulator reset function
  Parameter:  Pointer to SILABS_FE_Context
  Returns:    void
************************************************************************************************************************/
void  SiLabs_API_Demod_reset              (SILABS_FE_Context *front_end)
{
  #ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) {
    if (Si2168_L1_DD_RESTART  (front_end->Si2168_FE->demod)!=0) {
      SiTRACE("Demod reset failed!\n");
    }
  }
  #endif /* Si2168_COMPATIBLE */
}
#ifdef    SILABS_API_TEST_PIPE
/************************************************************************************************************************
  Silabs_API_Test function
  Use:        Generic test pipe function
              Used to send a generic command to the selected target.
  Returns:    0 if the command is unknow, 1 otherwise
  Porting:    Mostly used for debug during sw development.
************************************************************************************************************************/
int   Silabs_API_Test                     (SILABS_FE_Context *front_end, char *target, char *cmd, char *sub_cmd, int dval, int *retdval, char **rettxt)
{
  int res;
  front_end = front_end; /* To avoid compiler warning if not used */
  target    = target;    /* To avoid compiler warning if not used */
  cmd       = cmd;       /* To avoid compiler warning if not used */
  sub_cmd   = sub_cmd;   /* To avoid compiler warning if not used */
  dval      = dval;      /* To avoid compiler warning if not used */
  retdval   = retdval;   /* To avoid compiler warning if not used */
  rettxt    = rettxt;    /* To avoid compiler warning if not used */
  res = 0;
#ifdef    USB_Capability
  if (strcmp_nocase(target,"cypress") == 0) {
    if (strcmp_nocase(cmd ,"ts"       ) == 0) { L0_Cypress_Process  (cmd, sub_cmd, dval, retdval, rettxt); }
    if (strcmp_nocase(cmd ,"configure") == 0) { L0_Cypress_Configure(cmd, sub_cmd, dval, retdval, rettxt); }
    if (strcmp_nocase(cmd ,"cget"     ) == 0) { L0_Cypress_Cget     (cmd, sub_cmd, dval, retdval, rettxt); }
    return 1;
  }
#endif /* USB_Capability */
#ifdef    Si2168_COMPATIBLE
  if (front_end->chip ==   2168 ) { Si2168_L2_Test(front_end->Si2168_FE, target, cmd, sub_cmd, dval, retdval, rettxt); return 1;}
#endif /* Si2168_COMPATIBLE */
  sprintf(*rettxt, "no command implemented so far for '%s', can not process '%s %s %s %f'\n", cmd, target, cmd, sub_cmd, dval);

  return 0;
}
#endif /* SILABS_API_TEST_PIPE */
/************************************************************************************************************************
  NAME: SiLabs_API_TAG_TEXT
  DESCRIPTION: SiLabs API information function used to retrieve the version information of the SiLabs API wrapper
  Returns:    the SiLabs API version information string
************************************************************************************************************************/
char* SiLabs_API_TAG_TEXT      (void) { return (char *)"V2.0.6"; }

#ifdef    __cplusplus
}
#endif /* __cplusplus */
