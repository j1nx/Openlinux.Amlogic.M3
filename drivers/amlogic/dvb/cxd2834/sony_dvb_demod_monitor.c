/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 3748 $
    $Author: vanstone $

</dev:header>

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>

#include "sony_dvb_demod_monitor.h"
#include "sony_dvb_demod_monitorT.h"
#include "sony_dvb_demod_monitorT2.h"
#include "sony_dvb_demod_monitorC.h"
#include "sony_dvb_demod_monitorC2.h"
#include "sony_math.h"


sony_result_t sony_dvb_demod_monitor_TSLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t syncStat = 0;
    uint8_t tsLock = 0;
    uint8_t earlyUnlock = 0;
    uint8_t arLock = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitor_TSLock");

    if ((!pDemod) || (!pLock)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    *pLock = DEMOD_LOCK_RESULT_NOTDETECT;
    switch (pDemod->system) {
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncStat, &tsLock, &earlyUnlock);
        break;
    case SONY_DVB_SYSTEM_DVBT:
        result = sony_dvb_demod_monitorT_SyncStat (pDemod, &syncStat, &tsLock, &earlyUnlock);
        break;
    case SONY_DVB_SYSTEM_DVBC:
        result = sony_dvb_demod_monitorC_SyncStat (pDemod, &arLock, &tsLock, &earlyUnlock);
        break;

    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demod_monitorC2_SyncStat (pDemod, &syncStat, &tsLock, &earlyUnlock);
        break;
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }
    if (pDemod->system == SONY_DVB_SYSTEM_DVBC)
    {
        if ((arLock) && (tsLock)) {
            *pLock = DEMOD_LOCK_RESULT_LOCKED;
        }
        else {
            *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
        }
    }
    else
    {
        if ((syncStat == 6) && (tsLock)) {
            *pLock = DEMOD_LOCK_RESULT_LOCKED;
        }
        else {
            *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
        }
    }
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitor_CarrierOffset (sony_dvb_demod_t * pDemod, int32_t * pOffset)
{
    sony_result_t result = SONY_RESULT_OK;

    printk ("sony_dvb_demod_monitor_CarrierOffset,pDemod->system %d \n",pDemod->system);

    if ((!pDemod) || (!pOffset)) {
		printk("err arg 1 \n");
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch (pDemod->system) {
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demod_monitorT2_CarrierOffset (pDemod, pOffset);
        *pOffset = (*pOffset >= 0) ? (*pOffset + 500) / 1000 : (*pOffset - 500) / 1000;
        break;
    case SONY_DVB_SYSTEM_DVBT:
        result = sony_dvb_demod_monitorT_CarrierOffset (pDemod, pOffset);
        *pOffset = (*pOffset >= 0) ? (*pOffset + 500) / 1000 : (*pOffset - 500) / 1000;
        break;
    case SONY_DVB_SYSTEM_DVBC:
        result = sony_dvb_demod_monitorC_CarrierOffset (pDemod, pOffset);
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demod_monitorC2_CarrierOffset (pDemod, pOffset);
        *pOffset = (*pOffset >= 0) ? (*pOffset + 500) / 1000 : (*pOffset - 500) / 1000;
        break;

        /* Intentional fall through. */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
		printk("err arg 2 \n");
        result = SONY_RESULT_ERROR_ARG;
    }

    SONY_TRACE_RETURN (result);
}

/*--------------------------------------------------------------------
  RF AGC output monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  uint32_t         *pRFAGCOut   RF AGC output register
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitor_RFAGCOut (sony_dvb_demod_t * pDemod, uint32_t * pRFAGCOut)
{
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitor_RFAGCOut");

    if (!pDemod || !pRFAGCOut)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Read ramreg_ramon_adcdata */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xDF, rdata, 2) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    *pRFAGCOut = (rdata[0] << 2) | (rdata[1] & 0x03);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitor_IFAGCOut (sony_dvb_demod_t * pDemod, uint32_t * pIFAGC)
{
    sony_result_t result = SONY_RESULT_OK;
    

    SONY_TRACE_ENTER ("sony_dvb_demod_monitor_IFAGCOut");

    if ((!pDemod) || (!pIFAGC)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch (pDemod->system) {
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demod_monitorT2_IFAGCOut (pDemod, pIFAGC);
        break;
    case SONY_DVB_SYSTEM_DVBT:
        result = sony_dvb_demod_monitorT_IFAGCOut (pDemod, pIFAGC);
        break;
    case SONY_DVB_SYSTEM_DVBC:
        result = sony_dvb_demod_monitorC_IFAGCOut (pDemod, pIFAGC);
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demod_monitorC2_IFAGCOut (pDemod, pIFAGC);
        break;
        /* Intentional fall through. */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        result = SONY_RESULT_ERROR_ARG;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitor_ChipID (sony_dvb_demod_t * pDemod, sony_dvb_demod_chip_id_t * pChipId)
{
    uint8_t data = 0;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitor_ChipID");

    if ((!pDemod) || (!pChipId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set bank 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave    Bank    Addr    Bit               Register name               
     * ---------------------------------------------------------------
     * <SLV-T>   00h     FDh     [7:0]             chip_id_t
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xFD, &data, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    *pChipId = (sony_dvb_demod_chip_id_t) data;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitor_Quality (sony_dvb_demod_t * pDemod, uint8_t * pQuality)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitor_Quality");

    if ((!pDemod) || (!pQuality)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Interface only available when demod is active. */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (pDemod->system) {
    case SONY_DVB_SYSTEM_DVBT:
        result = sony_dvb_demod_monitorT_Quality (pDemod, pQuality);
        break;
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demod_monitorT2_Quality (pDemod,pQuality);
        break;

    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demod_monitorC2_Quality (pDemod,pQuality);
        break;
        /* Intentional fall-through. */
    case SONY_DVB_SYSTEM_DVBC:
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        result = SONY_RESULT_ERROR_NOSUPPORT;
        break;
    }

    SONY_TRACE_RETURN (result);
}
sony_result_t sony_dvb_demod_monitor_PER (sony_dvb_demod_t * pDemod, uint32_t * pPER)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitor_PER");
    if (!pDemod || !pPER) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    switch (pDemod->system) {
    case SONY_DVB_SYSTEM_DVBT:
        result = sony_dvb_demod_monitorT_PER (pDemod, pPER);
        break;
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demod_monitorT2_PER (pDemod, pPER);
        break;
    case SONY_DVB_SYSTEM_DVBC:
        result = sony_dvb_demod_monitorC_PER (pDemod, pPER);
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demod_monitorC2_PER (pDemod, pPER);
        break;
    default:
        result = SONY_RESULT_ERROR_ARG;
        break;
    }
    SONY_TRACE_RETURN (result);
}
