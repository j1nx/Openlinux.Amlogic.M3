#ifndef  _SiLabs_API_L3_Wrapper_H_
#define  _SiLabs_API_L3_Wrapper_H_


#ifdef __cplusplus
extern "C" {
#endif

#define DEMOD_Si2168
//#define TER_TUNER_CUSTOMTER


/* <porting> Project customization: chips selection                                                    */
/* At project level, define the DEMOD_Si21xx value corresponding to the required digital demodulator   */
#ifdef    DEMOD_Si2165
  #define       Si2165_COMPATIBLE
#endif /* DEMOD_Si2165 */
#ifdef    DEMOD_Si2167
  #define       Si2167_COMPATIBLE
  #define       DEMOD_DVB_S_S2_DSS
#endif /* DEMOD_Si2167 */
#ifdef    DEMOD_Si2167B
  #define       Si2167B_COMPATIBLE
#ifndef   NO_TER
#endif /* NO_TER */
  #define       DEMOD_DVB_S_S2_DSS
#endif /* DEMOD_Si2167B */
#ifdef    DEMOD_Si2168
  #define       Si2168_COMPATIBLE
#ifndef   NO_SAT
  #define       DEMOD_DVB_S_S2_DSS
#endif /* NO_SAT */
#endif /* DEMOD_Si2168 */
#ifdef    DEMOD_Si2185
  #define       Si2185_COMPATIBLE
  #define       TER_TUNER_Si2185
  #define       Si2185_COMMAND_PROTOTYPES
#endif /* DEMOD_Si2185 */


 /* At project level, define the TER_TUNER_Si21xx value corresponding to the required terrestrial tuner */
#ifdef    TER_TUNER_CUSTOMTER
  #define           CUSTOMTER_COMPATIBLE
  #include         "SiLabs_L1_RF_CUSTOMTER_API.h"
#endif /* TER_TUNER_CUSTOMTER */

#ifdef    TER_TUNER_Si2146
  #define           Si2146_COMPATIBLE
  #define           Si2146_COMMAND_PROTOTYPES
  #include         "Si2146_L2_API.h"
#endif /* TER_TUNER_Si2146 */

#ifdef    TER_TUNER_Si2148
  #define           Si2148_COMPATIBLE
  #define           Si2148_COMMAND_PROTOTYPES
  #include         "Si2148_L2_API.h"
#endif /* TER_TUNER_Si2148 */

#ifdef    TER_TUNER_Si2156
  #define           Si2156_COMPATIBLE
  #define           Si2156_COMMAND_PROTOTYPES
  #include         "Si2156_L2_API.h"
#endif /* TER_TUNER_Si2156 */

#ifdef    TER_TUNER_Si2158
  #define           Si2158_COMPATIBLE
  #define           Si2158_COMMAND_PROTOTYPES
  #include         "Si2158_L2_API.h"
#endif /* TER_TUNER_Si2158 */

#ifdef    TER_TUNER_Si2173
  #define           Si2173_COMPATIBLE
  #define           Si2173_COMMAND_PROTOTYPES
  #include         "Si2173_L2_API.h"
#endif /* TER_TUNER_Si2173 */

#ifdef    TER_TUNER_Si2176
  #define           Si2176_COMPATIBLE
  #define           Si2176_COMMAND_PROTOTYPES
  #include         "Si2176_L2_API.h"
#endif /* TER_TUNER_Si2176 */

#ifdef    TER_TUNER_Si2178
  #define           Si2178_COMPATIBLE
  #define           Si2178_COMMAND_PROTOTYPES
  #include         "Si2178_L2_API.h"
#endif /* TER_TUNER_Si2178 */

#ifdef    TER_TUNER_Si2190
  #define           Si2190_COMPATIBLE
  #define           Si2190_COMMAND_PROTOTYPES
  #include         "Si2190_L2_API.h"
#endif /* TER_TUNER_Si2190 */

#ifdef    TER_TUNER_Si2196
  #define           Si2196_COMPATIBLE
  #define           Si2196_COMMAND_PROTOTYPES
  #include         "Si2196_L2_API.h"
#endif /* TER_TUNER_Si2196 */



/* Loading the digital demodulator API based on the project-level selection */
#ifdef      Si2168_COMPATIBLE
  #define   Si2168_COMMAND_PROTOTYPES
  #include "Si2168_L2_API.h"
#endif   /* Si2168_COMPATIBLE */

#ifndef   DEMOD_Si2165
 #ifndef   DEMOD_Si2167
  #ifndef   DEMOD_Si2167B
   #ifndef   DEMOD_Si2168
    #ifndef   DEMOD_Si2185
    "If you get a compilation error on this line, it means that no digital demod has been selected. Please define DEMOD_Si2165, DEMOD_Si2167, DEMOD_Si2167B, DEMOD_Si2168, DEMOD_Si2185 at project level!";
    #endif /* DEMOD_Si2185 */
   #endif /* DEMOD_Si2168 */
  #endif /* DEMOD_Si2167B */
 #endif /* DEMOD_Si2167 */
#endif /* DEMOD_Si2165 */

#ifndef   TER_TUNER_CUSTOMTER
 #ifndef   TER_TUNER_DTT759x
  #ifndef   TER_TUNER_Si2146
   #ifndef   TER_TUNER_Si2148
    #ifndef   TER_TUNER_Si2156
     #ifndef   TER_TUNER_Si2158
      #ifndef   TER_TUNER_Si2173
       #ifndef   TER_TUNER_Si2176
        #ifndef   TER_TUNER_Si2178
         #ifndef   TER_TUNER_Si2185
          #ifndef   TER_TUNER_Si2190
           #ifndef   TER_TUNER_Si2196
            #ifndef   TER_TUNER_NO_TER
  //   "If you get a compilation error on this line, it means that no terrestrial tuner has been selected. Please define TER_TUNER_Sixxxx at project-level!";
            #endif /* TER_TUNER_NO_TER */
           #endif /* TER_TUNER_Si2196 */
          #endif /* TER_TUNER_Si2190 */
         #endif /* TER_TUNER_Si2185 */
        #endif /* TER_TUNER_Si2178 */
       #endif /* TER_TUNER_Si2176 */
      #endif /* TER_TUNER_Si2173 */
     #endif /* TER_TUNER_Si2158 */
    #endif /* TER_TUNER_Si2156 */
   #endif /* TER_TUNER_Si2148 */
  #endif /* TER_TUNER_Si2146 */
 #endif /* TER_TUNER_DTT759x */
#endif /* TER_TUNER_CUSTOMTER*/


/* Standard code values used by the top-level application               */
/* <porting> set these values to match the top-level application values */
typedef enum   CUSTOM_Standard_Enum          {
  SILABS_ANALOG = 4,
  SILABS_DVB_T  = 0,
  SILABS_DVB_C  = 1,
  SILABS_DVB_S  = 2,
  SILABS_DVB_S2 = 3,
  SILABS_DVB_T2 = 5,
  SILABS_DSS    = 6,
  SILABS_MCNS   = 7,
  SILABS_DVB_C2 = 8,
  SILABS_SLEEP  = 100,
} CUSTOM_Standard_Enum;

typedef enum   CUSTOM_Constel_Enum           {
  SILABS_QAMAUTO = -1,
  SILABS_QAM16   = 0,
  SILABS_QAM32   = 1,
  SILABS_QAM64   = 2,
  SILABS_QAM128  = 3,
  SILABS_QAM256  = 4,
  SILABS_QPSK    = 5,
  SILABS_8PSK    = 6
} CUSTOM_Constel_Enum;

typedef enum   CUSTOM_Stream_Enum            {
  SILABS_HP = 0,
  SILABS_LP = 1
} CUSTOM_Stream_Enum;

typedef enum   CUSTOM_T2_PLP_TYPE            {
  SILABS_PLP_TYPE_COMMON     = 0,
  SILABS_PLP_TYPE_DATA_TYPE1 = 1,
  SILABS_PLP_TYPE_DATA_TYPE2 = 2,
} CUSTOM_T2_PLP_TYPE;

typedef enum   CUSTOM_TS_Mode_Enum           {
  SILABS_TS_TRISTATE = 0,
  SILABS_TS_SERIAL   = 1,
  SILABS_TS_PARALLEL = 2,
  SILABS_TS_GPIF     = 3
} CUSTOM_TS_Mode_Enum;

typedef enum   CUSTOM_FFT_Mode_Enum          {
  SILABS_FFT_MODE_2K  = 0,
  SILABS_FFT_MODE_4K  = 1,
  SILABS_FFT_MODE_8K  = 2,
  SILABS_FFT_MODE_16K = 3,
  SILABS_FFT_MODE_32K = 4
} CUSTOM_FFT_Mode_Enum;

typedef enum   CUSTOM_GI_Enum                {
  SILABS_GUARD_INTERVAL_1_32    = 0,
  SILABS_GUARD_INTERVAL_1_16    = 1,
  SILABS_GUARD_INTERVAL_1_8     = 2,
  SILABS_GUARD_INTERVAL_1_4     = 3,
  SILABS_GUARD_INTERVAL_1_128   = 4,
  SILABS_GUARD_INTERVAL_19_128  = 5,
  SILABS_GUARD_INTERVAL_19_256  = 6
} CUSTOM_GI_Enum;

typedef enum   CUSTOM_Coderate_Enum          {
  SILABS_CODERATE_1_2  = 0,
  SILABS_CODERATE_2_3  = 1,
  SILABS_CODERATE_3_4  = 2,
  SILABS_CODERATE_4_5  = 3,
  SILABS_CODERATE_5_6  = 4,
  SILABS_CODERATE_7_8  = 5,
  SILABS_CODERATE_8_9  = 6,
  SILABS_CODERATE_9_10 = 7,
  SILABS_CODERATE_1_3  = 8,
  SILABS_CODERATE_1_4  = 9,
  SILABS_CODERATE_2_5  = 10,
  SILABS_CODERATE_3_5  = 11,
} CUSTOM_Coderate_Enum;

typedef enum   CUSTOM_Polarization_Enum      {
  SILABS_POLARIZATION_HORIZONTAL  = 0,
  SILABS_POLARIZATION_VERTICAL    = 1,

} CUSTOM_Polarization_Enum;

typedef enum   CUSTOM_Band_Enum              {
  SILABS_BAND_LOW  = 0,
  SILABS_BAND_HIGH = 1,

} CUSTOM_Band_Enum;

typedef enum   CUSTOM_Hierarchy_Enum         {
  SILABS_HIERARCHY_NONE  = 0,
  SILABS_HIERARCHY_ALFA1 = 1,
  SILABS_HIERARCHY_ALFA2 = 2,
  SILABS_HIERARCHY_ALFA4 = 3
} CUSTOM_Hierarchy_Enum;

typedef enum   CUSTOM_Video_Sys_Enum         {
  SILABS_VIDEO_SYS_B  = 0,
  SILABS_VIDEO_SYS_GH = 1,
  SILABS_VIDEO_SYS_M  = 2,
  SILABS_VIDEO_SYS_N  = 3,
  SILABS_VIDEO_SYS_I  = 4,
  SILABS_VIDEO_SYS_DK = 5,
  SILABS_VIDEO_SYS_L  = 6,
  SILABS_VIDEO_SYS_LP = 7
} CUSTOM_Video_Sys_Enum;

typedef enum   CUSTOM_Transmission_Mode_Enum {
  SILABS_TRANSMISSION_MODE_TERRESTRIAL = 0,
  SILABS_TRANSMISSION_MODE_CABLE       = 1
} CUSTOM_Transmission_Mode_Enum;

typedef enum   CUSTOM_Pilot_Pattern_Enum     {
  SILABS_PILOT_PATTERN_PP1 = 1,
  SILABS_PILOT_PATTERN_PP2 = 2,
  SILABS_PILOT_PATTERN_PP3 = 3,
  SILABS_PILOT_PATTERN_PP4 = 4,
  SILABS_PILOT_PATTERN_PP5 = 5,
  SILABS_PILOT_PATTERN_PP6 = 6,
  SILABS_PILOT_PATTERN_PP7 = 7,
  SILABS_PILOT_PATTERN_PP8 = 8,
} CUSTOM_Pilot_Pattern_Enum;

typedef enum   CUSTOM_Color_Enum             {
  SILABS_COLOR_PAL_NTSC  = 0,
  SILABS_COLOR_SECAM     = 1
} CUSTOM_Color_Enum;

typedef enum   CUSTOM_Audio_Sys_Enum         {
  SILABS_AUDIO_SYS_DEFAULT         = 0,
  SILABS_AUDIO_SYS_MONO            = 1,
  SILABS_AUDIO_SYS_MONO_NICAM      = 2,
  SILABS_AUDIO_SYS_A2              = 3,
  SILABS_AUDIO_SYS_A2_DK2          = 4,
  SILABS_AUDIO_SYS_A2_DK3          = 5,
  SILABS_AUDIO_SYS_BTSC            = 6,
  SILABS_AUDIO_SYS_EIAJ            = 7,
  SILABS_AUDIO_SYS_SCAN            = 8,
  SILABS_AUDIO_SYS_A2_DK4          = 9,
  SILABS_AUDIO_SYS_WIDE_SCAN       = 10,
  SILABS_AUDIO_SYS_MONO_NICAM_6DB  = 11,
  SILABS_AUDIO_SYS_MONO_NICAM_10DB = 12
} CUSTOM_Audio_Sys_Enum;

typedef struct SILABS_TER_TUNER_Config       {
  unsigned char nSel_dtv_out_type;
  unsigned char nSel_dtv_agc_source;
  int           nSel_dtv_lif_freq_offset;
  unsigned char nSel_dtv_mode_bw;
  unsigned char nSel_dtv_mode_invert_spectrum;
  unsigned char nSel_dtv_mode_modulation;
  unsigned char nSel_atv_video_mode_video_sys;
  unsigned char nSel_atv_audio_mode_audio_sys;
  unsigned char nSel_atv_atv_video_mode_tran;
  unsigned char nSel_atv_video_mod_color;
  unsigned char nSel_atv_mode_invert_spectrum;
  unsigned char nSel_atv_mode_invert_signal;
  char          nSel_atv_cvbs_out_fine_amp;
  char          nSel_atv_cvbs_out_fine_offset;
  unsigned char nSel_atv_sif_out_amp;
  unsigned char nSel_atv_sif_out_offset;
  unsigned char if_agc_speed;
  char          nSel_dtv_rf_top;
  char          nSel_atv_rf_top;
  unsigned long nLocked_Freq;
  unsigned long nCenter_Freq;
  int           nCriteriaStep;
  int           nLeftMax;
  int           nRightMax;
  int           nReal_Step;
  int           nBeforeStep;
} SILABS_TER_TUNER_Config;

typedef struct SILABS_SAT_TUNER_Config       {
    int    RF;
    int    IF;
    int    minRF;
    int    maxRF;
    int xtal;
    int LPF;
    int    tunerBytesCount;
    int    I2CMuxChannel;
    int RefDiv_value;
    int    Mash_Per;
    CONNECTION_TYPE connType;
    unsigned char tuner_log[40];
    unsigned char tuner_read[7];
    char          nSel_att_top;
} SILABS_SAT_TUNER_Config;

typedef struct SILABS_CARRIER_Config         {
  int                freq;
  int                bandwidth;
  int                stream;
  int                symbol_rate;
  int                constellation;
  int                polarization;
  int                band;
} SILABS_CARRIER_Config;

typedef struct SILABS_ANALOG_CARRIER_Config  {
  unsigned char      video_sys;
  unsigned char      trans;
  unsigned char      color;
  unsigned char      invert_signal;
  unsigned char      invert_spectrum;
} SILABS_ANALOG_CARRIER_Config;

typedef struct SILABS_ANALOG_SIF_Config      {
  unsigned char      stereo_sys;
} SILABS_ANALOG_SIF_Config;


typedef struct SILABS_FE_Context             {
  unsigned int       chip;
  unsigned int       tuner_ter;
  unsigned int       tuner_sat;
#ifdef    Si2168_COMPATIBLE
  struct Si2168_L2_Context  *Si2168_FE;
  struct Si2168_L2_Context   *Si2168_FE_Obj;//rsj  Si2168_FE_Obj
#endif /* Si2168_COMPATIBLE */
  int                standard;
  int                init_ok;
  int                polarization;
  int                band;
  SILABS_TER_TUNER_Config      TER_Tuner_Cfg;
  SILABS_SAT_TUNER_Config      SAT_Tuner_Cfg;
  SILABS_CARRIER_Config        Carrier_Cfg;
  SILABS_ANALOG_CARRIER_Config Analog_Cfg;
  SILABS_ANALOG_SIF_Config     Analog_Sif_Cfg;
} SILABS_FE_Context;

typedef struct CUSTOM_Status_Struct          {
  CUSTOM_Standard_Enum standard;
  int      clock_mode;
  int      demod_lock;
  int      fec_lock;
  int      fec_lock_in_range;
  int   c_n;
  int   ber;
  int   per;
  int   fer;
  int      uncorrs;
  int      RFagc;
  int      IFagc;
  int      RFlevel;
  long     freq_offset;
  long     timing_offset;
  int      bandwidth_Hz;
  int      stream;
  int      fft_mode;
  int      guard_interval;
  int      constellation;
  unsigned int symbol_rate;
  int      code_rate_hp;
  int      code_rate_lp;
  int      hierarchy;
  int      spectral_inversion;
  int      code_rate;
  int      pilots;
  int      cell_id;
  int    RSSI;
  int      SSI;
  int      SQI;
  int      tuner_lock;
  int      rotated;
  int      pilot_pattern;
  int      bw_ext;
  int      TS_bitrate_kHz;
  int      TS_clock_kHz;
  /*        T2/C2 specifics (PLP) */
  int      num_plp;
  int      plp_id;
  /* End of T2/C2 specifics (PLP) */
  int      tx_mode;
  int      short_frame;
  unsigned char attctrl;
  /* TUNER_STATUS */
  unsigned char   tcint;
  unsigned char   rssilint;
  unsigned char   rssihint;
           int    vco_code;
  unsigned char   tc;
  unsigned char   rssil;
  unsigned char   rssih;
           char   rssi;
  unsigned long   freq;
  unsigned char   mode;
  /* ATV_STATUS & DTV_STATUS */
  unsigned char   chl;
  /* ATV_STATUS */
  int      ATV_Sync_Lock;
  int      ATV_Master_Lock;
  unsigned char   audio_chan_filt_bw;
  unsigned char   snrl;
  unsigned char   snrh;
  unsigned char   video_snr;
           int    afc_freq;
           int    video_sc_spacing;
  unsigned char   video_sys;
  unsigned char   color;
  unsigned char   trans;
  unsigned char   audio_sys;
  unsigned char   audio_demod_mode;
  /* DTV_STATUS */
  unsigned char   chlint;
  unsigned char   bw;
  unsigned char   modulation;
  unsigned char   fef;
  /* MCNS STATUS */
  unsigned char   interleaving;
} CUSTOM_Status_Struct;

/* Translation functions from 'Custom' values to 'SiLabs' values */

int   Silabs_standardCode                 (SILABS_FE_Context *front_end, CUSTOM_Standard_Enum          standard);
int   Silabs_constelCode                  (SILABS_FE_Context *front_end, CUSTOM_Constel_Enum           constel);
int   Silabs_streamCode                   (SILABS_FE_Context *front_end, CUSTOM_Stream_Enum            stream);
int   Silabs_plptypeCode                  (SILABS_FE_Context *front_end, CUSTOM_T2_PLP_TYPE            plp_type);
int   Silabs_fftCode                      (SILABS_FE_Context *front_end, CUSTOM_FFT_Mode_Enum          fft);
int   Silabs_giCode                       (SILABS_FE_Context *front_end, CUSTOM_GI_Enum                gi);
int   Silabs_coderateCode                 (SILABS_FE_Context *front_end, CUSTOM_Coderate_Enum          coderate);
int   Silabs_hierarchyCode                (SILABS_FE_Context *front_end, CUSTOM_Hierarchy_Enum         hierarchy);
int   Silabs_pilotPatternCode             (SILABS_FE_Context *front_end, CUSTOM_Pilot_Pattern_Enum     pilot_pattern);
int   Silabs_videoSysCode                 (SILABS_FE_Context *front_end, CUSTOM_Video_Sys_Enum         video_sys);
int   Silabs_colorCode                    (SILABS_FE_Context *front_end, CUSTOM_Color_Enum             color);
int   Silabs_transmissionCode             (SILABS_FE_Context *front_end, CUSTOM_Transmission_Mode_Enum trans);

/* Translation functions from 'SiLabs' values to 'Custom' values */

int   Custom_standardCode                 (SILABS_FE_Context *front_end, int standard);
int   Custom_constelCode                  (SILABS_FE_Context *front_end, int constel);
int   Custom_streamCode                   (SILABS_FE_Context *front_end, int stream);
int   Custom_plptypeCode                  (SILABS_FE_Context *front_end, int plp_type);
int   Custom_fftCode                      (SILABS_FE_Context *front_end, int fft);
int   Custom_giCode                       (SILABS_FE_Context *front_end, int gi);
int   Custom_coderateCode                 (SILABS_FE_Context *front_end, int coderate);
int   Custom_hierarchyCode                (SILABS_FE_Context *front_end, int hierarchy);
int   Custom_pilotPatternCode             (SILABS_FE_Context *front_end, int pilot_pattern);

/* Text functions returning strings based on 'Custom' values. */

char *Silabs_Standard_Text                (CUSTOM_Standard_Enum     standard);
int   Silabs_Standard_Capability          (CUSTOM_Standard_Enum     standard);
char *Silabs_Constel_Text                 (CUSTOM_Constel_Enum      constel);
char *Silabs_Stream_Text                  (CUSTOM_Stream_Enum       stream);
char *Silabs_PLPType_Text                 (CUSTOM_T2_PLP_TYPE       plp_type);
char *Silabs_FFT_Text                     (CUSTOM_FFT_Mode_Enum     fft);
char *Silabs_GI_Text                      (CUSTOM_GI_Enum           gi);
char *Silabs_Coderate_Text                (CUSTOM_Coderate_Enum     coderate);
char *Silabs_Polarization_Text            (CUSTOM_Polarization_Enum polarization);
char *Silabs_Band_Text                    (CUSTOM_Band_Enum         band);
char *Silabs_Extended_BW_Text             (int bw_extended);
char *Silabs_Rotated_QAM_Text             (int rotated);
char *Silabs_FEF_Text                     (int fef);
char *Silabs_MISO_Text                    (int siso_miso);
char *Silabs_Pilot_Pattern_Text           (CUSTOM_Pilot_Pattern_Enum pilot_pattern);
char *Silabs_No_Short_Frame_Text          (int no_short_frame);
char *Silabs_Pilots_Text                  (int pilots);

/* Chip detection function (To Be Defined) */

int   SiLabs_chip_detect                  (int demodAdd);

/* Internal functions                      */

int   SiLabs_API_Demod_status             (SILABS_FE_Context *front_end, CUSTOM_Status_Struct *status);
int   SiLabs_API_TER_Tuner_status         (SILABS_FE_Context *front_end, CUSTOM_Status_Struct *status);
int   SiLabs_API_SAT_Tuner_status         (SILABS_FE_Context *front_end, CUSTOM_Status_Struct *status);
int   SiLabs_API_FE_status                (SILABS_FE_Context *front_end, CUSTOM_Status_Struct *status);
int   SiLabs_API_Text_status              (SILABS_FE_Context *front_end, CUSTOM_Status_Struct *status, char *formatted_status);

/*****************************************************************************************/
/*               SiLabs demodulator API functions (demod and tuner)                      */
/*****************************************************************************************/

char  SiLabs_API_SW_Init                  (SILABS_FE_Context *front_end, int demodAdd, int tunerAdd_Ter, int tunerAdd_Sat);
int   SiLabs_API_Infos                    (SILABS_FE_Context *front_end, char *infoString);
char  SiLabs_API_HW_Connect               (SILABS_FE_Context *front_end, CONNECTION_TYPE connection_mode);
char  SiLabs_API_bytes_trace              (SILABS_FE_Context *front_end, unsigned char track_mode);
int   SiLabs_API_ReadString               (SILABS_FE_Context *front_end, char *readString, unsigned char *pbtDataBuffer);
char  SiLabs_API_WriteString              (SILABS_FE_Context *front_end, char *writeString);
int   SiLabs_API_communication_check      (SILABS_FE_Context *front_end);
int   SiLabs_API_switch_to_standard       (SILABS_FE_Context *front_end, int  standard, unsigned char force_full_init);
int   SiLabs_API_set_standard             (SILABS_FE_Context *front_end, int  standard);
int   SiLabs_API_lock_to_carrier          (SILABS_FE_Context *front_end, int  standard, int  freq, int  bandwidth_Hz, int  stream, unsigned int  symbol_rate_bps, int  constellation, int  polarization, int  band, int plp_id);
int   SiLabs_API_Tune                     (SILABS_FE_Context *front_end, int  freq);
int   SiLabs_API_Channel_Lock_Abort       (SILABS_FE_Context *front_end);
int   SiLabs_API_Demod_Standby            (SILABS_FE_Context *front_end);
int   SiLabs_API_Demod_ClockOn            (SILABS_FE_Context *front_end);
int   SiLabs_API_Reset_Uncorrs            (SILABS_FE_Context *front_end);
int   SiLabs_API_TS_Mode                  (SILABS_FE_Context *front_end, int ts_mode);
int   SiLabs_API_Tuner_I2C_Enable         (SILABS_FE_Context *front_end);
int   SiLabs_API_Tuner_I2C_Disable        (SILABS_FE_Context *front_end);
int   SiLabs_API_Channel_Seek_Init        (SILABS_FE_Context *front_end,
                                            int rangeMin,     int rangeMax,
                                            int seekBWHz,     int seekStepHz,
                                            int minSRbps,     int maxSRbps,
                                            int minRSSIdBm,   int maxRSSIdBm,
                                            int minSNRHalfdB, int maxSNRHalfdB);
int   SiLabs_API_Channel_Seek_Next        (SILABS_FE_Context *front_end, int *standard, int *freq, int *bandwidth_Hz, int *stream, unsigned int *symbol_rate_bps, int *constellation, int *polarization, int *band, int *num_plp);
int   SiLabs_API_Channel_Seek_Abort       (SILABS_FE_Context *front_end);
int   SiLabs_API_Channel_Seek_End         (SILABS_FE_Context *front_end);
int   SiLabs_API_Get_PLP_ID_and_TYPE      (SILABS_FE_Context *front_end, int  plp_index, int *plp_id, int *plp_type);
int   SiLabs_API_Select_PLP               (SILABS_FE_Context *front_end, int  plp_id);
int   SiLabs_API_TER_AutoDetect           (SILABS_FE_Context *front_end, int  on_off);
int   SiLabs_API_TER_Tuner_Fine_Tune      (SILABS_FE_Context *front_end, int  offset_500hz);
int   SiLabs_API_TER_Tuner_Init           (SILABS_FE_Context *front_end);
int   SiLabs_API_TER_Tuner_SetMFT         (SILABS_FE_Context *front_end, int   nStep);
int   SiLabs_API_TER_Tuner_Text_status    (SILABS_FE_Context *front_end, char *separator, char *msg);
int   SiLabs_API_TER_Tuner_ATV_Text_status(SILABS_FE_Context *front_end, char *separator, char *msg);
int   SiLabs_API_TER_Tuner_DTV_Text_status(SILABS_FE_Context *front_end, char *separator, char *msg);
int   SiLabs_API_TER_Tuner_ATV_Tune       (SILABS_FE_Context *front_end);
int   SiLabs_API_TER_Tuner_Block_VCO      (SILABS_FE_Context *front_end, int vco_code);
void  SiLabs_API_Demod_reset              (SILABS_FE_Context *front_end);
#ifdef    SILABS_API_TEST_PIPE
int   Silabs_API_Test                     (SILABS_FE_Context *front_end, char *target, char *cmd, char *sub_cmd, int dval, int *retdval, char **rettxt);
#endif /* SILABS_API_TEST_PIPE */

char* SiLabs_API_TAG_TEXT      (void);

#if defined( __cplusplus )
}
#endif

#endif /* _SiLabs_API_L3_Wrapper_H_ */

