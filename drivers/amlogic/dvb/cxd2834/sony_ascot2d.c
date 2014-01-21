/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**
 @file    sony_ascot2d.c

      This file provides the Sony ASCOT2D DVB & Analog tuner driver.
      
*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "sony_dvb.h"
#include "sony_ascot2d.h"
#include "sony_common.h"
/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define XTAL_41MHZ                  41      /**< 41MHz XTAL. */
#define XTAL_16MHZ                  16      /**< 16MHz XTAL. */
#define XTAL_4MHZ                   4       /**< 4MHz XTAL. */
#define A2D_XPON_WAIT_MS            90      /**< Wait for CPU idle in X_pon. 80ms + margin. */
#define A2D_RFAGC_SETTLE_WAIT_MS    200     /**< RFAGC settling time. */
#define A2D_MIN_FKHZ                42000   /**< 42MHz min RF frequency. */
#define A2D_MAX_FKHZ                880000  /**< 880MHz max RF frequency. */
#define RF_FILTER_CONFIG_WAIT_MS    1000    /**< Wait for CPU Idle in sony_ascot2d_RFFilterConfig */
#define A2D_CHIP_VER                0x20    /**< Chip version number for the ASCOT2D */

/*------------------------------------------------------------------------------
 Statics
------------------------------------------------------------------------------*/

/**
 @brief Implements X_pon sequence.

        @note: Assumes that at least 50us has passed since 
        reset line is released.

 @param pTuner Tuner instance.
 
 @return SONY_RESULT_OK if successful.
*/
static sony_result_t XPOn (sony_ascot2d_t * pTuner);

/**
 @brief Implements A_init (Analog tuning initialisation) sequence.

 @param pTuner Tuner instance.
 @param videoID Indicates analog video system  used.
 @param isDvbC Boolean to indicate whether the DTV system is DVB-C. 
        DVB-C uses Analog tuning but needs to be switched onto the same 
        IF/IFAGC as other digital standards.

 @return SONY_RESULT_OK if successful.
*/
static sony_result_t AInit (sony_ascot2d_t * pTuner, sony_atv_video_t videoID, uint8_t isDvbC);

/**
 @brief Implements D_init (Digital tuning initialisation) sequence.

 @param pTuner Tuner instance.
 
 @return SONY_RESULT_OK if successful.
*/
static sony_result_t DInit (sony_ascot2d_t * pTuner, sony_dtv_system_t system);

/**
 @brief Implements X_fin sequence.

 @param pTuner Tuner instance.
 
 @return SONY_RESULT_OK if successful.
*/
static sony_result_t XFin (sony_ascot2d_t * pTuner);

/**
 @brief Implements X_tune sequence for digital systems.

 @param pTuner Tuner instance.
 
 @return SONY_RESULT_OK if successful.
*/
static sony_result_t DTune (sony_ascot2d_t * pTuner, uint32_t frequency, sony_dtv_system_t dtvSystem);

/**
 @brief Implements X_tune sequence for analog systems.

 @param pTuner Tuner instance.
 
 @return SONY_RESULT_OK if successful.
*/
static sony_result_t ATune (sony_ascot2d_t * pTuner, uint32_t frequency, sony_atv_video_t videoID);

static sony_result_t WaitCpuIdle (sony_ascot2d_t * pTuner, uint16_t timeoutMs);
static sony_result_t XTuneStart (sony_ascot2d_t * pTuner);
static sony_result_t XTuneEnd (sony_ascot2d_t * pTuner);
static sony_result_t CheckDeviceVersion (sony_ascot2d_t * pTuner);


/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

sony_result_t sony_ascot2d_Create (uint8_t i2cAddress,
                                       uint32_t xtalFreq,
                                       sony_i2c_t * pI2c, uint32_t configFlags, void *user, sony_ascot2d_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    printk("sony_ascot2d_Create\n");

    if ((!pTuner) || (!pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((xtalFreq != XTAL_41MHZ) && (xtalFreq != XTAL_4MHZ) && (xtalFreq != XTAL_16MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    pTuner->dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->atvAudioId = SONY_ATV_AUDIO_UNKNOWN;
    pTuner->atvVideoId = SONY_ATV_VIDEO_UNKNOWN;
    pTuner->frequency = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->xtalFreq = xtalFreq;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = user;
    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Initialise the tuner. 
------------------------------------------------------------------------------*/
sony_result_t sony_ascot2d_Initialize (sony_ascot2d_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("sony_ascot2d_Initialize\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = CheckDeviceVersion (pTuner);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    result = XPOn (pTuner);
    /* Device is in "Power Save" state. */
    pTuner->dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->atvAudioId = SONY_ATV_AUDIO_UNKNOWN;
    pTuner->atvVideoId = SONY_ATV_VIDEO_UNKNOWN;
    pTuner->frequency = 0;
    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Set tuner into Power Save mode.
------------------------------------------------------------------------------*/
sony_result_t sony_ascot2d_Finalize (sony_ascot2d_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("sony_ascot2d_Finalize\n");
    
    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    
    result = XFin (pTuner);
    /* Device is in "Power Save" state. */
    pTuner->dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->atvAudioId = SONY_ATV_AUDIO_UNKNOWN;
    pTuner->atvVideoId = SONY_ATV_VIDEO_UNKNOWN;
    pTuner->frequency = 0;
    SONY_TRACE_RETURN (result);
}

static sony_result_t SetupIf (sony_ascot2d_t * pTuner, uint8_t isDigital)
{
    uint8_t data = 0x00;        /* Default: IFOUT1, IFAGC1 */

    printk ("SetupIf\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (((pTuner->flags & SONY_ASCOT2D_CONFIG_IFAGCSEL_MASK) != SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL1) &&
        ((pTuner->flags & SONY_ASCOT2D_CONFIG_IFAGCSEL_MASK) != SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL2) &&
        ((pTuner->flags & SONY_ASCOT2D_CONFIG_IFAGCSEL_MASK) != SONY_ASCOT2D_CONFIG_IFAGCSEL_A1D2) &&
        ((pTuner->flags & SONY_ASCOT2D_CONFIG_IFAGCSEL_MASK) != SONY_ASCOT2D_CONFIG_IFAGCSEL_D1A2)) {
        /* Invalid IF / AGC configuration setting */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set up for digital configuration */
    if (isDigital) {
        /* Select IFOUT and IFAGC output/input. */
        if ((pTuner->flags & SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL2) || (pTuner->flags & SONY_ASCOT2D_CONFIG_IFAGCSEL_A1D2)) {
            data |= 0x04;       /* IFOUTP2/IFOUTN2: 1b */
            data |= 0x08;       /* AGC2: 01b */
        }
    }
    else {
        /* Set up for analog configuration */
        /* Select IFOUT and IFAGC output/input. */
        if ((pTuner->flags & SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL2) || (pTuner->flags & SONY_ASCOT2D_CONFIG_IFAGCSEL_D1A2)) {
            data |= 0x04;       /* IFOUTP2/IFOUTN2: 1b */
            data |= 0x08;       /* AGC2: 01b */
        }
    }
    if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x05, data, 0x1C) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Tuning to analog channel
--------------------------------------------------------------------*/
sony_result_t sony_ascot2d_AtvTune (sony_ascot2d_t * pTuner,
                                        uint32_t frequency, sony_atv_video_t videoID, sony_atv_audio_t audioID)
{
    sony_result_t result = SONY_RESULT_OK;

    printk ("sony_ascot2d_AtvTune\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (videoID != pTuner->atvVideoId) {    /* Video ID change */
        if (videoID == SONY_ATV_VIDEO_UNKNOWN ) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        result = AInit (pTuner, videoID, 0);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
    }

    result = ATune (pTuner, frequency, videoID);
    if (result != SONY_RESULT_OK) {
        pTuner->atvAudioId = SONY_ATV_AUDIO_UNKNOWN;
        pTuner->atvVideoId = SONY_ATV_VIDEO_UNKNOWN;
        pTuner->frequency = 0;
        SONY_TRACE_RETURN (result);
    }

    pTuner->frequency = frequency;
    pTuner->atvVideoId = videoID;
    pTuner->atvAudioId = audioID;
    pTuner->dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Tuning to digital channel
--------------------------------------------------------------------*/
sony_result_t sony_ascot2d_DtvTune (sony_ascot2d_t * pTuner, uint32_t frequency, sony_dtv_system_t dtvSystem)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("sony_ascot2d_DtvTune dtvSystem %d\n",dtvSystem);

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pTuner->dtvSystem != dtvSystem){
        switch (dtvSystem) {
        case SONY_DTV_SYSTEM_DVBC:
            /* Analog system initialisation. */
            result = AInit (pTuner, SONY_ATV_VIDEO_UNKNOWN, 1);
            break;

        /* Intentional fall-through. */
        case SONY_DTV_SYSTEM_ISDBT:
        case SONY_DTV_SYSTEM_DVBT_5:
        case SONY_DTV_SYSTEM_DVBT_6:
        case SONY_DTV_SYSTEM_DVBT_7:
        case SONY_DTV_SYSTEM_DVBT_8:
        case SONY_DTV_SYSTEM_DVBT2_5:
        case SONY_DTV_SYSTEM_DVBT2_6:
        case SONY_DTV_SYSTEM_DVBT2_7:
        case SONY_DTV_SYSTEM_DVBT2_8:
        case SONY_DTV_SYSTEM_DVBC2_6:
        case SONY_DTV_SYSTEM_DVBC2_8:
        
            /* Digital system initialisation. */
            result = DInit (pTuner, dtvSystem);
            break;

        /* Intentional fall-through. */
        case SONY_DTV_SYSTEM_QAM_8VSB:
        case SONY_DTV_SYSTEM_DTMB:
        case SONY_DTV_SYSTEM_UNKNOWN:
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        if (result != SONY_RESULT_OK) {
            pTuner->dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
            pTuner->frequency = 0;
            SONY_TRACE_RETURN (result);
        }
    }

    /* In digital tuning, step size is 25kHz. (using positive rounding) */
    frequency = ((frequency + 25/2) / 25) * 25;

    /* Tune: X_tune (Digital). */
    printk("Dtune frequency %d,dtvSystem %d\n",frequency,dtvSystem);
    result = DTune (pTuner, frequency, dtvSystem);
    if (result == SONY_RESULT_OK) {
        /* Assign current values. */
        pTuner->dtvSystem = dtvSystem;
        pTuner->atvAudioId = SONY_ATV_AUDIO_UNKNOWN;
        pTuner->atvVideoId = SONY_ATV_VIDEO_UNKNOWN;
        pTuner->frequency = frequency;
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t ATune (sony_ascot2d_t * pTuner, uint32_t frequency, sony_atv_video_t videoID)
{
    sony_result_t result = SONY_RESULT_OK;

    printk ("ATune\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = XTuneStart (pTuner);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    {
        uint8_t data[5];        /* 0x10 - 0x14 */
        uint8_t bw = 0;
        uint8_t fifOffset = 0;
        uint8_t bwOffset = 0;

        data[0] = (uint8_t) (frequency & 0xFF); /* 0x10: FRF_L */
        data[1] = (uint8_t) ((frequency >> 8) & 0xFF);  /* 0x11: FRF_M */
        data[2] = (uint8_t) ((frequency >> 16) & 0x0F); /* 0x12: FRF_H [3:0] */
        data[2] |= 0x80;        /* 0x12: IS_FP */
        data[3] = 0xFF;         /* 0x13: VCO calibration enable */
        data[4] = 0xFF;         /* 0x14: Analog block enable */

        /*
         * <PORTING>
         * Sony silicon tuner bandwidth and IF frequency setting for each broadcasting system.
         * These values are optimized for CXD2823ER Sony analog terrestrial demodulator.
         * The user have to change these values if other demodulator is used.
         * Please check Sony silicon tuner application note for detail.
         * 
         * bw : 6MHzBW(0x00) or 7MHzBW(0x01) or 8MHzBW(0x02)
         * fifOffset : FIF_OFFSET[4:0] value (Sub address: 0x0E)
         * bwOffset  : BW_OFFSET[4:0] value (Sub address: 0x0F)
         */
        switch (videoID) {
        case SONY_ATV_VIDEO_M:
            /* Do nothing */
            break;

        case SONY_ATV_VIDEO_M_KOREA:
            fifOffset = (0x03 << 3);
            break;

        case SONY_ATV_VIDEO_BG:
            bw = 0x01;
            fifOffset = (0x0B << 3);
            bwOffset = 0x08;
            break;

        /* Intentional fall-through. */
        case SONY_ATV_VIDEO_IDK:
        case SONY_ATV_VIDEO_L:
            bw = 0x02;
            fifOffset = (0x09 << 3);
            bwOffset = 0x07;
            break;

        case SONY_ATV_VIDEO_L_DASH:
            bw = 0x02;
            fifOffset = (0x1F << 3);
            bwOffset = 0x07;
            data[2] |= 0x40;    /* 0x12: IS_L_DASH */
            break;

            /* Intentional fall-through. */
        case SONY_ATV_VIDEO_UNKNOWN:
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        /* Set FIF_OFFSET */
        result = sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0E, fifOffset, 0xF8);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set BW_OFFSET */
        result = sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0F, bwOffset, 0x1F);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set BW */
        data[2] |= (uint8_t) (bw << 4);

        /* Tune (Burst write) */
        result = pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x10, data, sizeof (data));
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    result = XTuneEnd (pTuner);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
 Sleep the tuner module.
------------------------------------------------------------------------------*/
sony_result_t sony_ascot2d_Sleep (sony_ascot2d_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("sony_ascot2d_Sleep\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = XFin (pTuner);
    /* Device is in "Power Save" state. */
    pTuner->dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->atvAudioId = SONY_ATV_AUDIO_UNKNOWN;
    pTuner->atvVideoId = SONY_ATV_VIDEO_UNKNOWN;
    pTuner->frequency = 0;
    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Write to one of the GPIO pins
------------------------------------------------------------------------------*/
sony_result_t sony_ascot2d_Write_GPIO (sony_ascot2d_t * pTuner, uint8_t id, uint8_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("sony_ascot2d_Write_GPIO\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (id == 0){
        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x05, (uint8_t) (value? 0x01 : 0x00), 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
    else if (id == 1){
        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x05, (uint8_t) (value? 0x02 : 0x00), 0x02) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
    else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 X_pon
------------------------------------------------------------------------------*/
static sony_result_t XPOn (sony_ascot2d_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("XPOn\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* XTAL configuration. */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x01, (uint8_t) pTuner->xtalFreq) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* 41MHz special setting (external input or XTAL). */
    if (pTuner->xtalFreq == XTAL_41MHZ) {
        if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x44, 0x07) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    {
        uint8_t data = 0;

        /* NVM bank selection setting (INIT_NVM_CMD) */
        switch (pTuner->flags & SONY_ASCOT2D_CONFIG_NVMSEL_MASK) {
        case SONY_ASCOT2D_CONFIG_NVMSEL_0_1:
            data = 0x3C;
            break;
        case SONY_ASCOT2D_CONFIG_NVMSEL_AUTO:
            data = 0x00;
            break;
        case SONY_ASCOT2D_CONFIG_NVMSEL_0_3:
            data = 0x1C;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        result = pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x4D, data);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Logic wake up */
    result = pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x03, 0xC4);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Boot CPU. */
    result = pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x04, 0x40);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Using internal RFAGC (Overload) circuit. */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x21, 0x18) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* RFVGA optimization setting */
    {
        const uint8_t cdata[5] = { 0x10, 0x10, 0x10, 0x10, 0x10 };  /* RF_DIST1 - RF_DIST5 */
        result = pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x23, cdata, sizeof (cdata));
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* RF_GAIN auto control setting */
    result = pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x4E, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Wait CPU idle. */
    result = WaitCpuIdle (pTuner, A2D_XPON_WAIT_MS);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Check CPU error. */
    {
        uint8_t data = 0;
        if (pTuner->pI2c->ReadRegister (pTuner->pI2c, pTuner->i2cAddress, 0x1B, &data, 1) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if ((data & 0x3F) != 0x00) {
            /* NVM Error. */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
        }
    }

    /* XOSC_SEL. Max. of DVB-C/T/T2 drive current for given XTAL config. */
    {
        uint8_t data;

        if (pTuner->flags & SONY_ASCOT2D_CONFIG_EXT_REF) {
            data = 0x00; /* XOSC_SEL = disabled */
        }
        else {
            data = 0x10; /* XOSC_SEL = 400uA */
        }
        
        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x07, data, 0x1F) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Place CPU into deep sleep. */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x04, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Sleep logic. */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x03, 0xC0) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Enable Power Save and Disable PLL. */
    {
        static const uint8_t data[] = { 0x00, 0x04 };
        if (pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x14, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 X_fin
------------------------------------------------------------------------------*/
static sony_result_t XFin (sony_ascot2d_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("XFin\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Enable Power Save and Disable PLL. */
    {
        static const uint8_t data[] = { 0x00, 0x04 };
        if (pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x14, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 A_init
------------------------------------------------------------------------------*/
static sony_result_t AInit (sony_ascot2d_t * pTuner, sony_atv_video_t videoID, uint8_t isDvbC)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("AInit\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((videoID != SONY_ATV_VIDEO_UNKNOWN) && (isDvbC)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Disable Power Save and Enable PLL. */
    {
        static const uint8_t data[] = { 0xFB, 0x0F };
        if (pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x14, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Select correct IFOUT/IFAGC. */
    result = SetupIf (pTuner, isDvbC);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* REF_R: Depends on Xtal. */
    /* 41MHz = 40, 16MHz = 16, 4MHz = 4 */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x06,
                                       (uint8_t) (pTuner->xtalFreq == XTAL_41MHZ ? 40 : pTuner->xtalFreq)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* XOSC_SEL: (41MHz = 400uA, 16MHz =400uA, 4MHz = 50uA) */
    {
        uint8_t data;

        if (pTuner->flags & SONY_ASCOT2D_CONFIG_EXT_REF) {
            data = 0x00; /* XOSC_SEL = disabled */
        }
        else if (pTuner->xtalFreq == XTAL_4MHZ) {
            data = 0x02; /* XOSC_SEL = 50uA */
        }
        else {
            data = 0x10; /* XOSC_SEL = 400uA */
        }

        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x07, data, 0x1F) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* KBW: Wc = 1.5kHz */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x08, 0x0E) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* C2_BANK, R2_BANK, R2_RANGE, ORDER: C2=100pF, R2=10K, R2_RANGE=0, ORDER=1 (default)*/
    if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0B, 0x0F, 0x3F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* MIX_OLL */
    {
        uint8_t data = ((videoID == SONY_ATV_VIDEO_L || videoID == SONY_ATV_VIDEO_L_DASH))? 0x05 : 0x02; /* 4.0Vpp (L/L') or 2.0Vpp (Others) */
        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0C, data, 0x07) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* IF_BPF_GC */
    {
        uint8_t data = videoID == SONY_ATV_VIDEO_L || videoID == SONY_ATV_VIDEO_L_DASH ? 0x01 : 0x03; /* 0dB (System-L/L') or +4dB (Others) */
        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0E, data, 0x07) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* RF_OLDET_OLL, RF_OLDET_ENX */
    {
        /* -34dBm (DVB-C/ System M) or -32dBm (Others) */
        uint8_t rf_oldet_oll = ((isDvbC) || (videoID == SONY_ATV_VIDEO_M) || (videoID == SONY_ATV_VIDEO_M_KOREA)) ? 0x01 : 0x02; 
        if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x49, rf_oldet_oll) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 D_init
------------------------------------------------------------------------------*/
static sony_result_t DInit (sony_ascot2d_t * pTuner, sony_dtv_system_t system)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("DInit\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Disable Power Save and Enable PLL. */
    {
        static const uint8_t data[] = { 0xFB, 0x0F };
        if (pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x14, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Select correct IFOUT/IFAGC. */
    result = SetupIf (pTuner, 1);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* REF_R: Depends on Xtal. 41MHz = 10, 16MHz = 4, 4MHz = 1*/
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x06, (uint8_t) (pTuner->xtalFreq) / 4) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }


    /* XOSC_SEL: (41MHz = 400uA, 16MHz = 400uA, 4MHz = 400uA (LPN On) or 100uA (LPN Off)) */
    {
        uint8_t data;

        if (pTuner->flags & SONY_ASCOT2D_CONFIG_EXT_REF) {
            data = 0x00; /* XOSC_SEL = disabled */
        }
        else {
            data = 0x10; /* XOSC_SEL = 400uA */
        }

        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x07, data, 0x1F) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* KBW: Wc = 30kHz */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x08, 0x30) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* C2_BANK, R2_BANK, R2_RANGE, ORDER: C2=100pF, R2=10K, R2_RANGE=0, ORDER=1 */
    if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0B, 0x37, 0x3F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* MIX_OLL: 2.5Vp-p */
    if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0C, 0x03, 0x07) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        /* IF_BPF_GC: +12dB BPF, C2 = 0dB */
        uint8_t data;
        if ((system == SONY_DTV_SYSTEM_DVBC2_6 || system == SONY_DTV_SYSTEM_DVBC2_8)){
            data = 0x01;
        }
        else{
            data = 0x06;
        }

        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0E, data, 0x07) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* RF_OLDET_OLL, RF_OLDET_ENX : C2 = -30dB, Others = -26dB*/
    {
        uint8_t rf_oldet_oll = (system == SONY_DTV_SYSTEM_DVBC2_6 || system == SONY_DTV_SYSTEM_DVBC2_8) ? 0x03 : 0x05;
        
        if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x49, rf_oldet_oll) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }


    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 X_tune : Digital
------------------------------------------------------------------------------*/
static sony_result_t DTune (sony_ascot2d_t * pTuner, uint32_t frequency, sony_dtv_system_t dtvSystem)
{
    sony_result_t result = SONY_RESULT_OK;
    printk ("DTune\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((frequency < A2D_MIN_FKHZ) || (frequency > A2D_MAX_FKHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    /* XTuneStart */
    result = XTuneStart (pTuner);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /*Channel bandwidth and IF settings
     * System    BW      Actual BW   IF
     * -------------------------------------
     * DVB-T     8MHz    7.90MHz     4.80MHz
     * DVB-T     7MHz    6.80MHz     4.20MHz
     * DVB-T     6MHz    5.70MHz     3.60MHz
     * DVB-T     5MHz    5.70MHz     3.60MHz
     * DVB-T2    8MHz    7.90MHz     4.80MHz
     * DVB-T2    7MHz    6.80MHz     4.20MHz
     * DVB-T2    6MHz    5.70MHz     3.60MHz
     * DVB-T2    5MHz    5.70MHz     3.60MHz
     * DVB-C     ----    7.95MHz     4.90MHz
     * DVB-C2    8MHz    8.10MHz     4.90MHz
     * DVB-C2    6MHz    5.80MHz     3.70MHz
     */
    {
        uint8_t bw = 0;
        uint8_t fifOffset = 0;
        uint8_t bwOffset = 0;
        uint8_t data[5];

        data[0] = (uint8_t) (frequency & 0xFF);
        data[1] = (uint8_t) ((frequency & 0xFF00) >> 8);
        data[2] = (uint8_t) ((frequency & 0x0F0000) >> 16);
        data[3] = 0xFF;         /* VCO calibration. */
        data[4] = 0xFF;         /* Enable Analog block. */

        switch (dtvSystem) {
        case SONY_DTV_SYSTEM_ISDBT:
            /* Do Nothing */
            break;

        /* Intentional fall through. */
        case SONY_DTV_SYSTEM_DVBT_5:
        case SONY_DTV_SYSTEM_DVBT_6:
        case SONY_DTV_SYSTEM_DVBT2_5:
        case SONY_DTV_SYSTEM_DVBT2_6:
            fifOffset = (0x18 << 3);
            bwOffset = 0x1A;
            break;

        /* Intentional fall through. */
        case SONY_DTV_SYSTEM_DVBT_7:
        case SONY_DTV_SYSTEM_DVBT2_7:
            bw = 0x01;
            fifOffset = (0x1A << 3);
            bwOffset = 0x1C;
            break;

        /* Intentional fall through. */
        case SONY_DTV_SYSTEM_DVBT_8:
        case SONY_DTV_SYSTEM_DVBT2_8:
            bw = 0x02;
            fifOffset = (0x1C << 3);
            bwOffset = 0x1E;
            break;

        case SONY_DTV_SYSTEM_DVBC2_6:
            fifOffset = (0x1A << 3);
            bwOffset = 0x1C;
            break;

        case SONY_DTV_SYSTEM_DVBC2_8:
            bw = 0x02;
            fifOffset = (0x1E << 3);
            bwOffset = 0x02;
            break;

        case SONY_DTV_SYSTEM_DVBC:
            bw = 0x02;
            fifOffset = (0x1E << 3);
            bwOffset = 0x1F;
            break;

            /* Intentional fall-through. */
        case SONY_DTV_SYSTEM_QAM_8VSB:
        case SONY_DTV_SYSTEM_DTMB:
        case SONY_DTV_SYSTEM_UNKNOWN:
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* 0x0E: FIF_OFFSET: IF offset. */
        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0E, fifOffset, 0xF8) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* 0x0F: BW_OFFSET: BW offset. */
        if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0F, bwOffset, 0x1F) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set BW */
        data[2] |= (uint8_t) (bw << 4);

        /* Tune. */
        if (pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x10, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = XTuneEnd (pTuner);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t WaitCpuIdle (sony_ascot2d_t * pTuner, uint16_t timeoutMs)
{
    uint8_t i;
    uint8_t data = 0x00;
    sony_result_t result = SONY_RESULT_ERROR_TIMEOUT;
    printk ("WaitCpuIdle\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    {
        for (i = 0 ; i <= timeoutMs ; i++) {
            msleep(1);

            if (pTuner->pI2c->ReadRegister (pTuner->pI2c, pTuner->i2cAddress, 0x1A, &data, 1) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (data == 0x00) {
                result = SONY_RESULT_OK;
                break;
            }
        }
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t XTuneStart (sony_ascot2d_t * pTuner)
{
    printk ("XTuneStart\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* RFAGC Fast Mode : Set MIX_TOL = FastMode.  */
    if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0C, 0x10, 0x30) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        static const uint8_t data[] = { 0xC4, 0x40 };
        /* Wake logic/CPU. */
        if (pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x03, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t XTuneEnd (sony_ascot2d_t * pTuner)
{
    printk ("XTuneEnd\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Wait 200ms for RFAGC settling time. */
    msleep (A2D_RFAGC_SETTLE_WAIT_MS);

    /* Place CPU into deep sleep. */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x04, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Sleep logic. */
    if (pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x03, 0xC0) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    
    /* RFAGC Slow Mode */
    if (sony_i2c_SetRegisterBits (pTuner->pI2c, pTuner->i2cAddress, 0x0C, 0x00, 0x30) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_ascot2d_RFFilterConfig (sony_ascot2d_t * pTuner, uint8_t coeff, uint8_t offset)
{
    sony_result_t result = SONY_RESULT_OK;

    printk ("sony_ascot2d_RFFilterConfig\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Logic wake up */
    result = pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x03, 0xC4);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* CPU wake up */
    result = pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x04, 0x40);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        uint8_t data[3];

        /* Write VL_TRCKOUT_COEFF */
        data[0] = coeff;
        data[1] = 0x49;
        data[2] = 0x03;
        result = pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x16, data, sizeof (data));
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = WaitCpuIdle (pTuner, RF_FILTER_CONFIG_WAIT_MS);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Write VL_TRCKOUT_OFS */
        data[0] = offset;
        data[1] = 0x4B;
        data[2] = 0x03;
        result = pTuner->pI2c->WriteRegister (pTuner->pI2c, pTuner->i2cAddress, 0x16, data, sizeof (data));
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = WaitCpuIdle (pTuner, RF_FILTER_CONFIG_WAIT_MS);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* CPU deep sleep */
    result = pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x04, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Logic sleep */
    result = pTuner->pI2c->WriteOneRegister (pTuner->pI2c, pTuner->i2cAddress, 0x03, 0xC0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Read Version Register and confirm chip version is correct
--------------------------------------------------------------------*/
static sony_result_t CheckDeviceVersion (sony_ascot2d_t * pTuner)
{
    uint8_t data = 0;
    printk ("CheckDeviceVersion\n");

    if ((!pTuner) || (!pTuner->pI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Read Version Register and confirm chip version is correct. */
    printk("pTuner->addr 0x%02x\n",pTuner->i2cAddress);
    //printk("pTuner->pI2c->ReadRegister 0x%x\n",pTuner->pI2c->ReadRegister);
    if (pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7F, &data, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* ASCOT2D can have version number of b'0010XXXX or b'0011XXXX so checking bit 5 after masking out the lower 5bits is sufficient */
    if ((data & 0xE0) != A2D_CHIP_VER) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
