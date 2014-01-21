/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 3803 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>


#include "sony_dvb_demod_monitorT.h"
#include "sony_math.h"

#ifdef SONY_DVB_SUPPORT_DVBT


#define MONITORT_PRERSBER_TIMEOUT              1000
#define MONITORT_PRERSBER_CHECK_PERIOD         10   /* Time in ms between each read of the demod status */

#define t_FreezeReg(pDemod) ((pDemod)->pI2c->WriteOneRegister ((pDemod)->pI2c, (pDemod)->i2cAddress, 0x01, 0x01))
#define t_UnFreezeReg(pDemod) ((void)((pDemod)->pI2c->WriteOneRegister ((pDemod)->pI2c, (pDemod)->i2cAddress, 0x01, 0x00)))

/*------------------------------------------------------------------------------
 Static functions
------------------------------------------------------------------------------*/

static sony_result_t sony_dvb_demod_monitorT_SNRReg (sony_dvb_demod_t * pDemod, uint16_t * pSNRReg);
static sony_result_t IsTPSLocked (sony_dvb_demod_t * pDemod);

/*--------------------------------------------------------------------
  Sync state monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  uint8_t          *pSyncStat   Sync state (6: Lock)
  uint8_t          *pTSLockStat TS lock state (1: Lock)
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorT_SyncStat (sony_dvb_demod_t * pDemod, uint8_t * pSyncStat, uint8_t * pTSLockStat, uint8_t* pUnlockDetected)
{
    uint8_t rdata = 0x00;

 //   printk ("sony_dvb_demod_monitorT_SyncStat\n");

    if ((!pDemod) || (!pSyncStat) || (!pTSLockStat) || (!pUnlockDetected)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x10, &rdata, 1) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Slave    Bank    Addr    Bit     Name
     * ----------------------------------------------
     * <SLV-T>  00h     10h     [4]     IEARLY_NOOFDM
     */
     //printk("rdata 0x%x \n",rdata);
    *pUnlockDetected = (uint8_t)((rdata & 0x10)? 1 : 0);
    /* Slave    Bank    Addr    Bit     Name                Meaning
     * -----------------------------------------------------------------------
     * <SLV-T>  00h     10h     [2:0]   IREG_SEQ_OSTATE     0~5:UNLOCK,6:LOCK
     */
    *pSyncStat = (uint8_t)(rdata & 0x07);
    /* Slave    Bank    Addr    Bit     Name                Meaning
     * -----------------------------------------------------------------------
     * <SLV-T>  00h     10h     [5]     IREG_TSIF_TS_LOCK   0:UNLOCK,1:LOCK
     */ 
    *pTSLockStat = (uint8_t)((rdata & 0x20) ? 1 : 0);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  IF AGC output monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  uint32_t         *pIFAGCOut   IF AGC output register
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorT_IFAGCOut (sony_dvb_demod_t * pDemod, uint32_t * pIFAGCOut)
{
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_IFAGCOut");

    if (!pDemod || !pIFAGCOut)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Read pir_agc_gain */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x26, rdata, 2) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    *pIFAGCOut = ((rdata[0] & 0x0F) << 8) | rdata[1];

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  MODE/GUARD monitor

  sony_dvb_demod_t            *pDemod   Instance of demod control struct
  sony_dvb_demod_dvbt_mode_t  *pMode    Mode estimation result
  sony_dvb_demod_dvbt_guard_t *pGuard   Guard interval estimation result
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorT_ModeGuard (sony_dvb_demod_t * pDemod,
                                                     sony_dvbt_mode_t * pMode, sony_dvbt_guard_t * pGuard)
{
    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_ModeGuard");

    if (!pDemod || !pMode || !pGuard)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (t_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TPS Lock check */
    {
        sony_result_t result = IsTPSLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Read syncstat0 */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x40, &rdata, 1) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    t_UnFreezeReg (pDemod);

    *pMode = (sony_dvbt_mode_t) ((rdata >> 2) & 0x03);
    *pGuard = (sony_dvbt_guard_t) (rdata & 0x03);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Carrier offset monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  int32_t          *pOffset     Carrier offset value(kHz)
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorT_CarrierOffset (sony_dvb_demod_t * pDemod, int32_t * pOffset)
{
    uint8_t rdata[4];
    uint32_t ctlVal = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_CarrierOffset");

    if (!pDemod || !pOffset)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (t_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TPS Lock check */
    {
        sony_result_t result = IsTPSLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave    Bank    Addr    Bit              Name               
     * ---------------------------------------------------------------
     * <SLV-T>   00h     4Ch     [4:0]      IREG_CRCG_CTLVAL[28:24] 
     * <SLV-T>   00h     4Dh     [7:0]      IREG_CRCG_CTLVAL[23:16] 
     * <SLV-T>   00h     4Eh     [7:0]      IREG_CRCG_CTLVAL[15:8] 
     * <SLV-T>   00h     4Fh     [7:0]      IREG_CRCG_CTLVAL[7:0]  
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x4C, rdata, 4) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    t_UnFreezeReg (pDemod);

    /* 29bit signed value to 32bit signed value */
    ctlVal = ((rdata[0] & 0x1F) << 24) | (rdata[1] << 16) | (rdata[2] << 8) | (rdata[3]);
    *pOffset = sony_Convert2SComplement (ctlVal, 29);
    *pOffset = (-1 * ((*pOffset) * pDemod->bandWidth)) / 235;

    /* Compensate for inverted spectrum. */
    {
        /* Inverted RF spectrum with non-inverting tuner architecture. */
        if (pDemod->confSense == DVB_DEMOD_SPECTRUM_INV) {
            *pOffset *= -1;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT_MER (sony_dvb_demod_t * pDemod, int32_t * pMER)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_MER");

    result = sony_dvb_demod_monitorT_SNR (pDemod, pMER);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT_PreViterbiBER (sony_dvb_demod_t * pDemod, uint32_t * pBER)
{
    uint8_t rdata[2];
    uint32_t bitError = 0;
    uint32_t period = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_PreViterbiBER");

    if (!pDemod || !pBER)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (t_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit              Name           
     * ------------------------------------------------------------
     * <SLV-T>   00h     39h     [0]          IREG_VBER_ACT
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x39, rdata, 1) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((rdata[0] & 0x01) == 0) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */
    }

    /* slave    Bank    Addr    Bit              Name           
     * ------------------------------------------------------------
     * <SLV-T>   00h     22h     [7:0]        IREG_VBER_BITECNT[15:8]
     * <SLV-T>   00h     23h     [7:0]        IREG_VBER_BITECNT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x22, rdata, 2) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    bitError = (rdata[0] << 8) | rdata[1];

    /* Read vber_period */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x6F, rdata, 1) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    t_UnFreezeReg (pDemod);

    period = ((rdata[0] & 0x07) == 0) ? 256 : (0x1000 << (rdata[0] & 0x07));

    if ((period == 0) || (bitError > period)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /*
      BER = bitError * 10000000 / period
          = bitError * 78125 / (period / 128)
          = bitError * 3125 * 25 / (period / 128)
          (Divide in 2 steps to prevent overflow.)
    */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        div = period / 128;

        Q = (bitError * 3125) / div;
        R = (bitError * 3125) % div;

        R *= 25;
        Q = Q * 25 + R / div;
        R = R % div;

        /* rounding */
        if (R >= div/2) {
            *pBER = Q + 1;
        } else {
            *pBER = Q;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT_PreRSBER (sony_dvb_demod_t * pDemod, uint32_t * pBER)
{
    uint8_t rdata[3];
    uint32_t bitError = 0;
    uint32_t periodExp = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_PreRSBER");

    if (!pDemod || !pBER)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    rdata[0] = rdata[1] = rdata[2] = 0;

    /*   slave    Bank    Addr    Bit              Name           
     * ------------------------------------------------------------
     * <SLV-T>   00h     62h     [5:0]        IREG_BERN_BITECNT[21:16]
     * <SLV-T>   00h     63h     [7:0]        IREG_BERN_BITECNT[15:8]
     * <SLV-T>   00h     64h     [7:0]        IREG_BERN_BITECNT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x62, rdata, 3) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);


    /* Check IREG_BERN_VALID bit (bit 7) */
    /* slave    Bank    Addr    Bit              Name           
     * ------------------------------------------------------------
     * <SLV-T>   00h     62h     [7]          IREG_BERN_VALID
     */
    if ((rdata[0] & 0x80) == 0)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */

    bitError = ((rdata[0] & 0x3F) << 16) | (rdata[1] << 8) | rdata[2];

    /* Read ber_period */
    /* slave    Bank    Addr    Bit    default          Name
     * -------------------------------------------------------------------------
     * <SLV-T>   00h     60h    [4:0]     5'h0B        OREG_BERN_PERIOD[4:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x60, rdata, 1) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    periodExp = (rdata[0] & 0x1F);

    if ((periodExp <= 11) && (bitError > (1U << periodExp) * 204 * 8)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /*
      BER = bitError * 10000000 / (2^N * 204 * 8)
          = bitError * 312500 / (2^N * 51)
          = bitError * 250 * 1250 / (2^N * 51)
          (Divide in 2 steps to prevent overflow.)
    */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        if (periodExp <= 8) {
            div = (1U << periodExp) * 51;
        } else {
            div = (1U << 8) * 51;
        }

        Q = (bitError * 250) / div;
        R = (bitError * 250) % div;

        R *= 1250;
        Q = Q * 1250 + R / div;
        R = R % div;

        if (periodExp > 8) {
            /* rounding */
            *pBER = (Q + (1 << (periodExp - 9))) >> (periodExp - 8);
        } else {
            /* rounding */
            if (R >= div/2) {
                *pBER = Q + 1;
            } else {
                *pBER = Q;
            }
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}


/*--------------------------------------------------------------------
  TPS information monitor

  sony_dvb_demod_t          *pDemod  Instance of demod control struct
  sony_dvb_demod_tpsinfo_t  *pInfo   TPS information struct
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorT_TPSInfo (sony_dvb_demod_t * pDemod, sony_dvbt_tpsinfo_t * pInfo)
{
    uint8_t rdata[7];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_TPSInfo");

    if (!pDemod || !pInfo)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (t_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TPS Lock check */
    {
        sony_result_t result = IsTPSLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit              Name           
     * ------------------------------------------------------------
     * <SLV-T>   00h     2Fh     [7:6]       ITPS_CNST[1:0]
     * <SLV-T>   00h     2Fh     [5:3]       ITPS_HIER[2:0]
     * <SLV-T>   00h     2Fh     [2:0]       ITPS_HRATE[2:0] 
     * <SLV-T>   00h     30h     [7:5]       ITPS_LRATE[2:0]
     * <SLV-T>   00h     30h     [4:3]       ITPS_GI[1:0]
     * <SLV-T>   00h     30h     [2:1]       ITPS_MODE[1:0]
     * <SLV-T>   00h     31h     [7:6]       ITPS_FNUM[1:0]
     * <SLV-T>   00h     31h     [5:0]       ITPS_LENGTH_INDICATOR[5:0]
     * <SLV-T>   00h     32h     [7:0]       ITPS_CELLID[15:8]
     * <SLV-T>   00h     33h     [7:0]       ITPS_CELLID[7:0]
     * <SLV-T>   00h     34h     [5:0]       ITPS_RESERVE_EVEN[5:0]
     * <SLV-T>   00h     35h     [5:0]       ITPS_RESERVE_ODD[5:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x2F, rdata, 7) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    t_UnFreezeReg (pDemod);

    pInfo->constellation = (sony_dvbt_constellation_t) ((rdata[0] >> 6) & 0x03);  /* 0x2F, bit[7:6] */
    pInfo->hierarchy = (sony_dvbt_hierarchy_t) ((rdata[0] >> 3) & 0x07);  /* 0x2F, bit[5:3] */
    pInfo->rateHP = (sony_dvbt_coderate_t) (rdata[0] & 0x07); /* 0x2F, bit[2:0] */
    pInfo->rateLP = (sony_dvbt_coderate_t) ((rdata[1] >> 5) & 0x07);  /* 0x30, bit[7:5] */
    pInfo->guard = (sony_dvbt_guard_t) ((rdata[1] >> 3) & 0x03);  /* 0x30, bit[4:3] */
    pInfo->mode = (sony_dvbt_mode_t) ((rdata[1] >> 1) & 0x03);    /* 0x30, bit[2:1] */
    pInfo->fnum = (rdata[2] >> 6) & 0x03; /* 0x31 , bit[7:6] */
    pInfo->lengthIndicator = rdata[2] & 0x3F; /* 0x31 , bit[5:0] */
    pInfo->cellID = (uint16_t) ((rdata[3] << 8) | rdata[4]);    /* 0x32, 0x33 */
    pInfo->reservedEven = rdata[5] & 0x3F; /*  0x34 , bit[5:0] */
    pInfo->reservedOdd = rdata[6] & 0x3F; /*  0x35 , bit[5:0] */

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT_PacketErrorNumber (sony_dvb_demod_t * pDemod, uint32_t * pPEN)
{
    uint8_t rdata[3];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_PacketErrorNumber");

    if (!pDemod || !pPEN)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Read ber_cwrjctcnt */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xEA, rdata, 3) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (!(rdata[2] & 0x01)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    *pPEN = (rdata[0] << 8) | rdata[1];

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT_SpectrumSense (sony_dvb_demod_t * pDemod, sony_dvb_demod_spectrum_sense_t * pSense)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_SpectrumSense");

    if (!pDemod || !pSense)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (t_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TPS Lock check */
    {
        sony_result_t result = IsTPSLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    {
        uint8_t data = 0;

        /* Set bank 0x07 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x07) != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name                mean
         * ---------------------------------------------------------------------------------
         * <SLV-T>   07h     C6h     [0]        IREG_COSNE_SINV      0:not invert,   1:invert
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xC6, &data, sizeof (data)) != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        t_UnFreezeReg (pDemod);

        *pSense = (data & 0x01) ? DVB_DEMOD_SPECTRUM_INV : DVB_DEMOD_SPECTRUM_NORMAL;

        
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);

}

/*--------------------------------------------------------------------
  SNR monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  int32_t          *pSNR        SNR(dBx1000)

  uint16_t         *pSNRReg     IREG_SNMON_OD register value
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorT_SNR (sony_dvb_demod_t * pDemod, int32_t * pSNR)
{
    sony_result_t result = SONY_RESULT_OK;
    uint16_t reg = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_SNR");

    if (!pDemod || !pSNR)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_dvb_demod_monitorT_SNRReg (pDemod, &reg);
    if (result != SONY_RESULT_OK)
        SONY_TRACE_RETURN (result);

    if (reg == 0) {
        /* log10 function will return INF */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }
    if (reg >= 13000) {
        reg = 12999;
    }

    /*
     * SNR = 10 * log10 (reg / 8)
     * = 10 * log10 (reg) - log10(8)
     * 
     * Where: 
     * log10(8) = 0.90308
     */
    *pSNR = 10 * 10 * ((int32_t) sony_log10 (reg) - (int32_t) sony_log10 (13000 - reg));
    *pSNR += 32200;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT_SamplingOffset (sony_dvb_demod_t * pDemod, int32_t * pPPM)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_SamplingOffset");

    if ((!pDemod) || (!pPPM)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (t_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TPS Lock check */
    {
        sony_result_t result = IsTPSLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    {
        uint8_t data[5];
        uint32_t trlMon2 = 0;

        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Read IREG_TRL_CTLVAL_S */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x52, data, sizeof (data)) != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        t_UnFreezeReg (pDemod);

        /* Ignore top bits - not significant. */
        trlMon2 = data[1] << 24;
        trlMon2 |= data[2] << 16;
        trlMon2 |= data[3] << 8;
        trlMon2 |= data[4];

        switch (pDemod->bandWidth) {
        case 5:
            /* (1 / 0x1CB3333333) * 1e12 * 64 = 519.2 */
            /* Confirm offset range. */
            if ((data[0] & 0x7F) != 0x1C) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
            }
            *pPPM = (int32_t) (trlMon2 - 0xB3333333);
            *pPPM = (*pPPM + 4) / 8;
            *pPPM *= 519;
            break;
        case 6:
            /* (1 / 0x17EAAAAAAA) * 1e12 * 64 = 623.0 */
            /* Confirm offset range. */
            if ((data[0] & 0x7F) != 0x17) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
            }
            *pPPM = (int32_t) (trlMon2 - 0xEAAAAAAA);
            *pPPM = (*pPPM + 4) / 8;
            *pPPM *= 623;
            break;
        case 7:
            /* (1 / 0x1480000000 ) * 1e12 * 64 = 726.9 */
            /* Confirm offset range. */
            if ((data[0] & 0x7F) != 0x14) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
            }
            *pPPM = (int32_t) (trlMon2 - 0x80000000);
            *pPPM = (*pPPM + 4) / 8;
            *pPPM *= 727;
            break;
        case 8:
            /* (1 / 0x11F0000000) * 1e12 * 64 = 830.7 */
            /* Confirm offset range. */
            if ((data[0] & 0x7F) != 0x11) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
            }
            *pPPM = (int32_t) (trlMon2 - 0xF0000000);
            *pPPM = (*pPPM + 4) / 8;
            *pPPM *= 831;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
        }

        *pPPM = (*pPPM + (5000 * 8)) / (10000 * 8);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT_Quality (sony_dvb_demod_t * pDemod, uint8_t * pQuality)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dvbt_tpsinfo_t tps;
    sony_dvbt_profile_t profile = SONY_DVBT_PROFILE_HP;
    uint32_t ber = 0;
    int32_t sn = 0;
    int32_t snRel = 0;
    int32_t berSQI = 0;
    int32_t tmp = 0;
    /**
     @brief The list of DVB-T Nordig Profile 1 for Non Hierachical signal
            C/N values in dBx1000.
    */
    static const int32_t nordigNonHDVBTdB1000[3][5] = {
        /* 1/2,   2/3,   3/4,   5/6,   7/8 */    
        {5100, 6900, 7900, 8900, 9700},         /* QPSK */
        {10800, 13100, 14600, 15600, 16000},    /* 16-QAM */
        {16500, 18700, 20200, 21600, 22500}     /* 64-QAM */
    };

    /**
     @brief The list of DVB-T Nordig Profile 1 for Hierachical signal, HP
            C/N values in dBx1000.
    */
    static const int32_t nordigHierHpDVBTdB1000[3][2][5] = {
         /* alpha = 1 */ 
        {  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
            {9100,   12000,  13600,  15000,  16600},    /* LP = 16-QAM */
            {10900,  14100,  15700,  19400,  20600}    /* LP = 64-QAM */
        },/* alpha = 2 */ 
        {  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
            {6800,    9100,  10400,  11900,  12700},    /* LP = 16-QAM */
            {8500,   11000,  12800,  15000,  16000}    /* LP = 64-QAM */
        },/* alpha = 4 */ 
        {  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
            {5800,    7900,  9100,   10300,  12100},    /* LP = 16-QAM */
            {8000,    9300,  11600,  13000,  12900}    /* LP = 64-QAM */
        }
    };
    /**
     @brief The list of DVB-T Nordig Profile 1 for Hierachical signal, LP
            C/N values in dBx1000.
    */
    static const int32_t nordigHierLpDVBTdB1000[3][2][5] = {
           /* alpha = 1 */ 
        {  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
            {12500,  14300,  15300,  16300,  16900},    /* 16-QAM */
            {16700,  19100,  20900,  22500,  23700}    /* 64-QAM */
        }, /* alpha = 2 */ 
        {  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
            {15000,  17200,  18400,  19100,  20100},    /* 16-QAM */
            {18500,  21200,  23600,  24700,  25900}    /* 64-QAM */
        }, /* alpha = 4 */ 
        {  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
            {19500,  21400,  22500,  23700,  24700},    /* 16-QAM */
            {21900,  24200,  25600,  26900,  27800}    /* 64-QAM */
        }
    };

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_Quality");

    if ((!pDemod) || (!pQuality)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    /*
     * SQI (Signal Quality Indicator) given by:
     * SQI = (((C/Nrel - 3) / 10) + 1) * BER_SQI 
     * BER_SQI = 20*log10(1/BER) - 40
     * 
     * Re-arrange for ease of calculation:
     * BER_SQI = 20 * (log10(1) - log10(BER)) - 40
     * 
     * If BER in units of 1e-7
     * BER_SQI = 20 * (log10(1) - (log10(BER) - log10(1e7)) - 40
     * 
     * BER_SQI = log10(1e7) - 40 - log10(BER)
     * BER_SQI = -33 - 20*log10 (BER)
     */

    /* Monitor TPS for Modulation / Code Rate */
    result = sony_dvb_demod_monitorT_TPSInfo (pDemod, &tps);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Monitor profile for hierarchical signal*/
    if (tps.hierarchy != SONY_DVBT_HIERARCHY_NON) {
        uint8_t data = 0;
        /* Set bank 0 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x67, &data, 1) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        profile = ((data & 0x01) == 0x01) ? SONY_DVBT_PROFILE_LP : SONY_DVBT_PROFILE_HP;

    }

    /* Get Pre-RS (Post-Viterbi) BER. */
    result = sony_dvb_demod_monitorT_PreRSBER (pDemod, &ber);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Get SNR value. */
    result = sony_dvb_demod_monitorT_SNR (pDemod, &sn);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Ensure correct TPS values. */
    if ((tps.constellation >= SONY_DVBT_CONSTELLATION_RESERVED_3) || (tps.rateHP >= SONY_DVBT_CODERATE_RESERVED_5)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    /* Calculate. */

    /* Handle 0%. */
    if (tps.hierarchy == SONY_DVBT_HIERARCHY_NON) {
        snRel = sn - nordigNonHDVBTdB1000[tps.constellation][tps.rateHP];        
    } 
    else
    if ( profile == SONY_DVBT_PROFILE_LP ) {
        snRel = sn - nordigHierLpDVBTdB1000[(int32_t)tps.hierarchy-1][(int32_t)tps.constellation-1][tps.rateLP];
    }
    else {
        snRel = sn - nordigHierHpDVBTdB1000[(int32_t)tps.hierarchy-1][(int32_t)tps.constellation-1][tps.rateHP];
    }
    
    if (snRel < -7 * 1000) {
        *pQuality = 0;
        SONY_TRACE_RETURN (result);
    }

    /* Calculate BER_SQI and clip. BER_SQI (1000x) */
    if (ber == 0) {
        berSQI = 100 * 1000;

    }
    else {
        berSQI = (int32_t) (10 * sony_log10 (ber));
        berSQI = 20 * (7 * 1000 - (berSQI)) - 40 * 1000;
        if (berSQI < 20000) {
            berSQI = 0;
        }

    }

    /* Round up for rounding errors. */
    if (snRel >= 3 * 1000) {
        *pQuality = (uint8_t) ((berSQI + 500) / 1000);
    }
    else {
        tmp = (((snRel - (3 * 1000)) / 10) + 1000);
        *pQuality = (uint8_t) (((tmp * berSQI) + (5 * 1000 * 1000 / 10)) / (1000 * 1000)) & 0xFF;
    }

    if (*pQuality > 100) {
        *pQuality = 100;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT_PER (sony_dvb_demod_t * pDemod, uint32_t * pPER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t packetError = 0;
    uint32_t period = 0;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_PER");
    if (!pDemod || !pPER) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    {
        uint8_t rdata[4];
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x5C, rdata, 4) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if ((rdata[1] & 0x01) == 0)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */
        packetError = (rdata[2] << 8) | rdata[3];
        period = 1U << (rdata[0] & 0x0F);
    }
    if ((period == 0) || (packetError > period)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;
        div = period;
        Q = (packetError * 1000) / div;
        R = (packetError * 1000) % div;
        R *= 1000;
        Q = Q * 1000 + R / div;
        R = R % div;
        if ((div != 1) && (R >= div/2)) {
            *pPER = Q + 1;
        } else {
            *pPER = Q;
        }
    }
    SONY_TRACE_RETURN (result);
}
static sony_result_t IsTPSLocked (sony_dvb_demod_t * pDemod)
{
    uint8_t sync = 0;
    uint8_t tslock = 0;
    uint8_t earlyUnlock = 0;
    sony_result_t result = SONY_RESULT_OK;

    printk ("IsTPSLocked\n");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = sony_dvb_demod_monitorT_SyncStat (pDemod, &sync, &tslock, &earlyUnlock);
    if (result != SONY_RESULT_OK)
        SONY_TRACE_RETURN (result);

    if (sync != 6)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t sony_dvb_demod_monitorT_SNRReg (sony_dvb_demod_t * pDemod, uint16_t * pSNRReg)
{
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT_SNRReg");

    if (!pDemod || !pSNRReg)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT) {
        /* Not DVB-T */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (t_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TPS Lock check */
    {
        sony_result_t result = IsTPSLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            t_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Read snmon_od */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x28, rdata, 2) != SONY_RESULT_OK) {
        t_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    t_UnFreezeReg (pDemod);

    *pSNRReg = (rdata[0] << 8) | rdata[1];

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

#endif
