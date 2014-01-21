/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**
 @file    sony_dvb_ascot2d.c

      This file implements the Sony ASCOT2D DVB tuner port of the 
      Ascot2S tuner reference driver.
*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include <linux/kernel.h>
#include "sony_dvb.h"
#include "sony_dvb_ascot2d.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Statics
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
#if 0
#define pr_dbg(fmt, args...) printk(KERN_ERR "cxd2834: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif

#define pr_error(fmt, args...) printk(KERN_ERR "cxd2834: " fmt, ## args)

sony_result_t sony_dvb_ascot2d_Create (uint8_t i2cAddress,
                                           uint32_t xtalFreq,
                                           sony_i2c_t * pI2c,
                                           uint32_t configFlags, sony_ascot2d_t * pAscot2DTuner, sony_dvb_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    pr_dbg ("sony_dvb_ascot2d_Create");

    if ((!pTuner) || (!pI2c) || (!pAscot2DTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Create the underlying Ascot2D reference driver. */
    result = sony_ascot2d_Create (i2cAddress, xtalFreq, pI2c, configFlags, NULL, pAscot2DTuner);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Create local copy of instance data. */
    pTuner->Initialize = sony_dvb_ascot2d_Initialize;
    pTuner->Finalize = sony_dvb_ascot2d_Finalize;
    pTuner->Sleep = sony_dvb_ascot2d_Sleep;
    pTuner->Tune = sony_dvb_ascot2d_Tune;
    pTuner->system = SONY_DVB_SYSTEM_UNKNOWN;
    pTuner->frequency = 0;
    pTuner->bandWidth = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->xtalFreq = xtalFreq;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = pAscot2DTuner;

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Initialise the tuner. 
------------------------------------------------------------------------------*/
sony_result_t sony_dvb_ascot2d_Initialize (sony_dvb_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    pr_dbg ("sony_dvb_ascot2d_Initialize\n");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot2d_Initialize (((sony_ascot2d_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DVB_SYSTEM_UNKNOWN;
    pTuner->frequency = 0;
    pTuner->bandWidth = 0;

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Set tuner into Power Save mode.
------------------------------------------------------------------------------*/
sony_result_t sony_dvb_ascot2d_Finalize (sony_dvb_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    pr_dbg ("sony_dvb_ascot2d_Finalize");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot2d_Finalize (((sony_ascot2d_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DVB_SYSTEM_UNKNOWN;
    pTuner->frequency = 0;
    pTuner->bandWidth = 0;

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Setup the channel bandwidth.
 Tune to the specified frequency.
 Wait for lock.

------------------------------------------------------------------------------*/
sony_result_t sony_dvb_ascot2d_Tune (sony_dvb_tuner_t * pTuner,
                                         uint32_t frequency, sony_dvb_system_t system, uint8_t bandWidth)
{

    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    sony_ascot2d_t *pAscot2DTuner = NULL;
    pr_dbg ("sony_dvb_ascot2d_Tune\n");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Assign from user pointer. */
    pAscot2DTuner = (sony_ascot2d_t *) pTuner->user;

    /* Call into underlying driver. Convert system, bandwidth into dtv system. */
    switch (system) {
    case SONY_DVB_SYSTEM_DVBC:
        dtvSystem = SONY_DTV_SYSTEM_DVBC;
        break;

    case SONY_DVB_SYSTEM_DVBT:
    {
        switch (bandWidth) {
        case 5:
             dtvSystem = SONY_DTV_SYSTEM_DVBT_5;
            break;
        case 6:
            dtvSystem = SONY_DTV_SYSTEM_DVBT_6;
            break;
        case 7:
            dtvSystem = SONY_DTV_SYSTEM_DVBT_7;
            break;
        case 8:
            dtvSystem = SONY_DTV_SYSTEM_DVBT_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
    }
        break;
    case SONY_DVB_SYSTEM_DVBT2:
    {
        switch (bandWidth) {
        case 5:
            dtvSystem = SONY_DTV_SYSTEM_DVBT2_5;
            break;
        case 6:
            dtvSystem = SONY_DTV_SYSTEM_DVBT2_6;
            break;
        case 7:
            dtvSystem = SONY_DTV_SYSTEM_DVBT2_7;
            break;
        case 8:
            dtvSystem = SONY_DTV_SYSTEM_DVBT2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

        }
    }
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        switch (bandWidth) {
        case 6:
            dtvSystem = SONY_DTV_SYSTEM_DVBC2_6;
            break;
        case 8:
            dtvSystem = SONY_DTV_SYSTEM_DVBC2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
            break;
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    }
    result = sony_ascot2d_DtvTune (pAscot2DTuner, frequency, dtvSystem);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DVB_SYSTEM_UNKNOWN;
        pTuner->frequency = 0;
        pTuner->bandWidth = 0;
        SONY_TRACE_RETURN (result);
    }

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequency = pAscot2DTuner->frequency;
    pTuner->bandWidth = bandWidth;

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Sleep the tuner module.
------------------------------------------------------------------------------*/
sony_result_t sony_dvb_ascot2d_Sleep (sony_dvb_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    pr_dbg ("sony_dvb_ascot2d_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot2d_Sleep (((sony_ascot2d_t *) pTuner->user));

    pTuner->system = SONY_DVB_SYSTEM_UNKNOWN;
    pTuner->frequency = 0;
    pTuner->bandWidth = 0;
    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 RF level monitor.
------------------------------------------------------------------------------*/
sony_result_t sony_dvb_ascot2d_monitor_RFLevel (sony_dvb_tuner_t * pTuner,
                                                    uint32_t ifAgc, uint8_t rfAinValid, uint32_t rfAin, int32_t * pRFLeveldB)
{
    pr_dbg ("sony_dvb_ascot2d_monitor_RFLevel");

    if ((!pTuner) || (!pRFLeveldB)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Protect against overflow. IFAGC is unsigned 12-bit. */
    if (ifAgc > 0xFFF) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* 
     * RFAIN not enabled:
     * ==================
     * 1st order:
     * RF (dB) = 0.0374 * IFAGC - 101.52
     * 
     * 2nd order:
     * RF (dB) = -5.139E-6 * IFAGC^2 + 0.05014969962 * IFAGC - 106.59564776988
     */
    if (!rfAinValid) {
        *pRFLeveldB = (int32_t) (-51 * (int32_t) (ifAgc * ifAgc) + 501497 * (int32_t) ifAgc - 1065956478);
        *pRFLeveldB = (*pRFLeveldB < 0) ? *pRFLeveldB - 5000 : *pRFLeveldB + 5000;
        *pRFLeveldB /= 10000;
    }
    else {
        /* unused. */
        rfAin = rfAin;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_ascot2d_RFFilterConfig (sony_dvb_tuner_t * pTuner, uint8_t coeff, uint8_t offset)
{
    sony_result_t result = SONY_RESULT_OK;
    pr_dbg ("sony_dvb_ascot2d_RFFilterConfig");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot2d_RFFilterConfig (((sony_ascot2d_t *) pTuner->user), coeff, offset);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_ascot2d_Write_GPIO (sony_dvb_tuner_t * pTuner, uint8_t id, uint8_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    pr_dbg ("sony_dvb_ascot2d_Write_GPIO");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot2d_Write_GPIO (((sony_ascot2d_t *) pTuner->user), id, value);

    SONY_TRACE_RETURN (result);
}
