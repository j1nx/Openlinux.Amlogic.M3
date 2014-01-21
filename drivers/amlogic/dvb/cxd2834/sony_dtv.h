/*------------------------------------------------------------------------------

<dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**

 @file    sony_dtv.h

          This file provides Digital TV type definitions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DTV_H
#define SONY_DTV_H

/**
 @brief Digital TV system definitions.
*/
typedef enum {
    SONY_DTV_SYSTEM_UNKNOWN,
    SONY_DTV_SYSTEM_QAM_8VSB,       /**< ATSC/QAM */
    SONY_DTV_SYSTEM_ISDBT,          /**< ISDB-T */
    SONY_DTV_SYSTEM_DVBT_5,         /**< DVB-T 5MHz BW */
    SONY_DTV_SYSTEM_DVBT_6,         /**< DVB-T 6MHz BW */
    SONY_DTV_SYSTEM_DVBT_7,         /**< DVB-T 7MHz BW */
    SONY_DTV_SYSTEM_DVBT_8,         /**< DVB-T 8MHz BW */
    SONY_DTV_SYSTEM_DVBT2_5,        /**< DVB-T2 5MHz BW */
    SONY_DTV_SYSTEM_DVBT2_6,        /**< DVB-T2 6MHz BW */
    SONY_DTV_SYSTEM_DVBT2_7,        /**< DVB-T2 7MHz BW */
    SONY_DTV_SYSTEM_DVBT2_8,        /**< DVB-T2 8MHz BW */
    SONY_DTV_SYSTEM_DVBC,           /**< DVB-C */
    SONY_DTV_SYSTEM_DVBC2_6,        /**< DVB-C2 6MHz BW */
    SONY_DTV_SYSTEM_DVBC2_8,        /**< DVB-C2 8MHz BW */
    SONY_DTV_SYSTEM_DTMB            /**< DTMB */
} sony_dtv_system_t;

#endif /* SONY_DTV_H */
