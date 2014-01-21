/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 3910 $
    $Author: vanstone $

</dev:header>

------------------------------------------------------------------------------*/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>

#include "sony_dvb_demod_monitorC.h"
#include "sony_math.h"

#define c_FreezeReg(pDemod) ((pDemod)->pI2c->WriteOneRegister ((pDemod)->pI2c, (pDemod)->i2cAddress, 0x01, 0x01))
#define c_UnFreezeReg(pDemod) ((void)((pDemod)->pI2c->WriteOneRegister ((pDemod)->pI2c, (pDemod)->i2cAddress, 0x01, 0x00)))

static sony_result_t IsARLocked (sony_dvb_demod_t * pDemod);


sony_result_t sony_dvb_demod_monitorC_SyncStat (sony_dvb_demod_t * pDemod, uint8_t * pARLock, uint8_t * pTSLockStat, uint8_t* pUnlockDetected)
{
    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_SyncStat");

    if ((!pDemod) || (!pARLock) || (!pTSLockStat) || (!pUnlockDetected)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /*  slave    Bank    Addr    Bit              Name                meaning
     * ---------------------------------------------------------------------------------
     * <SLV-T>   40h     88h     [0]          IREG_AR_LOCK        0:Unlock 1:Lock
     * <SLV-T>   40h     88h     [1]          IREG_AR_TIMEOUT     0:Checking, 1:DVB-C signal does not exist
     * <SLV-T>   40h     72h     [0]          IREG_TSIF_TS_LOCK   0:Unlock 1:Lock
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x88, &rdata, 1) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    *pARLock = (uint8_t) ((rdata & 0x01) ? 1 : 0);    /* IREG_AR_LOCK */

    *pUnlockDetected = (uint8_t) ((rdata & 0x02) ? 1 : 0);

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x72, &rdata, 1) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    *pTSLockStat = (uint8_t) ((rdata & 0x01) ? 1 : 0);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  IF AGC output monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  int32_t          *pIFAGCOut   IF AGC output register (signed)
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorC_IFAGCOut (sony_dvb_demod_t * pDemod, uint32_t * pIFAGCOut)
{
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_IFAGCOut");

    if (!pDemod || !pIFAGCOut)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* slave    Bank    Addr    Bit              Name              
     * ------------------------------------------------------------
     * <SLV-T>   40h     49h     [3:0]       IIFAGC_OUT[11:8]
     * <SLV-T>   40h     4Ah     [7:0]       IIFAGC_OUT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x49, rdata, 2) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    *pIFAGCOut = (rdata[0] & 0x0F) << 8 | rdata[1];

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Constellation monitor

  sony_dvb_demod_t                    *pDemod Instance of demod control struct
  sony_dvbc_constellation_t            *pQAM   Constellation
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorC_QAM (sony_dvb_demod_t * pDemod, sony_dvbc_constellation_t * pQAM)
{
    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_QAM");

    if (!pDemod || !pQAM)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (c_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            c_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*   slave    Bank    Addr    Bit              Name           
     * ------------------------------------------------------------
     * <SLV-T>   40h     19h    [2:0]       IREG_QAM_LEVEL[2:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x19, &rdata, 1) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    c_UnFreezeReg (pDemod);

    *pQAM = (sony_dvbc_constellation_t) (rdata & 0x07);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Symbol rate monitor

  sony_dvb_demod_t *pDemod           Instance of demod control struct
  uint32_t         *pSymRate         Symbol rate (kSymbol/sec)
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorC_SymbolRate (sony_dvb_demod_t * pDemod, uint32_t * pSymRate)
{
    uint8_t rdata1[2];
    uint8_t rdata2[5];
    uint32_t trialsrate;
    int32_t  tlfIntegral;
    uint32_t tsmDrate;


    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_SymbolRate");

    if (!pDemod || !pSymRate)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (c_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            c_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit              Name           
     * ------------------------------------------------------------
     * <SLV-T>   40h     1Ah    [3:0]       IREG_TRIALSRATE[11:8]
     * <SLV-T>   40h     1Bh    [7:0]       IREG_TRIALSRATE[7:0]
     * <SLV-T>   40h     9Bh    [7:0]       IREG_TLFINTEGRAL[31:24]
     * <SLV-T>   40h     9Ch    [7:0]       IREG_TLFINTEGRAL[23:16]
     * <SLV-T>   40h     9Dh    [7:0]       IREG_TLFINTEGRAL[15:8]
     * <SLV-T>   40h     9Eh    [7:0]       IREG_TLFINTEGRAL[7:0]
     * <SLV-T>   40h     9Fh    [1:0]       IREG_TSMDRATE[1:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x1A, rdata1, sizeof(rdata1)) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Read IREG_TLFINTEGRAL and IREG_TSMDRATE */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x9B, rdata2, sizeof(rdata2)) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    c_UnFreezeReg (pDemod);

    trialsrate = ((rdata1[0] & 0x0F) << 8) | rdata1[1];
    tlfIntegral = (int32_t)((rdata2[0] << 24) | (rdata2[1] << 16) | (rdata2[2] << 8) | rdata2[3]);
    tsmDrate = rdata2[4] & 0x03;

    /* Symbol rate[MSymbol/sec] = ( IREG_TRIALSRATE +( IREG_TLFINTEGRAL /( 2^( 24 + IREG_TSMDRATE )))) * 41 / 16384*/


    *pSymRate = ((trialsrate * 4 + (tlfIntegral / (1 << (22 + tsmDrate)))) * 41000 + 32768) / 65536;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Carrier offset monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  int32_t          *pOffset     Carrier offset value(kHz)
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorC_CarrierOffset (sony_dvb_demod_t * pDemod, int32_t * pOffset)
{
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_CarrierOffset");

    if (!pDemod || !pOffset)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (c_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            c_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave    Bank    Addr    Bit              Name               
     * ---------------------------------------------------------------
     * <SLV-T>   40h     15h     [5:0]      IREG_FREQ_OFFSET[13:8] 
     * <SLV-T>   40h     16h     [7:0]      IREG_FREQ_OFFSET[7:0] 
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x15, rdata, 2) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    c_UnFreezeReg (pDemod);

    /* 14bit signed value to 32bit signed value */
    /* Carrier offset[KHz] = IREG_FREQ_OFFSET * 2.50244 */

    *pOffset = sony_Convert2SComplement (((rdata[0] & 0x3F) << 8) | rdata[1], 14);
    *pOffset = *pOffset * 41000 / 16384;    /* ADC Clock(41MHz) / 16384 */

    /* Compensate for inverted spectrum. */
    {
        /* Inverted RF spectrum with non-inverting tuner architecture. */
        if (pDemod->confSense == DVB_DEMOD_SPECTRUM_INV) {
            *pOffset *= -1;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Spectrum sense monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  int              *pSense   Spectrum is inverted or not(0: normal, 1: inverted)
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorC_SpectrumSense (sony_dvb_demod_t * pDemod, sony_dvb_demod_spectrum_sense_t * pSense)
{
    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_SpectrumSense");

    if (!pDemod || !pSense)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (c_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            c_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave    Bank    Addr    Bit              Name                mean
     * ---------------------------------------------------------------------------------
     * <SLV-T>   40h     19h     [7]    IREG_DET_SPECTRUM_INV     0:Not inverted,   1:inverted
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x19, &rdata, 1) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    c_UnFreezeReg (pDemod);

    *pSense = (rdata & 0x80) ? DVB_DEMOD_SPECTRUM_INV : DVB_DEMOD_SPECTRUM_NORMAL;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  SNR monitor

  sony_dvb_demod_t *pDemod      Instance of demod control struct
  int32_t           *pSNR       SNR(dB x 1000)

  uint8_t          *pSNRReg     IREG_SNR_ESTIMATE register value
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_monitorC_SNR (sony_dvb_demod_t * pDemod, int32_t * pSNR)
{
    sony_dvbc_constellation_t qam = SONY_DVBC_CONSTELLATION_16QAM;
    uint16_t reg = 0x00;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_SNR");

    if (!pDemod || !pSNR)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_dvb_demod_monitorC_QAM (pDemod, &qam);    /* Get constellation */
    if (result != SONY_RESULT_OK)
        SONY_TRACE_RETURN (result);

    result = sony_dvb_demod_monitorC_SNRReg (pDemod, &reg);
    if (result != SONY_RESULT_OK)
        SONY_TRACE_RETURN (result);

    if (reg == 0) {
        /* log function will return INF */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    switch (qam) {
    case SONY_DVBC_CONSTELLATION_16QAM:
    case SONY_DVBC_CONSTELLATION_64QAM:
    case SONY_DVBC_CONSTELLATION_256QAM:
        /* SNR(dB) = -9.50 * ln(IREG_SNR_ESTIMATE / (24320)) */
        *pSNR = -95 * (int32_t) sony_log (reg) + 95941;
        break;
    case SONY_DVBC_CONSTELLATION_32QAM:
    case SONY_DVBC_CONSTELLATION_128QAM:
        /*  SNR(dB) = -8.75 * ln(IREG_SNR_ESTIMATE / (20800)) */
        *pSNR = -88 * (int32_t) sony_log (reg) + 86999;
        break;
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Unknown value... */
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorC_SNRReg (sony_dvb_demod_t * pDemod, uint16_t * pSNRReg)
{
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_SNRReg");

    rdata[0] = rdata[1] = 0;

    if (!pDemod || !pSNRReg)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (c_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            c_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave    Bank    Addr    Bit              Name               
     * ------------------------------------------------------------
     * <SLV-T>   40h     4Ch     [4:0]      IREG_SNR_ESTIMATE[12:8]
     * <SLV-T>   40h     4Dh     [7:0]      IREG_SNR_ESTIMATE[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x4C, rdata, 2) != SONY_RESULT_OK) {
        c_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    c_UnFreezeReg (pDemod);

    *pSNRReg = ((rdata[0] & 0x1F) << 8) | rdata[1];

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorC_PreRSBER (sony_dvb_demod_t * pDemod, uint32_t * pBER)
{
    uint8_t rdata[3];
    uint32_t bitError = 0;
    uint32_t periodExp = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_PreRSBER");

    if (!pDemod || !pBER)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    rdata[0] = rdata[1] = rdata[2] = 0;

    /*  slave    Bank    Addr    Bit              Name           
     * ------------------------------------------------------------
     * <SLV-T>   40h     36h     [5:0]        IREG_BERN_BITECNT[21:16]
     * <SLV-T>   40h     37h     [7:0]        IREG_BERN_BITECNT[15:8]
     * <SLV-T>   40h     38h     [7:0]        IREG_BERN_BITECNT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x36, rdata, 3) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Check IREG_BERN_VALID bit (bit 7) */
    if ((rdata[0] & 0x80) == 0)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */

    bitError = ((rdata[0] & 0x3F) << 16) | (rdata[1] << 8) | rdata[2];

    /* Set bank 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /*  slave    Bank    Addr    Bit    default          Name
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

sony_result_t sony_dvb_demod_monitorC_PacketErrorNumber (sony_dvb_demod_t * pDemod, uint32_t * pPEN)
{
    uint8_t rdata[3];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_PacketErrorNumber");

    if (!pDemod || !pPEN)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK) {
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

sony_result_t sony_dvb_demod_monitorC_PER (sony_dvb_demod_t * pDemod, uint32_t * pPER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t packetError = 0;
    uint32_t period = 0;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorC_PER");
    if (!pDemod || !pPER) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBC) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    {
        uint8_t rdata[3];
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x40) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xE3, rdata, 3) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if ((rdata[0] & 0x01) == 0)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */
        packetError = (rdata[1] << 8) | rdata[2];
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x5C, rdata, 1) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
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
/*------------------------------------------------------------------------------
 Static functions
------------------------------------------------------------------------------*/
static sony_result_t IsARLocked (sony_dvb_demod_t * pDemod)
{
    uint8_t ar = 0;
    uint8_t tslock = 0;
    uint8_t earlyUnlock = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("IsARLocked");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = sony_dvb_demod_monitorC_SyncStat (pDemod, &ar, &tslock,&earlyUnlock);
    if (result != SONY_RESULT_OK)
        SONY_TRACE_RETURN (result);

    if (ar != 1)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
