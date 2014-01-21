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
#include "sony_dvb_demodT.h"
#include "sony_dvb_demod_monitorT.h"

#ifdef SONY_DVB_SUPPORT_DVBT


/**
 @brief Sleep the demodulator.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
static sony_result_t SL2NOT (sony_dvb_demod_t * pDemod);

/**
 @brief Sleep to active T (T) state: bandwidth configuration.
*/
static sony_result_t SL2NOT_BandSetting (sony_dvb_demod_t * pDemod);

/**
 @brief Active T state (T) to Sleep.
*/
static sony_result_t NOT2SL (sony_dvb_demod_t * pDemod);

sony_result_t sony_dvb_demodT_ScanInitialize (sony_dvb_demod_t * pDemod, sony_dvb_scan_params_t * pScanParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodT_ScanInitialize");

    if (!pDemod || !pScanParams)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    SONY_ARG_UNUSED (pScanParams);

    result = sony_dvb_demodT_EnableBlindAcquisition (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT_ScanFinalize (sony_dvb_demod_t * pDemod, sony_dvb_scan_params_t * pScanParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodT_ScanFinalize");

    if (!pDemod || !pScanParams)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    SONY_ARG_UNUSED (pDemod);
    SONY_ARG_UNUSED (pScanParams);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT_TuneInitialize (sony_dvb_demod_t * pDemod, sony_dvb_tune_params_t * pTuneParams)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t usePresets = 0;

    SONY_TRACE_ENTER ("sony_dvb_demodT_TuneInitialize");

    if (!pDemod || !pTuneParams)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    usePresets = pTuneParams->tParams.usePresets;

    /* Check DVB-T Preset tuning parameters. */
    //printk("usePresets 0x%x\n",usePresets);
    if (usePresets) {
        result = sony_dvb_demodT_SetConfig (pDemod, DEMOD_CONFIG_DVBT_FORCE_MODEGI, 1);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_dvb_demodT_SetConfig (pDemod, DEMOD_CONFIG_DVBT_MODE, (int32_t) pTuneParams->tParams.mode);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_dvb_demodT_SetConfig (pDemod, DEMOD_CONFIG_DVBT_GI, (int32_t) pTuneParams->tParams.gi);
    }
    else {
        result = sony_dvb_demodT_SetConfig (pDemod, DEMOD_CONFIG_DVBT_FORCE_MODEGI, 0);
    }

    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Select appropriate HP/LP stream. */
    result = sony_dvb_demodT_SetConfig (pDemod, DEMOD_CONFIG_DVBT_LPSELECT, (int32_t) pTuneParams->tParams.profile);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT_Tune (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodT_Tune");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = SL2NOT (pDemod);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT_Sleep (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodT_Sleep");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = NOT2SL (pDemod);
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodT_EnableBlindAcquisition (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodT_EnableBlindAcquisition");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Disable Forced Mode/GI */
    result = sony_dvb_demodT_SetConfig (pDemod, DEMOD_CONFIG_DVBT_FORCE_MODEGI, 0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
    /* Force HP Selection. */
    result = sony_dvb_demodT_SetConfig (pDemod, DEMOD_CONFIG_DVBT_LPSELECT, (int32_t) SONY_DVBT_PROFILE_HP);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demodT_BandwidthChanged (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodT_BandwidthChanged");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = SL2NOT_BandSetting (pDemod);
    SONY_TRACE_RETURN (result);
}

/*--------------------------------------------------------------------
  TPS/TS Lock Check static function
  (Called by dvb_demod_CheckDemodLock/dvb_demod_CheckTSLock)
  NOTE: This function must be called 100ms after soft reset.
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demodT_CheckDemodLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;

    uint8_t sync = 0;
    uint8_t tslock = 0;
    uint8_t unlockDetected = 0;

    SONY_TRACE_ENTER ("sony_dvb_demodT_CheckDemodLock\n");

    if (!pDemod || !pLock)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    *pLock = DEMOD_LOCK_RESULT_NOTDETECT;

    result = sony_dvb_demod_monitorT_SyncStat (pDemod, &sync, &tslock, &unlockDetected);

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

sony_result_t sony_dvb_demodT_CheckTSLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;

    uint8_t sync = 0;
    uint8_t tslock = 0;
    uint8_t unlockDetected = 0;

    SONY_TRACE_ENTER ("sony_dvb_demodT_CheckTSLock");

    if (!pDemod || !pLock)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    *pLock = DEMOD_LOCK_RESULT_NOTDETECT;

    result = sony_dvb_demod_monitorT_SyncStat (pDemod, &sync, &tslock, &unlockDetected);
   // printk("sync 0x%x,tslock 0x%x,result 0x%x,unLockDetectd 0x%x\n",sync,tslock,result,unlockDetected);
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

sony_result_t sony_dvb_demodT_SetConfig (sony_dvb_demod_t * pDemod, sony_dvb_demod_config_id_t configId, int32_t value)
{
    SONY_TRACE_ENTER ("sony_dvb_demodT_SetConfig");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    printk("configId 0x%x,value 0x%x\n",configId,value);
    switch (configId) {
        /* ============================= DVB-T ================================== */
    case DEMOD_CONFIG_DVBT_LPSELECT:
        /* OREG_LPSELECT (Hierarchical Modulation select) */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OREG_LPSELECT (bit 0) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x67,
                                          (uint8_t) ((value == (int32_t) SONY_DVBT_PROFILE_LP) ? 0x01 : 0x00), 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;
    case DEMOD_CONFIG_DVBT_FORCE_MODEGI:
        /* OREG_FORCE_MODEGI (Mode/GI estimation enable (Auto or Manual)) */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if (value) {
            /* Enable : Set OREG_FORCE_MODEGI (bit 4) */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD3, 0x10, 0x10) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        else {
            /* Disable -> OREG_MODE/OREG_GI must be zero! */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD3, 0x00, 0x1F) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;

    case DEMOD_CONFIG_DVBT_MODE:
        /* OREG_MODE (For manaul Mode/GI setting) */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OREG_MODE (bit 3:2) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD3,
                                          (uint8_t) (((uint32_t) (value) & 0x03) << 2), 0x0C) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_DVBT_GI:
        /* OREG_GI (For manual Mode/GI setting) */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OREG_GI (bit 1:0) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD3,
                                          (uint8_t) ((uint32_t) (value) & 0x03), 0x03) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;
    default:
        /* Unknown ID */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demodT_CheckBandwidth(uint8_t bandWidth)
{
    SONY_TRACE_ENTER ("sony_dvb_demodT_CheckBandwidth");

    if ((bandWidth != SONY_DVBT_BW_5_MHZ) && (bandWidth != SONY_DVBT_BW_6_MHZ) && (bandWidth != SONY_DVBT_BW_7_MHZ) && (bandWidth != SONY_DVBT_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);   /* Not supported bandwidth */
    }
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t SL2NOT (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("SL2NOT");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* DVB-T demod setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x80, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* DVB-T demod clock enable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x81, (pDemod->enable_rflvmon) ? 0xE1 : 0x61) !=
        SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* ADC enable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x85, 0x07) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    if (pDemod->enable_rflvmon) {
        /* RFlevel monitor enable */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, 0x02) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    else {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Write parameters for DVB-T depends on bandwidth */
    result = SL2NOT_BandSetting (pDemod);
    if (result != SONY_RESULT_OK)
        SONY_TRACE_RETURN (result);

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Hi-Z setting disable (other than TS) */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, 0x00, 0x05) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}


static sony_result_t SL2NOT_BandSetting (sony_dvb_demod_t * pDemod)
{
    SONY_TRACE_ENTER ("SL2NOT_BandSetting");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Set bank 1 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x01) != SONY_RESULT_OK)   /* Bank 01 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    /*
     * <BBAGC TARGET level setting>
     * slave    bank    addr    bit     default     value      name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   01h     6Ah    [7:0]     8'h50      8'h50      OREG_ITB_DAGC_TRGT[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x6A, 0x50) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /*
     * IFAGC gain setting>
     * slave    Bank    Addr    Bit    default     Value          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   00h     D2h    [4:0]     8'h0C      8'h0C      OCTL_IFAGC_COARSEGAIN[4:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xD2, 0x0C) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /*
     * slave    Bank    Addr    Bit    default     Value          Name
     * --------------------------------------------------------------------------
     * <SLV-T>   00h     E1h     [0]      8'h00      8'h01      OREG_CRCG_MONSINV
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xE1, 0x01) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    if (pDemod->is_ascot) {

        /* slave    Bank    Addr    Bit    default     Value          Name
         * ------------------------------------------------------------------------------------
         * <SLV-T>   00h     82h    [6:5]     8'hD6      8'bB6     OREG_ADC_ADCGAIN[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x82, 0x40, 0x60) != SONY_RESULT_OK)   /* ON */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     A5h     [0]      1'b1       1'b0      OREG_ITB_GDEQ_EN
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xA5, 0x01, 0x01) != SONY_RESULT_OK)   /* ON */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    }
    else {
        /* slave    Bank    Addr    Bit    default     Value          Name
         * ------------------------------------------------------------------------------------
         * <SLV-T>   00h     82h    [6:5]     8'hD6      8'hB6     OREG_ADC_ADCGAIN[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x82, 0x20, 0x60) != SONY_RESULT_OK)   /* ON */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     A5h     [0]      1'b1       1'b0      OREG_ITB_GDEQ_EN
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xA5, 0x00, 0x01) != SONY_RESULT_OK)   /* ON */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xE2, 0x80, 0x80) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch (pDemod->bandWidth) {
    case SONY_DVBT_BW_5_MHZ:
        /*
         * <Timing Recovery setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     9Fh    [5:0]     8'h11      8'h1C      OREG_TRCG_NOMINALRATE[37:32]
         * <SLV-T>   00h     A0h    [7:0]     8'hF0      8'hB3      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   00h     A1h    [7:0]     8'h00      8'h33      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   00h     A2h    [7:0]     8'h00      8'h33      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   00h     A3h    [7:0]     8'h00      8'h33      OREG_TRCG_NOMINALRATE[7:0]
         */
        {
            const uint8_t data[] = { 0x1C, 0xB3, 0x33, 0x33, 0x33 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9F, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         * <ASCOT2D group delay eq setting (for ASCOT2D)>
         * slave    bank    addr     bit      default   value        name
         * ----------------------------------------------------------------------------------
         *  <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h27      OREG_ITB_COEF01[7:0]
         *  <SLV-T>   00h     A7h    [7:0]     8'h1D      8'hA7      OREG_ITB_COEF02[7:0]
         *  <SLV-T>   00h     A8h    [7:0]     8'h29      8'h28      OREG_ITB_COEF11[7:0]
         *  <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hB3      OREG_ITB_COEF12[7:0]
         *  <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h02      OREG_ITB_COEF21[7:0]
         *  <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hF0      OREG_ITB_COEF22[7:0]
         *  <SLV-T>   00h     ACh    [7:0]     8'h29      8'h01      OREG_ITB_COEF31[7:0]
         *  <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hE8      OREG_ITB_COEF32[7:0]
         *  <SLV-T>   00h     AEh    [7:0]     8'h29      8'h00      OREG_ITB_COEF41[7:0]
         *  <SLV-T>   00h     AFh    [7:0]     8'hA4      8'hCF      OREG_ITB_COEF42[7:0]
         *  <SLV-T>   00h     B0h    [7:0]     8'h29      8'h00      OREG_ITB_COEF51[7:0]
         *  <SLV-T>   00h     B1h    [7:0]     8'h9A      8'hE6      OREG_ITB_COEF52[7:0]
         *  <SLV-T>   00h     B2h    [7:0]     8'h28      8'h23      OREG_ITB_COEF61[7:0]
         *  <SLV-T>   00h     B3h    [7:0]     8'h9E      8'hA4      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {
            const uint8_t data[] = { 0x27, 0xA7, 0x28, 0xB3, 0x02, 0xF0, 0x01, 0xE8, 0x00, 0xCF, 0x00, 0xE6, 0x23, 0xA4 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {
            /*
             * <ASCOT group delay eq setting (for ASCOT2S)>
             * slave    bank    addr    bit     default     value      name
             * ----------------------------------------------------------------------------------
             * <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h19      OREG_ITB_COEF01[7:0]
             * <SLV-T>   00h     A7h    [7:0]     8'h1D      8'h24      OREG_ITB_COEF02[7:0]
             * <SLV-T>   00h     A8h    [7:0]     8'h29      8'h2B      OREG_ITB_COEF11[7:0]
             * <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hB7      OREG_ITB_COEF12[7:0]
             * <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h2C      OREG_ITB_COEF21[7:0]
             * <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hAC      OREG_ITB_COEF22[7:0]
             * <SLV-T>   00h     ACh    [7:0]     8'h29      8'h29      OREG_ITB_COEF31[7:0]
             * <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hA6      OREG_ITB_COEF32[7:0]
             * <SLV-T>   00h     AEh    [7:0]     8'h29      8'h2A      OREG_ITB_COEF41[7:0]
             * <SLV-T>   00h     AFh    [7:0]     8'hA4      8'h9F      OREG_ITB_COEF42[7:0]
             * <SLV-T>   00h     B0h    [7:0]     8'h29      8'h2A      OREG_ITB_COEF51[7:0]
             * <SLV-T>   00h     B1h    [7:0]     8'h9A      8'h99      OREG_ITB_COEF52[7:0]
             * <SLV-T>   00h     B2h    [7:0]     8'h28      8'h2A      OREG_ITB_COEF61[7:0]
             * <SLV-T>   00h     B3h    [7:0]     8'h9E      8'h9B      OREG_ITB_COEF62[7:0]
             */
            const uint8_t data[] = { 0x19, 0x24, 0x2B, 0xB7, 0x2C, 0xAC, 0x29, 0xA6, 0x2A, 0x9F, 0x2A, 0x99, 0x2A, 0x9B };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         * <IF freq setting>
         * slave    bank    addr    bit     default     value      name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   00h     B6h    [7:0]     8'h1F   user define   OREG_DNCNV_LOFRQ[23:16]
         * <SLV-T>   00h     B7h    [7:0]     8'h38   user define   OREG_DNCNV_LOFRQ[15:8]
         * <SLV-T>   00h     B8h    [7:0]     8'h32   user define   OREG_DNCNV_LOFRQ[7:0]
         */
        {
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBT5 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBT5 >> 8) & 0xFF);
            data[2] = (uint8_t) (pDemod->iffreq_config.config_DVBT5 & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     D7h    [7:6]     8'h00      8'hC0      OREG_CHANNEL_WIDTH[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD7, 0xC0, 0xC0) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /* slave    Bank    Addr    Bit    default    Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     D9h    [5:0]     8'h01      8'h26      OREG_CDRB_GTDOFST[13:8]
         * <SLV-T>   00h     DAh    [7:0]     8'hE0      8'h3C      OREG_CDRB_GTDOFST[7:0]
         */
        {
            const uint8_t data[] = { 0x26, 0x3C };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xD9, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*  slave    bank     addr    bit      default   value      name
         *  ----------------------------------------------------------------------------------
         *  <SLV-T>   07h     38h    [1:0]     8'h00      8'h00      OREG_CAS_CCIFLT2_EN_CW2[1:0]
         *  <SLV-T>   07h     39h    [1:0]     8'h03      8'h03      OREG_CAS_CWSEQ_ON,OREG_CAS_CWSEQ_ON2
         */
        {
           const uint8_t data[] = { 0x00, 0x03 };
            /* Set bank 0x07 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x07) != SONY_RESULT_OK)   /* Bank 01 */
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x38, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;
    case SONY_DVBT_BW_6_MHZ:
        /*
         * <Timing Recovery setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     9Fh    [5:0]     6'h11      8'h17      OREG_TRCG_NOMINALRATE[37:32]
         * <SLV-T>   00h     A0h    [7:0]     8'hF0      8'hEA      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   00h     A1h    [7:0]     8'h00      8'hAA      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   00h     A2h    [7:0]     8'h00      8'hAA      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   00h     A3h    [7:0]     8'h00      8'hAA      OREG_TRCG_NOMINALRATE[7:0]
         */
        {
            const uint8_t data[] = { 0x17, 0xEA, 0xAA, 0xAA, 0xAA };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9F, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         * <ASCOT2D group delay eq setting (for ASCOT2D)>
         * slave    bank    addr     bit      default   value        name
         * ----------------------------------------------------------------------------------
         *  <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h27      OREG_ITB_COEF01[7:0]
         *  <SLV-T>   00h     A7h    [7:0]     8'h1D      8'hA7      OREG_ITB_COEF02[7:0]
         *  <SLV-T>   00h     A8h    [7:0]     8'h29      8'h28      OREG_ITB_COEF11[7:0]
         *  <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hB3      OREG_ITB_COEF12[7:0]
         *  <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h02      OREG_ITB_COEF21[7:0]
         *  <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hF0      OREG_ITB_COEF22[7:0]
         *  <SLV-T>   00h     ACh    [7:0]     8'h29      8'h01      OREG_ITB_COEF31[7:0]
         *  <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hE8      OREG_ITB_COEF32[7:0]
         *  <SLV-T>   00h     AEh    [7:0]     8'h29      8'h00      OREG_ITB_COEF41[7:0]
         *  <SLV-T>   00h     AFh    [7:0]     8'hA4      8'hCF      OREG_ITB_COEF42[7:0]
         *  <SLV-T>   00h     B0h    [7:0]     8'h29      8'h00      OREG_ITB_COEF51[7:0]
         *  <SLV-T>   00h     B1h    [7:0]     8'h9A      8'hE6      OREG_ITB_COEF52[7:0]
         *  <SLV-T>   00h     B2h    [7:0]     8'h28      8'h23      OREG_ITB_COEF61[7:0]
         *  <SLV-T>   00h     B3h    [7:0]     8'h9E      8'hA4      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {
            const uint8_t data[] = { 0x27, 0xA7, 0x28, 0xB3, 0x02, 0xF0, 0x01, 0xE8, 0x00, 0xCF, 0x00, 0xE6, 0x23, 0xA4 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {
            /*
             * <ASCOT group delay eq setting (for ASCOT2S)>
             * slave    bank    addr    bit     default     value      name
             * ----------------------------------------------------------------------------------
             * <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h19      OREG_ITB_COEF01[7:0]
             * <SLV-T>   00h     A7h    [7:0]     8'h1D      8'h24      OREG_ITB_COEF02[7:0]
             * <SLV-T>   00h     A8h    [7:0]     8'h29      8'h2B      OREG_ITB_COEF11[7:0]
             * <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hB7      OREG_ITB_COEF12[7:0]
             * <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h2C      OREG_ITB_COEF21[7:0]
             * <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hAC      OREG_ITB_COEF22[7:0]
             * <SLV-T>   00h     ACh    [7:0]     8'h29      8'h29      OREG_ITB_COEF31[7:0]
             * <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hA6      OREG_ITB_COEF32[7:0]
             * <SLV-T>   00h     AEh    [7:0]     8'h29      8'h2A      OREG_ITB_COEF41[7:0]
             * <SLV-T>   00h     AFh    [7:0]     8'hA4      8'h9F      OREG_ITB_COEF42[7:0]
             * <SLV-T>   00h     B0h    [7:0]     8'h29      8'h2A      OREG_ITB_COEF51[7:0]
             * <SLV-T>   00h     B1h    [7:0]     8'h9A      8'h99      OREG_ITB_COEF52[7:0]
             * <SLV-T>   00h     B2h    [7:0]     8'h28      8'h2A      OREG_ITB_COEF61[7:0]
             * <SLV-T>   00h     B3h    [7:0]     8'h9E      8'h9B      OREG_ITB_COEF62[7:0]
             */
            const uint8_t data[] = { 0x19, 0x24, 0x2B, 0xB7, 0x2C, 0xAC, 0x29, 0xA6, 0x2A, 0x9F, 0x2A, 0x99, 0x2A, 0x9B };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         * <IF freq setting>
         * slave    bank    addr    bit     default     value      name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   00h     B6h    [7:0]     8'h1F   user define   OREG_DNCNV_LOFRQ[23:16]
         * <SLV-T>   00h     B7h    [7:0]     8'h38   user define   OREG_DNCNV_LOFRQ[15:8]
         * <SLV-T>   00h     B8h    [7:0]     8'h32   user define   OREG_DNCNV_LOFRQ[7:0]
         */
        {
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBT6 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBT6 >> 8) & 0xFF);
            data[2] = (uint8_t) (pDemod->iffreq_config.config_DVBT6 & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*
         * <TSIF setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     D7h    [7:6]     2'b00      2'b10      OREG_CHANNEL_WIDTH[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD7, 0x80, 0xC0) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /*
         * <Demod internal delay setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     D9h    [5:0]     6'h01      6'h1F      OREG_CDRB_GTDOFST[13:8]
         * <SLV-T>   00h     DAh    [7:0]     8'hE0      8'hDC      OREG_CDRB_GTDOFST[7:0]
         */
        {
            const uint8_t data[] = { 0x1F, 0xDC };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xD9, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*  slave    bank     addr    bit      default   value      name
         *  ----------------------------------------------------------------------------------
         *  <SLV-T>   07h     38h    [1:0]     8'h00      8'h00      OREG_CAS_CCIFLT2_EN_CW2[1:0]
         *  <SLV-T>   07h     39h    [1:0]     8'h03      8'h03      OREG_CAS_CWSEQ_ON,OREG_CAS_CWSEQ_ON2
         */
        {
            const uint8_t data[] = { 0x00, 0x03 };
            /* Set bank 0x07 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x07) != SONY_RESULT_OK)   /* Bank 01 */
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x38, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;
    case SONY_DVBT_BW_7_MHZ:
        /*
         * <Timing Recovery setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     9Fh    [5:0]     6'h11      8'h14      OREG_TRCG_NOMINALRATE[37:32]
         * <SLV-T>   00h     A0h    [7:0]     8'hF0      8'h80      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   00h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   00h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   00h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
         */
        {
            const uint8_t data[] = { 0x14, 0x80, 0x00, 0x00, 0x00 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9F, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         * <ASCOT group delay eq setting (for ASCOT2D)>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h2C      OREG_ITB_COEF01[7:0]
         * <SLV-T>   00h     A7h    [7:0]     8'h1D      8'hBD      OREG_ITB_COEF02[7:0]
         * <SLV-T>   00h     A8h    [7:0]     8'h29      8'h02      OREG_ITB_COEF11[7:0]
         * <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hCF      OREG_ITB_COEF12[7:0]
         * <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h04      OREG_ITB_COEF21[7:0]
         * <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hF8      OREG_ITB_COEF22[7:0]
         * <SLV-T>   00h     ACh    [7:0]     8'h29      8'h23      OREG_ITB_COEF31[7:0]
         * <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hA6      OREG_ITB_COEF32[7:0]
         * <SLV-T>   00h     AEh    [7:0]     8'h29      8'h29      OREG_ITB_COEF41[7:0]
         * <SLV-T>   00h     AFh    [7:0]     8'hA4      8'hB0      OREG_ITB_COEF42[7:0]
         * <SLV-T>   00h     B0h    [7:0]     8'h29      8'h26      OREG_ITB_COEF51[7:0]
         * <SLV-T>   00h     B1h    [7:0]     8'h9A      8'hA9      OREG_ITB_COEF52[7:0]
         * <SLV-T>   00h     B2h    [7:0]     8'h28      8'h21      OREG_ITB_COEF61[7:0]
         * <SLV-T>   00h     B3h    [7:0]     8'h9E      8'hA5      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {
            const uint8_t data[] = { 0x2C, 0xBD, 0x02, 0xCF, 0x04, 0xF8, 0x23, 0xA6, 0x29, 0xB0, 0x26, 0xA9, 0x21, 0xA5 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        }
        else if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {
            /*
             * <ASCOT group delay eq setting (for ASCOT2S)>
             * slave    bank    addr    bit     default     value      name
             * ----------------------------------------------------------------------------------
             * <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h1B      OREG_ITB_COEF01[7:0]
             * <SLV-T>   00h     A7h    [7:0]     8'h1D      8'h22      OREG_ITB_COEF02[7:0]
             * <SLV-T>   00h     A8h    [7:0]     8'h29      8'h2B      OREG_ITB_COEF11[7:0]
             * <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hC1      OREG_ITB_COEF12[7:0]
             * <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h2C      OREG_ITB_COEF21[7:0]
             * <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hB3      OREG_ITB_COEF22[7:0]
             * <SLV-T>   00h     ACh    [7:0]     8'h29      8'h2B      OREG_ITB_COEF31[7:0]
             * <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hA9      OREG_ITB_COEF32[7:0]
             * <SLV-T>   00h     AEh    [7:0]     8'h29      8'h2B      OREG_ITB_COEF41[7:0]
             * <SLV-T>   00h     AFh    [7:0]     8'hA4      8'hA0      OREG_ITB_COEF42[7:0]
             * <SLV-T>   00h     B0h    [7:0]     8'h29      8'h2B      OREG_ITB_COEF51[7:0]
             * <SLV-T>   00h     B1h    [7:0]     8'h9A      8'h97      OREG_ITB_COEF52[7:0]
             * <SLV-T>   00h     B2h    [7:0]     8'h28      8'h2B      OREG_ITB_COEF61[7:0]
             * <SLV-T>   00h     B3h    [7:0]     8'h9E      8'h9B      OREG_ITB_COEF62[7:0]
             */
            const uint8_t data[] = { 0x1B, 0x22, 0x2B, 0xC1, 0x2C, 0xB3, 0x2B, 0xA9, 0x2B, 0xA0, 0x2B, 0x97, 0x2B, 0x9B };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         * <IF freq setting>
         * slave    bank    addr    bit     default     value      name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   00h     B6h    [7:0]     8'h1F   user define   OREG_DNCNV_LOFRQ[23:16]
         * <SLV-T>   00h     B7h    [7:0]     8'h38   user define   OREG_DNCNV_LOFRQ[15:8]
         * <SLV-T>   00h     B8h    [7:0]     8'h32   user define   OREG_DNCNV_LOFRQ[7:0]
         */
        {
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBT7 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBT7 >> 8) & 0xFF);
            data[2] = (uint8_t) (pDemod->iffreq_config.config_DVBT7 & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*
         * <TSIF setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     D7h    [7:6]     2'b00      2'b01      OREG_CHANNEL_WIDTH[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD7, 0x40, 0xC0) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /*
         * <Demod internal delay setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     D9h    [5:0]     6'h01      6'h12      OREG_CDRB_GTDOFST[13:8]
         * <SLV-T>   00h     DAh    [7:0]     8'hE0      8'hF8      OREG_CDRB_GTDOFST[7:0]
         */
        {
            const uint8_t data[] = { 0x12, 0xF8 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xD9, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*  slave    bank     addr    bit      default   value      name
         *  ----------------------------------------------------------------------------------
         *  <SLV-T>   07h     38h    [1:0]     8'h00      8'h00      OREG_CAS_CCIFLT2_EN_CW2[1:0]
         *  <SLV-T>   07h     39h    [1:0]     8'h03      8'h03      OREG_CAS_CWSEQ_ON,OREG_CAS_CWSEQ_ON2
         */
        {
            const uint8_t data[] = { 0x00, 0x03 };
            /* Set bank 0x07 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x07) != SONY_RESULT_OK)   /* Bank 01 */
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x38, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;
    case SONY_DVBT_BW_8_MHZ:
        /*
         * <Timing Recovery setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     9Fh    [5:0]     6'h11      8'h11      OREG_TRCG_NOMINALRATE[37:32]
         * <SLV-T>   00h     A0h    [7:0]     8'hF0      8'hF0      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   00h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   00h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   00h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
         */
        {
            const uint8_t data[] = { 0x11, 0xF0, 0x00, 0x00, 0x00 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9F, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         * <ASCOT group delay equalizer settings for ASCOT2D>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h26      OREG_ITB_COEF01[7:0]
         * <SLV-T>   00h     A7h    [7:0]     8'h1D      8'hAF      OREG_ITB_COEF02[7:0]
         * <SLV-T>   00h     A8h    [7:0]     8'h29      8'h06      OREG_ITB_COEF11[7:0]
         * <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hCD      OREG_ITB_COEF12[7:0]
         * <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h13      OREG_ITB_COEF21[7:0]
         * <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hBB      OREG_ITB_COEF22[7:0]
         * <SLV-T>   00h     ACh    [7:0]     8'h29      8'h28      OREG_ITB_COEF31[7:0]
         * <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hBA      OREG_ITB_COEF32[7:0]
         * <SLV-T>   00h     AEh    [7:0]     8'h29      8'h23      OREG_ITB_COEF41[7:0]
         * <SLV-T>   00h     AFh    [7:0]     8'hA4      8'hA9      OREG_ITB_COEF42[7:0]
         * <SLV-T>   00h     B0h    [7:0]     8'h29      8'h1F      OREG_ITB_COEF51[7:0]
         * <SLV-T>   00h     B1h    [7:0]     8'h9A      8'hA8      OREG_ITB_COEF52[7:0]
         * <SLV-T>   00h     B2h    [7:0]     8'h28      8'h2C      OREG_ITB_COEF61[7:0]
         * <SLV-T>   00h     B3h    [7:0]     8'h9E      8'hC8      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {
            const uint8_t data[] = { 0x26, 0xAF, 0x06, 0xCD, 0x13, 0xBB, 0x28, 0xBA, 0x23, 0xA9, 0x1F, 0xA8, 0x2C, 0xC8 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        }
        else if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {
            /*
             * <ASCOT group delay eq setting (for ASCOT2S)>
             * slave    bank    addr    bit     default     value      name
             * ----------------------------------------------------------------------------------
             * <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h1E      OREG_ITB_COEF01[7:0]
             * <SLV-T>   00h     A7h    [7:0]     8'h1D      8'h1D      OREG_ITB_COEF02[7:0]
             * <SLV-T>   00h     A8h    [7:0]     8'h29      8'h29      OREG_ITB_COEF11[7:0]
             * <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hC9      OREG_ITB_COEF12[7:0]
             * <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h2A      OREG_ITB_COEF21[7:0]
             * <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hBA      OREG_ITB_COEF22[7:0]
             * <SLV-T>   00h     ACh    [7:0]     8'h29      8'h29      OREG_ITB_COEF31[7:0]
             * <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hAD      OREG_ITB_COEF32[7:0]
             * <SLV-T>   00h     AEh    [7:0]     8'h29      8'h29      OREG_ITB_COEF41[7:0]
             * <SLV-T>   00h     AFh    [7:0]     8'hA4      8'hA4      OREG_ITB_COEF42[7:0]
             * <SLV-T>   00h     B0h    [7:0]     8'h29      8'h29      OREG_ITB_COEF51[7:0]
             * <SLV-T>   00h     B1h    [7:0]     8'h9A      8'h9A      OREG_ITB_COEF52[7:0]
             * <SLV-T>   00h     B2h    [7:0]     8'h28      8'h28      OREG_ITB_COEF61[7:0]
             * <SLV-T>   00h     B3h    [7:0]     8'h9E      8'h9E      OREG_ITB_COEF62[7:0]
             */
            const uint8_t data[] = { 0x1E, 0x1D, 0x29, 0xC9, 0x2A, 0xBA, 0x29, 0xAD, 0x29, 0xA4, 0x29, 0x9A, 0x28, 0x9E };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         * <IF freq setting>
         * slave    bank    addr    bit     default     value      name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   00h     B6h    [7:0]     8'h1F   user define   OREG_DNCNV_LOFRQ[23:16]
         * <SLV-T>   00h     B7h    [7:0]     8'h38   user define   OREG_DNCNV_LOFRQ[15:8]
         * <SLV-T>   00h     B8h    [7:0]     8'h32   user define   OREG_DNCNV_LOFRQ[7:0]
         */
        {
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBT8 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBT8 >> 8) & 0xFF);
            data[2] = (uint8_t) (pDemod->iffreq_config.config_DVBT8 & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*
         * <TSIF setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     D7h    [7:6]     2'b00      2'b00      OREG_CHANNEL_WIDTH[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD7, 0x00, 0xC0) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /*
         * <Demod internal delay setting>
         * slave    bank    addr    bit     default     value      name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     D9h    [5:0]     6'h01      6'h01      OREG_CDRB_GTDOFST[13:8]
         * <SLV-T>   00h     DAh    [7:0]     8'hE0      8'hE0      OREG_CDRB_GTDOFST[7:0]
         */
        {
            const uint8_t data[] = { 0x01, 0xE0 };
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xD9, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*  slave    bank     addr    bit      default   value      name
         *  ----------------------------------------------------------------------------------
         *  <SLV-T>   07h     38h    [1:0]     8'h00      8'h01      OREG_CAS_CCIFLT2_EN_CW2[1:0]
         *  <SLV-T>   07h     39h    [1:0]     8'h03      8'h02      OREG_CAS_CWSEQ_ON,OREG_CAS_CWSEQ_ON2
         */
        {
            const uint8_t data[] = { 0x01, 0x02 };
            /* Set bank 0x07 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x07) != SONY_RESULT_OK)   /* Bank 01 */
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x38, data, sizeof (data)) != SONY_RESULT_OK)
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t NOT2SL (sony_dvb_demod_t * pDemod)
{

    SONY_TRACE_ENTER ("NOT2SL");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* TS output disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xF1, 0x01) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Hi-Z setting */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, 0x05, 0x05) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* ADC disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c,
                                        pDemod->i2cAddress, 0x85, (pDemod->sharedIf ? 0x06 : 0x00)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* RFlevel monitor disable */
    if (pDemod->enable_rflvmon) {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* DVB-T demod Clock disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x81, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

#endif
