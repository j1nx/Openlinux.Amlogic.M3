/*****************************************************************
**
**  Copyright (C) 2009 Amlogic,Inc.
**  All rights reserved
**        Filename : avlfrontend.c
**
**  comment:
**        Driver for MN88436 demodulator
**  author :
**	    Shijie.Rong@amlogic
**  version :
**	    v1.0	 12/3/30
*****************************************************************/

/*
    Driver for MN88436 demodulator
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#ifdef ARC_700
#include <asm/arch/am_regs.h>
#else
#include <mach/am_regs.h>
#endif
#include <linux/i2c.h>
#include <linux/gpio.h>
#include "mnfrontend.h"
#include <linux/kthread.h>
#include "../aml_fe.h"



#if 1
#define pr_dbg	printk
//#define pr_dbg(fmt, args...) printk( KERN_DEBUG"DVB: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif

#define pr_error(fmt, args...) printk( KERN_ERR"DVB: " fmt, ## args)

static DMD_PARAMETER param;


static int MN88436_Reset(struct aml_fe_dev *dev)
{
//reset demod
	gpio_direction_output(dev->reset_gpio, dev->reset_value);
	msleep(200);
	gpio_direction_output(dev->reset_gpio, !dev->reset_value);
//enable tuner
	gpio_direction_output(dev->tuner_power_gpio, 1);
	return 0;
}


static int MN88436_Sleep(struct dvb_frontend *fe)
{
	struct mn88436_state *state = fe->demodulator_priv;
	return 0;
}

static int MN88436_Read_Status(struct dvb_frontend *fe, fe_status_t * status)
{
	unsigned char s=0;
	DMD_get_info(&param,DMD_E_INFO_LOCK);
	if(param.info[DMD_E_INFO_LOCK]==0)
		s=1;

	if(s==1)
	{
		*status = FE_HAS_LOCK|FE_HAS_SIGNAL|FE_HAS_CARRIER|FE_HAS_VITERBI|FE_HAS_SYNC;
	}
	else
	{
		*status = FE_TIMEDOUT;
	}
	
	return  0;
}

static int MN88436_Read_Ber(struct dvb_frontend *fe, u32 * ber)
{
	DMD_get_info(&param,DMD_E_INFO_PERRNUM);
	*ber=param.info[DMD_E_INFO_PERRNUM] ;
	return 0;
}

static int MN88436_Read_Signal_Strength(struct dvb_frontend *fe, u16 *strength)
{
	*strength=DMD_get_info(&param,DMD_E_INFO_ALL);
	return 0;
}

static int MN88436_Read_Snr(struct dvb_frontend *fe, u16 * snr)
{
	DMD_get_info(&param,DMD_E_INFO_CNR_DEC);
	*snr=param.info[DMD_E_INFO_CNR_INT] ;
	return 0;
}

static int MN88436_Read_Ucblocks(struct dvb_frontend *fe, u32 * ucblocks)
{
	ucblocks=NULL;
	return 0;
}

static int MN88436_Set_Frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	
	pr_dbg("MN88436_set_frontend\n");
	int count;
	struct mn88436_state *state = fe->demodulator_priv;
//	DMD_PARAMETER param;
	param.freq=p->frequency;
	param.funit=DMD_E_KHZ;
	MDrv_Tuner_SetTuner(param.freq,6);
	DMD_scan(&param);
	//DMD_tune(&param);
	state->freq=p->frequency;
	state->mode=p->u.vsb.modulation ; //these data will be writed to eeprom
	DMD_get_info(&param,DMD_E_INFO_CNR_DEC);
	DMD_get_info(&param,DMD_E_INFO_PERRNUM);
	for(count=0;count<20;count++){
		DMD_get_info(&param,DMD_E_INFO_LOCK);
		if(param.info[DMD_E_INFO_LOCK]==0)
			break;
		msleep(50);

	}
	pr_dbg("mn88436=>frequency=%d\r\n",p->frequency);
	return  0;
}

static int MN88436_Get_Frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{//these content will be writed into eeprom .

	struct mn88436_state *state = fe->demodulator_priv;
	
	p->frequency=state->freq;
	p->u.qam.modulation=state->mode;
	
	return 0;
}

static ssize_t mn_frontend_show_short_circuit(struct class* class, struct class_attribute* attr, char* buf)
{
	
	return ;
}

struct i2c_adapter *i2c_handle;
int mn88436_get_fe_config(struct mn88436_fe_config *cfg)
{
//	printk("\n frontend_i2c is %d,,frontend_demod_addr is %x,frontend_tuner_addr is %x,frontend_reset is %d",frontend_i2c,frontend_demod_addr,frontend_tuner_addr,frontend_reset);
	cfg->i2c_adapter =i2c_handle;
	return 1;
	
//	printk("\n frontend0_i2c is %d, frontend_i2c is %d,",frontend0_i2c,frontend_i2c);
	
}

void demodconfig(struct aml_fe_dev *dev)
{
	
	i2c_handle=dev->i2c_adap;
}


static int mn88436_fe_get_ops(struct aml_fe_dev *dev, int mode, void *ops)
{
	struct dvb_frontend_ops *fe_ops = (struct dvb_frontend_ops*)ops;

	fe_ops->info.frequency_min = 51000000;
	fe_ops->info.frequency_max = 858000000;
	fe_ops->info.frequency_stepsize = 0;
	fe_ops->info.frequency_tolerance = 0;
	fe_ops->info.caps = FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
			FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
			FE_CAN_QPSK | FE_CAN_QAM_16 |
			FE_CAN_QAM_64 | FE_CAN_QAM_AUTO |
			FE_CAN_TRANSMISSION_MODE_AUTO |
			FE_CAN_GUARD_INTERVAL_AUTO |
			FE_CAN_HIERARCHY_AUTO |
			FE_CAN_RECOVER |
			FE_CAN_MUTE_TS;

	fe_ops->set_frontend = MN88436_Set_Frontend,
	fe_ops->get_frontend = MN88436_Get_Frontend,	
	fe_ops->read_status = MN88436_Read_Status,
	fe_ops->read_ber = MN88436_Read_Ber,
	fe_ops->read_signal_strength =MN88436_Read_Signal_Strength,
	fe_ops->read_snr = MN88436_Read_Snr,
	fe_ops->read_ucblocks = MN88436_Read_Ucblocks,
	demodconfig(dev);
	return 0;
}

static int mn88436_fe_enter_mode(struct aml_fe *fe, int mode)
{
	//reset
	struct aml_fe_dev *dev = fe->dtv_demod;
	MN88436_Reset(dev);
	msleep(100);
	//Select device ID
	param.devid = 0;
	param.system=DMD_E_ATSC;
	param.ts_out=DMD_E_TSOUT_PARALLEL_FIXED_CLOCK;
//	DMD_PARAMETER param;
	
	//Initialize LSI
	printf("Initializing LSI .. \n");
	if( DMD_init(&param) == DMD_E_OK )
		pr_dbg("DEMOD OK\n");
	else
		pr_dbg("DEMOD NG\n");
	DMD_set_system(&param);
	if(MDrv_Tuner_Init()==1)
		pr_dbg("TUNER OK\n");
	else
		pr_dbg("TUNER NG\n");

	return 0;
}



static int mn88436_fe_resume(struct aml_fe_dev *dev)
{
	//reset
		MN88436_Reset(dev);
		msleep(100);
		//Select device ID
		param.devid = 0;
		param.system=DMD_E_ATSC;
		param.ts_out=DMD_E_TSOUT_PARALLEL_FIXED_CLOCK;
	//	DMD_PARAMETER param;
		
		//Initialize LSI
		printf("Initializing LSI .. \n");
		if( DMD_init(&param) == DMD_E_OK )
			pr_dbg("DEMOD OK\n");
		else
			pr_dbg("DEMOD NG\n");
		DMD_set_system(&param);
		if(MDrv_Tuner_Init()==1)
			pr_dbg("TUNER OK\n");
		else
			pr_dbg("TUNER NG\n");
	
		return 0;

}


static int mn88436_fe_suspend(struct aml_fe_dev *dev)
{
	return 0;
}

static struct aml_fe_drv mn88436_dtv_demod_drv = {
.id         = AM_DTV_DEMOD_MN88435,
.name       = "Mn88436",
.capability = AM_FE_ATSC,
.get_ops    = mn88436_fe_get_ops,
.enter_mode = mn88436_fe_enter_mode,
.suspend    = mn88436_fe_suspend,
.resume     = mn88436_fe_resume
};


static int __init mnfrontend_init(void)
{
	pr_dbg("register mn88436 demod driver\n");
	return aml_register_fe_drv(AM_DEV_DTV_DEMOD, &mn88436_dtv_demod_drv);
}


static void __exit mnfrontend_exit(void)
{
	pr_dbg("unregister mn88436 demod driver\n");
	aml_unregister_fe_drv(AM_DEV_DTV_DEMOD, &mn88436_dtv_demod_drv);
}


fs_initcall(mnfrontend_init);
module_exit(mnfrontend_exit);


MODULE_DESCRIPTION("mn88436 atsc Demodulator driver");
MODULE_AUTHOR("RSJ");
MODULE_LICENSE("GPL");


