/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**

 @file    sony_ascot2d.h

          This file provides the Sony ASCOT2D DVB & Analog tuner driver.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_ASCOT2D_H_
#define SONY_ASCOT2D_H_

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "sony_dvb.h"
#include "sony_i2c.h"
#include "sony_atv.h"
#include "sony_dtv.h"
/*------------------------------------------------------------------------------
 Globals
------------------------------------------------------------------------------*/

/**
 @brief Ascot2D driver version.
*/
extern const char *sony_ascot2d_version;

/**
 @brief Ascot2D driver build date.
*/
extern const char *sony_ascot2d_build_date;

/**
 @brief Ascot2D driver build time.
*/
extern const char *sony_ascot2d_build_time;

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
/**
 @brief ASCOT2D RF step size is <1kHz, so this sets the cutoff frequency 
        in the detected carrier offset to retune.
*/
#define SONY_ASCOT2D_OFFSET_CUTOFF_HZ       50000

/**
 @brief Set if using external XTAL (ref input). e.g If sharing 41MHz XTAL with demodulator.
        Apply as mask to ::sony_ascot2d_t::flags.
*/
#define SONY_ASCOT2D_CONFIG_EXT_REF         0x80000000

/**
 @brief IFOUT 1/AGCIN 1 is used for both digital and analog 
        Apply as mask to ::sony_ascot2d_t::flags.
*/
#define SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL1   0x00000000

/**
 @brief IFOUT 2/AGCIN 2 is used for both digital and analog
        Apply as mask to ::sony_ascot2d_t::flags.
*/
#define SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL2   0x08000000

/**
 @brief IFOUT 1/AGCIN 1 is for Analog, IFOUT 2/AGCIN 2 is for Digital
        Apply as mask to ::sony_ascot2d_t::flags.
*/
#define SONY_ASCOT2D_CONFIG_IFAGCSEL_A1D2   0x04000000

/**
 @brief IFOUT/AGCIN 1 is for Digital, IFOUT 2/AGCIN 2 is for Analog.
        Apply as mask to ::sony_ascot2d_t::flags.
*/
#define SONY_ASCOT2D_CONFIG_IFAGCSEL_D1A2   0x02000000

/**
 @brief IFOUT and AGCIN configuration settings mask.
*/
#define SONY_ASCOT2D_CONFIG_IFAGCSEL_MASK   0x0E000000

/**
 @brief ASCOT2D NVM selection. Bank Auto Select         
*/
#define SONY_ASCOT2D_CONFIG_NVMSEL_AUTO     0x00100000

/**
 @brief ASCOT2D NVM selection. NVM Bank 0/1 (default)        
*/
#define SONY_ASCOT2D_CONFIG_NVMSEL_0_1      0x00000000

/**
 @brief ASCOT2D NVM selection. NVM Bank 0/3       
*/
#define SONY_ASCOT2D_CONFIG_NVMSEL_0_3      0x00200000

/**
 @brief ASCOT2D NVM selection. NVM Bank mask         
*/
#define SONY_ASCOT2D_CONFIG_NVMSEL_MASK     0x00300000

/**
 @brief Tuner I2C address (8bit).
*/
#define SONY_ASCOT2D_ADDRESS                0xC0

/**
 @brief Structure to define the Sony Ascot2D tuner driver 
        object.
*/
typedef struct sony_ascot2d_t {
    uint32_t frequency;                                 /**< Current RF frequency(kHz) tuned. */
    sony_dtv_system_t dtvSystem;                        /**< Current digital system tuned. */
    sony_atv_video_t atvVideoId;                        /**< Current analog TV video ID. */
    sony_atv_audio_t atvAudioId;                        /**< Current analog TV audio ID. */
    uint32_t xtalFreq;                                  /**< Tuner Xtal frequency (MHz). */
    sony_i2c_t *pI2c;                                   /**< I2C driver instance. */
    uint8_t i2cAddress;                                 /**< I2C address. */
    uint32_t flags;                                     /**< Flags to control tuner configuration and behaviour. */
    void *user;                                         /**< User defined data. */

} sony_ascot2d_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates an instance of the ASCOT2D tuner driver. 
        
        See configFlags parameter for details of configurable 
        items in the tuner driver.
 
 @param i2cAddress The I2C address of the ASCOT2D device.
        Typically 0xC0.
 @param xtalFreq The crystal frequency of the tuner (MHz). 
        Supports 4MHz, 16MHz or 41MHz.
 @param pI2c The I2C driver that the tuner driver will use for 
        communication.
 @param configFlags See ::SONY_ASCOT2D_CONFIG_EXT_REF,
                        ::SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL1,
                        ::SONY_ASCOT2D_CONFIG_IFAGCSEL_ALL2,
                        ::SONY_ASCOT2D_CONFIG_IFAGCSEL_A1D2,
                        ::SONY_ASCOT2D_CONFIG_IFAGCSEL_D1A2,
                        ::SONY_ASCOT2D_CONFIG_NVMSEL_AUTO,
                        ::SONY_ASCOT2D_CONFIG_NVMSEL_0_1,
                        ::SONY_ASCOT2D_CONFIG_NVMSEL_0_3
 @param user User data.
 @param pTuner The tuner driver instance to create. Memory 
        must have been allocated for this instance before 
        creation.
 
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_ascot2d_Create (uint8_t i2cAddress,
                                       uint32_t xtalFreq,
                                       sony_i2c_t * pI2c, uint32_t configFlags, void *user, sony_ascot2d_t * pTuner);

/**
 @brief Initialize the tuner into a low power state.
        Uses the X_pon sequence.

 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_ascot2d_Initialize (sony_ascot2d_t * pTuner);

/**
 @brief Finalize the driver and places the tuner in a low power state.
        Uses the X_fin sequence.

 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_ascot2d_Finalize (sony_ascot2d_t * pTuner);

/**
 @brief Tune the ASCOT2D tuner to the specified digital system
        and frequency.
        
        Uses a combination of sequences:
        - A_init
        - D_init
        - X_tune

 @param pTuner Instance of the tuner driver.
 @param frequency RF frequency to tune too (kHz).
 @param dtvSystem The digital system to tune too.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_ascot2d_DtvTune (sony_ascot2d_t * pTuner, uint32_t frequency, sony_dtv_system_t dtvSystem);

/**
 @brief Tune the ASCOT2D tuner to the specified analog system
        and frequency.
        
        Uses a combination of sequences:
        - A_init
        - A_tune

 @param pTuner Instance of the tuner driver.
 @param frequency RF frequency to tune too (kHz).
 @param videoID The video system to tune too.
 @param audioID The audio system to tune too.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_ascot2d_AtvTune (sony_ascot2d_t * pTuner,
                                        uint32_t frequency, sony_atv_video_t videoID, sony_atv_audio_t audioID);

/**
 @brief Sleep the ASCOT2D tuner. 
        Places the tuner in a low power state.
        Uses the X_fin sequence.

 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_ascot2d_Sleep (sony_ascot2d_t * pTuner);

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
sony_result_t sony_ascot2d_RFFilterConfig (sony_ascot2d_t * pTuner, uint8_t coeff, uint8_t offset);

/**
 @brief Write to GPIO0 or GPIO1, pins 33 and 34 respectively.

 @param pTuner  Instance of the tuner driver.
 @param id      Pin ID 0 = GPIO0, 1 = GPIO1
 @param value   Output logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_ascot2d_Write_GPIO (sony_ascot2d_t * pTuner, uint8_t id, uint8_t value);

#endif /* SONY_ASCOT2D_H_ */
