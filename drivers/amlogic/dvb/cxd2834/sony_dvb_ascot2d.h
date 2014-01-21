/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**

 @file    sony_dvb_ascot2d.h

          This file provides the DVB port of the Sony ASCOT2D tuner driver.

          This driver wraps around the Ascot2D driver provided by 
          sony_ascot2d.h by using an instance of the Ascot2D (::sony_ascot2d_t) 
          driver in the ::sony_dvb_tuner_t::user pointer.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DVB_ASCOT2D_H_
#define SONY_DVB_ASCOT2D_H_

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "sony_dvb_tuner.h"
#include "sony_ascot2d.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates an instance of the ASCOT2D tuner driver
 
 @param i2cAddress The I2C address of the ASCOT2D device.
        Typically 0xC0.
 @param xtalFreq The crystal frequency of the tuner (MHz). 
        Supports 4MHz or 41MHz.
 @param pI2c The I2C driver that the tuner driver will use for 
        communication.
 @param configFlags See ::SONY_ASCOT2D_CONFIG_EXT_REF,
            ::SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL1,
            ::SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL2,
            ::SONY_ASCOT2D_CONFIG_IFAGCSEL_A1D2,
            ::SONY_ASCOT2D_CONFIG_IFAGCSEL_D1A2,
            ::SONY_ASCOT2D_CONFIG_NVMSEL_AUTO,
            ::SONY_ASCOT2D_CONFIG_NVMSEL_0_1,
            ::SONY_ASCOT2D_CONFIG_NVMSEL_0_3 defined in 
            \link sony_ascot2d.h \endlink
 @param pAscot2DTuner The Ascot2D tuner driver pointer to use. 
        Memory must have been allocated for the Ascot2D driver structure. 
 @param pTuner The tuner driver instance to create. Memory 
        must have been allocated for this instance before 
        creation.
 
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_dvb_ascot2d_Create (uint8_t i2cAddress,
                                           uint32_t xtalFreq,
                                           sony_i2c_t * pI2c,
                                           uint32_t configFlags, sony_ascot2d_t * pAscot2DTuner, sony_dvb_tuner_t * pTuner);

/**
 @brief Initialize the tuner into "Power Save" state.
        Uses the X_pon sequence.

 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_dvb_ascot2d_Initialize (sony_dvb_tuner_t * pTuner);

/**
 @brief Finalize the driver and places into a power state ("Power Save"). 
        Uses the X_fin sequence.

 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_dvb_ascot2d_Finalize (sony_dvb_tuner_t * pTuner);

/**
 @brief Tune the ASCOT2D tuner to the specified system, frequency 
        and bandwidth.
        Uses a combination of sequences:
        - A_init
        - D_init
        - X_tune

 @param pTuner Instance of the tuner driver.
 @param frequency RF frequency to tune too (kHz).
 @param system The type of channel to tune too (DVB-T/C/T2).
 @param bandWidth The bandwidth of the channel (MHz).
        - DVB-C: 8MHz only.
        - DVB-T: 6, 7 or 8.
        - DVB-T2: 5, 6, 7 or 8.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_dvb_ascot2d_Tune (sony_dvb_tuner_t * pTuner,
                                         uint32_t frequency, sony_dvb_system_t system, uint8_t bandWidth);

/**
 @brief Power down the ASCOT2D tuner. 
        Places the tuner in a low power state ("Power Save").
        Uses the X_fin sequence.

 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_dvb_ascot2d_Sleep (sony_dvb_tuner_t * pTuner);

/**
 @brief Monitors the RF signal level in dBx1000 at the 
        input of the tuner.

 @param pTuner Instance of the tuner driver.
 @param ifAgc IFAGC level as read from the demodulator.
 @param rfAinValid Indicates whether or not the RFAIN value 
        is valid and should be used for the RF level calculation.
 @param rfAin The RFAIN level (10-bit) read from the demodulator.
 @param pRFLeveldB The RF level at the tuner input in dBx1000.

 @return SONY_RESULT_OK if pRFLeveldB is valid.
*/
sony_result_t sony_dvb_ascot2d_monitor_RFLevel (sony_dvb_tuner_t * pTuner,
                                                    uint32_t ifAgc, uint8_t rfAinValid, uint32_t rfAin, int32_t * pRFLeveldB);

/**
 @brief RF filter compensation setting for VHF-L band.
 (Please see RFVGA Description of datasheet.)
 New setting will become effective after next tuning.

 mult = VL_TRCKOUT_COEFF(8bit unsigned) / 128
 ofs  = VL_TRCKOUT_OFS(8bit 2s complement)
 (compensated value) = (original value) * mult + ofs

 @param pTuner Instance of the tuner driver.
 @param coeff  VL_TRCKOUT_COEFF (multiplier)
 @param offset VL_TRCKOUT_OFS (additional term)

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_dvb_ascot2d_RFFilterConfig (sony_dvb_tuner_t * pTuner, uint8_t coeff, uint8_t offset);

/**
 @brief Write to GPIO0 or GPIO1, pins 33 and 34 respectively.

 @param pTuner  Instance of the tuner driver.
 @param id      Pin ID 0 = GPIO0, 1 = GPIO1
 @param value   Output logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_dvb_ascot2d_Write_GPIO (sony_dvb_tuner_t * pTuner, uint8_t id, uint8_t value);
#endif /* SONY_DVB_ASCOT2D_H_ */
