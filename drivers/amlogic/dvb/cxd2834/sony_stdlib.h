/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 3419 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**

 @file    sony_stdlib.h

          This file provides the C standard lib function mapping.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_STDLIB_H
#define SONY_STDLIB_H

#include "sony_dvb.h"

/*
 PORTING. Please modify if ANCI C standard library is not available.
*/
#include <string.h>

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/

/**
 @brief Alias for memcpy.
*/
#define sony_memcpy  memcpy

/**
 @brief Alias for memset.
*/
#define sony_memset  memset

/**
 @brief Alias for abs.
*/
#define sony_abs  abs

#endif /* SONY_STDLIB_H */
