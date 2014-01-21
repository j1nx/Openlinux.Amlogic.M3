/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 3893 $
    $Author: vanstone $

</dev:header>

------------------------------------------------------------------------------*/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>

#include "sony_math.h"
#include "sony_dvb_demod_monitorT2.h"
#include "sony_dvb_demod_monitorT.h"

sony_result_t sony_dvb_demod_monitorT2_SyncStat (sony_dvb_demod_t * pDemod, uint8_t * pSyncStat, uint8_t * pTSLockStat, uint8_t * pUnlockDetected)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_SyncStat");

    if ((!pDemod) || (!pSyncStat) || (!pTSLockStat) || (!pUnlockDetected)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        /* This api is accepted in ACTIVE or SLEEP states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        uint8_t data;
        /* slave    Bank    Addr    Bit              Name                Meaning
         * -------------------------------------------------------------------
         * <SLV-T>   20h     10h     [2:0]        IREG_SEQ_OSTATE     0-5:UNLOCK 6:LOCK
         * <SLV-T>  20h      10h     [4]          IEARLY_NOOFDM       1: No OFDM
         * <SLV-T>   20h     10h     [5]          IREG_SP_TSLOCK      0:UNLOCK 1:LOCK
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x10, &data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        *pSyncStat = data & 0x07;
        *pTSLockStat = ((data & 0x20) ? 1 : 0);
        *pUnlockDetected = ((data & 0x10) ? 1 : 0);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_CarrierOffset (sony_dvb_demod_t * pDemod, int32_t * pOffset)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_CarrierOffset");

    if ((!pDemod) || (!pOffset)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x20 */

    /* Read crl_ctlval 27:0 */
    {
        uint8_t data[4];
        uint32_t ctlVal = 0;
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        /* Freeze registers */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (syncState != 6) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name               
         * ---------------------------------------------------------------
         * <SLV-T>   20h     4Ch     [3:0]      IREG_CRCG_CTLVAL[27:24] 
         * <SLV-T>   20h     4Dh     [7:0]      IREG_CRCG_CTLVAL[23:16] 
         * <SLV-T>   20h     4Eh     [7:0]      IREG_CRCG_CTLVAL[15:8] 
         * <SLV-T>   20h     4Fh     [7:0]      IREG_CRCG_CTLVAL[7:0]  
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x4C, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        t2_UnFreezeReg (pDemod);

        /*
         * Format: s-3.30
         * Formula for converting this value to carrier frequency error is shown below
         * Carrier frequency error [Hz] (for BW=8MHz) = -((Register value in s-3.30) * Number of FFT points) / Tu (sec)
         * this is equivalent to the following equation for all FFT sizes
         * = -(IREG_CRCG_CTLVAL * (2^-30) * 64 ) / (7e-6) (All FFTsizes)
         * Carrier frequency error [Hz] (for BW=7MHz) = -(IREG_CRCG_CTLVAL * (2^-30) * 8  ) / (1e-6) (All FFTsizes)
         * Carrier frequency error [Hz] (for BW=6MHz) = -(IREG_CRCG_CTLVAL * (2^-30) * 48 ) / (7e-6) (All FFTsizes)
         * 
         * Or more generally: 
         * Offset [Hz] = -(IREG_CRCG_CTLVAL * (2^-30) * 8 * BW ) / (7e-6) (All FFTsizes)
         * 
         * Note: (2^-30 * 8 / 7e-6) = 1.064368657e-3
         * And: 1 /  (2^-30 * 8 / 7e-6) = 939.52 = 940
         * 
         */
        ctlVal = ((data[0] & 0x0F) << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
        *pOffset = sony_Convert2SComplement (ctlVal, 28);
        *pOffset = (-1 * ((*pOffset) * pDemod->bandWidth)) / 940;
    }

    /* Compensate for inverted spectrum. */
    {
        /* Inverted RF spectrum with non-inverting tuner architecture. */
        if (pDemod->confSense == DVB_DEMOD_SPECTRUM_INV) {
            *pOffset *= -1;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_IFAGCOut (sony_dvb_demod_t * pDemod, uint32_t * pIFAGC)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_IFAGCOut");

    if ((!pDemod) || (!pIFAGC)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }



    /* slave    Bank    Addr    Bit              Name              
     * --------------------------------------------------------
     * <SLV-T>   20h     26h     [3:0]        IIFAGC_OUT[11:8]     
     * <SLV-T>   20h     27h     [7:0]        IIFAGC_OUT[7:0]  
     */
    {
        uint8_t data[2];
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x26, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        *pIFAGC = ((data[0] & 0x0F) << 8) | (data[1]);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_L1Pre (sony_dvb_demod_t * pDemod, sony_dvbt2_l1pre_t * pL1Pre)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_L1Pre");

    if ((!pDemod) || (!pL1Pre)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[37];
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        uint8_t version = 0;
        /* Freeze the register. */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Only valid after L1-pre locked. */
        if (syncState < 4) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set bank 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name           
         * ------------------------------------------------------------------------
         * <SLV-T>   22h     20h     [7:0]        IL1PRE_TYPE[7:0]
         * <SLV-T>   22h     21h     [0]          IL1PRE_BWT_EXT
         * <SLV-T>   22h     22h     [2:0]        IL1PRE_S1[2:0]
         * <SLV-T>   22h     23h     [3:0]        IL1PRE_S2[3:0]
         * <SLV-T>   22h     24h     [0]          IL1PRE_L1_REPETITION_FLAG
         * <SLV-T>   22h     25h     [2:0]        IL1PRE_GUARD_INTERVAL[2:0]
         * <SLV-T>   22h     26h     [3:0]        IL1PRE_PAPR[3:0]
         * <SLV-T>   22h     27h     [3:0]        IL1PRE_L1_MOD[3:0]
         * <SLV-T>   22h     28h     [1:0]        IL1PRE_L1_COD[1:0]
         * <SLV-T>   22h     29h     [1:0]        IL1PRE_L1_FEC_TYPE[1:0]
         * <SLV-T>   22h     2Ah     [1:0]        IL1PRE_L1_POST_SIZE[17:16]
         * <SLV-T>   22h     2Bh     [7:0]        IL1PRE_L1_POST_SIZE[15:8]
         * <SLV-T>   22h     2Ch     [7:0]        IL1PRE_L1_POST_SIZE[7:0]
         * <SLV-T>   22h     2Dh     [1:0]        IL1PRE_L1_POST_INFO_SIZE[17:16]
         * <SLV-T>   22h     2Eh     [7:0]        IL1PRE_L1_POST_INFO_SIZE[15:8]
         * <SLV-T>   22h     2Fh     [7:0]        IL1PRE_L1_POST_INFO_SIZE[7:0]
         * <SLV-T>   22h     30h     [3:0]        IL1PRE_PILOT_PATTERN[3:0]
         * <SLV-T>   22h     31h     [7:0]        IL1PRE_TX_ID_AVAILABILITY[7:0]
         * <SLV-T>   22h     32h     [7:0]        IL1PRE_CELL_ID[15:8]
         * <SLV-T>   22h     33h     [7:0]        IL1PRE_CELL_ID[7:0]
         * <SLV-T>   22h     34h     [7:0]        IL1PRE_NETWORK_ID[15:8]
         * <SLV-T>   22h     35h     [7:0]        IL1PRE_NETWORK_ID[7:0]
         * <SLV-T>   22h     36h     [7:0]        IL1PRE_T2_SYSTEM_ID[15:8]
         * <SLV-T>   22h     37h     [7:0]        IL1PRE_T2_SYSTEM_ID[7:0]
         * <SLV-T>   22h     38h     [7:0]        IL1PRE_NUM_T2_FRAMES[7:0]
         * <SLV-T>   22h     39h     [3:0]        IL1PRE_NUM_DATA_SYMBOLS[11:8]
         * <SLV-T>   22h     3Ah     [7:0]        IL1PRE_NUM_DATA_SYMBOLS[7:0]
         * <SLV-T>   22h     3Bh     [2:0]        IL1PRE_REGEN_FLAG[2:0]
         * <SLV-T>   22h     3Ch     [0]          IL1PRE_L1_POST_EXTENSION
         * <SLV-T>   22h     3Dh     [2:0]        IL1PRE_NUM_RF[2:0]
         * <SLV-T>   22h     3Eh     [2:0]        IL1PRE_CURRENT_RF_IDX[2:0]
         * <SLV-T>   22h     3Fh     [1:0]        IL1PRE_RESERVED[9:8]
         * <SLV-T>   22h     40h     [7:0]        IL1PRE_RESERVED[7:0]
         * <SLV-T>   22h     41h     [7:0]        IL1PRE_CRC_32[31:24]
         * <SLV-T>   22h     42h     [7:0]        IL1PRE_CRC_32[23:16]
         * <SLV-T>   22h     43h     [7:0]        IL1PRE_CRC_32[15:8]
         * <SLV-T>   22h     44h     [7:0]        IL1PRE_CRC_32[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x20, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        t2_UnFreezeReg (pDemod);

        /* Convert data to appropriate format. */
        pL1Pre->type = (sony_dvbt2_l1pre_type_t) data[0];
        pL1Pre->bwExt = data[1] & 0x01;
        pL1Pre->s1 = (sony_dvbt2_s1_t) (data[2] & 0x07);
        pL1Pre->s2 = data[3] & 0x0F;
        pL1Pre->l1Rep = data[4] & 0x01;
        pL1Pre->gi = (sony_dvbt2_guard_t) (data[5] & 0x07);
        pL1Pre->papr = (sony_dvbt2_papr_t) (data[6] & 0x0F);
        pL1Pre->mod = (sony_dvbt2_l1post_constell_t) (data[7] & 0x0F);
        pL1Pre->cr = (sony_dvbt2_l1post_cr_t) (data[8] & 0x03);
        pL1Pre->fec = (sony_dvbt2_l1post_fec_type_t) (data[9] & 0x03);
        pL1Pre->l1PostSize = (data[10] & 0x03) << 16;
        pL1Pre->l1PostSize |= (data[11]) << 8;
        pL1Pre->l1PostSize |= (data[12]);
        pL1Pre->l1PostInfoSize = (data[13] & 0x03) << 16;
        pL1Pre->l1PostInfoSize |= (data[14]) << 8;
        pL1Pre->l1PostInfoSize |= (data[15]);
        pL1Pre->pp = (sony_dvbt2_pp_t) (data[16] & 0x0F);
        pL1Pre->txIdAvailability = data[17];
        pL1Pre->cellId = (data[18] << 8);
        pL1Pre->cellId |= (data[19]);
        pL1Pre->networkId = (data[20] << 8);
        pL1Pre->networkId |= (data[21]);
        pL1Pre->systemId = (data[22] << 8);
        pL1Pre->systemId |= (data[23]);
        pL1Pre->numFrames = data[24];
        pL1Pre->numSymbols = (data[25] & 0x0F) << 8;
        pL1Pre->numSymbols |= data[26];
        pL1Pre->regen = data[27] & 0x07;
        pL1Pre->postExt = data[28] & 0x01;
        pL1Pre->numRfFreqs = data[29] & 0x07;
        pL1Pre->rfIdx = data[30] & 0x07;
        version = (data[31] & 0x03) << 2;
        version |= (data[32] & 0xC0) >> 6;
        pL1Pre->t2Version = (sony_dvbt2_version_t) version;
        pL1Pre->resvd = (data[31] & 0x03) << 8;
        pL1Pre->resvd |= data[32];
        pL1Pre->crc32 = (data[33] << 24);
        pL1Pre->crc32 |= (data[34] << 16);
        pL1Pre->crc32 |= (data[35] << 8);
        pL1Pre->crc32 |= data[36];

        /* Get the FFT mode from the S2 signalling. */
        switch ((pL1Pre->s2 >> 1)) {
        case SONY_DVBT2_S2_M8K_G_DVBT2:
            pL1Pre->fftMode = SONY_DVBT2_M8K;
            break;
        case SONY_DVBT2_S2_M32K_G_DVBT2:
            pL1Pre->fftMode = SONY_DVBT2_M32K;
            break;
        default:
            pL1Pre->fftMode = (sony_dvbt2_mode_t) (pL1Pre->s2 >> 1);
            break;
        }

        /* Get the Mixed indicator (FEFs exist) from the S2 signalling. */
        pL1Pre->mixed = pL1Pre->s2 & 0x01;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT2_Version (sony_dvb_demod_t * pDemod, sony_dvbt2_version_t * pVersion)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t version = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_Version");

    if ((!pDemod) || (!pVersion)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[2];
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        /* Freeze the register. */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Only valid after L1-pre locked. */
        if (syncState < 4) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set bank 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x3F, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        t2_UnFreezeReg (pDemod);

        version = ((data[0] & 0x03) << 2);
        version |= ((data[1] & 0xC0) >> 6);
        *pVersion = (sony_dvbt2_version_t) version;
    }
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT2_Ofdm (sony_dvb_demod_t * pDemod, sony_dvbt2_ofdm_t * pOfdm)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_Ofdm");

    if ((!pDemod) || (!pOfdm)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[5];
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        sony_result_t result = SONY_RESULT_OK;
        
        /* Freeze the registers. */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Only valid after L1-pre locked. */
        if (syncState < 4) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name                mean
         * ---------------------------------------------------------------------------------
         * <SLV-T>   20h     5Ch     [5]          IREG_OFDM_MIXED         0: Not mixed, 1: Mixed
         * <SLV-T>   20h     5Ch     [4]          IREG_OFDM_MISO          0: SISO, 1: MISO
         * <SLV-T>   20h     5Ch     [2:0]        IREG_OFDM_FFTSIZE[2:0]  0:2K, 1:8K, 2:4K, 3:1K, 4:16K, 5:32K
         * <SLV-T>   20h     5Dh     [6:4]        IREG_OFDM_GI[2:0]       0:1/32, 1:1/16, 2:1/8, 3:1/4,4:1/128, 5:19/128, 6:19/256
         * <SLV-T>   20h     5Dh     [2:0]        IREG_OFDM_PP[2:0]       0:PP1, 1:PP2, 2:PP3, 3:PP4, 4:PP5,5:PP6, 6:PP7, 7: PP8
         * <SLV-T>   20h     5Eh     [4]          IREG_OFDM_BWT_EXT       0: Normal, 1: Extended
         * <SLV-T>   20h     5Eh     [3:0]        IREG_OFDM_PAPR[3:0]     0: None, 1: ACE, 2: TR, 3: ACE_TR,
         4-15: reserved
         * <SLV-T>   20h     5Fh     [3:0]        IREG_OFDM_NDSYM[11:8]
         * <SLV-T>   20h     60h     [7:0]        IREG_OFDM_NDSYM[7:0]    Number of data symbols in a T2 frame
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x5C, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Unfreeze the registers. */
        t2_UnFreezeReg (pDemod);

        /* Convert */
        pOfdm->mixed = ((data[0] & 0x20) ? 1 : 0);
        pOfdm->s1 = (sony_dvbt2_s1_t) ((data[0] & 0x10) >> 4);
        pOfdm->mode = (sony_dvbt2_mode_t) (data[0] & 0x07);
        pOfdm->gi = (sony_dvbt2_guard_t) ((data[1] & 0x70) >> 4);
        pOfdm->pp = (sony_dvbt2_pp_t) (data[1] & 0x07);
        pOfdm->bwExt = (data[2] & 0x10) >> 4;
        pOfdm->papr = (sony_dvbt2_papr_t) (data[2] & 0x0F);
        pOfdm->numSymbols = (data[3] << 8) | data[4];
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_DataPLPs (sony_dvb_demod_t * pDemod, uint8_t * pPLPIds, uint8_t * pNumPLPs)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_DataPLPs");

    if ((!pDemod) || (!pPLPIds) || (!pNumPLPs)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t l1PostOK = 0;

        /* Set bank 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Freeze the registers. */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name                mean
         * ---------------------------------------------------------------------------------
         * <SLV-T>   22h     12h     [0]            IL1POST_OK           0:invalid1:valid
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x12, &l1PostOK, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (!(l1PostOK & 0x01)) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Read the number of PLPs. */
        /* slave    Bank    Addr    Bit              Name                 mean
         * ---------------------------------------------------------------------------------
         * <SLV-T>   22h     7Fh     [7:0]      INUM_DATA_PLP_TS[7:0]    number of TS PLP (max 255)
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x7F, pNumPLPs, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Invalid number of PLPs detected. May occur if multiple threads accessing device. */
        if (*pNumPLPs == 0) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
        }

        /* Read the PLPs from the device. */
        /* Set Bank 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Read up to PLP_ID127 */
        /*  slave    Bank    Addr    Bit              Name                 mean
         * --------------------------------------------------------------------------
         * <SLV-T>   22h     80h     [7:0]      IL1POST_PLP_ID0[7:0]     TS PLP_ID #1
         * <SLV-T>   22h     FFh     [7:0]      IL1POST_PLP_ID127[7:0]   TS PLP_ID #127
         * ...
         * <SLV-T>   23h     10h     [7:0]      IL1POST_PLP_ID128[7:0]   TS PLP_ID #128
         * <SLV-T>   23h     8Eh     [7:0]      IL1POST_PLP_ID254[7:0]   TS PLP_ID #255
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x80, pPLPIds, *pNumPLPs > 128 ? 128 : *pNumPLPs) !=
            SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (*pNumPLPs > 128) {
            /* Set Bank 0x23 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x23) != SONY_RESULT_OK) {
                t2_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* Read the remaining data PLPs. */
            if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x10, pPLPIds + 128, *pNumPLPs - 128) !=
                SONY_RESULT_OK) {
                t2_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        t2_UnFreezeReg (pDemod);
    }
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_ActivePLP (sony_dvb_demod_t * pDemod,
                                                      sony_dvbt2_plp_btype_t type, sony_dvbt2_plp_t * pPLPInfo)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_ActivePLP");

    if ((!pDemod) || (!pPLPInfo)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[20];
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        uint8_t addr = 0;
        uint8_t bbheader_addr = 0;
        uint8_t issy_addr = 0;
        uint8_t bbheader_data[2];
        uint8_t issy_data = 0;
        uint8_t index = 0;
        sony_dvbt2_version_t version;
        sony_result_t result = SONY_RESULT_OK;

        /* Freeze the registers. */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Confirm SyncStat. */
        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }

        if (syncState < 5) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Read T2 Version
         * slave    Bank    Addr    Bit           Name   
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x3F, &data[0], 2) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        {
            uint8_t version_data = ((data[0] & 0x03) << 2);
            version_data |= ((data[1] & 0xC0) >> 6);
            version = (sony_dvbt2_version_t) version_data;
        }

        /* Read contents of the L1-post for appropriate item. */
        /* slave    Bank    Addr    Bit           Name   
         * -----------------------------------------------------------
         * <SLV-T>   22h     67h     [7:0]        IL1POST_PLP_ID_C[7:0]
         * <SLV-T>   22h     68h     [2:0]        IL1POST_PLP_TYPE_C[2:0]
         * <SLV-T>   22h     69h     [4:0]        IL1POST_PLP_PAYLOAD_TYPE_C[4:0]
         * <SLV-T>   22h     6Ah     [0]          IL1POST_FF_FLAG_C
         * <SLV-T>   22h     6Bh     [2:0]        IL1POST_FIRST_RF_IDX_C[2:0]
         * <SLV-T>   22h     6Ch     [7:0]        IL1POST_FIRST_FRAME_IDX_C[7:0]
         * <SLV-T>   22h     6Dh     [7:0]        IL1POST_PLP_GROUP_ID_C[7:0]]
         * <SLV-T>   22h     6Eh     [2:0]        IL1POST_PLP_COD_C[2:0]
         * <SLV-T>   22h     6Fh     [2:0]        IL1POST_PLP_MOD_C[2:0]
         * <SLV-T>   22h     70h     [0]          IL1POST_PLP_ROTATION_C
         * <SLV-T>   22h     71h     [1:0]        IL1POST_PLP_FEC_TYPE_C[1:0]
         * <SLV-T>   22h     72h     [1:0]        IL1POST_PLP_NUM_BLOCKS_MAX_C[9:8]
         * <SLV-T>   22h     73h     [7:0]        IL1POST_PLP_NUM_BLOCKS_MAX_C[7:0]
         * <SLV-T>   22h     74h     [7:0]        IL1POST_FRAME_INTERVAL_C[7:0]
         * <SLV-T>   22h     75h     [7:0]        IL1POST_TIME_IL_LENGTH_C[7:0]
         * <SLV-T>   22h     76h     [0]          IL1POST_TIME_IL_TYPE_C
         * <SLV-T>   22h     78h     [0]          IL1POST_IN_BAND_FLAG_C
         * <SLV-T>   22h     79h     [7:0]        IL1POST_RESERVED_1_C[15:8]
         * <SLV-T>   22h     7Ah     [7:0]        IL1POST_RESERVED_1_C[7:0]
         */

        /* slave    Bank    Addr    Bit              Name               meaning
         * --------------------------------------------------------------------------------
         * <SLV-T>   24h     ABh     [0]       IREG_SP_NPD_ON_COMMON    0: Off, 1: On  (Common)
         * <SLV-T>   24h     ACh     [0]       IREG_SP_PLP_MODE_COMMON   0: Normal, 1: HEM  (Common)
         */

        /* slave    Bank    Addr    Bit              Name               meaning
         * --------------------------------------------------------------------------------
         * <SLV-T>   24h     A6h     [0]       IREG_SP_ISSY_ON_COMMON   0: Off, 1: On  (Common)
         */

        if (type == SONY_DVBT2_PLP_COMMON) {
            addr = 0x67;
            bbheader_addr = 0xAB;
            issy_addr = 0xA6;

        }
        else {
            /* slave    Bank    Addr    Bit           Name   
             * --------------------------------------------
             * <SLV-T>   22h     54h     [7:0]        IL1POST_PLP_ID[7:0]
             * <SLV-T>   22h     55h     [2:0]        IL1POST_PLP_TYPE[2:0]
             * <SLV-T>   22h     56h     [4:0]        IL1POST_PLP_PAYLOAD_TYPE[4:0]
             * <SLV-T>   22h     57h     [0]          IL1POST_FF_FLAG
             * <SLV-T>   22h     58h     [2:0]        IL1POST_FIRST_RF_IDX[2:0]
             * <SLV-T>   22h     59h     [7:0]        IL1POST_FIRST_FRAME_IDX[7:0]
             * <SLV-T>   22h     5Ah     [7:0]        IL1POST_PLP_GROUP_ID[7:0]
             * <SLV-T>   22h     5Bh     [2:0]        IL1POST_PLP_COD[2:0]
             * <SLV-T>   22h     5Ch     [2:0]        IL1POST_PLP_MOD[2:0]
             * <SLV-T>   22h     5Dh     [0]          IL1POST_PLP_ROTATION
             * <SLV-T>   22h     5Eh     [1:0]        IL1POST_PLP_FEC_TYPE[1:0]
             * <SLV-T>   22h     5Fh     [1:0]        IL1POST_PLP_NUM_BLOCKS_MAX[9:8]
             * <SLV-T>   22h     60h     [7:0]        IL1POST_PLP_NUM_BLOCKS_MAX[7:0]
             * <SLV-T>   22h     61h     [7:0]        IL1POST_FRAME_INTERVAL[7:0]
             * <SLV-T>   22h     62h     [7:0]        IL1POST_TIME_IL_LENGTH[7:0]
             * <SLV-T>   22h     63h     [0]          IL1POST_TIME_IL_TYPE
             * <SLV-T>   22h     64h     [0]          IL1POST_IN_BAND_FLAG
             * <SLV-T>   22h     65h     [7:0]        IL1POST_RESERVED_1[15:8]
             * <SLV-T>   22h     66h     [7:0]        IL1POST_RESERVED_1[7:0]
             */
            /* slave    Bank    Addr    Bit              Name               meaning
             * --------------------------------------------------------------------------------
             * <SLV-T>   24h     95h     [0]       IREG_SP_NPD_ON_DATA    0: Off, 1: On  (Data)
             * <SLV-T>   24h     96h     [0]       IREG_SP_PLP_MODE_DATA   0: Normal, 1: HEM  (Data)
             */

            /* slave    Bank    Addr    Bit              Name               meaning
             * --------------------------------------------------------------------------------
             * <SLV-T>   24h     90h     [0]       IREG_SP_ISSY_ON_DATA     0: Off, 1: On  (Data)
             */

            addr = 0x54;
            bbheader_addr = 0x95;
            issy_addr = 0x90;
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, addr, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x24 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x24) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->
            ReadRegister (pDemod->pI2c, pDemod->i2cAddress, bbheader_addr, bbheader_data,
                          sizeof (bbheader_data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, issy_addr, &issy_data, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }


        t2_UnFreezeReg (pDemod);

        /* If common, check for no common PLP, frame_interval == 0. */
        if ((type == SONY_DVBT2_PLP_COMMON) && (data[13] == 0)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        pPLPInfo->id = data[index++];
        pPLPInfo->type = (sony_dvbt2_plp_type_t) (data[index++] & 0x07);
		printk("Get active plp, id %d, type %d\n",  pPLPInfo->id,pPLPInfo->type);
        pPLPInfo->payload = (sony_dvbt2_plp_payload_t) (data[index++] & 0x1F);
        pPLPInfo->ff = data[index++] & 0x01;
        pPLPInfo->firstRfIdx = data[index++] & 0x07;
        pPLPInfo->firstFrmIdx = data[index++];
        pPLPInfo->groupId = data[index++];
        pPLPInfo->plpCr = (sony_dvbt2_plp_code_rate_t) (data[index++] & 0x07);
        pPLPInfo->constell = (sony_dvbt2_plp_constell_t) (data[index++] & 0x07);
        pPLPInfo->rot = data[index++] & 0x01;
        pPLPInfo->fec = (sony_dvbt2_plp_fec_t) (data[index++] & 0x03);
        pPLPInfo->numBlocksMax = (uint16_t) ((data[index++] & 0x03)) << 8;
        pPLPInfo->numBlocksMax |= data[index++];
        pPLPInfo->frmInt = data[index++];
        pPLPInfo->tilLen = data[index++];
        pPLPInfo->tilType = data[index++] & 0x01;

        /* Skip 0x77 for common PLP. */
        if (type == SONY_DVBT2_PLP_COMMON) {
            index++;
        }

        pPLPInfo->inBandFlag = data[index++] & 0x01;
        pPLPInfo->rsvd = data[index++] << 8;
        pPLPInfo->rsvd |= data[index++];

        pPLPInfo->npd = bbheader_data[0] & 0x01;
        pPLPInfo->plpMode = bbheader_data[1] & 0x01;
        pPLPInfo->issy = issy_data & 0x01;
        pPLPInfo->inBandBFlag = (uint8_t) ((pPLPInfo->rsvd & 0x8000) >> 15);
        pPLPInfo->inBandBTSRate = 0;

        if ((version == SONY_DVBT2_V121) && (pPLPInfo->inBandBFlag)) {

            /* slave    Bank    Addr    Bit              Name           
             * ------------------------------------------------------------------------
             * <SLV-T>   25h     A6h     [2:0]        IREG_SP_IB_TS_RATE_COMMON[26:24]
             * <SLV-T>   25h     A7h     [7:0]        IREG_SP_IB_TS_RATE_COMMON[23:16]
             * <SLV-T>   25h     A8h     [7:0]        IREG_SP_IB_TS_RATE_COMMON[15:8]
             * <SLV-T>   25h     A9h     [7:0]        IREG_SP_IB_TS_RATE_COMMON[7:0]
             * <SLV-T>   25h     AAh     [2:0]        IREG_SP_IB_TS_RATE_DATA[26:24]
             * <SLV-T>   25h     ABh     [7:0]        IREG_SP_IB_TS_RATE_DATA[23:16]
             * <SLV-T>   25h     ACh     [7:0]        IREG_SP_IB_TS_RATE_DATA[15:8]
             * <SLV-T>   25h     ADh     [7:0]        IREG_SP_IB_TS_RATE_DATA[7:0]
             */

            uint8_t ts_rate_addr = 0xA6;
            uint8_t ts_rate_data[4];

            if (type == SONY_DVBT2_PLP_DATA) {
                ts_rate_addr = 0xAA;
            }

            /* Set bank 0x25 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x25) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (pDemod->pI2c->
                ReadRegister (pDemod->pI2c, pDemod->i2cAddress, ts_rate_addr, ts_rate_data,
                              sizeof (ts_rate_data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
            pPLPInfo->inBandBTSRate = ts_rate_data[0] << 24;
            pPLPInfo->inBandBTSRate |= ts_rate_data[1] << 16;
            pPLPInfo->inBandBTSRate |= ts_rate_data[2] << 8;
            pPLPInfo->inBandBTSRate |= ts_rate_data[3];
        }

    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_DataPLPError (sony_dvb_demod_t * pDemod, uint8_t * pPLPError)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_DataPLPError");

    if ((!pDemod) || (!pPLPError)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data;
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        sony_result_t result = SONY_RESULT_OK;

        /* Freeze the registers. */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Confirm SyncStat. */
        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (syncState < 5) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        /* Set bank 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit           Name                meaning
         *  --------------------------------------------------------------------------------
         *  <SLV-T>   22h     12h     [0]       IL1POST_OK         0:invalid 1:valid
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x12, &data, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        if ((data & 0x01) == 0x00) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        
        /* slave    Bank    Addr    Bit           Name                meaning
         * ---------------------------------------------------------------------------------
         * <SLV-T>   22h     13h     [0]       IPLP_SEL_ERR   0:found 1:not found
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x13, &data, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        t2_UnFreezeReg (pDemod);

        *pPLPError = data & 0x01;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_L1Change (sony_dvb_demod_t * pDemod, uint8_t * pL1Change)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_L1Change");

    if ((!pDemod) || (!pL1Change)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data;
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        sony_result_t result = SONY_RESULT_OK;

        /* Freeze the registers. */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Only valid after L1-post decoded at least once. */
        if (syncState < 5) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set bank 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name               mean
         * --------------------------------------------------------------------------------
         * <SLV-T>   22h     15h     [0]       IREG_L1_CHANGE_FLAG   0:Not changed, 1:Changed
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x15, &data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        *pL1Change = data & 0x01;
        if (*pL1Change) {
            /* Only clear if set, otherwise may miss indicator. */
            /* slave    Bank    Addr    Bit    default   Value          Name
             * ---------------------------------------------------------------------------------
             * <SLV-T>   22h     16h     [0]      1'b0       1'b1       OREGD_L1_CHANGE_FLAG_CLR
             */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x16, 0x01) != SONY_RESULT_OK) {
                t2_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        t2_UnFreezeReg (pDemod);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_L1Post (sony_dvb_demod_t * pDemod, sony_dvbt2_l1post_t * pL1Post)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_L1Post");

    if ((!pDemod) || (!pL1Post)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[15];
        uint8_t l1PostOK = 0;

        /* Freeze the register. */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name                mean
         * ---------------------------------------------------------------------------------
         * <SLV-T>   22h     12h     [0]            IL1POST_OK           0:invalid1:valid
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x12, &l1PostOK, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (!(l1PostOK & 0x01)) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Read L1-post data. */
        /* slave    Bank    Addr    Bit              Name           
         * ------------------------------------------------------------------------
         * <SLV-T>   22h     45h     [6:0]        IL1POST_SUB_SLICES_PER_FRAME[14:8]
         * <SLV-T>   22h     46h     [7:0]        IL1POST_SUB_SLICES_PER_FRAME[7:0]
         * <SLV-T>   22h     47h     [7:0]        IL1POST_NUM_PLP[7:0]
         * <SLV-T>   22h     48h     [3:0]        IL1POST_NUM_AUX[3:0]
         * <SLV-T>   22h     49h     [7:0]        IL1POST_AUX_CONFIG_RFU[7:0]
         * <SLV-T>   22h     4Ah     [2:0]        IL1POST_RF_IDX[2:0]
         * <SLV-T>   22h     4Bh     [7:0]        IL1POST_FREQUENCY[31:24]
         * <SLV-T>   22h     4Ch     [7:0]        IL1POST_FREQUENCY[23:16]
         * <SLV-T>   22h     4Dh     [7:0]        IL1POST_FREQUENCY[15:8]
         * <SLV-T>   22h     4Eh     [7:0]        IL1POST_FREQUENCY[7:0]
         * <SLV-T>   22h     4Fh     [3:0]        IL1POST_FEF_TYPE[3:0]
         * <SLV-T>   22h     50h     [5:0]        IL1POST_FEF_LENGTH[21:16]
         * <SLV-T>   22h     51h     [7:0]        IL1POST_FEF_LENGTH[15:8]
         * <SLV-T>   22h     52h     [7:0]        IL1POST_FEF_LENGTH[7:0]
         * <SLV-T>   22h     53h     [7:0]        IL1POST_FEF_INTERVAL[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x45, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Unfreeze registers. */
        t2_UnFreezeReg (pDemod);

        /* Convert data. */
        pL1Post->subSlicesPerFrame = (data[0] & 0x7F) << 8;
        pL1Post->subSlicesPerFrame |= data[1];
        pL1Post->numPLPs = data[2];
        pL1Post->numAux = data[3] & 0x0F;
        pL1Post->auxConfigRFU = data[4];
        pL1Post->rfIdx = data[5] & 0x07;
        pL1Post->freq = data[6] << 24;
        pL1Post->freq |= data[7] << 16;
        pL1Post->freq |= data[8] << 8;
        pL1Post->freq |= data[9];
        pL1Post->fefType = data[10] & 0x0F;
        pL1Post->fefLength = (data[11] & 0x3F) << 16;
        pL1Post->fefLength |= data[12] << 8;
        pL1Post->fefLength |= data[13];
        pL1Post->fefInterval = data[14];
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_SpectrumSense (sony_dvb_demod_t * pDemod, sony_dvb_demod_spectrum_sense_t * pSense)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t syncState   = 0;
    uint8_t tsLock      = 0;
    uint8_t earlyUnlock = 0;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_SpectrumSense");

    if ((!pDemod) || (!pSense)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze the register. */
    if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyUnlock);
    if (result != SONY_RESULT_OK) {
        t2_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Only valid after lock. */
    if (syncState != 6) {
        t2_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    {
        uint8_t data = 0;

        /* Set bank 0x28 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x28) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name                mean
         * ---------------------------------------------------------------------------------
         * <SLV-T>   28h     E6h     [0]        IREG_CRCG2_SINVP1      0:not invert,   1:invert
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xE6, &data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        t2_UnFreezeReg (pDemod);
        
        *pSense = (data & 0x01) ? DVB_DEMOD_SPECTRUM_INV : DVB_DEMOD_SPECTRUM_NORMAL;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_monitorT2_SNR (sony_dvb_demod_t * pDemod, int32_t * pSNR)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_SNR");

    if ((!pDemod) || (!pSNR)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint16_t reg;
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        uint8_t data[2];

        /* Freeze registers */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Confirm sequence state == 6. */
        if (syncState != 6) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /*
         * SNR = 10 * log10 (IREG_SNMON_OD / 4 / 2 ) ;
         * SNR = 10 * (log10 (IREG_SNMON_OD) - log10 (8))
         * 
         * log10(8) * 100 = 90.308
         */
        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name           
         * ------------------------------------------------------------
         * <SLV-T>   20h     28h     [7:0]        IREG_SNMON_OD[15:8]
         * <SLV-T>   20h     29h     [7:0]        IREG_SNMON_OD[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x28, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        t2_UnFreezeReg (pDemod);

        reg = ((data[0] & 0xFF) << 8) | data[1];
        if (reg == 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        if (reg >= 24000) {
            reg = 23999;
        }
        *pSNR = 10 * 10 * ((int32_t) sony_log10 (reg) - (int32_t) sony_log10 (24000 - reg));
        *pSNR += 34800;
    }
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT2_MER (sony_dvb_demod_t * pDemod, int32_t * pMER)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_MER");

    result = sony_dvb_demod_monitorT2_SNR (pDemod, pMER);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT2_PreLDPCBER (sony_dvb_demod_t * pDemod, uint32_t * pBER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t bitError = 0;
    uint32_t periodExp = 0;
    uint32_t n_ldpc = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_PreLDPCBER");

    if ((!pDemod) || (!pBER)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set bank 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
        t2_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        uint8_t data[4];
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x39, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (!(data[0] & 0x10)) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        bitError = ((data[0] & 0x0F) << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

        /* Read measurement period. */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x6F, data, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        periodExp = data[0] & 0x0F;

        /* Set Bank = 0x22. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Get the PLP type (Normal/Short). */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x5E, data, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        t2_UnFreezeReg (pDemod);

        if (((sony_dvbt2_plp_fec_t) (data[0] & 0x03)) == SONY_DVBT2_FEC_LDPC_16K) {
            /* Short. */
            n_ldpc = 16200;
        }
        else {
            /* Normal. */
            n_ldpc = 64800;
        }
    }

    if (bitError > ((1U << periodExp) * n_ldpc)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        if (periodExp >= 4) {
            /*
              BER = bitError * 10000000 / (2^N * n_ldpc)
                  = bitError * 3125 / (2^(N-4) * (n_ldpc / 200))
                  (NOTE: 10000000 / 2^4 / 200 = 3125)
                  = bitError * 5 * 625 / (2^(N-4) * (n_ldpc / 200))
                  (Divide in 2 steps to prevent overflow.)
            */
            div = (1U << (periodExp - 4)) * (n_ldpc / 200);

            Q = (bitError * 5) / div;
            R = (bitError * 5) % div;

            R *= 625;
            Q = Q * 625 + R / div;
            R = R % div;
        } else {
            /*
              BER = bitError * 10000000 / (2^N * n_ldpc)
                  = bitError * 50000 / (2^N * (n_ldpc / 200))
                  = bitError * 10 * 5000 / (2^N * (n_ldpc / 200))
                  (Divide in 2 steps to prevent overflow.)
            */
            div = (1U << periodExp) * (n_ldpc / 200);

            Q = (bitError * 10) / div;
            R = (bitError * 10) % div;

            R *= 5000;
            Q = Q * 5000 + R / div;
            R = R % div;
        }

        /* rounding */
        if (R >= div/2) {
            *pBER = Q + 1;
        } else {
            *pBER = Q;
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT2_PostBCHFER (sony_dvb_demod_t * pDemod, uint32_t * pFER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t fecError = 0;
    uint32_t period = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_PostBCHFER");

    if ((!pDemod) || (!pFER)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        uint8_t data[3];
        /* slave    Bank    Addr    Bit              Name           
         * ------------------------------------------------------------
         * <SLV-T>   20h     75h     [0]           IBBER_VALID
         * <SLV-T>   20h     76h     [6:0]         IBER2_FBERR[14:8]
         * <SLV-T>   20h     77h     [7:0]         IBER2_FBERR[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x75, data, 3) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (!(data[0] & 0x01)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        fecError = ((data[1] & 0x7F) << 8) | (data[2]);

        /* Read measurement period. */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x72, data, 1) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Period = 2^BBER_MES */
        period = (1 << (data[0] & 0x0F));
    }

    if ((period == 0) || (fecError > period)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /*
      FER = fecError * 1000000 / period
          = fecError * 1000 * 1000 / period
          (Divide in 2 steps to prevent overflow.)
    */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        div = period;

        Q = (fecError * 1000) / div;
        R = (fecError * 1000) % div;

        R *= 1000;
        Q = Q * 1000 + R / div;
        R = R % div;

        /* rounding */
        if ((div != 1) && (R >= div/2)) {
            *pFER = Q + 1;
        } else {
            *pFER = Q;
        }
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_dvb_demod_monitorT2_PreBCHBER (sony_dvb_demod_t * pDemod, uint32_t * pBER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t bitError = 0;
    uint32_t periodExp = 0;
    uint32_t n_bch = 0;

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_PreBCHBER");

    if ((!pDemod) || (!pBER)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        /* Not DVB-T2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Monitor the BER parameters. */
    {
        uint8_t data[4];
        sony_dvbt2_plp_fec_t plpFecType = SONY_DVBT2_FEC_LDPC_16K;
        sony_dvbt2_plp_code_rate_t plpCr = SONY_DVBT2_R1_2;
        static const uint16_t nBCHBitsLookup[SONY_DVBT2_FEC_RSVD1][SONY_DVBT2_R_RSVD1] = {
          /* R1_2   R3_5   R2_3   R3_4   R4_5   R5_6 */
            {7200,  9720,  10800, 11880, 12600, 13320}, /* Short */
            {32400, 38880, 43200, 48600, 51840, 54000}  /* Normal */
        };

        /* Freeze registers */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x24 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x24) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name           
         * ------------------------------------------------------------
         * <SLV-T>   24h     40h     [0]          IBBER_VALID
         * <SLV-T>   24h     41h     [5:0]        IBER0_BITERR[21:16]
         * <SLV-T>   24h     42h     [7:0]        IBER0_BITERR[15:8]
         * <SLV-T>   24h     43h     [7:0]        IBER0_BITERR[7:0]
         * <SLV-T>   22h     5Bh     [2:0]        IL1POST_PLP_COD[2:0]
         * <SLV-T>   22h     5Eh     [1:0]        IL1POST_PLP_FEC_TYPE[1:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x40, data, 4) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (!(data[0] & 0x01)) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE); /* Not Ready */
        }

        bitError = ((data[1] & 0x3F) << 16) | (data[2] << 8) | data[3];

        /* Set Bank = 0x22. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x22) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Get the PLP FEC type (Normal/Short). */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x5E, data, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        plpFecType = (sony_dvbt2_plp_fec_t) (data[0] & 0x03);

        /* Get the PLP code rate. */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x5B, data, 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        
        plpCr = (sony_dvbt2_plp_code_rate_t) (data[0] & 0x07);

        t2_UnFreezeReg (pDemod);

        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*
         * slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   20h     72h     [3:0]        OREG_BBER_MES[3:0]
         */
        
        /* Read measurement period. */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x72, data, 1) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Period (4bit) */
        periodExp = data[0] & 0x0F;

        /* Confirm FEC Type / Code Rate */
        if ((plpFecType > SONY_DVBT2_FEC_LDPC_64K) || (plpCr > SONY_DVBT2_R5_6)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        n_bch = nBCHBitsLookup[plpFecType][plpCr];
    }

    if (bitError > ((1U << periodExp) * n_bch)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        if (periodExp >= 6) {
            /*
              BER = bitError * 1000000000 / (2^N * n_bch)
                  = bitError * 390625 / (2^(N-6) * (n_bch / 40))
                  (NOTE: 1000000000 / 2^6 / 40 = 390625)
                  = bitError * 625 * 625 / (2^(N-6) * (n_bch / 40))
                  (Divide in 2 steps to prevent overflow.)
            */
            div = (1U << (periodExp - 6)) * (n_bch / 40);

            Q = (bitError * 625) / div;
            R = (bitError * 625) % div;

            R *= 625;
            Q = Q * 625 + R / div;
            R = R % div;
        } else {
            /*
              BER = bitError * 1000000000 / (2^N * n_bch)
                  = bitError * 25000000 / (2^N * (n_bch / 40))
                  = bitError * 1000 * 25000 / (2^N * (n_bch / 40))
                  (Divide in 2 steps to prevent overflow.)
            */
            div = (1U << periodExp) * (n_bch / 40);

            Q = (bitError * 1000) / div;
            R = (bitError * 1000) % div;

            R *= 25000;
            Q = Q * 25000 + R / div;
            R = R % div;
        }

        /* rounding */
        if (R >= div/2) {
            *pBER = Q + 1;
        } else {
            *pBER = Q;
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT2_PacketErrorNumber (sony_dvb_demod_t * pDemod, uint32_t * pPEN)
{
    sony_result_t result = SONY_RESULT_OK;
    
    uint8_t data[3];

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_PacketErrorNumber");

    if ((!pDemod) || (!pPEN)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	}

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0x24 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x24) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xFD, data, sizeof (data)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (!(data[0] & 0x01) ) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    *pPEN =  ((data[1] << 0x08) | data[2]);
	//*pPEN = data[3];

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_dvb_demod_monitorT2_SamplingOffset (sony_dvb_demod_t * pDemod, int32_t * pPPM)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_SamplingOffset");

    if ((!pDemod) || (!pPPM)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[5];
        uint32_t trlMon2 = 0;
        sony_result_t result = SONY_RESULT_OK;
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;

        /* Freeze registers */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (syncState != 6) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x52, data, sizeof (data)) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Unfreeze registers. */
        t2_UnFreezeReg (pDemod);

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

sony_result_t sony_dvb_demod_monitorT2_Quality (sony_dvb_demod_t * pDemod, uint8_t * pQuality)
{
    sony_result_t result = SONY_RESULT_OK;
    int32_t snr = 0;
    int32_t snrRel = 0;
    uint32_t ber = 0;
    uint32_t berSQI = 0;
    int32_t tmp = 0;
    sony_dvbt2_plp_t plpInfo;
    static const int32_t snrNordigP1dB1000[4][6] = {
        /*   1/2,   3/5,   2/3,   3/4,    4/5,    5/6 */
        {3400, 4700, 5500, 6500, 7100, 7600},      /* QPSK */
        {8300, 10000, 11200, 12400, 13300, 13900}, /* 16-QAM */
        {12200, 14400, 15900, 17700, 18800, 19500}, /* 64-QAM */
        {15500, 18500, 20500, 23100, 24800, 25800}, /* 256-QAM */
    };

    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_Quality");

    if ((!pDemod) || (!pQuality)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DVB_SYSTEM_DVBT2) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    /* Get Pre BCH BER. */
    result = sony_dvb_demod_monitorT2_PreBCHBER (pDemod, &ber);
    
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

     /* Get SNR */
    result = sony_dvb_demod_monitorT2_SNR (pDemod, &snr);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Get Active PLP information. */
    result = sony_dvb_demod_monitorT2_ActivePLP (pDemod, SONY_DVBT2_PLP_DATA, &plpInfo);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Ensure correct plp info. */
    if ((plpInfo.plpCr > SONY_DVBT2_R5_6) || (plpInfo.constell > SONY_DVBT2_QAM256)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    /*
     * SQI (Signal Quality Indicator) given by:
     * SQI = (((C/Nrel - 3) / 10) + 1) * BER_SQI 
     * BER_SQI = 20*log10(1/(BER*2e4) - 40
     * 
     * Re-arrange for ease of calculation:
     * BER_SQI = 20 * (log10(1) - log10(BER) - log10(2e4)) - 40
     * 
     * If BER in units of 1e-9
     * BER_SQI = 20 * (log10(1) - log10(BER/1e9) - log10(2e4)) - 40
     * BER_SQI = 20 * (log10(1) - (log10(BER) - log10(1e9)) - log10(2e4)) - 40
     * BER_SQI = 20 * (log10(1e9) - log10(2e4) - log10(BER)) - 40
     * BER_SQI = 20 * (9 - log10(2e4) - log10(BER)) - 40
     * BER_SQI = 140 - 20 * log10(2e4) - 20 * log10(BER)
     * BER_SQI = 140 - 20 * log10(2e4) - 20 * log10(BER)
     * BER_SQI = 53.979 - 20 * log10(BER)
     * BER_SQI * 1000 = 53979 - 200 * sony_log10(BER)
     *   NOTE: sony_log10(BER) returns log10(BER) * 100
     * 
     * BER = received ber
     */

    if (ber > 50) { /*  > 1E-3 / 2E4 = 5E-8 */
        berSQI = 0;
    } else if (ber == 0) { /* <= 1E-7 / 2E4 = 5E-12 */ 
        berSQI = 100*1000;
    } else {
        berSQI = 53979 - 200 * sony_log10(ber);
    }
  

    snrRel = snr - snrNordigP1dB1000[plpInfo.constell][plpInfo.plpCr];

    if ( snrRel < -7000 ) {
        *pQuality = 0;
        SONY_TRACE_RETURN (result);

    } 
    else if ( snrRel > 3000 ) {
        *pQuality = (uint8_t) ((berSQI + 500) / 1000);

    } 
    else {
        tmp = (((snrRel-3000) / 10) + 1000);
        *pQuality = (uint8_t) ((((uint32_t)tmp * berSQI) + (1000 * 1000 / 2)) / (1000 * 1000));
 
    }
 
    if (*pQuality > 100) {
        *pQuality = 100;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_monitorT2_TsRate (sony_dvb_demod_t * pDemod, uint32_t * pTsRateKbps)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t rd_smooth_dp = 0;
    uint32_t ep_ck_nume = 0;
    uint32_t ep_ck_deno = 0;
    uint8_t issy_on_data = 0;
    uint32_t tsRatex32 = 0;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_TsRate");

    if ((!pDemod) || (!pTsRateKbps)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        /* Not DVB-T2 or DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[8];
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        /* Freeze registers */
        if (t2_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_dvb_demod_monitorT2_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Check TS lock. */
        if (!tsLock) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set Bank 0x20. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xE5, &data[0], 4) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        rd_smooth_dp = (uint32_t) ((data[0] & 0x0F) << 24);
        rd_smooth_dp |= (uint32_t) (data[1] << 16);
        rd_smooth_dp |= (uint32_t) (data[2] << 8);
        rd_smooth_dp |= (uint32_t) data[3];
        /* Set Bank 0x24. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x24) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x90, &data[0], 1) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        issy_on_data = data[0] & 0x01;
        /* Unfreeze registers */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x25) != SONY_RESULT_OK) {
        t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xB2, &data[0], 8) != SONY_RESULT_OK) {
            t2_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        ep_ck_nume = (uint32_t) ((data[0] & 0x07) << 24);
        ep_ck_nume |= (uint32_t) (data[1] << 16);
        ep_ck_nume |= (uint32_t) (data[2] << 8);
        ep_ck_nume |= (uint32_t) data[3];

        ep_ck_deno = (uint32_t) ((data[4] & 0x07) << 24);
        ep_ck_deno |= (uint32_t) (data[5] << 16);
        ep_ck_deno |= (uint32_t) (data[6] << 8);
        ep_ck_deno |= (uint32_t) data[7];

        t2_UnFreezeReg (pDemod);
    }
    if (rd_smooth_dp == 0) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }
    if (issy_on_data) {
        if ((ep_ck_deno == 0) || (ep_ck_nume == 0) || (ep_ck_deno >= ep_ck_nume)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
    }
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;
        div = rd_smooth_dp;
        Q = 82U * 262144U * 100U / div;
        R = 82U * 262144U * 100U % div;
        R *= 320U;
        Q = Q * 320 + R / div;
        R = R % div;
        if (R >= div/2) {
            tsRatex32 = Q + 1;
        } else {
            tsRatex32 = Q;
        }
    }
    if (issy_on_data) {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;
        uint32_t tsRateComp = 0;
        div = ep_ck_nume - ep_ck_deno;
        Q = ep_ck_nume / div;
        R = ep_ck_nume % div;
        tsRateComp = tsRatex32 / ((Q * 32) + (((R * 32) + div/2) / div));
        *pTsRateKbps = (tsRatex32 + 16) / 32 - tsRateComp;
    }  else {
        *pTsRateKbps = (tsRatex32 + 16) / 32;
    }
    SONY_TRACE_RETURN (result);
}
sony_result_t sony_dvb_demod_monitorT2_PER (sony_dvb_demod_t * pDemod, uint32_t * pPER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t packetError = 0;
    uint32_t period = 0;
    SONY_TRACE_ENTER ("sony_dvb_demod_monitorT2_PER");
    if (!pDemod || !pPER) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if ((pDemod->system != SONY_DVB_SYSTEM_DVBT2) && (pDemod->system != SONY_DVB_SYSTEM_DVBC2)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    {
        uint8_t rdata[3];
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x24) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xFA, rdata, 3) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if ((rdata[0] & 0x01) == 0)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */
        packetError = (rdata[1] << 8) | rdata[2];
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0xDC, rdata, 1) != SONY_RESULT_OK)
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
