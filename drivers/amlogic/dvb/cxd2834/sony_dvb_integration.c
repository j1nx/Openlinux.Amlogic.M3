/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>


#include "sony_dvb_integration.h"
#include "sony_dvb_demod_monitor.h"
#include "sony_dvb_demod_monitorT2.h"
#include "sony_dvb_demodT2.h"
#include "sony_dvb_demodC2.h"
//#include "sony_stdlib.h"


/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/
#ifndef DEMOD_TUNE_POLL_INTERVAL
#define DEMOD_TUNE_POLL_INTERVAL    10
#endif
#define SONY_DVBT2_MAX_PLPS          0xFF     /**< DVBT2 PLP maximum. */

#if 0
#define pr_dbg(fmt, args...) printk(KERN_ERR "cxd2834: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif

#define pr_error(fmt, args...) printk(KERN_ERR "cxd2834: " fmt, ## args)


/*------------------------------------------------------------------------------
  Static functions
------------------------------------------------------------------------------*/

static sony_result_t Scan (sony_dvb_integration_t * pInteg,
                               sony_dvb_scan_params_t * pScanParams, sony_dvb_scan_callback_t callBack);

static sony_result_t BlindTune (sony_dvb_integration_t * pInteg,
                                    sony_dvb_system_t system, uint32_t centreFreqkHz, uint8_t bwMHz);

static sony_result_t Tune (sony_dvb_integration_t * pInteg, sony_dvb_tune_params_t * pTuneParams);

static sony_result_t DoTune (sony_dvb_integration_t * pInteg, uint32_t centreFreqkHz, sony_dvb_system_t system, uint8_t bwMHz);


static sony_result_t ScanCallback(sony_dvb_integration_t * pInteg, sony_dvb_scan_callback_t callback, sony_dvb_scan_result_t * scanResult);

/*------------------------------------------------------------------------------
  Implementation of public functions
------------------------------------------------------------------------------*/

sony_result_t sony_dvb_integration_Create (sony_dvb_xtal_t xtalFreq,
                                               uint8_t I2CAddress,
                                               sony_i2c_t * pDemodI2c,
                                               sony_dvb_demod_t * pDemod,
                                               sony_dvb_tuner_t * pTuner, sony_dvb_integration_t * pInteg)
{

    sony_result_t result = SONY_RESULT_OK;

    pr_dbg ("sony_dvb_integration_Create\n");

    if ((!pDemod) || (!pDemodI2c) || (!pInteg) || (!pTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    sony_atomic_set (&(pInteg->cancel), 0);
    pInteg->pDemod = pDemod;
    pInteg->pTuner = pTuner;

    result = sony_dvb_demod_Create (xtalFreq, I2CAddress, pDemodI2c, pDemod);
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_integration_Initialize (sony_dvb_integration_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;

    pr_dbg ("sony_dvb_integration_Initialize\n");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((!pInteg->pTuner) || (!pInteg->pTuner->Initialize)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_dvb_demod_Initialize (pInteg->pDemod);
    if (result == SONY_RESULT_OK) {
        /* Initialize the tuner. */
        pr_dbg("SONY_RESULT_OK\n");
        result = pInteg->pTuner->Initialize (pInteg->pTuner);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_integration_Finalize (sony_dvb_integration_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;

    pr_dbg ("sony_dvb_integration_Finalize\n");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = pInteg->pTuner->Finalize (pInteg->pTuner);
    if (result == SONY_RESULT_OK) {
        result = sony_dvb_demod_Finalize (pInteg->pDemod);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_integration_Scan (sony_dvb_integration_t * pInteg,
                                             sony_dvb_scan_params_t * pScanParams, sony_dvb_scan_callback_t callBack)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t r = SONY_RESULT_OK;  /* Used for return codes AFTER scan completed. */
	
    pr_dbg ("sony_dvb_integration_Scan\n");

    /* Argument verification. */
    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pTuner) || (!pScanParams)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Ensure the scan parameters are valid. */
    if (pScanParams->endFrequencykHz < pScanParams->startFrequencykHz) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pScanParams->stepFrequencykHz == 0) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_dvb_demod_ScanInitialize (pInteg->pDemod, pScanParams);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Perform the scan */
    result = Scan (pInteg, pScanParams, callBack);

    r = sony_dvb_demod_ScanFinalize (pInteg->pDemod, pScanParams);
    if (r != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (r);
    }

    /* Clear cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 0);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_integration_BlindTune (sony_dvb_integration_t * pInteg,
                                                  sony_dvb_system_t system, uint32_t centreFreqkHz, uint8_t bwMHz)
{
    sony_result_t result = SONY_RESULT_OK;

    pr_dbg ("sony_dvb_integration_BlindTune\n");

    /* Argument verification. */
    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Blind tune is not supported for DVBC2 */
    if (system == SONY_DVB_SYSTEM_DVBC2){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    result = BlindTune (pInteg, system, centreFreqkHz, bwMHz);
    
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_integration_Tune (sony_dvb_integration_t * pInteg, sony_dvb_tune_params_t * pTuneParams)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t r = SONY_RESULT_OK;  /* Used for return codes AFTER scan completed. */


    pr_dbg ("sony_dvb_integration_Tune\n");

    /* Argument verification. */
    if ((!pInteg) || (!pInteg->pTuner) || (!pTuneParams) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Setup any required preset information. */
    result = sony_dvb_demod_TuneInitialize (pInteg->pDemod, pTuneParams);

    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Perform Tune */
    result = Tune (pInteg, pTuneParams);

    if ((result == SONY_RESULT_OK) && (pTuneParams->system == SONY_DVB_SYSTEM_DVBT2)) {
        uint16_t waitTime = 0;
        for(;;) {
            if (sony_atomic_read (&(pInteg->cancel)) != 0) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
            }
    r = sony_dvb_demod_TuneFinalize (pInteg->pDemod, pTuneParams, result);
            if (r == SONY_RESULT_ERROR_HW_STATE) {
                if (waitTime >= DVB_DEMOD_TUNE_T2_L1POST_TIMEOUT) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
                } else {
                    msleep (DEMOD_TUNE_POLL_INTERVAL);
                    waitTime += DEMOD_TUNE_POLL_INTERVAL;
                }
            } else {
                SONY_TRACE_RETURN (r); /* OK or other error */
            }
        }
    } else {
        r = sony_dvb_demod_TuneFinalize (pInteg->pDemod, pTuneParams, result);
    if (r != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (r);
    }
    SONY_TRACE_RETURN (result);
    }
}

sony_result_t sony_dvb_integration_Sleep (sony_dvb_integration_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;

    pr_dbg ("sony_dvb_integration_Sleep\n");

    if ((!pInteg) ||(!pInteg->pDemod) || (!pInteg->pTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = pInteg->pTuner->Sleep (pInteg->pTuner);
    if (result == SONY_RESULT_OK) {
        result = sony_dvb_demod_Sleep (pInteg->pDemod);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_integration_WaitTSLock (sony_dvb_integration_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;

    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;

    uint16_t timeout = 0;
    static sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    static uint32_t elapsed = 0;
    uint8_t i;
    
    /* Argument verification. */
    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    //printk("sdiwtskl pInteg->pDemod->system %d\n",pInteg->pDemod->system);
    switch (pInteg->pDemod->system) {
    case SONY_DVB_SYSTEM_DVBC:
        timeout = DVB_DEMOD_DVBC_WAIT_TS_LOCK;
        break;
    case SONY_DVB_SYSTEM_DVBT2:
        printk("SONY_DVB_SYSTEM_DVBT2\n");
        timeout = DVB_DEMOD_DVBT2_WAIT_TS_LOCK;
        break;
    case SONY_DVB_SYSTEM_DVBT:
        printk("SONY_DVB_SYSTEM_DVBT\n");
        timeout = DVB_DEMOD_DVBT_WAIT_TS_LOCK;
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        timeout = DVB_DEMOD_DVBC2_WAIT_TS_LOCK;
        break;

        /* Intentional fall-through. */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {

        result = sony_stopwatch_elapsed(&timer,&elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        if (elapsed >= timeout) {
            continueWait = 0;
        }
	        result = sony_dvb_demod_CheckTSLock (pInteg->pDemod, &lock);
	        printk("sony_dvb_demod_CheckTSLock result 0x%x,lock 0x%x\n",result,lock);
	        if (result != SONY_RESULT_OK) {
	            SONY_TRACE_RETURN (result);
	        	}

        switch (lock) {
        case DEMOD_LOCK_RESULT_LOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        case DEMOD_LOCK_RESULT_UNLOCKED:

            /* Intentional fall-through. */
        case DEMOD_LOCK_RESULT_NOTDETECT:
        default:
            break;              /* continue waiting... */
        }

        /* Check cancellation. */
        if (sony_atomic_read (&(pInteg->cancel)) != 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }
        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, DEMOD_TUNE_POLL_INTERVAL);
            if (result != SONY_RESULT_OK) {
    SONY_TRACE_RETURN (result);
            }
        } 
        else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_integration_Cancel (sony_dvb_integration_t * pInteg)
{
    pr_dbg ("sony_dvb_integration_Cancel\n");

    /* Argument verification. */
    if ((!pInteg)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set the cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 1);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_integration_SetConfig (sony_dvb_integration_t * pInteg, sony_dvb_demod_config_id_t configId,
                                                  int32_t value)
{

    pr_dbg ("sony_dvb_integration_SetConfig\n");

    /* Argument verification. */
    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN (sony_dvb_demod_SetConfig (pInteg->pDemod, configId, value));
}

static sony_result_t ScanCallback( sony_dvb_integration_t * pInteg, sony_dvb_scan_callback_t callback, sony_dvb_scan_result_t * scanResult)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t numPLPs = 0;
    uint8_t PLPIds[SONY_DVBT2_MAX_PLPS];
    int32_t index = 0;

    pr_dbg ("ScanCallback\n");

    if (!pInteg || !scanResult )
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    if (scanResult->system == SONY_DVB_SYSTEM_DVBT2 && scanResult->tuneResult == SONY_RESULT_OK) {
        uint16_t waitTime = 0;
        for (index = 0; index < SONY_DVBT2_MAX_PLPS; index++) {
            PLPIds[index] = 0;
        }
        
        for(;;) {
            if (sony_atomic_read (&(pInteg->cancel)) != 0) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
            }
        result = sony_dvb_demod_monitorT2_DataPLPs (pInteg->pDemod, PLPIds, &numPLPs );

            if (result == SONY_RESULT_OK) {
                break;
            } else if (result == SONY_RESULT_ERROR_HW_STATE) {
                if (waitTime >= DVB_DEMOD_TUNE_T2_L1POST_TIMEOUT) {
                    scanResult->tuneResult = SONY_RESULT_ERROR_TIMEOUT;
                callback( pInteg, scanResult);
                SONY_TRACE_RETURN(SONY_RESULT_OK);
                } else {
                    msleep (DEMOD_TUNE_POLL_INTERVAL);
                    waitTime += DEMOD_TUNE_POLL_INTERVAL;
                }
            } else {
                SONY_TRACE_RETURN (result); /* Other (fatal) error */
            }
        }

        for (index = 0; index < numPLPs; index++) {
            scanResult->t2_plp_id = PLPIds[index];
            callback( pInteg, scanResult);
        }

    } else {
        callback (pInteg, scanResult);
    }

    SONY_TRACE_RETURN(result);

}

static sony_result_t Scan (sony_dvb_integration_t * pInteg,
                               sony_dvb_scan_params_t * pScanParams, sony_dvb_scan_callback_t callBack)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dvb_scan_result_t scanResult;
    sony_dvb_system_t systems[] = { SONY_DVB_SYSTEM_UNKNOWN, SONY_DVB_SYSTEM_UNKNOWN, SONY_DVB_SYSTEM_UNKNOWN };
    uint8_t i = 0;

    pr_dbg ("Scan\n");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    systems[0] = pScanParams->system;

    /* Setup mixed system scanning */
    if (pScanParams->multiple != 0) {
        switch(pScanParams->system)
        {
        /* Intentional fall through */
        case SONY_DVB_SYSTEM_DVBT:
        case SONY_DVB_SYSTEM_DVBT2:
            systems[1] = (pScanParams->system == SONY_DVB_SYSTEM_DVBT2) ? SONY_DVB_SYSTEM_DVBT : SONY_DVB_SYSTEM_DVBT2;
            break;

#ifdef SONY_DVB_SUPPORT_DVBC2
        /* Intentional fall through */
        case SONY_DVB_SYSTEM_DVBC:
        case SONY_DVB_SYSTEM_DVBC2:
            systems[1] = (pScanParams->system == SONY_DVB_SYSTEM_DVBC2) ? SONY_DVB_SYSTEM_DVBC : SONY_DVB_SYSTEM_DVBC2;
            break;
#endif

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
    }

    scanResult.centreFreqkHz = pScanParams->startFrequencykHz;
    scanResult.scanParams = pScanParams;

#ifdef SONY_DVB_SUPPORT_DVBC2
    if ((systems[0] == SONY_DVB_SYSTEM_DVBC2) || (systems[1] == SONY_DVB_SYSTEM_DVBC2))
    {
        sony_dvbc2_scan_seq_t scanSeq;
        sony_stopwatch_t timer;
        uint32_t sleepStartTime = 0;
        uint32_t elapsedTime = 0;
        uint32_t scanFreqkHz = pScanParams->startFrequencykHz;

        /* Set current system. */
        scanResult.system = SONY_DVB_SYSTEM_DVBC2;

        /* Start stopwatch to measure total waiting time */
        result = sony_stopwatch_start (&timer);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        while (scanFreqkHz <= pScanParams->endFrequencykHz) {
            scanResult.tuneResult = SONY_RESULT_OK;
            (void) sony_dvb_demodC2_InitScanSeq (pInteg->pDemod, pScanParams, &scanSeq);

            if (pScanParams->bwMHz == 8) {
                scanSeq.nextScanFrequency = scanFreqkHz + 2000;
            }else if (pScanParams->bwMHz == 6) {
                scanSeq.nextScanFrequency = scanFreqkHz + 1500;
            }
            else {
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }
            scanSeq.tuneParams.centreFreqkHz = scanFreqkHz;
            result = DoTune (pInteg, scanFreqkHz, SONY_DVB_SYSTEM_DVBC2, pScanParams->bwMHz);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            while (scanSeq.running) {
                if (scanSeq.sleepTime == 0) {
                    /* Set current total time before calling demod layer function. */
                    /* This is timed from scan start */
                    result = sony_stopwatch_elapsed(&timer, &scanSeq.currentTime);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }

                    result = sony_dvb_demodC2_ScanSeq (pInteg->pDemod, &scanSeq);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }

                    /* DVB-C2 Scan tunes to all potenial Dataslice/PLP combinations */
                    if (scanSeq.tuneRequired) {
                        scanResult.tuneResult = sony_dvb_integration_Tune (pInteg, &scanSeq.tuneParams);

                        scanSeq.tuneRequired = 0; /* FALSE */
                        
                        switch (scanResult.tuneResult) {
                            case SONY_RESULT_OK:
                                /* Do nothing */

                            case SONY_RESULT_ERROR_UNLOCK:
                            case SONY_RESULT_ERROR_TIMEOUT:
                                /* Notify caller. */
                                break;

                                /* Intentional fall through. */
                            case SONY_RESULT_ERROR_ARG:
                            case SONY_RESULT_ERROR_I2C:
                            case SONY_RESULT_ERROR_SW_STATE:
                            case SONY_RESULT_ERROR_HW_STATE:
                            case SONY_RESULT_ERROR_RANGE:
                            case SONY_RESULT_ERROR_NOSUPPORT:
                            case SONY_RESULT_ERROR_CANCEL:
                            case SONY_RESULT_ERROR_OTHER:
                            default:
                                /* Serious error occurred -> cancel operation. */
                                SONY_TRACE_RETURN (scanResult.tuneResult);
                        }
                    }

                    if (scanSeq.callbackRequired) 
                    {
                        if (scanSeq.loopResult != SONY_RESULT_OK){
                            scanResult.tuneResult = scanSeq.loopResult;
                            scanSeq.loopResult = SONY_RESULT_OK;
                        }

                        if (scanResult.tuneResult == SONY_RESULT_OK || 
                            scanResult.tuneResult == SONY_RESULT_ERROR_UNLOCK ||
                            scanResult.tuneResult == SONY_RESULT_ERROR_TIMEOUT)
                        {
                            /* Set up callback structure to the latest data */                
                            scanResult.c2Params = scanSeq.tuneParams.c2Params;
                            scanResult.centreFreqkHz = scanSeq.tuneParams.centreFreqkHz;    
                        }

                        if (callBack) 
                        {
                            callBack (pInteg, &scanResult);
                        }                    

                        scanSeq.callbackRequired = 0; /* FALSE */
                    }    

                    if (scanSeq.sleepTime > 0) {
                        /* Sleep requested, store start time now */
                        result = sony_stopwatch_elapsed(&timer, &sleepStartTime);
                        if (result != SONY_RESULT_OK) {
                            SONY_TRACE_RETURN (result);
                        }
                    }
                }

                if (scanSeq.sleepTime > 0) {
                    /*    This polling interval is not always same as interval in scan sequence.
                        This interval affects interval for checking cancellation flag. */
                    result = sony_stopwatch_sleep(&timer, SCAN_CHECK_CANCEL_INTERVAL);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }

                    result = sony_stopwatch_elapsed(&timer, &elapsedTime);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }

                    if (elapsedTime - sleepStartTime >= scanSeq.sleepTime) {
                        /* Finish sleep */
                        scanSeq.sleepTime = 0;
                    }
                }

                /* Sanity check on overall wait time. */
                result = sony_stopwatch_elapsed(&timer, &elapsedTime);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                if (elapsedTime > C2_MAX_SCAN_DURATION) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
                }

                /* Check cancellation. */
                if (sony_atomic_read (&(pInteg->cancel)) != 0) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
                }
            }

            /* Jump over found signals bandwidth for next frequency to be scanned */
                /* DVB-C2 Signal succesfully found and notified so jump directly to the first potential
                 * centre tuning frequency after current signals bandwidth ends
                 */
            scanFreqkHz = scanSeq.nextScanFrequency;

            /* Setup next scan iteration frequency */
            scanSeq.tuneParams.centreFreqkHz = scanFreqkHz;
        }
    }
#endif

    /* Reset scanResult.centreFreqkHz for multiple system scan cases */
    scanResult.centreFreqkHz = pScanParams->startFrequencykHz;

    /* Scan routine for DVBT, T2 and C */
    while (scanResult.centreFreqkHz <= pScanParams->endFrequencykHz) {

        /* Attempt blind tune (Set RF frequency). Handle mixed scanning. */
        i = 0;
        while (systems[i] != SONY_DVB_SYSTEM_UNKNOWN) {

            /* Get current system. */
            scanResult.system = systems[i++];
            
            if (scanResult.system != SONY_DVB_SYSTEM_DVBC2){
                scanResult.tuneResult = DoTune (pInteg, scanResult.centreFreqkHz, scanResult.system, pScanParams->bwMHz);
                if (scanResult.tuneResult == SONY_RESULT_OK) {
                    scanResult.tuneResult = WaitDemodLock (pInteg);
                }
                switch (scanResult.tuneResult) {
                    /* Intentional fall-through. */
                case SONY_RESULT_OK:
                case SONY_RESULT_ERROR_UNLOCK:
                case SONY_RESULT_ERROR_TIMEOUT:
                    /* Notify caller. */
                    if (callBack) {
                        result = ScanCallback (pInteg, callBack, &scanResult);
                        if (result != SONY_RESULT_OK) {
                            SONY_TRACE_RETURN (result);
                        }
                    }
                    break;

                    /* Intentional fall through. */
                case SONY_RESULT_ERROR_ARG:
                case SONY_RESULT_ERROR_I2C:
                case SONY_RESULT_ERROR_SW_STATE:
                case SONY_RESULT_ERROR_HW_STATE:
                case SONY_RESULT_ERROR_RANGE:
                case SONY_RESULT_ERROR_NOSUPPORT:
                case SONY_RESULT_ERROR_CANCEL:
                case SONY_RESULT_ERROR_OTHER:
                default:
                    /* Serious error occurred -> cancel operation. */
                    SONY_TRACE_RETURN (scanResult.tuneResult);
                }
            }
        }
        scanResult.centreFreqkHz += pScanParams->stepFrequencykHz;

        /* Check cancellation. */
        if (sony_atomic_read (&(pInteg->cancel)) != 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }
    }

    SONY_TRACE_RETURN (result);
}



static sony_result_t BlindTune (sony_dvb_integration_t * pInteg, sony_dvb_system_t system, uint32_t centreFreqkHz,
                                    uint8_t bwMHz)
{
    sony_result_t result = SONY_RESULT_OK;

    pr_dbg ("BlindTune \n");

    /* Argument verification. */
    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
	//pr_dbg ("BlindTune 000000000\n");

    /* Ensure demod is configured correctly for blind acquisition. */
    result = sony_dvb_demod_EnableBlindAcquisition (pInteg->pDemod, system);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
	//pr_dbg ("BlindTune 11111111111\n");

    /* Perform demodulator tuning operation. */
    result = DoTune (pInteg, centreFreqkHz, system, bwMHz);
#if 0
    if (result == SONY_RESULT_OK) {
        result = WaitDemodLock (pInteg);
    }
#endif
    /* Clear cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 0);

    SONY_TRACE_RETURN (result);
}

static sony_result_t Tune (sony_dvb_integration_t * pInteg, sony_dvb_tune_params_t * pTuneParams)
{
    	sony_result_t result = SONY_RESULT_OK;
	#ifdef MULTI_SYS
	sony_dvb_system_t systems[] = { SONY_DVB_SYSTEM_UNKNOWN, SONY_DVB_SYSTEM_UNKNOWN, SONY_DVB_SYSTEM_UNKNOWN };
	unsigned char i = 0;
	#endif

    pr_dbg ("Tune\n");

    if ((!pInteg) || (!pInteg->pDemod) || (!pTuneParams)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef MULTI_SYS
    systems[0] = pTuneParams->system;
    /* Setup mixed system scanning */

        switch(pTuneParams->system)
        {
        /* Intentional fall through */
        case SONY_DVB_SYSTEM_DVBT:
        case SONY_DVB_SYSTEM_DVBT2:
            systems[1] = (pTuneParams->system == SONY_DVB_SYSTEM_DVBT2) ? SONY_DVB_SYSTEM_DVBT : SONY_DVB_SYSTEM_DVBT2;
            break;

#ifdef SONY_DVB_SUPPORT_DVBC2
        /* Intentional fall through */
        case SONY_DVB_SYSTEM_DVBC:
        case SONY_DVB_SYSTEM_DVBC2:
            systems[1] = (pTuneParams->system == SONY_DVB_SYSTEM_DVBC2) ? SONY_DVB_SYSTEM_DVBC : SONY_DVB_SYSTEM_DVBC2;
            break;
#endif

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
#endif

#ifdef SONY_DVB_SUPPORT_DVBC2
    if (pTuneParams->system == SONY_DVB_SYSTEM_DVBC2){
        sony_dvbc2_tune_seq_t tuneSeq;
        sony_stopwatch_t timer;
        uint32_t sleepStartTime = 0;
        uint32_t elapsedTime = 0;
        if (pTuneParams->c2Params.c2TuningFrequencykHz == 0){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        if (pTuneParams->c2Params.isDependantStaticDS && (pTuneParams->c2Params.c2TuningFrequencyDSDSkHz == 0)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        /* Start stopwatch to measure total waiting time */
        result = sony_stopwatch_start (&timer);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_dvb_demodC2_InitTuneSeq (pInteg->pDemod, pTuneParams, &tuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        while (tuneSeq.running) {
            if (tuneSeq.sleepTime == 0) {
                /* Set current total time before calling demod layer function. */
                /* This is timed from scan start */
                result = sony_stopwatch_elapsed(&timer, &tuneSeq.currentTime);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                result = sony_dvb_demodC2_TuneSeq (pInteg->pDemod, &tuneSeq);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                /* Check if RF tuning is invoked in demod layer. */
                if (tuneSeq.tuneRequired) {
                    result = pInteg->pTuner->Tune (pInteg->pTuner, tuneSeq.rfFrequency,
                                                    SONY_DVB_SYSTEM_DVBC2, pInteg->pDemod->bandWidth);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }

                    tuneSeq.tuneRequired = 0; /* FALSE */
                }

                if (tuneSeq.sleepTime > 0) {
                    /* Sleep requested, store start time now */
                    result = sony_stopwatch_elapsed(&timer, &sleepStartTime);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
                }
            }

            if (tuneSeq.sleepTime > 0) {
                /*    This polling interval is not always same as interval in scan sequence.
                    This interval affects interval for checking cancellation flag. */
                result = sony_stopwatch_sleep(&timer, TUNE_CHECK_CANCEL_INTERVAL);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                result = sony_stopwatch_elapsed(&timer, &elapsedTime);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                if (elapsedTime - sleepStartTime >= tuneSeq.sleepTime) {
                    /* Finish sleep */
                    tuneSeq.sleepTime = 0;
                }
            }

            /* Sanity check on overall wait time. */
            result = sony_stopwatch_elapsed(&timer, &elapsedTime);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            if (elapsedTime > C2_MAX_TUNE_DURATION) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
            }

            /* Check cancellation. */
            if (sony_atomic_read (&(pInteg->cancel)) != 0) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
            }
        }

        pTuneParams->c2Params.tuneResult = (uint8_t)tuneSeq.lockResult;
        result = tuneSeq.lockResult;

        /* Deferred exit if tune failed */
        if (tuneSeq.lockResult != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (tuneSeq.lockResult);
        }
    }
#endif

#if 1
    if ((pTuneParams->system == SONY_DVB_SYSTEM_DVBT) || (pTuneParams->system == SONY_DVB_SYSTEM_DVBT2) || (pTuneParams->system == SONY_DVB_SYSTEM_DVBC))
    {
#else
while (systems[i] != SONY_DVB_SYSTEM_UNKNOWN) {
            /* Get current system. */
		pTuneParams->system = systems[i++];
#endif
	printk("pTuneParams->system %d\n",pTuneParams->system);
        result = DoTune (pInteg, pTuneParams->centreFreqkHz, pTuneParams->system, pTuneParams->bwMHz);
        //printk("Dotune result 0x%x \n",result);
                

                
    }
    	
    /* Clear cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 0);

    SONY_TRACE_RETURN (result);
}


static sony_result_t DoTune (sony_dvb_integration_t * pInteg, uint32_t centreFreqkHz, sony_dvb_system_t system, uint8_t bwMHz)
{
    sony_result_t result = SONY_RESULT_OK;

    pr_dbg ("DoTune\n");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
	pr_dbg ("DoTune 0000000\n");

    /* Enable acquisition on the demodulator. */
    result = sony_dvb_demod_Tune (pInteg->pDemod, system, bwMHz);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Perform RF tuning. */
	#if 1
	pr_dbg ("DoTune pInteg->pTuner->Tune %d,%d\n",centreFreqkHz,bwMHz);
    result = pInteg->pTuner->Tune (pInteg->pTuner, centreFreqkHz, system, bwMHz);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
	#endif
	
    

    result = sony_dvb_demod_TuneEnd (pInteg->pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t WaitDemodLock (sony_dvb_integration_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;

    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;
    
    uint16_t timeout = 0;
    static sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;
    //printk ("WaitDemodLock pInteg->pDemod->system %d\n",pInteg->pDemod->system);

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch (pInteg->pDemod->system) {

    case SONY_DVB_SYSTEM_DVBT2:
        timeout = DVB_DEMOD_DVBT2_WAIT_LOCK;
        break;

    case SONY_DVB_SYSTEM_DVBC2:
        timeout = DVB_DEMOD_DVBC2_WAIT_LOCK;
        break;

    case SONY_DVB_SYSTEM_DVBC:
        /* DVB-C: Wait TS lock timeout. */
        timeout = DVB_DEMOD_DVBC_WAIT_TS_LOCK;
        break;

    case SONY_DVB_SYSTEM_DVBT:
        /* DVB-T: Wait for early unlock valid. */
        timeout = DVB_DEMOD_DVBT_WAIT_LOCK;
        break;

        /* Intentional fall-through. */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    /* Wait for demod lock */
 result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
  
        if (elapsed >= timeout) {
            continueWait = 0;
        }
        
	//msleep(1000);
        result = sony_dvb_demod_CheckDemodLock (pInteg->pDemod, &lock);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        switch (lock) {
        case DEMOD_LOCK_RESULT_LOCKED:
            if (pInteg->pDemod->system == SONY_DVB_SYSTEM_DVBT2) {
                result = sony_dvb_demodT2_OptimizeMISO(pInteg->pDemod);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }
            }
            SONY_TRACE_RETURN (SONY_RESULT_OK);

        case DEMOD_LOCK_RESULT_UNLOCKED:
            /* Intentional fall-through. */
        case DEMOD_LOCK_RESULT_NOTDETECT:
        default:
            break;              /* continue waiting... */
        }

        /* Check cancellation. */
        if (sony_atomic_read (&(pInteg->cancel)) != 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }

        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, DEMOD_TUNE_POLL_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }
    
    SONY_TRACE_RETURN (result);
}
