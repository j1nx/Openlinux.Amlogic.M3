/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2776 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>

#include "sony_dvb_demod.h"
#include "sony_dvb_demodT2.h"
#include "sony_dvb_demod_monitor.h"
#include "sony_dvb_demod_monitorT2.h"

#ifdef SONY_DVB_SUPPORT_DVBT2

/**
 @brief Sleep to active T2 (G) state.
*/
static sony_result_t SL2NOG (sony_dvb_demod_t * pDemod);
/**
 @brief Sleep to active T2 (G) state: bandwidth configuration.
*/
static sony_result_t SL2NOG_BandSetting (sony_dvb_demod_t * pDemod);
/**
 @brief Active T2 state (G) to Sleep.
*/
static sony_result_t NOG2SL (sony_dvb_demod_t * pDemod);

sony_result_t sony_dvb_demodT2_ScanInitialize (sony_dvb_demod_t * pDemod, sony_dvb_scan_params_t * pScanParams)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodT2_ScanInitialize");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    SONY_ARG_UNUSED(pScanParams);

    result = sony_dvb_demodT2_EnableBlindAcquisition (pDemod);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_ScanFinalize (sony_dvb_demod_t * pDemod, sony_dvb_scan_params_t * pScanParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodT2_ScanFinalize");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_ARG_UNUSED(pScanParams);
    
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_TuneInitialize (sony_dvb_demod_t * pDemod, sony_dvb_tune_params_t * pTuneParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodT2_TuneInitialize");

    if (!pDemod || !pTuneParams)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    pTuneParams->t2Params.tuneResult = 0x00;

    /* Configure for non-auto PLP selection. */
    result = sony_dvb_demodT2_SetPLPConfig (pDemod, 0x01, pTuneParams->t2Params.dataPLPId);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_TuneFinalize (sony_dvb_demod_t * pDemod, sony_dvb_tune_params_t * pTuneParams, sony_result_t tuneResult)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("sony_dvb_demodT2_TuneFinalize");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    if(tuneResult == SONY_RESULT_OK) { 
    
        result = sony_dvb_demod_monitorT2_DataPLPError (pDemod, &data);
        if ((result == SONY_RESULT_OK) && (data)) {
            pTuneParams->t2Params.tuneResult |= SONY_DVBT2_TUNE_RESULT_DATA_PLP_NOT_FOUND;
        }
    }
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_Tune (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodT2_Tune");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = SL2NOG (pDemod);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_Sleep (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodT2_Sleep");
    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = NOG2SL (pDemod);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_BandwidthChanged (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodT2_BandwidthChanged");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = SL2NOG_BandSetting (pDemod);
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_EnableBlindAcquisition (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodT2_EnableBlindAcquisition");

    if ((!pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Enable Auto PLP selection. */
    result = sony_dvb_demodT2_SetPLPConfig (pDemod, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demodT2_CheckDemodLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
  
    uint8_t sync = 0;
    uint8_t tslock = 0;
    uint8_t unlockDetected = 0;

    SONY_TRACE_ENTER ("sony_dvb_demodT2_CheckDemodLock");

    if (!pDemod || !pLock)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    *pLock = DEMOD_LOCK_RESULT_NOTDETECT;

    result = sony_dvb_demod_monitorT2_SyncStat( pDemod, &sync, &tslock, &unlockDetected);

    if (result != SONY_RESULT_OK) {
       SONY_TRACE_RETURN (result);
    }

    if (unlockDetected) {
        *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    if (sync >= 6) {
        *pLock = DEMOD_LOCK_RESULT_LOCKED;
    }
    
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_CheckTSLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t sync = 0; 
    uint8_t tslock = 0;
    uint8_t unlockDetected = 0;

    SONY_TRACE_ENTER ("sony_dvb_demodT2_CheckTSLock");

    if (!pDemod || !pLock)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    *pLock = DEMOD_LOCK_RESULT_NOTDETECT;

    result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &sync, &tslock, &unlockDetected);
    
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
    
    if (unlockDetected) {
        *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    if ((sync >= 6) && tslock) {
        *pLock = DEMOD_LOCK_RESULT_LOCKED;
    }
    
    SONY_TRACE_RETURN (result);

}

sony_result_t sony_dvb_demodT2_SetPLPConfig (sony_dvb_demod_t * pDemod, uint8_t autoPLP, uint8_t plpId)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodT2_SetPLPConfig");
    
    if ((!pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE && pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP) {
        /* This api is accepted in ACTIVE or SLEEP states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Argument check */
    if (autoPLP >= 3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set Bank 0x23 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x23) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (autoPLP != 0) {
        /* Manual PLP selection mode. Set the data PLP Id. */
        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   23h     AFh    [7:0]     8'h00      8'h**      OREGD_FP_PLP_ID[7:0]
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xAF, plpId) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Auto PLP select (Scanning mode = 0x00). Data PLP select = 0x01. */
    /* slave    Bank    Addr    Bit    default   Value          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   23h     ADh    [1:0]     8'h00      8'h01      OREGD_FP_PLP_AUTO_SEL[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xAD, autoPLP & 0x03) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_SetConfig (sony_dvb_demod_t * pDemod, sony_dvb_demod_config_id_t configId, int32_t value)
{
    SONY_TRACE_ENTER ("sony_dvb_demodT2_SetConfig");
    
    SONY_ARG_UNUSED (pDemod);
    SONY_ARG_UNUSED (configId);
    SONY_ARG_UNUSED (value);
    
    SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    
    
}

sony_result_t sony_dvb_demodT2_InitializeMISOSetting (sony_dvb_demod_t * pDemod)
{
    const uint8_t data1[3] = {0x00,0x01,0x00};
    const uint8_t data2[2] = {0x00,0x6C};

    SONY_TRACE_ENTER ("sony_dvb_demodT2_InitializeMISOSetting");

    if ((!pDemod)) {
       SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE)
        && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        /* This api is accepted in ACTIVE or SLEEP states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set Bank 0x27 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x27) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x26, data1, sizeof (data1)) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x35, data2, sizeof (data2)) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demodT2_OptimizeMISO (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dvbt2_ofdm_t ofdm;

    SONY_TRACE_ENTER ("sony_dvb_demodT2_OptimizeMISO");

    if ((!pDemod)) {
       SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_dvb_demod_monitorT2_Ofdm(pDemod, &ofdm);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if ( ofdm.s1 == SONY_DVBT2_S1_MISO ) {
        const uint8_t data1[3] = {0x00,0x01,0xB3};
        const uint8_t data2[2] = {0x01,0x8C};
        /* Set Bank 0x27 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x27) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
 
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x26, data1, sizeof (data1)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x35, data2, sizeof (data2)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT2_CheckBandwidth(uint8_t bandWidth)
{
    SONY_TRACE_ENTER ("sony_dvb_demodT2_CheckBandwidth");

    if ((bandWidth != SONY_DVBT2_BW_5_MHZ) && (bandWidth != SONY_DVBT2_BW_6_MHZ) && (bandWidth != SONY_DVBT2_BW_7_MHZ) && (bandWidth != SONY_DVBT2_BW_8_MHZ)) {
        /* Not supported bandwidth */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t SL2NOG (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("SL2NOG");

    /* Argument verification. */
    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* DVB-T2 demod setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x80, 0x02) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* DVB-T2 demod clock enable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x81,(pDemod->enable_rflvmon)? 0x8D:0x0D) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* ADC enable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x85, 0x07) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* RFlevel monitor enable */
    if (pDemod->enable_rflvmon) {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, 0x02) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    } 

    /* Set bank 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* eDRAM enable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x69, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Setup band setting. */
    result = SL2NOG_BandSetting (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Hi-Z setting disable (other than TS) */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, 0x00, 0x05) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t NOG2SL (sony_dvb_demod_t * pDemod)
{

    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("NOG2SL");

    /* Argument verification. */
    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set bank 0x0. */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable TS output. (0=Output, 1=Hi-Z) */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xF1, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Hi-| setting. (0=Output, 1=Hi-Z) */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, 0x05, 0x05) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = sony_dvb_demodT2_InitializeMISOSetting (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set bank 0x0. */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* ADC disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c,
        pDemod->i2cAddress, 0x85, (pDemod->sharedIf ? 0x06 : 0x00)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* RFlevel monitor disable */
    if (pDemod->enable_rflvmon) {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    } 

    /* Set bank 0x20. */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* eDRAM disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x69, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set bank 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* DVB-T2 demod Clock disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x81, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* DVB-T2 demod disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x80, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Update state. */
    pDemod->state = SONY_DVB_DEMOD_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t SL2NOG_BandSetting (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("SL2NOG_BandSetting");

    /* Argument verification. */
    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    /*  slave    Bank    Addr    Bit    default     Value          Name
     *  ----------------------------------------------------------------------------------
     *  <SLV-T>   00h     D2h    [4:0]     8'h0C      8'h0C      OCTL_IFAGC_COARSEGAIN[4:0]
     */

    /* Set bank 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xD2, 0x0C) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);


    /*  slave    Bank    Addr    Bit    default     Value          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   01h     6Ah    [7:0]     8'h50      8'h50      OREG_ITB_DAGC_TRGT[7:0]
     */
    /* Set bank 0x01 */


    /* Set bank 0x00 */

    
    if (pDemod->is_ascot) {
        /* slave    Bank    Addr    Bit    default   Value               Name
         * ------------------------------------------------------------------------------------
         * <SLV-T>   00h     82h    [6:5]     8'hD6      8'hD6     OREG_ADC_ADCGAIN[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x82, 0x40, 0x60) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     A5h     [0]      8'h01      8'h01     OREG_ITB_GDEQ_EN
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xA5, 0x01, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
    else {
        /* slave    Bank    Addr    Bit    default   Value               Name
         * ------------------------------------------------------------------------------------
         * <SLV-T>   00h     82h    [6:5]     8'hD6      8'hB6     OREG_ADC_ADCGAIN[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x82, 0x20, 0x60) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     A5h     [0]      8'h01      8'h00     OREG_ITB_GDEQ_EN
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xA5, 0x00, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x2B) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xA5, 0x11, 0x7F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }   
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch (pDemod->bandWidth) {
    case SONY_DVBT2_BW_8_MHZ:
    {
        /* TRL settings for 8MHz. */
        /* slave    Bank    Addr    Bit    default   Value          Name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   20h     9Fh    [5:0]     8'h11      8'h11      OREG_TRCG_NOMINALRATE[37:32]
         * <SLV-T>   20h     A0h    [7:0]     8'hF0      8'hF0      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   20h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   20h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   20h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
         */
        {
            static const uint8_t data[] = { 0x11, 0xF0, 0x00, 0x00, 0x00 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9F, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        /*
         * <ASCOT group delay equalizer settings for ASCOT2D>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T2>   20h     A6h    [7:0]     8'h1E      8'h26      OREG_ITB_COEF01[7:0]
         * <SLV-T2>   20h     A7h    [7:0]     8'h1D      8'hAF      OREG_ITB_COEF02[7:0]
         * <SLV-T2>   20h     A8h    [7:0]     8'h29      8'h06      OREG_ITB_COEF11[7:0]
         * <SLV-T2>   20h     A9h    [7:0]     8'hC9      8'hCD      OREG_ITB_COEF12[7:0]
         * <SLV-T2>   20h     AAh    [7:0]     8'h2A      8'h13      OREG_ITB_COEF21[7:0]
         * <SLV-T2>   20h     ABh    [7:0]     8'hBA      8'hBB      OREG_ITB_COEF22[7:0]
         * <SLV-T2>   20h     ACh    [7:0]     8'h29      8'h28      OREG_ITB_COEF31[7:0]
         * <SLV-T2>   20h     ADh    [7:0]     8'hAD      8'hBA      OREG_ITB_COEF32[7:0]
         * <SLV-T2>   20h     AEh    [7:0]     8'h29      8'h23      OREG_ITB_COEF41[7:0]
         * <SLV-T2>   20h     AFh    [7:0]     8'hA4      8'hA9      OREG_ITB_COEF42[7:0]
         * <SLV-T2>   20h     B0h    [7:0]     8'h29      8'h1F      OREG_ITB_COEF51[7:0]
         * <SLV-T2>   20h     B1h    [7:0]     8'h9A      8'hA8      OREG_ITB_COEF52[7:0]
         * <SLV-T2>   20h     B2h    [7:0]     8'h28      8'h2C      OREG_ITB_COEF61[7:0]
         * <SLV-T2>   20h     B3h    [7:0]     8'h9E      8'hC8      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {    /* Group delay equalizer settings for ASCOT2D tuner 8MHz. */
            static const uint8_t data[] =
                { 0x26, 0xAF, 0x06, 0xCD, 0x13, 0xBB, 0x28, 0xBA, 0x23, 0xA9, 0x1F, 0xA8, 0x2C, 0xC8 };
            /* 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3 */
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        }
        else if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {   /* Group delay equalizer settings for ASCOT2S tuner 8MHz. */
            /*
             * <ASCOT group delay equalizer settings for ASCOT/ASCOT2S>
             * slave    bank    addr   bit      default    value       name
             * ---------------------------------------------------------------------------------
             * <SLV-T>   20h     A6h    [7:0]     8'h1E      8'h1E      OREG_ITB_COEF01[7:0]
             * <SLV-T>   20h     A7h    [7:0]     8'h1D      8'h1D      OREG_ITB_COEF02[7:0]
             * <SLV-T>   20h     A8h    [7:0]     8'h29      8'h29      OREG_ITB_COEF11[7:0]
             * <SLV-T>   20h     A9h    [7:0]     8'hC9      8'hC9      OREG_ITB_COEF12[7:0]
             * <SLV-T>   20h     AAh    [7:0]     8'h2A      8'h2A      OREG_ITB_COEF21[7:0]
             * <SLV-T>   20h     ABh    [7:0]     8'hBA      8'hBA      OREG_ITB_COEF22[7:0]
             * <SLV-T>   20h     ACh    [7:0]     8'h29      8'h29      OREG_ITB_COEF31[7:0]
             * <SLV-T>   20h     ADh    [7:0]     8'hAD      8'hAD      OREG_ITB_COEF32[7:0]
             * <SLV-T>   20h     AEh    [7:0]     8'h29      8'h29      OREG_ITB_COEF41[7:0]
             * <SLV-T>   20h     AFh    [7:0]     8'hA4      8'hA4      OREG_ITB_COEF42[7:0]
             * <SLV-T>   20h     B0h    [7:0]     8'h29      8'h29      OREG_ITB_COEF51[7:0]
             * <SLV-T>   20h     B1h    [7:0]     8'h9A      8'h9A      OREG_ITB_COEF52[7:0]
             * <SLV-T>   20h     B2h    [7:0]     8'h28      8'h28      OREG_ITB_COEF61[7:0]
             * <SLV-T>   20h     B3h    [7:0]     8'h9E      8'h9E      OREG_ITB_COEF62[7:0]
             */
            static const uint8_t data[] =
                { 0x1E, 0x1D, 0x29, 0xC9, 0x2A, 0xBA, 0x29, 0xAD, 0x29, 0xA4, 0x29, 0x9A, 0x28, 0x9E };
            /*0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3 */
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   20h     B6h    [7:0]     8'h1F      8'h1E      OREG_DNCNV_LOFRQ[23:16]
         * <SLV-T>   20h     B7h    [7:0]     8'h38      8'h98      OREG_DNCNV_LOFRQ[15:8]
         * <SLV-T>   20h     B8h    [7:0]     8'h32      8'h5A      OREG_DNCNV_LOFRQ[7:0]
         */
        {
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_8 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_8 >> 8) & 0xFF);
            data[2] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_8) & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   20h     D7h    [7:6]     8'h00      8'h00      OREG_CHANNEL_WIDTH[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD7, 0x00, 0xC0) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
        break;

    case SONY_DVBT2_BW_7_MHZ:
    {
        /*slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     9Fh    [5:0]     8'h11      8'h14      OREG_TRCG_NOMINALRATE[37:32]
         * <SLV-T>   20h     A0h    [7:0]     8'hF0      8'h80      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   20h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   20h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   20h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
         */
        {
            static const uint8_t data[] = { 0x14, 0x80, 0x00, 0x00, 0x00 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9F, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /*
         * <ASCOT group delay equalizer settings for ASCOT2D>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     A6h    [7:0]     8'h1E      8'h2C      OREG_ITB_COEF01[7:0]
         * <SLV-T>   20h     A7h    [7:0]     8'h1D      8'hBD      OREG_ITB_COEF02[7:0]
         * <SLV-T>   20h     A8h    [7:0]     8'h29      8'h02      OREG_ITB_COEF11[7:0]
         * <SLV-T>   20h     A9h    [7:0]     8'hC9      8'hCF      OREG_ITB_COEF12[7:0]
         * <SLV-T>   20h     AAh    [7:0]     8'h2A      8'h04      OREG_ITB_COEF21[7:0]
         * <SLV-T>   20h     ABh    [7:0]     8'hBA      8'hF8      OREG_ITB_COEF22[7:0]
         * <SLV-T>   20h     ACh    [7:0]     8'h29      8'h23      OREG_ITB_COEF31[7:0]
         * <SLV-T>   20h     ADh    [7:0]     8'hAD      8'hA6      OREG_ITB_COEF32[7:0]
         * <SLV-T>   20h     AEh    [7:0]     8'h29      8'h29      OREG_ITB_COEF41[7:0]
         * <SLV-T>   20h     AFh    [7:0]     8'hA4      8'hB0      OREG_ITB_COEF42[7:0]
         * <SLV-T>   20h     B0h    [7:0]     8'h29      8'h26      OREG_ITB_COEF51[7:0]
         * <SLV-T>   20h     B1h    [7:0]     8'h9A      8'hA9      OREG_ITB_COEF52[7:0]
         * <SLV-T>   20h     B2h    [7:0]     8'h28      8'h21      OREG_ITB_COEF61[7:0]
         * <SLV-T>   20h     B3h    [7:0]     8'h9E      8'hA5      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {    /* Group delay equalizer settings for ASCOT2D tuner 7MHz. */
            static const uint8_t data[] =
                { 0x2C, 0xBD, 0x02, 0xCF, 0x04, 0xF8, 0x23, 0xA6, 0x29, 0xB0, 0x26, 0xA9, 0x21, 0xA5 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        }
        else if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {   /* Group delay equalizer settings for ASCOT2S tuner 7MHz. */
            /*
             * <ASCOT group delay equalizer settings for ASCOT/ASCOT2S>
             * slave    bank    addr    bit    default   value          name
             * ----------------------------------------------------------------------------------
             * <SLV-T>   20h     A6h    [7:0]     8'h1E      8'h1B      OREG_ITB_COEF01[7:0]
             * <SLV-T>   20h     A7h    [7:0]     8'h1D      8'h22      OREG_ITB_COEF02[7:0]
             * <SLV-T>   20h     A8h    [7:0]     8'h29      8'h2B      OREG_ITB_COEF11[7:0]
             * <SLV-T>   20h     A9h    [7:0]     8'hC9      8'hC1      OREG_ITB_COEF12[7:0]
             * <SLV-T>   20h     AAh    [7:0]     8'h2A      8'h2C      OREG_ITB_COEF21[7:0]
             * <SLV-T>   20h     ABh    [7:0]     8'hBA      8'hB3      OREG_ITB_COEF22[7:0]
             * <SLV-T>   20h     ACh    [7:0]     8'h29      8'h2B      OREG_ITB_COEF31[7:0]
             * <SLV-T>   20h     ADh    [7:0]     8'hAD      8'hA9      OREG_ITB_COEF32[7:0]
             * <SLV-T>   20h     AEh    [7:0]     8'h29      8'h2B      OREG_ITB_COEF41[7:0]
             * <SLV-T>   20h     AFh    [7:0]     8'hA4      8'hA0      OREG_ITB_COEF42[7:0]
             * <SLV-T>   20h     B0h    [7:0]     8'h29      8'h2B      OREG_ITB_COEF51[7:0]
             * <SLV-T>   20h     B1h    [7:0]     8'h9A      8'h97      OREG_ITB_COEF52[7:0]
             * <SLV-T>   20h     B2h    [7:0]     8'h28      8'h2B      OREG_ITB_COEF61[7:0]
             * <SLV-T>   20h     B3h    [7:0]     8'h9E      8'h9B      OREG_ITB_COEF62[7:0]
             */
            static const uint8_t data[] =
                { 0x1B, 0x22, 0x2B, 0xC1, 0x2C, 0xB3, 0x2B, 0xA9, 0x2B, 0xA0, 0x2B, 0x97, 0x2B, 0x9B };
            /* xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3 */
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     B6h    [7:0]     8'h1F      8'h1B      OREG_DNCNV_LOFRQ[23:16]
         * <SLV-T>   20h     B7h    [7:0]     8'h38      8'h79      OREG_DNCNV_LOFRQ[15:8]
         * <SLV-T>   20h     B8h    [7:0]     8'h32      8'h22      OREG_DNCNV_LOFRQ[7:0]
         */
        {
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_7 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_7 >> 8) & 0xFF);
            data[2] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_7) & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     D7h    [7:6]     8'h00      8'h40      OREG_CHANNEL_WIDTH[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD7, 0x40, 0xC0) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
        break;

    case SONY_DVBT2_BW_6_MHZ:
    {
        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     9Fh    [5:0]     8'h11      8'h17      OREG_TRCG_NOMINALRATE[37:32]
         * <SLV-T>   20h     A0h    [7:0]     8'hF0      8'hEA      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   20h     A1h    [7:0]     8'h00      8'hAA      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   20h     A2h    [7:0]     8'h00      8'hAA      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   20h     A3h    [7:0]     8'h00      8'hAA      OREG_TRCG_NOMINALRATE[7:0]
         */
        {
            static const uint8_t data[] = { 0x17, 0xEA, 0xAA, 0xAA, 0xAA };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9F, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /*
         * <ASCOT group delay equalizer settings for ASCOT2D>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     A6h    [7:0]     8'h1E      8'h27      OREG_ITB_COEF01[7:0]
         * <SLV-T>   20h     A7h    [7:0]     8'h1D      8'hA7      OREG_ITB_COEF02[7:0]
         * <SLV-T>   20h     A8h    [7:0]     8'h29      8'h28      OREG_ITB_COEF11[7:0]
         * <SLV-T>   20h     A9h    [7:0]     8'hC9      8'hB3      OREG_ITB_COEF12[7:0]
         * <SLV-T>   20h     AAh    [7:0]     8'h2A      8'h02      OREG_ITB_COEF21[7:0]
         * <SLV-T>   20h     ABh    [7:0]     8'hBA      8'hF0      OREG_ITB_COEF22[7:0]
         * <SLV-T>   20h     ACh    [7:0]     8'h29      8'h01      OREG_ITB_COEF31[7:0]
         * <SLV-T>   20h     ADh    [7:0]     8'hAD      8'hE8      OREG_ITB_COEF32[7:0]
         * <SLV-T>   20h     AEh    [7:0]     8'h29      8'h00      OREG_ITB_COEF41[7:0]
         * <SLV-T>   20h     AFh    [7:0]     8'hA4      8'hCF      OREG_ITB_COEF42[7:0]
         * <SLV-T>   20h     B0h    [7:0]     8'h29      8'h00      OREG_ITB_COEF51[7:0]
         * <SLV-T>   20h     B1h    [7:0]     8'h9A      8'hE6      OREG_ITB_COEF52[7:0]
         * <SLV-T>   20h     B2h    [7:0]     8'h28      8'h23      OREG_ITB_COEF61[7:0]
         * <SLV-T>   20h     B3h    [7:0]     8'h9E      8'hA4      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {    /* Group delay equalizer settings for ASCOT2D tuner 6MHz. */
            static const uint8_t data[] =
                { 0x27, 0xA7, 0x28, 0xB3, 0x02, 0xF0, 0x01, 0xE8, 0x00, 0xCF, 0x00, 0xE6, 0x23, 0xA4 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        }
        else if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {   /* Group delay equalizer settings for ASCOT2S tuner 6MHz. */
            /*
             * <ASCOT group delay equalizer settings for ASCOT/ASCOT2S>
             * slave    bank    addr    bit    default   value          name
             * -----------------------------------------------------------------------------
             * <SLV-T>   20h     A6h    [7:0]     8'h1E      8'h19      OREG_ITB_COEF01[7:0]
             * <SLV-T>   20h     A7h    [7:0]     8'h1D      8'h24      OREG_ITB_COEF02[7:0]
             * <SLV-T>   20h     A8h    [7:0]     8'h29      8'h2B      OREG_ITB_COEF11[7:0]
             * <SLV-T>   20h     A9h    [7:0]     8'hC9      8'hB7      OREG_ITB_COEF12[7:0]
             * <SLV-T>   20h     AAh    [7:0]     8'h2A      8'h2C      OREG_ITB_COEF21[7:0]
             * <SLV-T>   20h     ABh    [7:0]     8'hBA      8'hAC      OREG_ITB_COEF22[7:0]
             * <SLV-T>   20h     ACh    [7:0]     8'h29      8'h29      OREG_ITB_COEF31[7:0]
             * <SLV-T>   20h     ADh    [7:0]     8'hAD      8'hA6      OREG_ITB_COEF32[7:0]
             * <SLV-T>   20h     AEh    [7:0]     8'h29      8'h2A      OREG_ITB_COEF41[7:0]
             * <SLV-T>   20h     AFh    [7:0]     8'hA4      8'h9F      OREG_ITB_COEF42[7:0]
             * <SLV-T>   20h     B0h    [7:0]     8'h29      8'h2A      OREG_ITB_COEF51[7:0]
             * <SLV-T>   20h     B1h    [7:0]     8'h9A      8'h99      OREG_ITB_COEF52[7:0]
             * <SLV-T>   20h     B2h    [7:0]     8'h28      8'h2A      OREG_ITB_COEF61[7:0]
             * <SLV-T>   20h     B3h    [7:0]     8'h9E      8'h9B      OREG_ITB_COEF62[7:0]
             */
            static const uint8_t data[] =
                { 0x19, 0x24, 0x2B, 0xB7, 0x2C, 0xAC, 0x29, 0xA6, 0x2A, 0x9F, 0x2A, 0x99, 0x2A, 0x9B };
            /* xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3 */
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     B6h    [7:0]     8'h1F      8'h18      OREG_DNCNV_LOFRQ[23:16]
         * <SLV-T>   20h     B7h    [7:0]     8'h38      8'h59      OREG_DNCNV_LOFRQ[15:8]
         * <SLV-T>   20h     B8h    [7:0]     8'h32      8'hEA      OREG_DNCNV_LOFRQ[7:0]
         */
        {
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_6 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_6 >> 8) & 0xFF);
            data[2] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_6) & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     D7h    [7:6]     8'h00      8'h80      OREG_CHANNEL_WIDTH[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD7, 0x80, 0xC0) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
        break;
    case SONY_DVBT2_BW_5_MHZ:
    {
        /* slave    Bank    Addr    Bit    default   Value          Name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   20h     9Fh    [5:0]     8'h11      8'h1C      OREG_TRCG_NOMINALRATE[37:32]
         * <SLV-T>   20h     A0h    [7:0]     8'hF0      8'hB3      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   20h     A1h    [7:0]     8'h00      8'h33      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   20h     A2h    [7:0]     8'h00      8'h33      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   20h     A3h    [7:0]     8'h00      8'h33      OREG_TRCG_NOMINALRATE[7:0]
         */
        {
            static const uint8_t data[] = { 0x1C, 0xB3, 0x33, 0x33, 0x33 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9F, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /*
         * <ASCOT group delay equalizer settings for ASCOT2D>
         * slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     A6h    [7:0]     8'h1E      8'h27      OREG_ITB_COEF01[7:0]
         * <SLV-T>   20h     A7h    [7:0]     8'h1D      8'hA7      OREG_ITB_COEF02[7:0]
         * <SLV-T>   20h     A8h    [7:0]     8'h29      8'h28      OREG_ITB_COEF11[7:0]
         * <SLV-T>   20h     A9h    [7:0]     8'hC9      8'hB3      OREG_ITB_COEF12[7:0]
         * <SLV-T>   20h     AAh    [7:0]     8'h2A      8'h02      OREG_ITB_COEF21[7:0]
         * <SLV-T>   20h     ABh    [7:0]     8'hBA      8'hF0      OREG_ITB_COEF22[7:0]
         * <SLV-T>   20h     ACh    [7:0]     8'h29      8'h01      OREG_ITB_COEF31[7:0]
         * <SLV-T>   20h     ADh    [7:0]     8'hAD      8'hE8      OREG_ITB_COEF32[7:0]
         * <SLV-T>   20h     AEh    [7:0]     8'h29      8'h00      OREG_ITB_COEF41[7:0]
         * <SLV-T>   20h     AFh    [7:0]     8'hA4      8'hCF      OREG_ITB_COEF42[7:0]
         * <SLV-T>   20h     B0h    [7:0]     8'h29      8'h00      OREG_ITB_COEF51[7:0]
         * <SLV-T>   20h     B1h    [7:0]     8'h9A      8'hE6      OREG_ITB_COEF52[7:0]
         * <SLV-T>   20h     B2h    [7:0]     8'h28      8'h23      OREG_ITB_COEF61[7:0]
         * <SLV-T>   20h     B3h    [7:0]     8'h9E      8'hA4      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {    /* Group delay equalizer settings for ASCOT2D tuner 6MHz. */
            static const uint8_t data[] =
                { 0x27, 0xA7, 0x28, 0xB3, 0x02, 0xF0, 0x01, 0xE8, 0x00, 0xCF, 0x00, 0xE6, 0x23, 0xA4 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        }
        else if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {   /* Group delay equalizer settings for ASCOT2S tuner 6MHz. */
            /*
             * <ASCOT group delay equalizer settings for ASCOT/ASCOT2S>
             * slave    bank    addr    bit    default   value          name
             * -----------------------------------------------------------------------------
             * <SLV-T>   20h     A6h    [7:0]     8'h1E      8'h19      OREG_ITB_COEF01[7:0]
             * <SLV-T>   20h     A7h    [7:0]     8'h1D      8'h24      OREG_ITB_COEF02[7:0]
             * <SLV-T>   20h     A8h    [7:0]     8'h29      8'h2B      OREG_ITB_COEF11[7:0]
             * <SLV-T>   20h     A9h    [7:0]     8'hC9      8'hB7      OREG_ITB_COEF12[7:0]
             * <SLV-T>   20h     AAh    [7:0]     8'h2A      8'h2C      OREG_ITB_COEF21[7:0]
             * <SLV-T>   20h     ABh    [7:0]     8'hBA      8'hAC      OREG_ITB_COEF22[7:0]
             * <SLV-T>   20h     ACh    [7:0]     8'h29      8'h29      OREG_ITB_COEF31[7:0]
             * <SLV-T>   20h     ADh    [7:0]     8'hAD      8'hA6      OREG_ITB_COEF32[7:0]
             * <SLV-T>   20h     AEh    [7:0]     8'h29      8'h2A      OREG_ITB_COEF41[7:0]
             * <SLV-T>   20h     AFh    [7:0]     8'hA4      8'h9F      OREG_ITB_COEF42[7:0]
             * <SLV-T>   20h     B0h    [7:0]     8'h29      8'h2A      OREG_ITB_COEF51[7:0]
             * <SLV-T>   20h     B1h    [7:0]     8'h9A      8'h99      OREG_ITB_COEF52[7:0]
             * <SLV-T>   20h     B2h    [7:0]     8'h28      8'h2A      OREG_ITB_COEF61[7:0]
             * <SLV-T>   20h     B3h    [7:0]     8'h9E      8'h9B      OREG_ITB_COEF62[7:0]
             */
            static const uint8_t data[] =
                { 0x19, 0x24, 0x2B, 0xB7, 0x2C, 0xAC, 0x29, 0xA6, 0x2A, 0x9F, 0x2A, 0x99, 0x2A, 0x9B };
            /* xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3 */
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     B6h    [7:0]     8'h1F      8'h18      OREG_DNCNV_LOFRQ[23:16]
         * <SLV-T>   20h     B7h    [7:0]     8'h38      8'h59      OREG_DNCNV_LOFRQ[15:8]
         * <SLV-T>   20h     B8h    [7:0]     8'h32      8'hEA      OREG_DNCNV_LOFRQ[7:0]
         */
        {
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_5 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_5 >> 8) & 0xFF);
            data[2] = (uint8_t) ((pDemod->iffreq_config.config_DVBT2_5) & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   20h     D7h    [7:6]     8'h00      8'hC0      OREG_CHANNEL_WIDTH[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD7, 0xC0, 0xC0) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
        break;
    default:
        result = SONY_RESULT_ERROR_ARG;
        break;
    }

    SONY_TRACE_RETURN (result);
}

#endif
