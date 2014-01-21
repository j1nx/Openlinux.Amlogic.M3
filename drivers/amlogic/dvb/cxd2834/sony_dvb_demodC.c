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
#include "sony_dvb_demodC.h"
#include "sony_dvb_demod_monitorC.h"
/**
 @brief Sleep the demodulator.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
static sony_result_t SL2NOC (sony_dvb_demod_t * pDemod);

/**
 @brief Active C state (C) to Sleep.
*/
static sony_result_t NOC2SL (sony_dvb_demod_t * pDemod);

sony_result_t sony_dvb_demodC_ScanInitialize (sony_dvb_demod_t * pDemod, sony_dvb_scan_params_t * pScanParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodC_ScanInitialize");

    if (!pDemod || !pScanParams) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_ARG_UNUSED(pScanParams);

    result = sony_dvb_demodC_SetConfig (pDemod, DEMOD_CONFIG_DVBC_SCANMODE, 1);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodC_ScanFinalize (sony_dvb_demod_t * pDemod, sony_dvb_scan_params_t * pScanParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demodC_ScanFinalize");

    if (!pDemod || !pScanParams) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_ARG_UNUSED(pScanParams);

    result = sony_dvb_demodC_SetConfig (pDemod, DEMOD_CONFIG_DVBC_SCANMODE, 0);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodC_Tune (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodC_Tune");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = SL2NOC (pDemod);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodC_Sleep (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demodC_Sleep");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = NOC2SL (pDemod);
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demodC_CheckDemodLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t arlock = 0;
    uint8_t tslock = 0;
    uint8_t unlockDetected = 0;

    SONY_TRACE_ENTER ("sony_dvb_demodC_CheckDemodLock");

    if (!pDemod || !pLock)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    *pLock = DEMOD_LOCK_RESULT_NOTDETECT;

    result = sony_dvb_demod_monitorC_SyncStat( pDemod, &arlock, &tslock, &unlockDetected);

    if (result != SONY_RESULT_OK) {
       SONY_TRACE_RETURN (result);
    }

    if (pDemod->dvbc_scanmode) {

        if (unlockDetected == 1) {
           *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
           SONY_TRACE_RETURN (SONY_RESULT_OK);
        }
    }

    if (arlock == 0) {
        *pLock = DEMOD_LOCK_RESULT_NOTDETECT;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    *pLock = tslock ? DEMOD_LOCK_RESULT_LOCKED : DEMOD_LOCK_RESULT_NOTDETECT;
    
    SONY_TRACE_RETURN (result);

}

sony_result_t sony_dvb_demodC_CheckTSLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{

    SONY_TRACE_ENTER ("sony_dvb_demodC_CheckTSLock");
    /* CheckTSlock is the same as CheckDemodLock */
    SONY_TRACE_RETURN (sony_dvb_demodC_CheckDemodLock (pDemod, pLock));

}

sony_result_t sony_dvb_demodC_SetConfig (sony_dvb_demod_t * pDemod, sony_dvb_demod_config_id_t configId, int32_t value)
{
    SONY_TRACE_ENTER ("sony_dvb_demodC_SetConfig");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    switch (configId) {
        /* ============================= DVB-C ================================== */
    case DEMOD_CONFIG_DVBC_SCANMODE:
        /* OREG_AR_SCANNING (Scan mode select) */
        /* Set bank 0x40 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OREG_AR_SCANNING (bit 5) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x86,
                                          (uint8_t) (value ? 0x20 : 0x00), 0x20) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        pDemod->dvbc_scanmode = value ? 1 : 0;
        break;
    default:
        /* Unknown ID */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demodC_CheckBandwidth(uint8_t bandWidth)
{
    SONY_TRACE_ENTER ("sony_dvb_demodC_CheckBandwidth");

    if (bandWidth != SONY_DVBC_BW_8_MHZ) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG); /* Not supported bandwidth */
    }
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t NOC2SL (sony_dvb_demod_t * pDemod)
{
    SONY_TRACE_ENTER ("NOC2SL");

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
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x85, (pDemod->sharedIf ? 0x06 : 0x00)) !=
        SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* RFlevel monitor disable */
    if (pDemod->enable_rflvmon) {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* DVB-C demod Clock disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x81, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* DVB-C demod disable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x80, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t SL2NOC (sony_dvb_demod_t * pDemod)
{
    SONY_TRACE_ENTER ("SL2NOC");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* DVB-C demod setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x80, 0x01) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* DVB-C demod clock enable. *Burst write is forbidden* */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress,
                                        0x81, (pDemod->enable_rflvmon) ? 0xB1 : 0x31) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    }

    /* ADC enable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x85, 0x07) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* RFlevel monitor enable */
    if (pDemod->enable_rflvmon) {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, 0x02) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    else {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     * <IFAGC gain setting>
     * slave    Bank    Addr    Bit    default     Value          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   00h     D2h    [4:0]     8'h0C      8'h09      OCTL_IFAGC_COARSEGAIN[4:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xD2, 0x09) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /*
     * <BBAGC TARGET level setting>
     *  slave    Bank    Addr    Bit    default     Value          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   01h     6Ah    [7:0]     8'h50      8'h48      OREG_ITB_DAGC_TRGT[7:0]
     */
    /* Set bank 1 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x01) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x6A, 0x48) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    if(pDemod->is_ascot) {
        /* slave    Bank    Addr    Bit    default     Value          Name
         * ------------------------------------------------------------------------------------
         * <SLV-T>   00h     82h    [6:5]     2'b10      2'b01     OREG_ADC_ADCGAIN[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x82, 0x40, 0x60) != SONY_RESULT_OK) /* ON */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /*  slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     A5h     [0]   1'b1       1'b0          OREG_ITB_GDEQ_EN
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA5, 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    } else {
        /* slave    Bank    Addr    Bit    default     Value          Name
         * ------------------------------------------------------------------------------------
         * <SLV-T>   00h     82h    [6:5]     2'b10      2'b01     OREG_ADC_ADCGAIN[1:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x82, 0x20, 0x60) != SONY_RESULT_OK) /* ON */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /*  slave    Bank    Addr    Bit    default   Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   00h     A5h     [0]   1'b1       1'b0          OREG_ITB_GDEQ_EN
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA5,0x00) != SONY_RESULT_OK)
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
        uint8_t data[3] = { 0x1F, 0x88, 0x00 };
        data[0] = (uint8_t) ((pDemod->iffreq_config.config_DVBC >> 16) & 0xFF);
        data[1] = (uint8_t) ((pDemod->iffreq_config.config_DVBC >> 8) & 0xFF);
        data[2] = (uint8_t) (pDemod->iffreq_config.config_DVBC & 0xFF);
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB6, data, sizeof (data)) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     * <ASCOT group delay eq setting (for ASCOT2D)>
     * slave    bank    addr    bit     default     value      name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h2D      OREG_ITB_COEF01[7:0]
     * <SLV-T>   00h     A7h    [7:0]     8'h1D      8'hC7      OREG_ITB_COEF02[7:0]
     * <SLV-T>   00h     A8h    [7:0]     8'h29      8'h04      OREG_ITB_COEF11[7:0]
     * <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hF4      OREG_ITB_COEF12[7:0]
     * <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h07      OREG_ITB_COEF21[7:0]
     * <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hC5      OREG_ITB_COEF22[7:0]
     * <SLV-T>   00h     ACh    [7:0]     8'h29      8'h2A      OREG_ITB_COEF31[7:0]
     * <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hB8      OREG_ITB_COEF32[7:0]
     * <SLV-T>   00h     AEh    [7:0]     8'h29      8'h27      OREG_ITB_COEF41[7:0]
     * <SLV-T>   00h     AFh    [7:0]     8'hA4      8'h9E      OREG_ITB_COEF42[7:0]
     * <SLV-T>   00h     B0h    [7:0]     8'h29      8'h27      OREG_ITB_COEF51[7:0]
     * <SLV-T>   00h     B1h    [7:0]     8'h9A      8'hA4      OREG_ITB_COEF52[7:0]
     * <SLV-T>   00h     B2h    [7:0]     8'h28      8'h29      OREG_ITB_COEF61[7:0]
     * <SLV-T>   00h     B3h    [7:0]     8'h9E      8'hAB      OREG_ITB_COEF62[7:0]
     */
    if (pDemod->is_ascot == DVB_TUNER_ASCOT2D) {
        const uint8_t data[] = { 0x2D, 0xC7, 0x04, 0xF4, 0x07, 0xC5, 0x2A, 0xB8, 0x27, 0x9E, 0x27, 0xA4, 0x29, 0xAB };
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    }
    
    if (pDemod->is_ascot == DVB_TUNER_ASCOT2S) {
        /*
         * <ASCOT group delay eq setting (for ASCOT2S)>
         * slave    bank    addr    bit     default     value      name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   00h     A6h    [7:0]     8'h1E      8'h1C      OREG_ITB_COEF01[7:0]
         * <SLV-T>   00h     A7h    [7:0]     8'h1D      8'hB1      OREG_ITB_COEF02[7:0]
         * <SLV-T>   00h     A8h    [7:0]     8'h29      8'h23      OREG_ITB_COEF11[7:0]
         * <SLV-T>   00h     A9h    [7:0]     8'hC9      8'hB1      OREG_ITB_COEF12[7:0]
         * <SLV-T>   00h     AAh    [7:0]     8'h2A      8'h09      OREG_ITB_COEF21[7:0]
         * <SLV-T>   00h     ABh    [7:0]     8'hBA      8'hC7      OREG_ITB_COEF22[7:0]
         * <SLV-T>   00h     ACh    [7:0]     8'h29      8'h09      OREG_ITB_COEF31[7:0]
         * <SLV-T>   00h     ADh    [7:0]     8'hAD      8'hCA      OREG_ITB_COEF32[7:0]
         * <SLV-T>   00h     AEh    [7:0]     8'h29      8'h28      OREG_ITB_COEF41[7:0]
         * <SLV-T>   00h     AFh    [7:0]     8'hA4      8'hCC      OREG_ITB_COEF42[7:0]
         * <SLV-T>   00h     B0h    [7:0]     8'h29      8'h1F      OREG_ITB_COEF51[7:0]
         * <SLV-T>   00h     B1h    [7:0]     8'h9A      8'hAB      OREG_ITB_COEF52[7:0]
         * <SLV-T>   00h     B2h    [7:0]     8'h28      8'h27      OREG_ITB_COEF61[7:0]
         * <SLV-T>   00h     B3h    [7:0]     8'h9E      8'hBC      OREG_ITB_COEF62[7:0]
         */
        const uint8_t data[] = { 0x1C, 0xB1, 0x23, 0xB1, 0x09, 0xC7, 0x09, 0xCA, 0x28, 0xCC, 0x1F, 0xAB, 0x27, 0xBC };
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xA6, data, sizeof (data)) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

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
