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



#if 1
#define pr_dbg	printk
//#define pr_dbg(fmt, args...) printk( KERN_DEBUG"DVB: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif

#define pr_error(fmt, args...) printk( KERN_ERR"DVB: " fmt, ## args)

MODULE_PARM_DESC(frontend0_reset, "\n\t\t Reset GPIO of frontend0");
static int frontend0_reset = -1;
module_param(frontend0_reset, int, S_IRUGO);

MODULE_PARM_DESC(frontend0_i2c, "\n\t\t IIc adapter id of frontend0");
static int frontend0_i2c = -1;
module_param(frontend0_i2c, int, S_IRUGO);

MODULE_PARM_DESC(frontend0_tuner_addr, "\n\t\t Tuner IIC address of frontend0");
static int frontend0_tuner_addr = -1;
module_param(frontend0_tuner_addr, int, S_IRUGO);

MODULE_PARM_DESC(frontend0_demod_addr, "\n\t\t Demod IIC address of frontend0");
static int frontend0_demod_addr = -1;
module_param(frontend0_demod_addr, int, S_IRUGO);

static struct aml_fe mn88436_fe[FE_DEV_COUNT];

MODULE_PARM_DESC(frontend_reset, "\n\t\t Reset GPIO of frontend");
static int frontend_reset = -1;
module_param(frontend_reset, int, S_IRUGO);

MODULE_PARM_DESC(frontend_i2c, "\n\t\t IIc adapter id of frontend");
static int frontend_i2c = -1;
module_param(frontend_i2c, int, S_IRUGO);

MODULE_PARM_DESC(frontend_tuner_addr, "\n\t\t Tuner IIC address of frontend");
static int frontend_tuner_addr = -1;
module_param(frontend_tuner_addr, int, S_IRUGO);

MODULE_PARM_DESC(frontend_demod_addr, "\n\t\t Demod IIC address of frontend");
static int frontend_demod_addr = -1;
module_param(frontend_demod_addr, int, S_IRUGO);

MODULE_PARM_DESC(frontend_power, "\n\t\t ANT_PWR_CTRL of frontend");
static int frontend_power = -1;
module_param(frontend_power, int, S_IRUGO);



static DMD_PARAMETER param;


static int MN88436_Reset(void)
{
//reset demod
	gpio_direction_output(frontend_reset, 0);
	msleep(200);
	gpio_direction_output(frontend_reset, 1);
//enable tuner
	gpio_direction_output(frontend_power, 1);
	return 0;
}


static int MN88436_I2c_Gate_Ctrl(struct dvb_frontend *fe, int enable)
{

	return 0;
}


static int MN88436_Init(struct dvb_frontend *fe)
{
	pr_dbg("frontend_reset is %d\n",frontend_reset);
	//reset
	MN88436_Reset();
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

//	pr_dbg("0x%x(ptuner),0x%x(pavchip)=========================demod init\r\n",mn88436pTuner->m_uiSlaveAddress,pAVLChip_all->m_SlaveAddr);
	msleep(200);
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
	//MDrv_Tuner_SetTuner(474000,6);
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

static void MN88436_Release(struct dvb_frontend *fe)
{
	struct mn88436_state *state = fe->demodulator_priv;
	kfree(state);
}

static ssize_t mn_frontend_show_short_circuit(struct class* class, struct class_attribute* attr, char* buf)
{
	
	return ;
}

static struct class_attribute mn_frontend_class_attrs[] = {
	__ATTR(short_circuit,  S_IRUGO | S_IWUSR, mn_frontend_show_short_circuit, NULL),
	__ATTR_NULL
};

static struct class mn_frontend_class = {
	.name = "mn_frontend",
	.class_attrs = mn_frontend_class_attrs,
};

static struct dvb_frontend_ops mn88436_ops;

struct dvb_frontend *mn88436_attach(const struct mn88436_fe_config *config)
{
	struct mn88436_state *state = NULL;

	/* allocate memory for the internal state */
	
	state = kmalloc(sizeof(struct mn88436_state), GFP_KERNEL);
	if (state == NULL)
		return NULL;

	/* setup the state */
	state->config = *config;
	
	/* create dvb_frontend */
	memcpy(&state->fe.ops, &mn88436_ops, sizeof(struct dvb_frontend_ops));
	state->fe.demodulator_priv = state;
	
	return &state->fe;
}

static struct dvb_frontend_ops mn88436_ops = {


		.info = {
		 .name = "AMLOGIC ATSC",
		.type = FE_ATSC,
		.frequency_min = 51000000,
		.frequency_max = 858000000,
		.frequency_stepsize = 0,
		.frequency_tolerance = 0,
		.caps =
			FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
			FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
			FE_CAN_QPSK | FE_CAN_QAM_16 |
			FE_CAN_QAM_64 | FE_CAN_QAM_AUTO |
			FE_CAN_TRANSMISSION_MODE_AUTO |
			FE_CAN_GUARD_INTERVAL_AUTO |
			FE_CAN_HIERARCHY_AUTO |
			FE_CAN_RECOVER |
			FE_CAN_MUTE_TS
	},

	.release = MN88436_Release,

	.init = MN88436_Init,
	.sleep = MN88436_Sleep,
	.i2c_gate_ctrl = MN88436_I2c_Gate_Ctrl,

	.set_frontend = MN88436_Set_Frontend,
	.get_frontend = MN88436_Get_Frontend,	
	.read_status = MN88436_Read_Status,
	.read_ber = MN88436_Read_Ber,
	.read_signal_strength =MN88436_Read_Signal_Strength,
	.read_snr = MN88436_Read_Snr,
	.read_ucblocks = MN88436_Read_Ucblocks,

};

static void mn88436_fe_release(struct aml_dvb *advb, struct aml_fe *fe)
{
	if(fe && fe->fe) {
		pr_dbg("release mn88436 frontend %d\n", fe->id);
		dvb_unregister_frontend(fe->fe);
		dvb_frontend_detach(fe->fe);
		if(fe->cfg){
			kfree(fe->cfg);
			fe->cfg = NULL;
		}
		fe->id = -1;
	}
}

static int mn88436_fe_init(struct aml_dvb *advb, struct platform_device *pdev, struct aml_fe *fe, int id)
{
	struct dvb_frontend_ops *ops;
	int ret;
	struct resource *res;
	struct mn88436_fe_config *cfg;
	char buf[32];
	
	pr_dbg("init mn88436 frontend %d\n", id);


	cfg = kzalloc(sizeof(struct mn88436_fe_config), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;
	
	cfg->reset_pin = frontend_reset;
	if(cfg->reset_pin==-1) {
		snprintf(buf, sizeof(buf), "frontend%d_reset_pin", id);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
		if (!res) {
			pr_error("cannot get resource \"%s\"\n", buf);
			ret = -EINVAL;
			goto err_resource;
		}
		cfg->reset_pin = res->start;		
	}
	
	cfg->i2c_id = frontend_i2c;
	if(cfg->i2c_id==-1) {
		snprintf(buf, sizeof(buf), "frontend%d_i2c", id);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
		if (!res) {
			pr_error("cannot get resource \"%s\"\n", buf);
			ret = -EINVAL;
			goto err_resource;
		}
		cfg->i2c_id = res->start;
	}
	
	cfg->tuner_addr = frontend_tuner_addr;
	
	if(cfg->tuner_addr==-1) {
		snprintf(buf, sizeof(buf), "frontend%d_tuner_addr", id);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
		if (!res) {
			pr_error("cannot get resource \"%s\"\n", buf);
			ret = -EINVAL;
			goto err_resource;
		}
		cfg->tuner_addr = res->start>>1;
	}
	
	cfg->demod_addr = frontend_demod_addr;
	if(cfg->demod_addr==-1) {
		snprintf(buf, sizeof(buf), "frontend%d_demod_addr", id);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
		if (!res) {
			pr_error("cannot get resource \"%s\"\n", buf);
			ret = -EINVAL;
			goto err_resource;
		}
		cfg->demod_addr = res->start>>1;
	}
	if(frontend_power==-1) {
		snprintf(buf, sizeof(buf), "frontend%d_power", id);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
		if (!res) {
			pr_error("cannot get resource \"%s\"\n", buf);
			ret = -EINVAL;
			goto err_resource;
		}
		frontend_power = res->start;
	}
	
	frontend_reset = cfg->reset_pin;
	frontend_i2c = cfg->i2c_id;
	frontend_tuner_addr = cfg->tuner_addr;
	frontend_demod_addr = cfg->demod_addr;	
	fe->fe = mn88436_attach(cfg);
	if (!fe->fe) {
		ret = -ENOMEM;
		goto err_resource;
	}

	if ((ret=dvb_register_frontend(&advb->dvb_adapter, fe->fe))) {
		pr_error("frontend registration failed!");
		ops = &fe->fe->ops;
		if (ops->release != NULL)
			ops->release(fe->fe);
		fe->fe = NULL;
		goto err_resource;
	}
	
	fe->id = id;
	fe->cfg = cfg;
	
	return 0;

err_resource:
	kfree(cfg);
	return ret;
}

int mn88436_get_fe_config(struct mn88436_fe_config *cfg)
{
	struct i2c_adapter *i2c_handle;
	cfg->i2c_id = frontend_i2c;
	cfg->demod_addr = frontend_demod_addr;
	cfg->tuner_addr = frontend_tuner_addr;
	cfg->reset_pin = frontend_reset;
//	printk("\n frontend_i2c is %d,,frontend_demod_addr is %x,frontend_tuner_addr is %x,frontend_reset is %d",frontend_i2c,frontend_demod_addr,frontend_tuner_addr,frontend_reset);
	i2c_handle = i2c_get_adapter(cfg->i2c_id);
	if (!i2c_handle) {
		printk("cannot get i2c adaptor\n");
		return 0;
	}
	cfg->i2c_adapter =i2c_handle;
	return 1;
	
//	printk("\n frontend0_i2c is %d, frontend_i2c is %d,",frontend0_i2c,frontend_i2c);
	
}

static int mn88436_fe_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct aml_dvb *dvb = aml_get_dvb_device();
	
	if(mn88436_fe_init(dvb, pdev, &mn88436_fe[0], 0)<0)
		return -ENXIO;

	platform_set_drvdata(pdev, &mn88436_fe[0]);

	if((ret = class_register(&mn_frontend_class))<0) {
		pr_error("register class error\n");

		struct aml_fe *drv_data = platform_get_drvdata(pdev);
		
		platform_set_drvdata(pdev, NULL);
	
		mn88436_fe_release(dvb, drv_data);

		return ret;
	}
	
	return ret;
}

static int mn88436_fe_remove(struct platform_device *pdev)
{
	struct aml_fe *drv_data = platform_get_drvdata(pdev);
	struct aml_dvb *dvb = aml_get_dvb_device();

	class_unregister(&mn_frontend_class);

	platform_set_drvdata(pdev, NULL);
	
	mn88436_fe_release(dvb, drv_data);
	
	return 0;
}

static int mn88436_fe_resume(struct platform_device *pdev)
{
	pr_dbg("mn88436_fe_resume \n");
	//reset
		MN88436_Reset();
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

static int mn88436_fe_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}


static struct platform_driver aml_fe_driver = {
	.probe		= mn88436_fe_probe,
	.remove		= mn88436_fe_remove,
	.resume		= mn88436_fe_resume,
	.suspend	= mn88436_fe_suspend,
	.driver		= {
		.name	= "mn88436",
		.owner	= THIS_MODULE,
	}
};

static int __init mnfrontend_init(void)
{
	return platform_driver_register(&aml_fe_driver);
}


static void __exit mnfrontend_exit(void)
{
	platform_driver_unregister(&aml_fe_driver);
}

module_init(mnfrontend_init);
module_exit(mnfrontend_exit);


MODULE_DESCRIPTION("mn88436 DVB-S2 Demodulator driver");
MODULE_AUTHOR("RSJ");
MODULE_LICENSE("GPL");


