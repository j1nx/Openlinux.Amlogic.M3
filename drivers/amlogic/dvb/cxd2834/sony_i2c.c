/*------------------------------------------------------------------------------

 Copyright 2009-2010 Sony Corporation

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sony Corporation.
 No part of this file may be copied, modified, sold, and distributed in any
 form or by any means without prior explicit permission in writing from
 Sony Corporation.

 Date: 2010/06/18
 Revision: 1.0.1.0

------------------------------------------------------------------------------*/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "sony_i2c.h"
#if 0
#define pr_dbg(fmt, args...) printk(KERN_ERR "cxd2834: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif

#define pr_error(fmt, args...) printk(KERN_ERR "cxd2834: " fmt, ## args)


#define BURST_WRITE_MAX 128 /* Max length of burst write */

sony_result_t sony_i2c_CommonReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
    void *state;
    SONY_TRACE_I2C_ENTER("sony_i2c_CommonReadRegister");

    if(!pI2c){
    		printk("sony_i2c_CommonReadRegister\n");
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = pI2c->Write(state,pI2c, deviceAddress, &subAddress, 1, SONY_I2C_START_EN);
    if(result == SONY_RESULT_OK){
        result = pI2c->Read(state,pI2c, deviceAddress, pData, size, SONY_I2C_START_EN | SONY_I2C_STOP_EN);
    }

    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t sony_i2c_CommonWriteRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t buffer[BURST_WRITE_MAX + 1];
    void *state;
    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(size > BURST_WRITE_MAX){
        /* Buffer is too small... */
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    buffer[0] = subAddress;
    memcpy(&(buffer[1]), pData, size);
    //pr_dbg("+++++pI2c->Write device address 0x%x\n,size",deviceAddress,size);

    /* send the new buffer */
    result = pI2c->Write(state,pI2c, deviceAddress, buffer, size+1, SONY_I2C_START_EN | SONY_I2C_STOP_EN);
    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t sony_i2c_CommonWriteOneRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data)
{
    sony_result_t result = SONY_RESULT_OK;
    //pr_dbg("+++++sony_i2c_CommonWriteOneRegister\n");
    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteOneRegister");

    if(!pI2c){
        pr_dbg("+++++pI2c NULL\n");
        
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    //pr_dbg("+++++WriteRegister\n");
    result = pI2c->WriteRegister(pI2c, deviceAddress, subAddress, &data, 1);
    SONY_TRACE_I2C_RETURN(result);
}

/* For Read-Modify-Write */
sony_result_t sony_i2c_SetRegisterBits(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data, uint8_t mask)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_SetRegisterBits");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(mask == 0x00){
        /* Nothing to do */
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    }
    
    if(mask != 0xFF){
        uint8_t rdata = 0x00;
        result = pI2c->ReadRegister(pI2c, deviceAddress, subAddress, &rdata, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_I2C_RETURN(result); }
        data = (uint8_t)((data & mask) | (rdata & (mask ^ 0xFF)));
    }

    result = pI2c->WriteOneRegister(pI2c, deviceAddress, subAddress, data);
    SONY_TRACE_I2C_RETURN(result);
}

