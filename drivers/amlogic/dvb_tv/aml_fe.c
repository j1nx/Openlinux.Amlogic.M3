/*
 * AMLOGIC DVB frontend driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/fcntl.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include "aml_fe.h"

#ifdef pr_err
#undef pr_err
#endif

#define pr_err(a...) printk("FE: " a)
#define pr_dbg(a...) printk("FE: " a)


static struct aml_fe_drv *tuner_drv_list = NULL;
static struct aml_fe_drv *atv_demod_drv_list = NULL;
static struct aml_fe_drv *dtv_demod_drv_list = NULL;
static struct aml_fe_man  fe_man;
static DEFINE_SPINLOCK(lock);

static struct aml_fe_drv** aml_get_fe_drv_list(aml_fe_dev_type_t type)
{
	switch(type){
		case AM_DEV_TUNER:
			return &tuner_drv_list;
		case AM_DEV_ATV_DEMOD:
			return &atv_demod_drv_list;
		case AM_DEV_DTV_DEMOD:
			return &dtv_demod_drv_list;
		default:
			return NULL;
	}
}

int aml_register_fe_drv(aml_fe_dev_type_t type, struct aml_fe_drv *drv)
{
	if(drv){
		struct aml_fe_drv **list = aml_get_fe_drv_list(type);
		unsigned long flags;

		spin_lock_irqsave(&lock, flags);

		drv->next = *list;
		*list = drv;

		drv->ref = 0;

		spin_unlock_irqrestore(&lock, flags);
	}

	return 0;
}

int aml_unregister_fe_drv(aml_fe_dev_type_t type, struct aml_fe_drv *drv)
{
	int ret = 0;

	if(drv){
		struct aml_fe_drv *pdrv, *pprev;
		struct aml_fe_drv **list = aml_get_fe_drv_list(type);
		unsigned long flags;

		spin_lock_irqsave(&lock, flags);

		if(drv->ref){
			for(pprev = NULL, pdrv = *list;
				pdrv;
				pprev = pdrv, pdrv = pdrv->next){
				if(pdrv == drv){
					if(pprev)
						pprev->next = pdrv->next;
					else
						*list = pdrv->next;
					break;
				}
			}
		}else{
			pr_err("fe driver %d is inused\n", drv->id);
			ret = -1;
		}

		spin_unlock_irqrestore(&lock, flags);
	}

	return ret;
}

static int aml_fe_analog_set_frontend(struct dvb_frontend* fe, struct dvb_frontend_parameters* params)
{
	struct aml_fe *afe = fe->demodulator_priv;
	struct analog_parameters p;
	int ret = -1;

	p.frequency = params->frequency;
	p.audmode   = params->u.analog.audmode;
	p.std       = params->u.analog.std;

	if(fe->ops.analog_ops.set_params){
		fe->ops.analog_ops.set_params(fe, &p);
		ret = 0;
	}else if(fe->ops.tuner_ops.set_analog_params){
		ret = fe->ops.tuner_ops.set_analog_params(fe, &p);
	}

	if(ret == 0){
		afe->params = *params;
	}

	return ret;
}

static int aml_fe_analog_get_frontend(struct dvb_frontend* fe, struct dvb_frontend_parameters* params)
{
	struct aml_fe *afe = fe->demodulator_priv;

	*params = afe->params;

	return 0;
}

static int aml_fe_analog_read_status(struct dvb_frontend* fe, fe_status_t* status)
{
	return FE_HAS_LOCK;
}

static int aml_fe_analog_read_signal_strength(struct dvb_frontend* fe, u16 *strength)
{
	int ret = -1;

	if(fe->ops.analog_ops.has_signal){
		int s = fe->ops.analog_ops.has_signal(fe);
		*strength = s;
		ret = 0;
	}else if(fe->ops.tuner_ops.get_rf_strength){
		ret = fe->ops.tuner_ops.get_rf_strength(fe, strength);
	}

	return ret;
}

static int aml_fe_set_mode(struct dvb_frontend *dev, fe_type_t type)
{
	struct aml_fe *fe;
	aml_fe_mode_t mode;
	unsigned long flags;

	fe = dev->demodulator_priv;
	switch(type){
		case FE_QPSK:
			mode = AM_FE_QPSK;
			pr_dbg("set mode -> QPSK\n");
			break;
		case FE_QAM:
			pr_dbg("set mode -> QAM\n");
			mode = AM_FE_QAM;
			break;
		case FE_OFDM:
			pr_dbg("set mode -> OFDM\n");
			mode = AM_FE_OFDM;
			break;
		case FE_ATSC:
			pr_dbg("set mode -> ATSC\n");
			mode = AM_FE_ATSC;
			break;
		case FE_ANALOG:
			pr_dbg("set mode -> ANALOG\n");
			mode = AM_FE_ANALOG;
			break;
		default:
			pr_err("illegal fe type %d\n", type);
			return -1;
	}

	if(fe->mode == mode)
		return 0;
	
	if(fe->mode != AM_FE_UNKNOWN){
		if(fe->dtv_demod && (fe->dtv_demod->drv->capability & fe->mode) && fe->dtv_demod->drv->leave_mode)
			fe->dtv_demod->drv->leave_mode(fe, fe->mode);
		if(fe->atv_demod && (fe->atv_demod->drv->capability & fe->mode) && fe->atv_demod->drv->leave_mode)
			fe->atv_demod->drv->leave_mode(fe, fe->mode);
		if(fe->tuner && (fe->tuner->drv->capability & fe->mode) && fe->tuner->drv->leave_mode)
			fe->tuner->drv->leave_mode(fe, fe->mode);

		fe->mode = AM_FE_UNKNOWN;
	}

	if(!(mode & fe->capability)){
		int i;

		spin_lock_irqsave(&lock, flags);
		for(i = 0; i < FE_DEV_COUNT; i++){
			if(mode & fe_man.fe[i].capability)
				break;
		}
		spin_unlock_irqrestore(&lock, flags);

		if(i >= FE_DEV_COUNT){
			pr_err("frontend %p do not support mode %x, capability %x\n", fe, mode, fe->capability);
			return -1;
		}

		fe = &fe_man.fe[i];
		dev->demodulator_priv = fe;
	}

	if(fe->mode & AM_FE_DTV_MASK){
		aml_dmx_register_frontend(fe->ts, NULL);
		fe->mode = 0;
	}

	spin_lock_irqsave(&fe->slock, flags);

	memset(&fe->fe.ops.tuner_ops, 0, sizeof(&fe->fe.ops.tuner_ops));
	memset(&fe->fe.ops.analog_ops, 0, sizeof(fe->fe.ops.analog_ops));
	fe->fe.ops.release = NULL;
	fe->fe.ops.release_sec = NULL;
	fe->fe.ops.init = NULL;
	fe->fe.ops.sleep = NULL;
	fe->fe.ops.write = NULL;
	fe->fe.ops.tune = NULL;
	fe->fe.ops.get_frontend_algo = NULL;
	fe->fe.ops.set_frontend = NULL;
	fe->fe.ops.get_tune_settings = NULL;
	fe->fe.ops.get_frontend = NULL;
	fe->fe.ops.read_status = NULL;
	fe->fe.ops.read_ber = NULL;
	fe->fe.ops.read_signal_strength = NULL;
	fe->fe.ops.read_snr = NULL;
	fe->fe.ops.read_ucblocks = NULL;
	fe->fe.ops.diseqc_reset_overload = NULL;
	fe->fe.ops.diseqc_send_master_cmd = NULL;
	fe->fe.ops.diseqc_recv_slave_reply = NULL;
	fe->fe.ops.diseqc_send_burst = NULL;
	fe->fe.ops.set_tone = NULL;
	fe->fe.ops.set_voltage = NULL;
	fe->fe.ops.enable_high_lnb_voltage = NULL;
	fe->fe.ops.dishnetwork_send_legacy_command = NULL;
	fe->fe.ops.i2c_gate_ctrl = NULL;
	fe->fe.ops.ts_bus_ctrl = NULL;
	fe->fe.ops.search = NULL;
	fe->fe.ops.track = NULL;
	fe->fe.ops.set_property = NULL;
	fe->fe.ops.get_property = NULL;
	memset(&fe->fe.ops.blindscan_ops, 0, sizeof(fe->fe.ops.blindscan_ops));
	
	if(fe->tuner && fe->tuner->drv && (mode & fe->tuner->drv->capability) && fe->tuner->drv->get_ops){
		fe->tuner->drv->get_ops(fe->tuner, mode, &fe->fe.ops.tuner_ops);
	}

	if(fe->atv_demod && fe->atv_demod->drv && (mode & fe->atv_demod->drv->capability) && fe->atv_demod->drv->get_ops){
		fe->atv_demod->drv->get_ops(fe->atv_demod, mode, &fe->fe.ops.analog_ops);
		fe->fe.ops.set_frontend = aml_fe_analog_set_frontend;
		fe->fe.ops.get_frontend = aml_fe_analog_get_frontend;
		fe->fe.ops.read_status  = aml_fe_analog_read_status;
		fe->fe.ops.read_signal_strength = aml_fe_analog_read_signal_strength;
	}

	if(fe->dtv_demod && fe->dtv_demod->drv && (mode & fe->dtv_demod->drv->capability) && fe->dtv_demod->drv->get_ops){
		fe->dtv_demod->drv->get_ops(fe->dtv_demod, mode, &fe->fe.ops);
	}

	spin_unlock_irqrestore(&fe->slock, flags);

	if(fe->dtv_demod && (fe->dtv_demod->drv->capability & mode) && fe->dtv_demod->drv->enter_mode)
		fe->dtv_demod->drv->enter_mode(fe, mode);
	if(fe->atv_demod && (fe->atv_demod->drv->capability & mode) && fe->atv_demod->drv->enter_mode)
		fe->atv_demod->drv->enter_mode(fe, mode);
	if(fe->tuner && (fe->tuner->drv->capability & mode) && fe->tuner->drv->enter_mode)
		fe->tuner->drv->enter_mode(fe, mode);

	if(mode & AM_FE_DTV_MASK){
		aml_dmx_register_frontend(fe->ts, &fe->fe);
	}

	strcpy(fe->fe.ops.info.name, "amlogic dvb frontend");

	fe->fe.ops.info.type = type;
	fe->mode = mode;

	return 0;
}

static int aml_fe_read_ts(struct dvb_frontend *dev, int *ts)
{
	struct aml_fe *fe;

	fe = dev->demodulator_priv;

	*ts = fe->ts;
	return 0;
}

static int aml_fe_dev_init(struct aml_dvb *dvb, struct platform_device *pdev, aml_fe_dev_type_t type, struct aml_fe_dev *dev, int id)
{
	struct resource *res;
	char *name = NULL;
	char buf[32];
	int ret;

	switch(type){
		case AM_DEV_TUNER:
			name = "tuner";
			break;
		case AM_DEV_ATV_DEMOD:
			name = "atv_demod";
			break;
		case AM_DEV_DTV_DEMOD:
			name = "dtv_demod";
			break;
		default:
			break;
	}

	snprintf(buf, sizeof(buf), "%s%d", name, id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		struct aml_fe_drv **list = aml_get_fe_drv_list(type);
		struct aml_fe_drv *drv;
		int type = res->start;
		unsigned long flags;

		spin_lock_irqsave(&lock, flags);

		for(drv = *list; drv; drv = drv->next){
			if(drv->id == type){
				drv->ref++;
				break;
			}
		}

		spin_unlock_irqrestore(&lock, flags);

		if(drv){
			dev->drv = drv;
		}else{
			pr_err("cannot find %s%d driver: %d\n", name, id, type);
			return -1;
		}
	}else{
		return 0;
	}

	snprintf(buf, sizeof(buf), "%s%d_i2c_adap_id", name, id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int adap = res->start;

		dev->i2c_adap_id = adap;
		dev->i2c_adap = i2c_get_adapter(adap);
	}else{
		dev->i2c_adap_id = -1;
	}

	snprintf(buf, sizeof(buf), "%s%d_i2c_addr", name, id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int addr = res->start;

		dev->i2c_addr = addr;
	}else{
		dev->i2c_addr = -1;
	}

	snprintf(buf, sizeof(buf), "%s%d_reset_gpio", name, id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int gpio = res->start;

		dev->reset_gpio = gpio;
	}else{
		dev->reset_gpio = -1;
	}

	snprintf(buf, sizeof(buf), "%s%d_reset_value", name, id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int v = res->start;

		dev->reset_value = v;
	}else{
		dev->reset_gpio = 0;
	}

	snprintf(buf, sizeof(buf), "%s%d_tunerpower", name, id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int gpio = res->start;

		dev->tuner_power_gpio = gpio;
	}else{
		dev->tuner_power_gpio = -1;
	}	

	snprintf(buf, sizeof(buf), "%s%d_lnbpower", name, id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int gpio = res->start;

		dev->lnb_power_gpio = gpio;
	}else{
		dev->lnb_power_gpio = -1;
	}

	snprintf(buf, sizeof(buf), "%s%d_antoverload", name, id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int gpio = res->start;

		dev->antoverload_gpio = gpio;
	}else{
		dev->antoverload_gpio = -1;
	}	

	if(dev->drv->init){
		ret = dev->drv->init(dev);
		if(ret != 0){
			dev->drv = NULL;
			return ret;
		}
	}

	return 0;
}

static int aml_fe_dev_release(struct aml_dvb *dvb, aml_fe_dev_type_t type, struct aml_fe_dev *dev)
{
	if(dev->drv && dev->drv->release){
		dev->drv->ref--;
		dev->drv->release(dev);
	}

	dev->drv = NULL;
	return 0;
}

static int aml_fe_man_init(struct aml_dvb *dvb, struct platform_device *pdev, struct aml_fe *fe, int id)
{
	struct resource *res;
	char buf[32];
	int tuner_cap = 0xFFFFFFFF;
	int demod_cap = 0;

	snprintf(buf, sizeof(buf), "fe%d_tuner", id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int id = res->start;
		if((id < 0) || (id >= FE_DEV_COUNT) || !fe_man.tuner[id].drv){
			pr_err("invalid tuner device id %d\n", id);
			return -1;
		}

		fe->tuner = &fe_man.tuner[id];
		fe->init = 1;

		tuner_cap &= fe->tuner->drv->capability;
	}

	snprintf(buf, sizeof(buf), "fe%d_atv_demod", id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int id = res->start;
		if((id < 0) || (id >= FE_DEV_COUNT) || !fe_man.atv_demod[id].drv){
			pr_err("invalid ATV demod device id %d\n", id);
			return -1;
		}

		fe->atv_demod = &fe_man.atv_demod[id];
		fe->init = 1;

		demod_cap |= fe->atv_demod->drv->capability;
	}

	snprintf(buf, sizeof(buf), "fe%d_dtv_demod", id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int id = res->start;
		if((id < 0) || (id >= FE_DEV_COUNT) || !fe_man.dtv_demod[id].drv){
			pr_err("invalid DTV demod device id %d\n", id);
			return -1;
		}

		fe->dtv_demod = &fe_man.dtv_demod[id];
		fe->init = 1;

		demod_cap |= fe->dtv_demod->drv->capability;
	}

	snprintf(buf, sizeof(buf), "fe%d_ts", id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int id = res->start;
		aml_ts_source_t ts = AM_TS_SRC_TS0;

		switch(id){
			case 0:
				ts = AM_TS_SRC_TS0;
				break;
			case 1:
				ts = AM_TS_SRC_TS1;
				break;
			case 2:
				ts = AM_TS_SRC_TS2;
				break;
			default:
				break;
		}

		fe->ts = ts;
	}

	snprintf(buf, sizeof(buf), "fe%d_dev", id);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, buf);
	if(res){
		int id = res->start;
		fe->dev_id = id;
	}else{
		fe->dev_id = 0;
	}


	if(fe->init){
		int ret;

		spin_lock_init(&fe->slock);
		fe->mode = AM_FE_UNKNOWN;
		fe->capability = (tuner_cap & demod_cap);
		pr_dbg("fe: %p cap: %x tuner: %x demod: %x\n", fe, fe->capability, tuner_cap, demod_cap);
		fe->fe.demodulator_priv = fe;
		fe->fe.ops.set_mode = aml_fe_set_mode;
		fe->fe.ops.read_ts  = aml_fe_read_ts;

		ret = dvb_register_frontend(&dvb->dvb_adapter, &fe->fe);
		if(ret){
			pr_err("register fe%d failed\n", id);
			return -1;
		}
	}

	return 0;
}

static int aml_fe_man_release(struct aml_dvb *dvb, struct aml_fe *fe)
{
	if(fe->init){
		aml_dmx_register_frontend(fe->ts, NULL);
		dvb_unregister_frontend(&fe->fe);
		dvb_frontend_detach(&fe->fe);
		fe->init = 0;
	}

	return 0;
}

static int aml_fe_probe(struct platform_device *pdev)
{
	struct aml_dvb *dvb = aml_get_dvb_device();
	int i;
	
	for(i = 0; i < FE_DEV_COUNT; i++){
		if(aml_fe_dev_init(dvb, pdev, AM_DEV_TUNER, &fe_man.tuner[i], i)<0)
			return -1;
		if(aml_fe_dev_init(dvb, pdev, AM_DEV_ATV_DEMOD, &fe_man.atv_demod[i], i)<0)
			return -1;
		if(aml_fe_dev_init(dvb, pdev, AM_DEV_DTV_DEMOD, &fe_man.dtv_demod[i], i)<0)
			return -1;
	}

	for(i = 0; i < FE_DEV_COUNT; i++){
		if(aml_fe_man_init(dvb, pdev, &fe_man.fe[i], i)<0)
			return -1;
	}

	platform_set_drvdata(pdev, &fe_man);
	
	return 0;
}

static int aml_fe_remove(struct platform_device *pdev)
{
	struct aml_fe_man *fe_man = platform_get_drvdata(pdev);
	struct aml_dvb *dvb = aml_get_dvb_device();
	int i;

	platform_set_drvdata(pdev, NULL);
	
	for(i = 0; i < FE_DEV_COUNT; i++){
		aml_fe_man_release(dvb, &fe_man->fe[i]);
	}

	for(i = 0; i < FE_DEV_COUNT; i++){
		aml_fe_dev_release(dvb, AM_DEV_DTV_DEMOD, &fe_man->dtv_demod[i]);
		aml_fe_dev_release(dvb, AM_DEV_ATV_DEMOD, &fe_man->atv_demod[i]);
		aml_fe_dev_release(dvb, AM_DEV_TUNER, &fe_man->tuner[i]);
	}

	return 0;
}

static int aml_fe_suspend(struct platform_device *dev, pm_message_t state)
{
	int i;

	for(i = 0; i < FE_DEV_COUNT; i++){
		struct aml_fe *fe = &fe_man.fe[i];

		if(fe->tuner && fe->tuner->drv && fe->tuner->drv->suspend){
			fe->tuner->drv->suspend(fe->tuner);
		}

		if(fe->atv_demod && fe->atv_demod->drv && fe->atv_demod->drv->suspend){
			fe->atv_demod->drv->suspend(fe->atv_demod);
		}

		if(fe->dtv_demod && fe->dtv_demod->drv && fe->dtv_demod->drv->suspend){
			fe->dtv_demod->drv->suspend(fe->dtv_demod);
		}
	}

	return 0;
}

static int aml_fe_resume(struct platform_device *dev)
{
	int i;

	for(i = 0; i < FE_DEV_COUNT; i++){
		struct aml_fe *fe = &fe_man.fe[i];

		if(fe->tuner && fe->tuner->drv && fe->tuner->drv->resume){
			fe->tuner->drv->resume(fe->tuner);
		}

		if(fe->atv_demod && fe->atv_demod->drv && fe->atv_demod->drv->resume){
			fe->atv_demod->drv->resume(fe->atv_demod);
		}

		if(fe->dtv_demod && fe->dtv_demod->drv && fe->dtv_demod->drv->resume){
			fe->dtv_demod->drv->resume(fe->dtv_demod);
		}
	}

	return 0;
}

static struct platform_driver aml_fe_driver = {
	.probe		= aml_fe_probe,
	.remove		= aml_fe_remove,
	.suspend        = aml_fe_suspend,
	.resume         = aml_fe_resume,
	.driver		= {
		.name	= "amlogic-dvb-fe",
		.owner	= THIS_MODULE,
	}
};

static int __init aml_fe_init(void)
{
	return platform_driver_register(&aml_fe_driver);
}


static void __exit aml_fe_exit(void)
{
	platform_driver_unregister(&aml_fe_driver);
}

module_init(aml_fe_init);
module_exit(aml_fe_exit);


MODULE_DESCRIPTION("avl6211 DVB-S2 Demodulator driver");
MODULE_AUTHOR("RSJ");
