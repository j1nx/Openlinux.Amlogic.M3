/*------------------------------------------------------------------------------

<dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**

 @file    sony_atv.h

          This file provides Analog TV type definitions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_ATV_H
#define SONY_ATV_H

#include "sony_dvb.h"
#include "sony_i2c.h"

/**
 @brief Digital TV system definitions.
*/
typedef enum {
    SONY_ATV_VIDEO_UNKNOWN  = 0x00,
    SONY_ATV_VIDEO_M        = 0x01,       /**< System-M */
    SONY_ATV_VIDEO_M_KOREA  = 0x02,       /**< System-M for Korea Region*/
    SONY_ATV_VIDEO_BG       = 0x03,       /**< System-BG */
    SONY_ATV_VIDEO_IDK      = 0x04,       /**< System-DK */
    SONY_ATV_VIDEO_L        = 0x05,       /**< System-L */
    SONY_ATV_VIDEO_L_DASH   = 0x06,       /**< System-L DASH */
} sony_atv_video_t;

/**
 @brief Audio system definitions.
*/
typedef enum {
    SONY_ATV_AUDIO_UNKNOWN            = 0x00,
    SONY_ATV_AUDIO_BTSC_STEREO        = 0x01,   /**< BTSC STEREO */
    SONY_ATV_AUDIO_M_DUAL_FM_STEREO   = 0x02,   /**< M DUAL FM STEREO */
    SONY_ATV_AUDIO_M_EIA_J_STEREO     = 0x03,   /**< M EIA J STEREO */
    SONY_ATV_AUDIO_N_BTSC             = 0x04,   /**< N BTSC */
    SONY_ATV_AUDIO_BG_DUAL_FM_STEREO  = 0x05,   /**< BG DUAL FM STEREO */
    SONY_ATV_AUDIO_BG_NICAM_FM        = 0x06,   /**< BG NICAM FM */
    SONY_ATV_AUDIO_DK1_DUAL_FM_STEREO = 0x08,   /**< DK1 DUAL FM STEREO */
    SONY_ATV_AUDIO_DK2_DUAL_FM_STEREO = 0x09,   /**< DK2 DUAL FM STEREO */
    SONY_ATV_AUDIO_DK3_DUAL_FM_STEREO = 0x0A,   /**< DK3 DUAL FM STEREO */
    SONY_ATV_AUDIO_DK_NICAM_FM        = 0x0B,   /**< DK NICAM FM */
    SONY_ATV_AUDIO_I_NICAM_FM         = 0x0C,   /**< I NICAM FM */
    SONY_ATV_AUDIO_L_NICAM_AM         = 0x0D    /**< L NICAM AM */
} sony_atv_audio_t;

#endif /* SONY_ATV_H */
