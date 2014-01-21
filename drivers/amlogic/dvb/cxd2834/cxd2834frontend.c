/*****************************************************************
**
**  Copyright (C) 2009 Amlogic,Inc.
**  All rights reserved
**        Filename : gxfrontend.c
**
**  comment:
**        Driver for CDX2834 demodulator
**  author :
**	    jianfeng_wang@amlogic
**  version :
**	    v1.0	 09/03/04
*****************************************************************/

/*
    Driver for CDX2834 demodulator
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

#include <linux/dvb/frontend.h>
#include <linux/i2c.h>
#include "../../../media/dvb/dvb-core/dvb_frontend.h"
#include "../aml_dvb.h"

#include "sony_dvb_integration.h"
#include "sony_dvb_demod_monitor.h"
#include "cxd2831_cdx2834_api.h"
#include "sony_dvb_t2.h"

static int cdx2834_mode = 0;   //dvb-t:0  //dvb-t2:1


#if 0
#define pr_dbg(fmt, args...) printk( KERN_DEBUG"DVB: " fmt, ## args)
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

static struct aml_fe cdx2834_fe[FE_DEV_COUNT];

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

static struct aml_fe cdx2834_fe[FE_DEV_COUNT];

static int cdx2834_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	//struct cdx2834_state *state = fe->demodulator_priv;

	return 0;
}

static int cdx2834_init(struct dvb_frontend *fe)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	demod_init(state);
	msleep(200);
	return 0;
}

static int cdx2834_sleep(struct dvb_frontend *fe)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	return 0;
}

static int cdx2834_read_status(struct dvb_frontend *fe, fe_status_t * status)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	unsigned char s=0;
	
	demod_check_locked(&s);
	if(s==1)
	{
		*status = FE_HAS_LOCK|FE_HAS_SIGNAL|FE_HAS_CARRIER|FE_HAS_VITERBI|FE_HAS_SYNC;
	}
	else
	{
		*status = FE_TIMEDOUT;
	}
	printk("cdx2834_read_status 0x%02x\n",*status);
	return  0;
}

static int cdx2834_read_ber(struct dvb_frontend *fe, u32 * ber)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	
	demod_get_signal_errorate(state, ber);
	return 0;
}

static int cdx2834_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	uint   rec_strength;
	
	demod_get_signal_strength(state, &rec_strength);
	*strength=rec_strength;
    //printk("signall strenth %d\n",rec_strength);
	return 0;
}

static int cdx2834_read_snr(struct dvb_frontend *fe, u16 * snr)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	uint   rec_snr ;
	
	demod_get_signal_quality(state, &rec_snr) ;
	*snr = rec_snr;//>>16;
	
    //printk("signall rec_snr %d\n",rec_snr);
	return 0;
}

static int cdx2834_read_ucblocks(struct dvb_frontend *fe, u32 * ucblocks)
{
	ucblocks=NULL;
	return 0;
}

static int cdx2834_set_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	unsigned char s=0;
	state->mode = 0;
    //dump_stack();

	state->freq=p->frequency;
	state->bandwidth=p->u.ofdm.bandwidth ;
	state->mode = p->u.ofdm.ofdm_mode;

	printk("state->mode %d \n",state->mode);
    printk("cdx2834=>frequency=%d,bandwidth=%d\r\n",p->frequency,state->bandwidth);
	demod_connect(state, p->frequency,p->u.ofdm.bandwidth);
	demod_check_locked(&s);
	if(s!=1)
	{
			state->mode = 1;
			p->u.ofdm.ofdm_mode = state->mode;
			printk(" Not T ,Lock T2\n");
			demod_connect(state, p->frequency,p->u.ofdm.bandwidth);			
	}
	return  0;
}

static int cdx2834_get_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{//these content will be writed into eeprom .

	struct cdx2834_state *state = fe->demodulator_priv;
	
	p->frequency=state->freq;
	p->u.ofdm.bandwidth=state->bandwidth;
	p->u.ofdm.ofdm_mode = state->mode;
	//p->u.qam.symbol_rate=state->symbol_rate;
	
	return 0;
}

static void cdx2834_release(struct dvb_frontend *fe)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	
	demod_deinit(state);
	kfree(state);
}

static int cdx2834_set_property(struct dvb_frontend *fe, struct dtv_property *p)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	int r = 0;
	
	switch (p->cmd) {
		case DTV_DVBT2_PLP_ID:
			if (demod_set_data_plp(state, (uint8_t)p->u.data) != 0) {
				r = -EINVAL;
			}
			break;
		default:
			r = EOPNOTSUPP;
			break;
	}

	return r;
}

static int cdx2834_get_property(struct dvb_frontend *fe, struct dtv_property *p)
{
	struct cdx2834_state *state = fe->demodulator_priv;
	int r = 0;
	
	switch (p->cmd) {
		case DTV_DVBT2_PLP_ID:
			{
				sony_dvbt2_plp_t plp_info;
				if (demod_get_active_data_plp(state, &plp_info) != 0) {
					p->u.buffer.len = 0;
					r = -EINVAL;
				} else {
					p->u.buffer.len = 2;
					p->u.buffer.data[0] = plp_info.id;
					p->u.buffer.data[1] = plp_info.type;
				}
			}
			break;
		case DTV_DVBT2_DATA_PLPS:
			{
				uint8_t plpids[256];
				uint8_t plpnum = 0;

				p->u.buffer.len = 0;
				p->u.buffer.reserved1[0] = 0;
				if (p->u.buffer.reserved2 != NULL) {
					demod_get_data_plps(state, plpids, &plpnum);
					/* As linux dvb has property_dump, buffer.len cannot be used in this case, 
					 * it must < 32 , we use u.buffer.resvered1[0] to save plp num instead */
					p->u.buffer.reserved1[0] = plpnum;
					if (plpnum > 0 && 
						copy_to_user(p->u.buffer.reserved2, plpids, plpnum * sizeof(uint8_t))) {
						p->u.buffer.reserved1[0] = 0;
					}
				}
			}
			break;
		default:
			r = EOPNOTSUPP;
			break;
	}

	return r;
}

static struct dvb_frontend_ops cdx2834_ops;

struct dvb_frontend *cdx2834_attach(const struct cdx2834_fe_config *config)
{
	struct cdx2834_state *state = NULL;

	/* allocate memory for the internal state */
	
	state = kmalloc(sizeof(struct cdx2834_state), GFP_KERNEL);
	if (state == NULL)
		return NULL;

	/* setup the state */
	state->config = *config;
	
	/* create dvb_frontend */
	memcpy(&state->fe.ops, &cdx2834_ops, sizeof(struct dvb_frontend_ops));
	state->fe.demodulator_priv = state;
	
	return &state->fe;
}

static struct dvb_frontend_ops cdx2834_ops = {

	.info = {
		 .name = "AMLOGIC CDX2834  DVB-T2",
		 .type = FE_OFDM,
		 .frequency_min = 1000,
		 .frequency_max = 860000000,
		 .frequency_stepsize = 8000,
		 .symbol_rate_min = 870000,
		 .symbol_rate_max = 11700000,
		 .caps = BANDWIDTH_8_MHZ|BANDWIDTH_7_MHZ|BANDWIDTH_6_MHZ|BANDWIDTH_AUTO},

	.release = cdx2834_release,

	.init = cdx2834_init,
	.sleep = cdx2834_sleep,
	.i2c_gate_ctrl = cdx2834_i2c_gate_ctrl,

	.set_frontend = cdx2834_set_frontend,
	.get_frontend = cdx2834_get_frontend,

	.read_status = cdx2834_read_status,
	.read_ber = cdx2834_read_ber,
	.read_signal_strength =cdx2834_read_signal_strength,
	.read_snr = cdx2834_read_snr,
	.read_ucblocks = cdx2834_read_ucblocks,
	.set_property = cdx2834_set_property,
	.get_property = cdx2834_get_property,
};

static void cdx2834_fe_release(struct aml_dvb *advb, struct aml_fe *fe)
{
	if(fe && fe->fe) {
		pr_dbg("release GX1001 frontend %d\n", fe->id);
		dvb_unregister_frontend(fe->fe);
		dvb_frontend_detach(fe->fe);
		if(fe->cfg){
			kfree(fe->cfg);
			fe->cfg = NULL;
		}
		fe->id = -1;
	}
}

static int cdx2834_fe_init(struct aml_dvb *advb, struct platform_device *pdev, struct aml_fe *fe, int id)
{
	struct dvb_frontend_ops *ops;
	int ret;
	struct resource *res;
	struct cdx2834_fe_config *cfg;
	char buf[32];
	
	pr_dbg("init CDX2834 frontend %d\n", id);


	cfg = kzalloc(sizeof(struct cdx2834_fe_config), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;
	
	cfg->reset_pin = frontend_reset;
	if(cfg->reset_pin==-1) {
		snprintf(buf, sizeof(buf), "frontend%d_reset", id);
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
	
	fe->fe = cdx2834_attach(cfg);
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


static ssize_t cxd2834_show_mode(struct class *class, char *buf)
{
	ssize_t ret = 0;
	char *src;
	
	switch(cdx2834_mode) {
		case 1:
			src = "dvb-t2";
		break;
		case 0:
			src = "dvb-t";
		break;
		
		default:
			src = "";
		break;
	}
	
	ret = sprintf(buf, "%s\n", src);
	return ret;
}

/*Set the STB input source*/
static ssize_t cxd2834_store_mode(struct class *class,
                          const char *buf,
                          size_t size)
{
	if(strncmp("dvb-t", buf, 5)) {
    	cdx2834_mode = 0;
    } else if(!strncmp("dvb-t2", buf, 6)) {
    	cdx2834_mode = 1;
    }
  
    return size;
}

static struct class_attribute aml_demod_cxd2834_class_attrs[] = {
	__ATTR(frontend_mode,  S_IRUGO | S_IWUSR, cxd2834_show_mode, cxd2834_store_mode),
	__ATTR_NULL
};

static struct class aml_cxd2834_class = {
	.name = "cxd2834",
	.class_attrs = aml_demod_cxd2834_class_attrs,
};


static int cdx2834_fe_probe(struct platform_device *pdev)
{
	struct aml_dvb *dvb = aml_get_dvb_device();
	
	if(cdx2834_fe_init(dvb, pdev, &cdx2834_fe[0], 0)<0)
		return -ENXIO;
		
	if(class_register(&aml_cxd2834_class)<0) {
                pr_error("register class error\n");      ;
        }
   
	platform_set_drvdata(pdev, &cdx2834_fe[0]);
	error:
		
	return 0;
}

static int cdx2834_fe_remove(struct platform_device *pdev)
{
	struct aml_fe *drv_data = platform_get_drvdata(pdev);
	struct aml_dvb *dvb = aml_get_dvb_device();

	platform_set_drvdata(pdev, NULL);
	
	cdx2834_fe_release(dvb, drv_data);
	
	return 0;
}

static struct platform_driver cxd2834_fe_driver = {
	.probe		= cdx2834_fe_probe,
	.remove		= cdx2834_fe_remove,	
	.driver		= {
		.name	= "cxd2834",
		.owner	= THIS_MODULE,
	}
};

static int __init cxd2834frontend_init(void)
{
	return platform_driver_register(&cxd2834_fe_driver);
}


static void __exit cxd2834frontend_exit(void)
{
	platform_driver_unregister(&cxd2834_fe_driver);
}

module_init(cxd2834frontend_init);
module_exit(cxd2834frontend_exit);


MODULE_DESCRIPTION("CDX2834 DVB-T2 Demodulator driver");
MODULE_AUTHOR("Dennis Noermann and Andrew de Quincey");
MODULE_LICENSE("GPL");


