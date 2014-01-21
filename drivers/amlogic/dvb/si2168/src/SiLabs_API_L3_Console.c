/*************************************************************************************************************/
/*                                  Silicon Laboratories                                                     */
/*                                  Broadcast Video Group                                                    */
/*                              SiLabs API Console Functions                                                 */
/*-----------------------------------------------------------------------------------------------------------*/
/*   This source code contains all console functions for a SiLabs digital TV demodulator Evaluation board    */
/*************************************************************************************************************/
/* Change log:

  As from V2.0.6:
    In Silabs_demoloop:
      Adding ‘tune ‘option
      Adding ‘blindlock’ option

  As from V2.0.1:
    main function compatible with dual TER front-end applications where a single xtal is shared between 2 front-end:
     XTAL on FrontEnd_Table[0] TER tuner, provided to:
      FrontEnd_Table[0] Demod
      FrontEnd_Table[1] TER tuner
     clock from FrontEnd_Table[1] TER tuner, provided to:
      FrontEnd_Table[1] Demod

    --------      ---------------  EN_XOUT/CONFIG_CLOCKS:CLOCK_MODE_XTAL                          ---------------
    | XTAL |----->| TER Tuner 0 |---------------------------------------------------------------->|   Demod 0   |
    --------      ---------------  |                                                              ---------------
              CLOCK_MODE_XTAL      |                                                            CLK_MODE_CLK_CLKIO
                                   |
                                   |
                                   |  ---------------  EN_XOUT/CONFIG_CLOCKS:CLOCK_MODE_XTAL      ---------------
                                   -->| TER Tuner 1 |-------------------------------------------->|   Demod 1   |
                                      ---------------                                              ---------------
                                  CLOCK_MODE_EXTCLK                                             CLK_MODE_CLK_CLKIO

    This is provided as an example for Si2158:
    if (fe==0) {
      front_end->Si216x_FE->tuner_ter->cmd->power_up.clock_mode      = Si2158_POWER_UP_CMD_CLOCK_MODE_XTAL;
      front_end->Si216x_FE->tuner_ter->cmd->power_up.en_xout         = Si2158_POWER_UP_CMD_EN_XOUT_EN_XOUT;
      front_end->Si216x_FE->tuner_ter->cmd->config_clocks.clock_mode = Si2158_CONFIG_CLOCKS_CMD_CLOCK_MODE_XTAL;
    }
    if (fe==1) {
      front_end->Si216x_FE->tuner_ter->cmd->power_up.clock_mode      = Si2158_POWER_UP_CMD_CLOCK_MODE_EXTCLK;
      front_end->Si216x_FE->tuner_ter->cmd->power_up.en_xout         = Si2158_POWER_UP_CMD_EN_XOUT_EN_XOUT;
      front_end->Si216x_FE->tuner_ter->cmd->config_clocks.clock_mode = Si2158_CONFIG_CLOCKS_CMD_CLOCK_MODE_EXTCLK;
    }

    It also requires adaptations in the corresponding Si21x8_L2_API.c/Si21x8_PowerUpWithPatch function:
    The clock_mode and en_xout values needs to be set based on the above values:
    (example for Si2158):
      if ((return_code = Si2158_L1_POWER_UP (api,
                            Si2158_POWER_UP_CMD_SUBCODE_CODE,
                            api->cmd->power_up.clock_mode,
                            api->cmd->power_up.en_xout,
                            . . .

  As from V2.0.0:
    Defining FRONT_END_COUNT if not defined at project level.
      This allows setting FRONT_END_COUNT at project level only for multi front-end applications

  As from V1.9.9:
    Adding NO_SAT tags to allow using Si2168 code without SAT features

  As from V1.9.8:
   last_plp_id stored during init as '-1' to indicate auto mode

  As from V1.9.7:
   Handling MCNS in Silabs_UserInput_qam, Silabs_UserInput_standard, Silabs_UserInput_Lock
   Removing TER_TUNER_MENU from Silabs_menu
   Removing TER_TUNER_LOOP from Silabs_demoloop

  As from V1.9.6:
   In main: setting i2c to L0_FastI2C(); by default

  As from V1.9.0:
   In Silabs_demoloop:
    Removing duplicate SATauto options

  As from V1.8.9:
   Treating symbol_rate as unsigned int (for proper display of high SAT SR when scanning)

   Adding Test Pipe feature (only if SILABS_API_TEST_PIPE is defined at project level), using new Silabs_UserInput_Test function
   In Silabs_UserInput_SeekNext:
    Checking num_plp if locked in DVB-T2, check PLP infos for each PLP ID and store one 'channel' per PLP ID
   In SiLabs_Scan_Loop:
    Adding scan duration display

   Adding console options:
    AutoSAT   : SiLabs_API_SAT_AutoDetect(front_end, 1)
    NoAutoSAT : SiLabs_API_SAT_AutoDetect(front_end, 0)
    autocheck : Display auto detect settings
    install   : SiLabs_API_SAT_Unicable_Install
    positionA : unicable->satellite_position = UNICABLE_POSITION_A
    positionB : unicable->satellite_position = UNICABLE_POSITION_B


  As from V1.8.8:
   In Silabs_demoloop: adding 'ATV', 'up' and 'down' options
   Adding functions to manage easily 'up' and 'down' zapping.

  As from V1.8.6:
   Compatibility with export for non 'SATELLITE_FRONT_END'

  As from V1.8.5:
   In Silabs_UserInput_SeekInit: set default bw as 8MHz (for DVB-C)
   In Silabs_UserInput_SeekNext: tracing detected standard as well, to test auto-detect
   In SiLabs_Scan_Table_Carrier_Text: tracing freq un Ku band if Unicable

   Adding console options:
    AutoTER   : SiLabs_API_TER_AutoDetect(front_end, 1)
    NoAutoTER : SiLabs_API_TER_AutoDetect(front_end, 0)
    autocheck : Display auto detect settings
    voltage_and_tone
    AutoSAT   : SiLabs_API_SAT_AutoDetect(front_end, 1)
    NoAutoSAT : SiLabs_API_SAT_AutoDetect(front_end, 0)
    install   : SiLabs_API_SAT_Unicable_Install
    horizontal: unicable->polarization = SILABS_POLARIZATION_HORIZONTAL
    vertical  : unicable->polarization = SILABS_POLARIZATION_VERTICAL
    low       : unicable->band         = UNICABLE_LOW_BAND
    high      : unicable->band         = UNICABLE_HIGH_BAND
    diseqc    : tracing Diseqc data (toggle)
    hardtune  : selecting Unicable values
    unicable  : SiLabs_Unicable_API_Tune_Infos
    tones     : SiLabs_Unicable_API_All_Tones
    tones_off : SiLabs_Unicable_API_All_Off

  As from V1.7.9:
   Adding 'slow' and 'fast' options in demo loop to allow easy testing of several i2c speeds

  As from V1.7.6:
   Some SiTRACE calls surrounded by curly brackets to allow compiling without SITRACES

  As from V1.7.5:
   Silabs_UserInput_qam changed to take text values for constellations

  As from V1.7.3:
    some lines moved for greater compatibility with Visual Studio

  As from V1.6.3:
    compatibility with SAT-only exports in Silabs_UserInput_bw_Hz

  As from V1.5.6:
    Silabs_UserInput_SeekInit uses 8000000 as default seekBWHz
 *************************************************************************************************************/
/* TAG V2.0.6 */

#include "SiLabs_API_L3_Console.h"

#define MAX_ENTRY_LENGTH 200
int   commandCount  = 0;
int   fe;
int   full_menu;
char  entryBuffer[MAX_ENTRY_LENGTH];
char *userEntry;
char  messageBuffer[10000];
int  last_standard       ;
int  last_freq           ;
int  last_bandwidth_Hz   ;
int  last_stream         ;
unsigned int  last_symbol_rate_bps;
int  last_constellation  ;
int  last_polarization   ;
int  last_band           ;
int  last_plp_id         ;
int  last_carrier_index  ;

SiLabs_Carriers_Struct *Carriers_Table = NULL;

/* define how many front-ends will be used */
#ifndef   FRONT_END_COUNT
  #define FRONT_END_COUNT  1
#endif /* FRONT_END_COUNT */
SILABS_FE_Context     FrontEnd_Table   [FRONT_END_COUNT];
SILABS_FE_Context    *front_end;
CUSTOM_Status_Struct  FE_Status;
CUSTOM_Status_Struct *custom_status;

/* Also init a simple i2c context to allow i2c communication checking */
L0_Context* i2c;
L0_Context  i2c_context;
CONNECTION_TYPE mode;

/************************************************************************************************************************
  Silabs_UserInputString function
  Use:        retrieve a string entered by the user
  Parameters: text, a pointer to the string buffer
              max_size, the size allocated by the calling function for the text buffer
  Behavior:   Retrieves the string using fgets to avoid overflows. As fgets keeps the 'carriage return' except
                if the string is longer than max_size, replace it (if present) by the terminating character
  Returns:    the length of the string
************************************************************************************************************************/
int  Silabs_UserInputString      (const char* text) {
  char *newline;
  printf("%s ",text);
  if ( (newline = fgets(userEntry, MAX_ENTRY_LENGTH, stdin)) != NULL ) {
    newline = strchr(userEntry, '\n');        /* search for newline character        */
    if ( newline != NULL ) { *newline = '\0'; /* overwrite trailing newline (if any) */ }
  }
  return (int)strlen(userEntry);
}
#ifdef    SiTRACES
/************************************************************************************************************************
  Silabs_UserInput_traces function
  Use:        send a traces configuration message to the L0 SiTRACES functions
              the Silabs_UserInput_traces function asks the user which type of configuration he wishes.
              Then, memorize the message and calls SiTraceConfiguration.
  Returns:    void
  Porting:    Not compiled if SiTRACES is not defined in Silabs_L0_API.h
************************************************************************************************************************/
void Silabs_UserInput_traces     (void) {
    char *configuration;
    char *msg;
    configuration = userEntry;
    msg           = (char*)malloc(sizeof(char)*100);
    Silabs_UserInputString("Enter the traces configuration string (-<param> ,<value>): ");
    sprintf(msg, "traces %s", configuration);
    printf("%s", SiTraceConfiguration (msg));
}
#endif /* SiTRACES */
void SiLabs_I2C_UserInput_read       (L0_Context* i2c) {
    int i;
    int readBytes;
    int loop;
    int nbArgs;
    int indexSize;
    unsigned int address;
    unsigned int uintval;
    unsigned int hexval;
    char *input;
    char *array[50];
    int   bytes[50];
    unsigned int iI2CIndex;
    int iNbBytes;
    unsigned char *pbtDataBuffer;

    input = userEntry;
    Silabs_UserInputString("SiLabs_I2C_UserInput_read: i2c read ");

    /* strtok splitting input and storing all items, returning first item */
    array[0] = strtok(input," ");
    if(array[0]==NULL) {return;}
    /* retrieving all remaining items */
    for(loop=1;loop<50;loop++) {
        array[loop] = strtok(NULL," ");
        if(array[loop]==NULL) break;
    }
    nbArgs = loop;
    /* scanning arguments, allowing decimal or hexadecimal input */
    for(loop=0;loop<nbArgs;loop++) {
        hexval=0;
        sscanf(array[loop],"%u", &uintval);
        if (!uintval) sscanf(array[loop],"%x", &hexval);
        bytes[loop] = hexval + uintval;
    }

    address   = bytes[0];
    indexSize = nbArgs - 2;
    if (indexSize <0) return;
    iI2CIndex = 0;
    for (i=1; i<= indexSize; i++) {
        iI2CIndex = (iI2CIndex<<8) + bytes[i];
    }
    iNbBytes  = bytes[loop-1];
/*  printf("address 0x%02x, indexSize %d, ",  address,   indexSize); printf("iI2CIndex 0x%02x, iNbBytes %d\n", iI2CIndex, iNbBytes); */
    pbtDataBuffer = (unsigned char*)malloc(sizeof(unsigned char)*iNbBytes);
    L0_SetAddress   (i2c, address, indexSize);
    readBytes = L0_ReadRawBytes (i2c, iI2CIndex, iNbBytes, pbtDataBuffer);
/*  printf("%d bytes read\n", readBytes); */
    for (i=0; i<readBytes; i++) { printf ("0x%02x ", pbtDataBuffer[i]); }
    if (readBytes) printf("\n");
}
void SiLabs_I2C_UserInput_write      (L0_Context* i2c) {
    int i;
    int writeBytes;
    int loop;
    int nbArgs;
    int indexSize;
    unsigned int address;
    unsigned int uintval;
    unsigned int hexval;
    char *input;
    char *array[50];
    int  bytes[50];
    unsigned int iI2CIndex;
    int iNbBytes;
    unsigned char *pbtDataBuffer;

    input = userEntry;

    Silabs_UserInputString("i2c write ");

    /* strtok splitting input and storing all items, returning first item */
    array[0] = strtok(input," ");
    if(array[0]==NULL) {return;}
    /* retrieving all remaining items */
    for(loop=1;loop<50;loop++) {
        array[loop] = strtok(NULL," ");
        if(array[loop]==NULL) break;
    }
    nbArgs = loop;
    /* scanning arguments, allowing decimal or hexadecimal input */
    for(loop=0;loop<nbArgs;loop++) {
        hexval=0;
        sscanf(array[loop],"%u", &uintval);
        if (!uintval) sscanf(array[loop],"%x", &hexval);
        bytes[loop] = hexval + uintval;
    }

    address   = bytes[0];
    indexSize = 0;
    if (indexSize <0) return;
    iI2CIndex = 0;
    iNbBytes  = nbArgs-1;
/*  printf("address 0x%02x, indexSize %d, ",  address,   indexSize); printf("iI2CIndex 0x%02x, iNbBytes %d\n", iI2CIndex, iNbBytes);*/
    pbtDataBuffer = (unsigned char*)malloc(sizeof(unsigned char)*iNbBytes);
    for (i=0; i<iNbBytes; i++) { pbtDataBuffer[i] = bytes[i+1]; }
/*    for (i=0; i<iNbBytes; i++) { printf ("0x%02x ", pbtDataBuffer[i]); }*/
    L0_SetAddress   (i2c, address, indexSize);
    writeBytes = L0_WriteRawBytes (i2c, iI2CIndex, iNbBytes, pbtDataBuffer);
    if (writeBytes) {printf("%d bytes written\n", writeBytes);} else {printf("Write error!\n");}
}
/************************************************************************************************************************
  Silabs_UserInput_freq function
  Use:        user-level frequency entry function, depending on the standard
              Used to have the user enter the desired tuning frequency, in Hz for TER and in kHz for SAT
  Parameter: standard, the current standard
  Behavior:   Asks the user to enter the tune freq
************************************************************************************************************************/
int  Silabs_UserInput_tune                  (CUSTOM_Standard_Enum standard)
{
  int   rf;

  switch (standard)
  { /* DVB_T, DVB_T2 and DVB_C treated first, then DVB_S, DVB_S2 and DSS treated as default */
    case SILABS_DVB_C :
    case SILABS_DVB_T :
    case SILABS_DVB_T2: {
      printf("tuning frequency (in Hz)? ");
      break;
    }
    default:              { /*  Used by SAT standards */
      printf("tuning frequency (in kHz)? ");
      break;
    }
  }

  Silabs_UserInputString("");

  sscanf(userEntry,"%d",&rf);
  return rf;
}
/************************************************************************************************************************
  Silabs_UserInput_qam function
  Use:        user-level qam selection function, depending on the standard
  Parameter: standard, the current standard
  Behavior:   Asks the user to enter the qam, then return the corresponding code
************************************************************************************************************************/
CUSTOM_Constel_Enum Silabs_UserInput_qam    (CUSTOM_Standard_Enum standard)
{
    char *qam;
    qam = userEntry;
    switch (standard)
    {
      case SILABS_DVB_C  : {
        printf("DVB-C qam (auto qam16 qam32 qam64 qam128 qam256)?  ");
        scanf("%s",qam);
        if (strcmp_nocase(qam,"auto"  )==0) { return SILABS_QAMAUTO;}
        if (strcmp_nocase(qam,"qam16" )==0) { return SILABS_QAM16  ;}
        if (strcmp_nocase(qam,"qam32" )==0) { return SILABS_QAM32  ;}
        if (strcmp_nocase(qam,"qam64" )==0) { return SILABS_QAM64  ;}
        if (strcmp_nocase(qam,"qam128")==0) { return SILABS_QAM128 ;}
        if (strcmp_nocase(qam,"qam256")==0) { return SILABS_QAM256 ;}
        break;
      }
      case SILABS_MCNS   : {
        printf("MCNS qam (auto qam64 qam256)?  ");
        scanf("%s",qam);
        if (strcmp_nocase(qam,"auto"  )==0) { return SILABS_QAMAUTO;}
        if (strcmp_nocase(qam,"qam64" )==0) { return SILABS_QAM64  ;}
        if (strcmp_nocase(qam,"qam256")==0) { return SILABS_QAM256 ;}
        break;
      }
      case SILABS_DVB_S2 : {
        Silabs_UserInputString("DVB-S2 qam (qpsk 8psk)? ");
        if (strcmp_nocase(qam,"qpsk"  )==0) { return SILABS_QPSK   ;}
        if (strcmp_nocase(qam,"8psk"  )==0) { return SILABS_8PSK   ;}
        break;
      }
      default:             {
        printf("No QAM choice for the selected standard");
        break;
      }
    }
    return SILABS_QAMAUTO;
}
/************************************************************************************************************************
  Silabs_UserInput_stream function
  Use:        user-level stream selection function
  Behavior:   Asks the user to enter the qam, then returns the corresponding code
************************************************************************************************************************/
CUSTOM_Stream_Enum  Silabs_UserInput_stream (void)
{
  Silabs_UserInputString("DVB-T Stream (HP LP)? ");
  if (strcmp_nocase(userEntry,"hp"  )==0) { SiTRACE("HP\n"); return SILABS_HP;}
  if (strcmp_nocase(userEntry,"lp"  )==0) { SiTRACE("HP\n"); return SILABS_LP;}
  return SILABS_HP;
}
/************************************************************************************************************************
  Silabs_UserInput_plp_id function
  Use:        user-level DVB-T2 PLP ID selection function
  Behavior:   Asks the user to enter the PLP ID, then returns the corresponding value
************************************************************************************************************************/
int  Silabs_UserInput_plp_id (void)
{
  int dvbt_t2_plp_id;
  Silabs_UserInputString("PLP ID ('auto' or <0-255>) ? ");
  if (strcmp_nocase(userEntry,"auto"  )==0) { SiTRACE("PLP AUTO\n"); return -1;}
  sscanf(userEntry,"%d",&dvbt_t2_plp_id);
  if (dvbt_t2_plp_id <   0) {dvbt_t2_plp_id = 0;}
  if (dvbt_t2_plp_id > 255) {dvbt_t2_plp_id = 0;}
  SiTRACE("PLP ID %d\n", dvbt_t2_plp_id);
  return dvbt_t2_plp_id;
}
/************************************************************************************************************************
  Silabs_UserInput_standard function
  Use:        standard selection function
              Used to have the user enter the desired digital TV standard
  Returns: the selected standard, -1 if error
************************************************************************************************************************/
int  Silabs_UserInput_standard   (void)
{
  char *entry;
  entry = userEntry;

  printf("Standard ( ");
  printf("T "  );
  printf("T2 " );
  printf("C "  );
  printf(")? ");
  if ( Silabs_UserInputString("") != 0) {
         if (strcmp_nocase (entry, "T"   )==0) { return SILABS_DVB_T ;}
         if (strcmp_nocase (entry, "T2"  )==0) { return SILABS_DVB_T2;}
         if (strcmp_nocase (entry, "C"   )==0) { return SILABS_DVB_C ;}
  }
  printf("Invalid standard selection !\n");
  return -1;
}
/************************************************************************************************************************
  Silabs_UserInput_bw_Hz function
  Use:        bw selection function
              Used to have the user enter the DVB-T or DVB-T2 bandwidth as desired
  Returns: the selected bw in Hz
************************************************************************************************************************/
int  Silabs_UserInput_bw_Hz     (CUSTOM_Standard_Enum standard)
{
  float bandwidth_MHz = 0;
  bandwidth_MHz = 0; /* To avoid compiler warning if not used */

  switch (standard)
  {
    default             : break;
    case SILABS_DVB_T2  : {
      Silabs_UserInputString("Bandwidth (in MHz) (1.7, 5, 6, 7 or 8)? ");
      sscanf(userEntry,"%f",&bandwidth_MHz);
      if (bandwidth_MHz == 1.7) return 1700000;
      if (bandwidth_MHz == 5) return 5000000;
      break;
    }
    case SILABS_DVB_T   : {
      Silabs_UserInputString("Bandwidth (in MHz) (6, 7 or 8)?      ");
      sscanf(userEntry,"%f",&bandwidth_MHz);
      break;
    }
  }

  if (bandwidth_MHz == 6) return 6000000;
  if (bandwidth_MHz == 7) return 7000000;
  if (bandwidth_MHz == 8) return 8000000;
  printf("\nSilabs_UserInput_bw_Hz : incorrect BW value! Using 8 MHz...\n");
  return 800000;
}
/************************************************************************************************************************
  Silabs_UserInput_SR_bps function
  Use:        user-level symbol rate function
              Used to set the symbol rate
  Behavior:   Asks the user to enter the symbol rate in Mbps, then return the value in bps
************************************************************************************************************************/
long Silabs_UserInput_SR_bps     (void)
{
  float sr;
  long sr_bps;
  printf("Symbol rate (in Msymb/s)? ");
  scanf("%f",&sr);
  sr_bps = (long)(sr*1000000);
  printf("     --> SR in baud %ld \n ",sr_bps);
  return sr_bps;
}
/************************************************************************************************************************
  Silabs_UserInput_TS_rate function
  Use:        user-level TS mode selection function
              Used to set the TS mode
  Behavior:   Asks the user to select a TS mode and then call
************************************************************************************************************************/
long Silabs_UserInput_TS         (void)
{
  char *entry;
  entry = userEntry;
  printf("TS mode (SERIAL PARALLEL TRISTATE ");
#ifdef    USB_Capability
  printf("GPIF ");
#endif /* USB_Capability */
  if ( Silabs_UserInputString(")? ") != 0) {
         if (strcmp_nocase (entry, "SERIAL"  )==0) { return SiLabs_API_TS_Mode (front_end, SILABS_TS_SERIAL  );}
    else if (strcmp_nocase (entry, "PARALLEL")==0) { return SiLabs_API_TS_Mode (front_end, SILABS_TS_PARALLEL);}
    else if (strcmp_nocase (entry, "TRISTATE")==0) { return SiLabs_API_TS_Mode (front_end, SILABS_TS_TRISTATE);}
#ifdef    USB_Capability
    else if (strcmp_nocase (entry, "GPIF"    )==0) { return SiLabs_API_TS_Mode (front_end, SILABS_TS_GPIF    );}
#endif /* USB_Capability */
  }
  printf("Invalid TS mode selection !\n");
  return 1;
}
/************************************************************************************************************************
  Silabs_UserInput_Polarization function
  Use:        user-level lnb polarization input function
              Used to ask the user to enter the lnb polarization
  Behavior:   Asks the user to enter the polarization
  Returns:    The final polarization value
************************************************************************************************************************/
int  Silabs_UserInput_Polarization(void)
{
  printf("\nSAT polarization <h v> (h = 'Horizontal', v = 'Vertical') ?");
  while ( Silabs_UserInputString("") == 0) {/* */}
  if (strcmp_nocase(userEntry,"h"  )==0) { SiTRACE("Horizontal\n"); return SILABS_POLARIZATION_HORIZONTAL;}
  if (strcmp_nocase(userEntry,"v"  )==0) { SiTRACE("Vertical  \n"); return SILABS_POLARIZATION_VERTICAL  ;}
  SiTRACE("????\n");
  return SILABS_POLARIZATION_HORIZONTAL;
}
/************************************************************************************************************************
  Silabs_UserInput_Band function
  Use:        user-level lnb band input function
              Used to ask the user to enter the lnb band
  Behavior:   Asks the user to enter the band
  Returns:    The final band value
************************************************************************************************************************/
int  Silabs_UserInput_Band       (void)
{
  printf("\nSAT band         <l h> (l = 'Low', h = 'High')            ?");
  while ( Silabs_UserInputString("") == 0) {/* */}
  if (strcmp_nocase(userEntry,"l"  )==0) { SiTRACE("LOW\n" ); return SILABS_BAND_LOW; }
  if (strcmp_nocase(userEntry,"h"  )==0) { SiTRACE("HIGH\n"); return SILABS_BAND_HIGH;}
  SiTRACE("????\n");
  return SILABS_BAND_LOW;
}
/************************************************************************************************************************
  Silabs_UserInput_lock function
  Use:        user-level lock fields input function
              Used to ask the user to enter required fields to lock on a carrier, depending on the standard
  Behavior:   Asks the user to enter the fields, then call SiLabs_API_lock_to_carrier
  Returns:    The final lock status
************************************************************************************************************************/
int  Silabs_UserInput_Lock       (void)
{
  int                  lock;
  int                  freq;
  int                  bandwidth_Hz;
  unsigned int         symbol_rate_bps;
  CUSTOM_Constel_Enum  constellation;
  CUSTOM_Stream_Enum   stream;
  int                  standard;
  int                  polarization;
  int                  band;
  int                  plp_id;

  standard = Silabs_UserInput_standard();
  if (standard < 0 ) return 0;

  /* Ask for the frequency for all standards */
  freq = Silabs_UserInput_tune (standard);
  /* Ask for the bandwidth for DVB-T and DVB-T2 */
  switch (standard) {
    case SILABS_DVB_T  :
    case SILABS_DVB_T2 : { bandwidth_Hz    =  Silabs_UserInput_bw_Hz(standard); break; }
    default:             { bandwidth_Hz    =  0; break; }
  }
  /* Ask for the Stream for DVB-T */
  switch (standard) {
    case SILABS_DVB_T  : { stream          = Silabs_UserInput_stream()        ; break; }
    default:             { stream          = 0                                ; break; }
  }
  /* Ask for the PLP ID for DVB-T2/C2 */
  switch (standard) {
    case SILABS_DVB_T2 : { plp_id   =  Silabs_UserInput_plp_id()       ; break; }
    default:             { plp_id   =  0; break; }
  }
  /* Ask for the symbol rate for DVB-C and all SAT standards*/
  switch (standard) {
    case SILABS_DVB_C  :
    case SILABS_MCNS   :
    case SILABS_DVB_S  :
    case SILABS_DVB_S2 :
    case SILABS_DSS    : { symbol_rate_bps = Silabs_UserInput_SR_bps()        ; break; }
    default:             { symbol_rate_bps = 0                                ; break; }
  }
  /* Ask for the constellation for DVB-C and DVB-S2 */
  switch (standard) {
    case SILABS_DVB_C  : { constellation   = Silabs_UserInput_qam(standard)   ; break; }
    case SILABS_MCNS   : { constellation   = Silabs_UserInput_qam(standard)   ; break; }
    default:             { constellation   = 0                                ; break; }
  }
  /* Ask for the polarization for SAT */
  switch (standard) {
    case SILABS_DVB_S  :
    case SILABS_DVB_S2 :
    case SILABS_DSS    : { polarization    = Silabs_UserInput_Polarization()  ; break; }
    default:             { polarization    = 0                                ; break; }
  }
  /* Ask for the band for SAT */
  switch (standard) {
    case SILABS_DVB_S  :
    case SILABS_DVB_S2 :
    case SILABS_DSS    : { band            = Silabs_UserInput_Band()          ; break; }
    default:             { band            = 0                                ; break; }
  }

  /* Call SiLabs_API_switch_to_standard, in case the standard is different or the init has not been done yet */
  if (SiLabs_API_switch_to_standard      (front_end, standard, 0) ==0) return 0;

  last_standard        = standard;
  last_freq            = freq;
  last_bandwidth_Hz    = bandwidth_Hz;
  last_stream          = stream;
  last_symbol_rate_bps = symbol_rate_bps;
  last_constellation   = constellation;
  last_polarization    = polarization;
  last_band            = band;
  last_plp_id          = plp_id;

  /* Call SiLabs_API_lock_to_carrier with the fields entered by the user */
  lock  = SiLabs_API_lock_to_carrier (front_end, standard, freq, bandwidth_Hz, stream, symbol_rate_bps, constellation, polarization, band, plp_id);

  return lock;
}
/************************************************************************************************************************
  Silabs_UserInput_SeekInit function
  Use:        user-level seek fields input function
              Used to ask the user to enter required fields to seek for carriers, depending on the standard
  Behavior:   Asks the user to enter the fields, then call SiLabs_API_SeekInit
  Returns:    The final lock status
************************************************************************************************************************/
int  Silabs_UserInput_SeekInit   (void)
{
  unsigned long rangeMin;
  unsigned long rangeMax;
  int           seekBWHz;
  int           seekStepHz;
  int           minSRbps;
  int           maxSRbps;
  int           minRSSIdBm;
  int           maxRSSIdBm;
  int           minSNRHalfdB;
  int           maxSNRHalfdB;
  CUSTOM_Standard_Enum standard;

  standard = front_end->standard;

  /* Ask for the Start frequency for all standards */
  printf("Seek Start ");
  rangeMin = Silabs_UserInput_tune(standard);

  /* Ask for the Stop  frequency for all standards */
  printf("Seek Stop  ");
  rangeMax = Silabs_UserInput_tune(standard);

  /* Ask for the bandwidth for DVB-T and DVB-T2 */
  switch (standard) {
    case SILABS_DVB_T  :
    case SILABS_DVB_T2 : { seekBWHz = Silabs_UserInput_bw_Hz(standard); break; }
    default:             { seekBWHz = 8000000 ; break; }
  }

  /* Ask for the step      for DVB-T and DVB-T2 */
  switch (standard) {
    case SILABS_DVB_T  :
    case SILABS_DVB_T2 : { Silabs_UserInputString("Seek Step (in Hz)?                   "); sscanf(userEntry,"%d",&seekStepHz); break; }
    default:             { seekStepHz = 8000000                         ; break; }
  }

  /* Ask for the MIN symbol rate for DVB-C and all SAT standards*/
  switch (standard) {
    case SILABS_DVB_C  :
    case SILABS_DVB_S  :
    case SILABS_DVB_S2 :
    case SILABS_DSS    : { printf("Min "); minSRbps = Silabs_UserInput_SR_bps(); break; }
    default:             { minSRbps= 0                        ; break; }
  }

  /* Ask for the MAX symbol rate for DVB-C and all SAT standards*/
  switch (standard) {
    case SILABS_DVB_C  :
    case SILABS_DVB_S  :
    case SILABS_DVB_S2 :
    case SILABS_DSS    : { printf("Max "); maxSRbps = Silabs_UserInput_SR_bps(); break; }
    default:             { maxSRbps= 0                        ; break; }
  }

  if (front_end->chip == 9999) {
    Silabs_UserInputString("minRSSI (in dBm)? ");
    sscanf(userEntry,"%d",&minRSSIdBm);

    Silabs_UserInputString("maxRSSI (in dBm)? ");
    sscanf(userEntry,"%d",&maxRSSIdBm);

    Silabs_UserInputString("minSNR  (in dB)? ");
    sscanf(userEntry,"%d",&minSNRHalfdB);
    minSNRHalfdB = minSNRHalfdB*2;

    Silabs_UserInputString("maxSNR  (in dB)? ");
    sscanf(userEntry,"%d",&maxSNRHalfdB);
    maxSNRHalfdB = maxSNRHalfdB*2;
  } else {
    minRSSIdBm   = 0;
    maxRSSIdBm   = 0;
    minSNRHalfdB = 0;
    maxSNRHalfdB = 0;
  }

  return  SiLabs_API_Channel_Seek_Init    (front_end, rangeMin, rangeMax, seekBWHz, seekStepHz, minSRbps, maxSRbps, minRSSIdBm, maxRSSIdBm, minSNRHalfdB, maxSNRHalfdB);
}
/************************************************************************************************************************
  Silabs_UserInput_SeekNext   function
  Use:        user-level seek function
              Used to seek for carriers, depending on the standard
  Behavior:   Asks the user to enter the fields, then call SiLabs_API_SeekInit
  Returns:    The final lock status
************************************************************************************************************************/
int  Silabs_UserInput_SeekNext   (void)
{
  int lock;
  int standard;
  int freq;
  int bandwidth_Hz;
  int stream;
  unsigned int symbol_rate_bps;
  int constellation;
  int polarization;
  int band;
  int num_plp;
  int carrier_index;
  int i;
  int plp_id;
  int plp_type;
  i = 0;

  lock = SiLabs_API_Channel_Seek_Next(front_end, &standard, &freq,  &bandwidth_Hz, &stream, &symbol_rate_bps, &constellation, &polarization, &band, &num_plp);

  if (lock==1) {
    printf("%s Channel detected (standard %d). freq %d", Silabs_Standard_Text(standard), standard, freq);
    SiLabs_API_FE_status          (front_end, custom_status);
    SiLabs_API_Text_status        (front_end, custom_status, messageBuffer);
 //   printf(messageBuffer);
    if ( (standard == SILABS_DVB_T2) & (num_plp > 1) ) {
      printf ("DVB_T2 NUM PLP %d\n", num_plp);
      for (i=0; i<num_plp; i++) {
        SiLabs_API_Get_PLP_ID_and_TYPE   (front_end, i, &plp_id, &plp_type);
        if (plp_id == -1) {
          printf ("ERROR retrieving PLP info for plp index %d\n", i);
          SiERROR("ERROR retrieving PLP info\n");
        } else {
          if (plp_type != SILABS_PLP_TYPE_COMMON) {
            carrier_index = SiLabs_Scan_Table_AddOneCarrier (standard, freq, bandwidth_Hz, stream, symbol_rate_bps, constellation, polarization, band, plp_id);
            SiLabs_Scan_Table_Carrier_Text(carrier_index, messageBuffer); printf("%s\n", messageBuffer);
          }
        }
      }
    } else {
      carrier_index = SiLabs_Scan_Table_AddOneCarrier (standard, freq, bandwidth_Hz, stream, symbol_rate_bps, constellation, polarization, band, -1);
      SiLabs_Scan_Table_Carrier_Text(carrier_index, messageBuffer); printf("%s\n", messageBuffer);
    }
 //   printf("Use the status functions to check the channel characteristics\n");
 //   printf("Call SeekNext to search the next channel\n");
  }
  else
  {
    printf("\n   Seek complete  now call SeekEnd to finish properly the scan\n");
  }
  return lock;
}
/************************************************************************************************************************
  SiLabs_Scan_Table_Init function
  Use:      carriers table initialization function
            Used to allocate the carriers table structure
  Returns: 0 if OK (allocation worked or already allocated), -1 if allocation error
************************************************************************************************************************/
int  SiLabs_Scan_Table_Init      (void)
{
  if (Carriers_Table == NULL) {

    Carriers_Table       = (SiLabs_Carriers_Struct*) malloc(sizeof(SiLabs_Carriers_Struct));
    if (Carriers_Table == NULL) { return -1; }

    Carriers_Table->carriers_count  = 0;
    Carriers_Table->standard        = (int *) malloc(sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->freq            = (int *) malloc(sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->bandwidth_Hz    = (int *) malloc(sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->stream          = (int *) malloc(sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->symbol_rate_bps = (unsigned int *) malloc(sizeof(unsigned int )*Carriers_Table->carriers_count);
    Carriers_Table->constellation   = (int *) malloc(sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->polarization    = (int *) malloc(sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->band            = (int *) malloc(sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->plp_id          = (int *) malloc(sizeof(int )*Carriers_Table->carriers_count);
  }
  return 0;
}
/************************************************************************************************************************
  SiLabs_Scan_Table_Clear function
  Use:      carriers table de-initialization function
            Used to clear the carriers table structure
  Returns: 0
************************************************************************************************************************/
int  SiLabs_Scan_Table_Clear     (void)
{
  if    (Carriers_Table != NULL) {
    free(Carriers_Table->standard        );
    free(Carriers_Table->freq            );
    free(Carriers_Table->bandwidth_Hz    );
    free(Carriers_Table->stream          );
    free(Carriers_Table->symbol_rate_bps );
    free(Carriers_Table->constellation   );
    free(Carriers_Table->polarization    );
    free(Carriers_Table->band            );
    Carriers_Table->carriers_count = 0;
    free(Carriers_Table);
    Carriers_Table = NULL;
  }
  return 0;
}
/************************************************************************************************************************
  SiLabs_Scan_Table_Count function
  Use:      retrieving the number of channels in the carriers table
            Used to know how many carriers are in the carriers table
  Returns: The number of carriers in the table
************************************************************************************************************************/
int  SiLabs_Scan_Table_Count     (void)
{
  if (SiLabs_Scan_Table_Init() == 0) {
    return Carriers_Table->carriers_count;
  }
  return 0;
}
/************************************************************************************************************************
  SiLabs_Scan_Table_AddOneCarrier function
  Use:      carrier addition function
            Used to add a carrier in the scan table
  Returns: The index of the new carrier (-1 if allocation error)
************************************************************************************************************************/
int  SiLabs_Scan_Table_AddOneCarrier (                   int  standard, int  freq, int  bandwidth_Hz, int  stream, unsigned int  symbol_rate_bps, int  constellation, int polarization, int band, int plp_id)
{
  int carrier_index;
  if (SiLabs_Scan_Table_Init() == 0) {
    Carriers_Table->carriers_count  = Carriers_Table->carriers_count +1;
    Carriers_Table->standard        = (int *) realloc(Carriers_Table->standard       , sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->freq            = (int *) realloc(Carriers_Table->freq           , sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->bandwidth_Hz    = (int *) realloc(Carriers_Table->bandwidth_Hz   , sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->stream          = (int *) realloc(Carriers_Table->stream         , sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->symbol_rate_bps = (unsigned int *) realloc(Carriers_Table->symbol_rate_bps, sizeof(unsigned int )*Carriers_Table->carriers_count);
    Carriers_Table->constellation   = (int *) realloc(Carriers_Table->constellation  , sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->polarization    = (int *) realloc(Carriers_Table->polarization   , sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->band            = (int *) realloc(Carriers_Table->band           , sizeof(int )*Carriers_Table->carriers_count);
    Carriers_Table->plp_id          = (int *) realloc(Carriers_Table->plp_id         , sizeof(int )*Carriers_Table->carriers_count);

    carrier_index = Carriers_Table->carriers_count -1;

    Carriers_Table->standard[carrier_index]        = standard;
    Carriers_Table->freq[carrier_index]            = freq;
    Carriers_Table->bandwidth_Hz[carrier_index]    = bandwidth_Hz;
    Carriers_Table->stream[carrier_index]          = stream;
    Carriers_Table->symbol_rate_bps[carrier_index] = symbol_rate_bps;
    Carriers_Table->constellation[carrier_index]   = constellation;
    Carriers_Table->polarization[carrier_index ]   = polarization;
    Carriers_Table->band[carrier_index]            = band;
    Carriers_Table->plp_id[carrier_index]          = plp_id;

    return carrier_index;
  } else {
    SiTRACE("SiLabs_Scan_Table_AddOneCarrier: Carriers_Table allocation error !\n");
    return -1;
  }
}
/************************************************************************************************************************
  SiLabs_Scan_Table_Carrier_Info function
  Use:      retrieving the carrier parameters from the carriers table
  Parameters: carrier_index, the index of the carrier in the table
              standard, a pointer to the standard integer
              freq    , a pointer to the freq integer (in Hz for TER, in kHz for SAT)
              bandwidth_Hz, a pointer to the bandwidth_Hz integer (in Hz)
              stream, a pointer to the stream integer
              symbol_rate_bps, a pointer to the symbol_rate_bps integer (in bps)
              constellation, a pointer to the constellation integer
              polarization, a pointer to the satellite horizontal/vertical polarization information
              band, a pointer to the satellite band in
  Returns: 1 if OK, 0 otherwise
************************************************************************************************************************/
int  SiLabs_Scan_Table_Carrier_Info  (int carrier_index, int *standard, int *freq, int *bandwidth_Hz, int *stream, unsigned int *symbol_rate_bps, int *constellation, int *polarization, int *band, int *plp_id)
{
  /* return immediately if the required index is not in the table */
  if (carrier_index >= SiLabs_Scan_Table_Count()) {return 0;}

  *standard        = Carriers_Table->standard[carrier_index];
  *freq            = Carriers_Table->freq[carrier_index];
  *bandwidth_Hz    = Carriers_Table->bandwidth_Hz[carrier_index];
  *stream          = Carriers_Table->stream[carrier_index];
  *symbol_rate_bps = Carriers_Table->symbol_rate_bps[carrier_index];
  *constellation   = Carriers_Table->constellation[carrier_index];
  *polarization    = Carriers_Table->polarization[carrier_index];
  *band            = Carriers_Table->band[carrier_index];
  *plp_id          = Carriers_Table->plp_id[carrier_index];

  return 1;
}
/************************************************************************************************************************
  SiLabs_Scan_Table_Carrier_Text function
  Use:      retrieving the carrier parameters from the carriers table in a string
  Parameters: carrier_index, the index of the carrier in the table
  Returns: 1 if OK, 0 otherwise
************************************************************************************************************************/
int  SiLabs_Scan_Table_Carrier_Text  (int carrier_index, char *formatted_infos)
{

  /* return immediately if the required index is not in the table */
  if (carrier_index >= SiLabs_Scan_Table_Count()) {return 0;}

  sprintf(formatted_infos, "Carrier %3d: %-7s", carrier_index, Silabs_Standard_Text(Carriers_Table->standard[carrier_index]));

  switch (Carriers_Table->standard[carrier_index])
  {
    case SILABS_ANALOG:
    case SILABS_DVB_T :
    case SILABS_DVB_T2:
    case SILABS_DVB_C : {
      sprintf(formatted_infos, "%s %9d   Hz", formatted_infos, Carriers_Table->freq[carrier_index]);
      break;
    }
    case SILABS_DVB_S :
    case SILABS_DSS   :
    case SILABS_DVB_S2: {
      sprintf(formatted_infos, "%s %9d kHz" , formatted_infos, Carriers_Table->freq[carrier_index]);
      break;
    }
    default           : {
      sprintf(formatted_infos, "%s INVALID standard (%d)!!!!\n"  , formatted_infos, Carriers_Table->standard[carrier_index]);
      return 0;
      break;
    }
  }
  switch (Carriers_Table->standard[carrier_index])
  {
    case SILABS_ANALOG: {
      break;
    }
    case SILABS_DVB_T : {
      sprintf(formatted_infos, "%s  %1.1f MHz,"     , formatted_infos, Carriers_Table->bandwidth_Hz[carrier_index]/1000000);
      sprintf(formatted_infos, "%s %s "             , formatted_infos, Silabs_Stream_Text (Carriers_Table->stream[carrier_index])        );
      break;
    }
    case SILABS_DVB_C : {
      sprintf(formatted_infos, "%s %5.2f Mbps"      , formatted_infos, Carriers_Table->symbol_rate_bps[carrier_index]/1000000);
      sprintf(formatted_infos, "%s %-8s "           , formatted_infos, Silabs_Constel_Text(Carriers_Table->constellation[carrier_index]) );
      break;
    }
    case SILABS_DVB_S :
    case SILABS_DSS   :
    case SILABS_DVB_S2: {
      sprintf(formatted_infos, "%s %5.2f Mbps"      , formatted_infos, Carriers_Table->symbol_rate_bps[carrier_index]/1000000);
      sprintf(formatted_infos, "%s (%s)"            , formatted_infos, Silabs_Polarization_Text(Carriers_Table->polarization[carrier_index]) );
      sprintf(formatted_infos, "%s (%s)"            , formatted_infos, Silabs_Band_Text        (Carriers_Table->band[carrier_index])         );
      break;
    }
    case SILABS_DVB_T2: {
      sprintf(formatted_infos, "%s  %1.1f MHz,"     , formatted_infos, Carriers_Table->bandwidth_Hz[carrier_index]/1000000);
      sprintf(formatted_infos, "%s  PLP ID %d,"     , formatted_infos, Carriers_Table->plp_id[carrier_index]);
      break;
    }
    default           : {
      break;
    }
  }
  return 1;
}
/************************************************************************************************************************
  SiLabs_Scan_Table_Infos function
  Use:      displaying the channels information in the console
            Used to know how many carriers are in the carriers table, and which values are needed to re-tune them.
  Returns: The number of carriers in the table
************************************************************************************************************************/
int  SiLabs_Scan_Table_Infos     (void)
{
  int i;
  if (Carriers_Table == NULL) {
    printf("No carrier in table.\n");
    return 0;
  }
  for (i=0; i<SiLabs_Scan_Table_Count(); i++) { if (SiLabs_Scan_Table_Carrier_Text(i, messageBuffer)) printf ("%s\n",messageBuffer); }
  return i;
}
/************************************************************************************************************************
  SiLabs_Scan_Loop function
  Use:      scan the band using SiLabs_API_Channel_Seek_Init and SiLabs_API_Channel_Seek_Next
            Used to fill the carriers table
  Returns: the number of detected channels during this loop
************************************************************************************************************************/
int  SiLabs_Scan_Loop            (void)
{
  int previous_count;
  int new_count;
  int standard;
  int i;
  int start;

  previous_count = SiLabs_Scan_Table_Count();

  standard = Silabs_UserInput_standard();
  if (standard < 0 ) return 0;

  start = system_time();

  /* Call SiLabs_API_switch_to_standard, in case the standard is different or the init has not been done yet */
  SiLabs_API_switch_to_standard      front_end(, standard, 0);

  /* Init the scan loop for the user-selected standard */
  Silabs_UserInput_SeekInit();

  while (Silabs_UserInput_SeekNext()) { /* call Silabs_UserInput_SeekNext as long as it detects new carriers */ }

  /* Put the demodulator out of scan mode */
  SiLabs_API_Channel_Seek_End(front_end);

  printf("\nScan duration %.3f s\n", (system_time() - start)/1000 );

  new_count = SiLabs_Scan_Table_Count();

  if (new_count) {SiTRACE("There are %d carriers now in the table\n", new_count);}

  for (i = previous_count; i < new_count; i++) {
    SiLabs_Scan_Table_Carrier_Text(i, messageBuffer);
    printf("%s\n", messageBuffer);
  }

  return SiLabs_Scan_Table_Count() - previous_count;
}
/************************************************************************************************************************
  SiLabs_Scan_Zap_To_Index function
  Use:      select a carrier in the table and lock on this carrier
            Used to lock to carriers stored in the table
  Parameter: index, the index of the required carrier
  Returns: the lock status
************************************************************************************************************************/
int  SiLabs_Scan_Zap_To_Index    (int carrier_index)
{
  int standard;
  int freq;
  int bandwidth_Hz;
  int stream;
  unsigned int symbol_rate_bps;
  int constellation;
  int polarization;
  int band;
  int plp_id;
  int res;
  int carriers_count;
  res = 0;

  if ((carriers_count = SiLabs_Scan_Table_Count() ) == 0) {printf("Please use 'lock/store' or 'scan' or 'SeekInit/SeekNext/SeekNext/.../SeekEnd' to fill the table\n"); return 0;}

  if (SiLabs_Scan_Table_Carrier_Info  (carrier_index, &standard, &freq, &bandwidth_Hz, &stream, &symbol_rate_bps, &constellation, &polarization, &band, &plp_id) ) {
    last_carrier_index = carrier_index;
    res = SiLabs_API_lock_to_carrier  (front_end,      standard,  freq,  bandwidth_Hz,  stream,  symbol_rate_bps,  constellation,  polarization,  band,  plp_id);
    SiLabs_API_FE_status              (front_end, custom_status);
    SiLabs_API_Text_status            (front_end, custom_status, messageBuffer);
    printf(messageBuffer);
  } else {
    printf ("Invalid index! Please select indexes between %d and %d\n", 0, carriers_count-1);
  }
  return res;
}
/************************************************************************************************************************
  SiLabs_Scan_Zap_Up function
  Use:      select the next carrier in the table and lock on this carrier
            Used to lock to carriers stored in the table
  Returns: the lock status
************************************************************************************************************************/
int  SiLabs_Scan_Zap_Up          (void)
{
  int carriers_count;
  int carrier_index;

  if ((carriers_count = SiLabs_Scan_Table_Count() ) == 0) {printf("Please use 'lock/store' or 'scan' or 'SeekInit/SeekNext/SeekNext/.../SeekEnd' to fill the table\n"); return 0;}

  carrier_index = last_carrier_index+1;

  if (carrier_index >= carriers_count) {carrier_index = 0;}

  printf("Zap on carrier %d\n", carrier_index);

  return SiLabs_Scan_Zap_To_Index    (carrier_index);
}
/************************************************************************************************************************
  SiLabs_Scan_Zap_Down function
  Use:      select the previous carrier in the table and lock on this carrier
            Used to lock to carriers stored in the table
  Returns: the lock status
************************************************************************************************************************/
int  SiLabs_Scan_Zap_Down        (void)
{
  int carriers_count;
  int carrier_index;

  if ((carriers_count = SiLabs_Scan_Table_Count() ) == 0) {printf("Please use 'lock/store' or 'scan' or 'SeekInit/SeekNext/SeekNext/.../SeekEnd' to fill the table\n"); return 0;}

  carrier_index = last_carrier_index-1;

  if (carrier_index < 0) {carrier_index = carriers_count-1;}

  printf("Zap on carrier %d\n", carrier_index);

  return SiLabs_Scan_Zap_To_Index    (carrier_index);
}
/************************************************************************************************************************
  SiLabs_Scan_Zap function
  Use:      select a carrier in the table and lock on this carrier
            Used to lock to carriers stored in the table
  Returns: the lock status
************************************************************************************************************************/
int  SiLabs_Scan_Zap             (void)
{
  int carriers_count;
  int carrier_index;

  if ((carriers_count = SiLabs_Scan_Table_Infos() ) == 0) {printf("Please use 'lock/store' or 'scan' or 'SeekInit/SeekNext/SeekNext/.../SeekEnd' to fill the table\n"); return 0;}

  sprintf(messageBuffer, "Select a carrier index (%d to %d)? ", 0, carriers_count-1);

  Silabs_UserInputString(messageBuffer);

  sscanf(userEntry,"%d",&carrier_index);

  return SiLabs_Scan_Zap_To_Index    (carrier_index);
}
/************************************************************************************************************************
  Silabs_bytes_trace function
  Use:        console application byte-level trace function
              Used to toggle L0 traces
************************************************************************************************************************/
int  Silabs_bytes_trace          (void)
{
  unsigned char trackMode;
  trackMode = !i2c->trackWrite;
  i2c->trackWrite = trackMode;
  i2c->trackRead  = trackMode;
  return SiLabs_API_bytes_trace(front_end, trackMode);
}
/************************************************************************************************************************
  Silabs_help function
  Use:        console application help function
              Used to explain how to init the EVB, tune and scan
************************************************************************************************************************/
int  Silabs_help                 (void)
{
    printf("\n\n\
----------------------------------------------------------------------------\n\
    This is a demonstration application used to illustrate how to use       \n\
       the SiLabs digital TV API delivered by Silicon Laboratories          \n\
                                                                            \n\
    It demonstrates a dual front-end case, and can deal with up to 4        \n\
    NB: Most platforms will only have one front-end, therefore              \n\
        the other ones will not work correctly unless properly wired        \n\
                                                                            \n\
    Front-end selection: '<(0 to 3)>' (default is '0')                      \n\
                                                                            \n\
    Scanning sequences:                                                     \n\
                                                                            \n\
      DVB_T : 'scan' 'T'   <min_freq> <max_freq> <bandwidth> <step>         \n\
      DVB_T2: 'scan' 'T2'  <min_freq> <max_freq> <bandwidth> <step>         \n\
      DVB_C : 'scan' 'C'   <min_freq> <max_freq> <min_SR> <max_SR>          \n\
      DVB-S : 'scan' 'S'   <min_freq> <max_freq> <min_SR> <max_SR>          \n\
      DVB-S2: 'scan' 'S2'  <min_freq> <max_freq> <min_SR> <max_SR>          \n\
      DSS   : 'scan' 'DSS' <min_freq> <max_freq> <min_SR> <max_SR>          \n\
                                                                            \n\
    Locking sequences:                                                      \n\
                                                                            \n\
      DVB_T : 'lock' 'T'   <frequency> <bandwidth>   <stream>               \n\
      DVB_T2: 'lock' 'T2'  <frequency> <bandwidth>                          \n\
      DVB_C : 'lock' 'C'   <frequency> <symbol_rate> <qam>                  \n\
      DVB-S : 'lock' 'S'   <frequency> <symbol_rate>                        \n\
      DVB-S2: 'lock' 'S2'  <frequency> <symbol_rate>                        \n\
      DSS   : 'lock' 'DSS' <frequency> <symbol_rate>                        \n\
                                                                            \n\
");
#ifdef    USB_Capability
    printf("\n\
    Connection via the Cypress chip:    'USB'                               \n\
");
#endif /* USB_Capability*/
    printf("\n\
    Connection via the CUSTOM i2c  :    'CUST'                              \n\
    Init after power cycle or HW reset: 'hwinit'                            \n\
                                                                            \n\
    Repeatedly press the return key to update the status                    \n\
    Enter 'help' to display the help.                                       \n\
    Enter 'm'    to display the menu.                                       \n\
----------------------------------------------------------------------------\n\
");
  return 0;
}
/************************************************************************************************************************
  NAME: Silabs_menu
  DESCRIPTION: Silabs user menu function
              Used to display the various possibilities offered to the user
  Behavior:   Prints the menu in the console
************************************************************************************************************************/
void Silabs_menu                 (unsigned char full)
{
  if (full) {
  printf("\
-----------------------------------------------------------------------\n\
   Console mode Application Menu:\n\
-----------------------------------------------------------------------\n\
");
  printf(" ------  i2c ---------\n\
read           : read bytes from i2c       \n\
write          : write bytes to i2c        \n\
");
#ifdef    USB_Capability
    printf("\n\
USB            : connect i2c in USB  mode  \n\
");
#endif /* USB_Capability*/
    printf("\n\
CUST           : connect i2c in CUST mode  \n\
trace          : toggle L0 traces\n\
traces         : manage L0 traces (use 'traces' then 'help' for details)\n\
");
  printf("\
 ------  Digital TV demodulator ------\n\
");
}
    printf("\
 ...... Locking        ......\n\
lock     : call SiLabs_API_lock_to_carrier    (front_end, standard, freq, bandwidth_Hz, stream, symbol_rate_bps, constellation);\n\
store    : store last carrier info in zap table\n\
 ...... Monitoring     ......\n\
''       : update status\n\
r        : call SiLabs_API_Demod_reset        (front_end);\n\
u        : call SiLabs_API_Reset_Uncorrs      (front_end);\n\
 ...... TS control     ......\n\
ts       : call SiLabs_API_TS_Mode            (front_end, TS_SERIAL   );\n\
 ...... Scanning       ......\n\
SeekInit : call SiLabs_API_Channel_Seek_Init  (front_end, rangeMin, rangeMax, minRSSIdBm, maxRSSIdBm, minSNRHalfdB, maxSNRHalfdB);\n\
SeekNext : call SiLabs_API_Channel_Seek_Next  (front_end); and display the result.\n\
SeekEnd  : call SiLabs_API_Channel_Seek_End   (front_end);\n\
scan     : call 'SeekInit' and then 'SeekNext' until completion, store detected channels in channel table;\n\
count    : display number of channels in the channel table;\n\
channels : display  channels present  in the channel table;\n\
zap      : calls SiLabs_API_lock_to_carrier for a carrier selected in the table;\n\
clear    : clear all channels from the channel table;\n\
");
printf("\
AutoTER   : SiLabs_API_TER_AutoDetect(front_end, 1);\n\
NoAutoTER : SiLabs_API_TER_AutoDetect(front_end, 0);\n\
");
printf("\
PLP       : SiLabs_API_Select_PLP    (front_end, plp);\n\
");
printf("\
Autocheck : Display auto detect settings\n\
");

  if (full) {
  printf(" ------ console -----\n\
help           : display application help\n\
cls            : clear screen\n\
m              : display this menu\n\
exit           : exit application\n\
minimal        : only displays minimal menu\n\
full           : displays full menu\n\
");
  }
  printf("----------------------------------------------------------------------------\n");
}
#ifdef    SILABS_API_TEST_PIPE
int Silabs_UserInput_Test   (void)
{
  char  targetBuffer[100];
  char  cmdBuffer[100];
  char  sub_cmdBuffer[100];
  char *rettxt;
  char *target;
  char *cmd;
  char *sub_cmd;
  char *entry;
  int dval;
  int retdval;
  int    valid_target;

  entry   = &entryBuffer[0];
  rettxt  = &messageBuffer[0];
  target  = &targetBuffer[0];
  cmd     = &cmdBuffer[0];
  sub_cmd = &sub_cmdBuffer[0];
  dval    = 0;
  valid_target = 0;

  sprintf(cmd,     "%s", "");
  sprintf(sub_cmd, "%s", "");

  Silabs_UserInputString("target  ? "); sprintf(target    , "%s", entry);

  if (strcmp_nocase(target,"help" )!=0) {
    Silabs_UserInputString("cmd     ? "); sprintf(cmd    , "%s" ,  entry);
    Silabs_UserInputString("sub_cmd ? "); sprintf(sub_cmd, "%s" ,  entry);
    Silabs_UserInputString("dval    ? "); sscanf(entry   , "%lf", &dval);
  }

  printf("'%s' '%s' '%s' '%f'...\n", target, cmd, sub_cmd, dval);

  Silabs_API_Test (front_end, target, cmd, sub_cmd, dval, &retdval, &rettxt);
  printf("%s", messageBuffer);
  return retdval;
}
#endif /* SILABS_API_TEST_PIPE */
/************************************************************************************************************************
  Silabs_demoloop function
  Use:        Demo application loop function
              Used to call all available demod functions, based on the user's entry
  Behavior:   Wait for the user to enter a string, then calls the selected function (no string will redisplay the demod status)
************************************************************************************************************************/
int  Silabs_demoloop             (void)
{
    char *entry;
    char txtBuffer[1000];
    char *txt;
    int   res;
    int   lock;
    int dval;
    int retdval;


    entry   = entryBuffer;
    res     = 0;
    dval    = 0;
    retdval = 0;

    if (fe > FRONT_END_COUNT) return 0;

    txt           = txtBuffer;
    front_end     = &(FrontEnd_Table[fe]);
    custom_status = &(FE_Status);

    CHECK_FOR_ERRORS

    printf("\n%04d FrontEnd[%d] Command > ", ++commandCount, fe);
    Silabs_UserInputString("");

    /* In case the init failed or has not been done, reconnect the board to get a working connection when the board is powered and connected */
    if (front_end->init_ok == 0) {L0_Connect(i2c, mode); SiLabs_API_HW_Connect(front_end, mode);}

         /* front end selection */
         if ( (strcmp_nocase(entry, "0")==0) & (FRONT_END_COUNT>0) ) { fe = 0; printf ("Controlling FrontEnd %d\n",fe); return 1;}
         if ( (strcmp_nocase(entry, "1")==0) & (FRONT_END_COUNT>1) ) { fe = 1; printf ("Controlling FrontEnd %d\n",fe); return 1;}
         if ( (strcmp_nocase(entry, "2")==0) & (FRONT_END_COUNT>2) ) { fe = 2; printf ("Controlling FrontEnd %d\n",fe); return 1;}
         if ( (strcmp_nocase(entry, "3")==0) & (FRONT_END_COUNT>3) ) { fe = 3; printf ("Controlling FrontEnd %d\n",fe); return 1;}

         if (strcmp_nocase(entry, ""         )==0) {
           if (front_end->init_ok == 0) { printf("Init not ok. Please use 'lock' or 'scan' to peform the init first!\n"); return 1;}
           printf("Monitoring ...\n");
           SiLabs_API_FE_status          (front_end, custom_status);
           SiLabs_API_Text_status        (front_end, custom_status, messageBuffer);
           printf(messageBuffer);
           return 1;
        }
         if (strcmp_nocase(entry, "hwinit"   )==0) { SiLabs_API_switch_to_standard (front_end, front_end->standard, 1);}
    else if (strcmp       (entry, "comm"     )==0) { SiLabs_API_communication_check(front_end); return 1;}
    else if (strcmp_nocase(entry, "infos"    )==0) { SiLabs_API_Infos(front_end, txt); printf("%s",txt);}
    else if (strcmp       (entry, "ATV"      )==0) { SiLabs_API_switch_to_standard (front_end, SILABS_ANALOG, 0);}
    else if (strcmp       (entry, "T"        )==0) { SiLabs_API_switch_to_standard (front_end, SILABS_DVB_T , 0);}
    else if (strcmp       (entry, "T2"       )==0) { SiLabs_API_switch_to_standard (front_end, SILABS_DVB_T2, 0);}
    else if (strcmp_nocase(entry, "PLP"      )==0) { SiLabs_API_Select_PLP         (front_end, Silabs_UserInput_plp_id() );}
    else if (strcmp       (entry, "C"        )==0) { SiLabs_API_switch_to_standard (front_end, SILABS_DVB_C , 0);}
    else if (strcmp       (entry, "SLEEP"    )==0) { SiLabs_API_switch_to_standard (front_end, SILABS_SLEEP , 0);}
    else if (strcmp       (entry, "lock"     )==0) {
      if (Silabs_UserInput_Lock() == 1) {
        SiLabs_API_FE_status          (front_end, custom_status);
        SiLabs_API_Text_status        (front_end, custom_status, messageBuffer);
        printf(messageBuffer);
      }
      return 1;
    }
    else if (strcmp_nocase(entry, "store"    )==0) {
      res = SiLabs_Scan_Table_AddOneCarrier(last_standard, last_freq, last_bandwidth_Hz, last_stream, last_symbol_rate_bps, last_constellation, last_polarization, last_band, last_plp_id);
      SiLabs_Scan_Table_Carrier_Text(res, messageBuffer);
      printf("Stored %s\n", messageBuffer);
      return 1;
    }
    else if (strcmp_nocase(entry, "SeekInit" )==0) { Silabs_UserInput_SeekInit();}
    else if (strcmp_nocase(entry, "SeekNext" )==0) { Silabs_UserInput_SeekNext();}
    else if (strcmp_nocase(entry, "SeekEnd"  )==0) { SiLabs_API_Channel_Seek_End (front_end);}
    else if (strcmp_nocase(entry, "r"        )==0) { SiLabs_API_Demod_reset      (front_end);}
    else if (strcmp_nocase(entry, "u"        )==0) { SiLabs_API_Reset_Uncorrs    (front_end);}
    else if (strcmp_nocase(entry, "ts"       )==0) { Silabs_UserInput_TS         ();}
    else if (strcmp_nocase(entry, "u"        )==0) { SiLabs_API_Reset_Uncorrs    (front_end);}
#ifdef    SiTRACES
    else if (strcmp_nocase(entry, "traces"   )==0) { Silabs_UserInput_traces();}
#endif /* SiTRACES */
    else if (strcmp_nocase(entry, "trace"    )==0) { Silabs_bytes_trace();}
    else if (strcmp_nocase(entry, "minimal"  )==0) { full_menu = 0; return 1;}
    else if (strcmp_nocase(entry, "full"     )==0) { full_menu = 1; return 1;}
    else if (strcmp_nocase(entry, "AutoTER"  )==0) { SiLabs_API_TER_AutoDetect(front_end, 1);}
    else if (strcmp_nocase(entry, "NoAutoTER")==0) { SiLabs_API_TER_AutoDetect(front_end, 0);}
    else if (strcmp_nocase(entry, "autocheck")==0) {
      printf("auto_detect TER %d ", SiLabs_API_TER_AutoDetect(front_end, 2));
      printf("\n");
    }
    else if (strcmp_nocase(entry, "m"        )==0) { Silabs_menu(full_menu); return 1;}
    else if (strcmp_nocase(entry, "help"     )==0) { Silabs_help(); return 1;}
    else if (strcmp_nocase(entry, "cls"      )==0) { system("cls");}
    else if (strcmp_nocase(entry, "quit"     )==0) { return 0;}
    else if (strcmp_nocase(entry, "exit"     )==0) { exit(0);}
    else if (strcmp_nocase(entry, "read"     )==0) { SiLabs_I2C_UserInput_read (i2c);}
    else if (strcmp_nocase(entry, "write"    )==0) { SiLabs_I2C_UserInput_write(i2c);}
    else if (strcmp_nocase(entry, "usb"      )==0) { mode = USB;      L0_Connect(i2c, mode); SiLabs_API_HW_Connect(front_end, mode); return 1;}
#ifdef    L0_SLOW_I2C
    else if (strcmp_nocase(entry, "slow"     )==0) { L0_SlowI2C();   return 1;}
    else if (strcmp_nocase(entry, "fast"     )==0) { L0_FastI2C();   return 1;}
#endif /* L0_SLOW_I2C */
    else if (strcmp_nocase(entry, "cust"     )==0) { mode = CUSTOMER; L0_Connect(i2c, mode); SiLabs_API_HW_Connect(front_end, mode); return 1;}
    else if (strcmp_nocase(entry, "scan"     )==0) { SiLabs_Scan_Loop();        return 1;}
    else if (strcmp_nocase(entry, "clear"    )==0) { SiLabs_Scan_Table_Clear(); return 1;}
    else if (strcmp_nocase(entry, "channels" )==0) { SiLabs_Scan_Table_Infos(); return 1;}
    else if (strcmp_nocase(entry, "zap"      )==0) { SiLabs_Scan_Zap();         return 1;}
    else if (strcmp_nocase(entry, "up"       )==0) { SiLabs_Scan_Zap_Up();      return 1;}
    else if (strcmp_nocase(entry, "down"     )==0) { SiLabs_Scan_Zap_Down();    return 1;}
    else if (strcmp_nocase(entry, "tune"     )==0) {
      SiLabs_API_Tune      (front_end, Silabs_UserInput_tune(front_end->standard) );
      return 1;
    }
    else if (strcmp_nocase(entry, "blindlock")==0) {
      if (front_end->standard != SILABS_DVB_C) { SiLabs_API_switch_to_standard(front_end, SILABS_DVB_C, 0); }
      last_freq = SiLabs_API_Tune              (front_end, Silabs_UserInput_tune(front_end->standard) );
      last_symbol_rate_bps = Silabs_UserInput_SR_bps();
      SiLabs_API_Channel_Seek_Init       (front_end, last_freq, last_freq, 0, 0, last_symbol_rate_bps, (int)(last_symbol_rate_bps*1.06), 0, 0, 0, 0);
      lock = SiLabs_API_Channel_Seek_Next(front_end, &last_standard, &last_freq,  &last_bandwidth_Hz, &last_stream, &last_symbol_rate_bps, &last_constellation, &last_polarization, &last_band, &res);
      SiLabs_API_Channel_Seek_End        (front_end);
      printf("lock %d\n", lock);
      if (lock) printf("symbol_rate %d\n", last_symbol_rate_bps);
      return 1;
    }

#ifdef    SILABS_API_TEST_PIPE
    else if (strcmp_nocase(entry, "testPipe" )==0) { Silabs_UserInput_Test();   return 1;}
#endif /* SILABS_API_TEST_PIPE */
    else {
      Silabs_menu(full_menu);
    }

    return 1;
}
/************************************************************************************************************************
  main function
  Use:        main usage demonstration function
              Used to show how to init the SW and HW, tune and lock on any standard
  Porting:    For demonstration purpose, should be replaced by the final application
  Behavior:   Starts by the EVB initialization, then displays the user menu
************************************************************************************************************************/
int  main(int argc, char *argv[])
{
#ifndef   TUNER_ADDRESS_SAT
 #define  TUNER_ADDRESS_SAT 0
#endif /* TUNER_ADDRESS_SAT */

#ifndef   TUNER_ADDRESS_TER
 #define  TUNER_ADDRESS_TER 0
#endif /* TUNER_ADDRESS_TER */

  int res;
  unsigned char  ucBufferArea[1000];
  unsigned char *ucMyBuffer;
  ucMyBuffer = ucBufferArea;

  argc=argc; /* To avoid compiler warning */
  argv=argv; /* To avoid compiler warning */

  /* Initial communication mode */
  mode = USB;
  res = 1;

  Carriers_Table = NULL;

  SiTraceDefaultConfiguration();
  SiTraceConfiguration("traces -output stdout -file off -verbose off -function off -line off\n");

  userEntry = entryBuffer;

  /* I2C pointer initialization (to allow i2c validation) */
  i2c = &i2c_context;
  L0_Init(i2c);
  i2c->mustReadWithoutStop =0;

  Silabs_help();

  /* Software Init */
  for (fe=0; fe<FRONT_END_COUNT; fe++) {
    SiLabs_API_SW_Init   (&(FrontEnd_Table[fe]), DEMOD_ADDRESS+(fe*2), TUNER_ADDRESS_TER+(fe*2), TUNER_ADDRESS_SAT+(fe*2));
    SiLabs_API_HW_Connect(&(FrontEnd_Table[fe]), mode);
    front_end           = &(FrontEnd_Table[fe]);
#ifdef    Si2168_COMPATIBLE
  #ifdef    TER_TUNER_Si2158
    if (fe==0) {
      front_end->Si2168_FE->tuner_ter->cmd->power_up.clock_mode      = Si2158_POWER_UP_CMD_CLOCK_MODE_XTAL;         /* options: XTAL     EXTCLK   */
      front_end->Si2168_FE->tuner_ter->cmd->power_up.en_xout         = Si2158_POWER_UP_CMD_EN_XOUT_EN_XOUT;         /* options: EN_XOUT  DIS_XOUT */
      front_end->Si2168_FE->tuner_ter->cmd->config_clocks.clock_mode = Si2158_CONFIG_CLOCKS_CMD_CLOCK_MODE_XTAL;    /* options: XTAL     EXTCLK   */
    }
    if (fe==1) {
      front_end->Si2168_FE->tuner_ter->cmd->power_up.clock_mode      = Si2158_POWER_UP_CMD_CLOCK_MODE_EXTCLK;       /* options: XTAL     EXTCLK   */
      front_end->Si2168_FE->tuner_ter->cmd->power_up.en_xout         = Si2158_POWER_UP_CMD_EN_XOUT_EN_XOUT;         /* options: EN_XOUT  DIS_XOUT */
      front_end->Si2168_FE->tuner_ter->cmd->config_clocks.clock_mode = Si2158_CONFIG_CLOCKS_CMD_CLOCK_MODE_EXTCLK;  /* options: XTAL     EXTCLK   */
    }
  #endif /* TER_TUNER_Si2158 */
#endif /* Si2168_COMPATIBLE */
  }

  printf("\n\
  Managing %d front-end(s)                                        \n\
  (base at 0x%02x, ter tuner at 0x%02x, sat tuner at 0x%02x)      \n",
  FRONT_END_COUNT, DEMOD_ADDRESS, TUNER_ADDRESS_TER, TUNER_ADDRESS_SAT );

#ifdef    SILABS_API_TEST_PIPE
  printf("\nSILABS_API_TEST_PIPE available\n\n");
#endif /* SILABS_API_TEST_PIPE */

  fe = 0;
  front_end     = &(FrontEnd_Table[fe]);

  full_menu = 1;
 /* Setting default values for 'store' parameters */
  last_standard        = SILABS_DVB_T;
  last_freq            = 474000000   ;
  last_bandwidth_Hz    = 8000000;
  last_stream          = SILABS_LP;
  last_symbol_rate_bps = 6900000;
  last_constellation   = SILABS_QAMAUTO;
  last_carrier_index   = -1;
  last_plp_id          = -1;

  #ifdef    USB_Capability
  /* Checking USB communication with the cypress chip */
  L0_Connect(i2c, USB);
  L0_FastI2C();
  sprintf(messageBuffer, "0xa0 0 5");
  if ((int)L0_ReadString(i2c, messageBuffer , ucMyBuffer) != 5) {
    SiERROR ("Cypress Communication error !\nConnect the EVB, reset the USB interface!\n");
    res = 0;
  } else {
    SiTRACE ("Cypress   Communication OK\n");
    res = 1;
  }
  #endif /* USB_Capability */

  if (res == 1) {
    /* Checking DEMOD communication */
    sprintf(messageBuffer, "0x%02x 1", DEMOD_ADDRESS);
    if ((int)L0_ReadString(i2c, messageBuffer , ucMyBuffer) != 1) {
      SiERROR ("Demod Communication error !\n");
      SiERROR ("Connect the EVB to the I2C interface!\n");
    } else {
      SiTRACE ("Demod     Communication OK\n");
      res = TUNER_ADDRESS_TER;
      if (res !=0) {
        /* Checking TER TUNER communication */
        SiLabs_API_Tuner_I2C_Enable(front_end);
        sprintf(messageBuffer, "0x%02x 1", TUNER_ADDRESS_TER);
        if ((int)L0_ReadString(i2c, messageBuffer , ucMyBuffer) != 1) {
          SiERROR ("TER Tuner Communication error !\n");
          res = 0;
        } else {
          SiTRACE ("TER_Tuner Communication OK\n");
          res = 1;
        }
        SiLabs_API_Tuner_I2C_Disable(front_end);
      }
    }
  }

  while (Silabs_demoloop());

  return 0;
}

