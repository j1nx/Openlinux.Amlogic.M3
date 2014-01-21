

#include <linux/delay.h>
#include <linux/mutex.h>

#include <linux/dvb/frontend.h>
#include <linux/i2c.h>
#include "../../../media/dvb/dvb-core/dvb_frontend.h"
#include "../aml_dvb.h"

#include <mach/gpio.h>

#include "sony_dvb_integration.h"
#include "sony_dvb_demod_monitor.h"


#include "sony_dvb_ascot2d.h"
#include "sony_dvb_tuner.h"
#include "sony_dvb_demod.h"
#include "sony_dvb_demod_monitorC2.h"
#include "sony_dvb_demod_monitorC.h"
#include "sony_dvb_demod_monitorT2.h"
#include "sony_dvb_demod_monitorT.h"
#include "cxd2831_cdx2834_api.h"
#include "sony_dvb_demodT2.h"



#define TUNER_IFAGCPOS            
//**< Define for IFAGC sense positive. */
//* #define TUNER_SPECTRUM_INV       1 */
//**< Define for spectrum inversion. */
#define TUNER_RFLVLMON_DISABLE      1       /**< Define to disable RF level monitoring. */
#define TUNER_SONY_ASCOT            1       /**< Define for Sony CXD2815 / Ascot2S tuner. */


#define DEMOD_TUNE_POLL_INTERVAL    10


#if 0
#define pr_dbg(fmt, args...) printk(KERN_ERR "cxd2834: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif

#define pr_error(fmt, args...) printk(KERN_ERR "cxd2834: " fmt, ## args)


#define MAX_I2C_WRITE 256


static const char *FormatResult (sony_result_t result);
static const char *FormatSystem (sony_dvb_system_t system);
sony_result_t CXD2831_CDX2834_Scan(struct cdx2834_state *state,uint32_t startFreqkHz,uint8_t bwMHz);

static struct cdx2834_state* cdx2834;


int cdx2834_set_addr(void *pstate,unsigned char ChipAddress, unsigned char regaddr)
{
	struct cdx2834_state *state = (struct cdx2834_state *)pstate;
	int  nRetCode = 0;
	char buffer[2];
	struct i2c_msg msg;
	

	buffer[0] = regaddr;
	memset(&msg, 0, sizeof(msg));
	msg.addr = ChipAddress;
	msg.flags &=  ~I2C_M_RD;  //write  I2C_M_RD=0x01
	msg.len = 1;
	msg.buf = buffer;
	nRetCode = i2c_transfer(state->i2c, &msg, 1);
	
	if(nRetCode != 1)
	{
		pr_error("cdx2834_writeregister reg 0x%x failure,ret %d!\n",regaddr,nRetCode);
		return 0;
	}
	return 1;   //success
}

////////////////////I2c & Delay////////////////////
int I2cReadWrite(void *pstate,int mode, unsigned char ChipAddress, unsigned char *Data, int NbData)
{
	int ret = 0;
	char regbuf[1];
	struct cdx2834_state *state = (struct cdx2834_state *)pstate;
    //ChipAddress = 0xA5;
    //pr_dbg("cdx2834 read register mode 0x%x,ChipAddress 0x%x\n",mode,ChipAddress);
	if(Data == 0 || NbData == 0)
	{
		pr_err("cdx2834 read register parameter error !!\n");
		return 0;
	}
    
	if(mode == 0) // read mode
	{ 
		struct i2c_msg msg;
 		memset(&msg, 0, sizeof(msg));
		msg.addr = ChipAddress;
		msg.flags |=  I2C_M_RD;  //write  I2C_M_RD=0x01
		msg.len = NbData;
		msg.buf = Data;
 		ret = i2c_transfer(state->i2c,  &msg, 1);
  	}
    
	else if(mode == 3)
	{
		struct i2c_msg msg; 
		cdx2834_set_addr(pstate,ChipAddress, Data[0]) ; //set reg address first
 		//read real data 
		memset(&msg, 0, sizeof(msg));
		msg.addr = ChipAddress;
		msg.flags |=  I2C_M_RD;  //write  I2C_M_RD=0x01
		msg.len = NbData;
		msg.buf = Data;
		ret = i2c_transfer(state->i2c,  &msg, 1);
	
	}
	else
	{
      

		struct i2c_msg msg[2];

		regbuf[0] = Data[0] & 0xff;
		
		memset(msg, 0, sizeof(msg));
	

        #if 1
        /*write reg address*/
        
        /*write value*/
        msg[0].addr = ChipAddress;
        msg[0].flags &= ~I2C_M_RD;   /*i2c_transfer will emit a stop flag, so we should send 2 msg together,
                                                                     * and the second msg's flag=I2C_M_NOSTART, to get the right timing*/
                                                                         
        msg[0].buf = Data;
        msg[0].len = NbData;
        
        if((*msg[0].buf)== 0x09)
        {
           // dump_stack();
        }

        ret = i2c_transfer(state->i2c, msg, 1);

        #else
            /*write reg address*/
            msg[0].addr = ChipAddress;                  
            msg[0].flags = 0;
            msg[0].buf = regbuf;
            msg[0].len = 1;
            
            /*write value*/
            msg[1].addr = ChipAddress;
            msg[1].flags = I2C_M_NOSTART;   /*i2c_transfer will emit a stop flag, so we should send 2 msg together,
                                                                         * and the second msg's flag=I2C_M_NOSTART, to get the right timing*/
                                                                             
            msg[1].buf = Data;
            msg[1].len = NbData;
            
            if((*msg[0].buf)== 0x09)
            {
               // dump_stack();
            }
            //pr_dbg("msg[0].buf 0x%x,msg[0].addr[0] 0x%x,msg[1].buf 0x%x,msg[1].addr 0x%x,",*msg[0].buf,msg[0].addr,*msg[1].buf,msg[1].addr);
            pr_dbg("cdx2834 i2c_transfer\n");
            ret = i2c_transfer(state->i2c, msg, 2);
		#endif
		
	}
	
	if(ret<0){
		pr_err (" %s: writereg error, errno is %d \n", __FUNCTION__, ret);
		return -1;
	}
	else
		return 0;
}
////////////////////I2c & Delay////////////////////
int TunerI2cReadWrite(void *pstate,int mode, unsigned char ChipAddress, unsigned char *Data, int NbData)
{
    int ret = 0;
    char regbuf[1];
    struct cdx2834_state *state = (struct cdx2834_state *)pstate;
    //ChipAddress = 0xA5;
    pr_dbg("cdx2834 read register mode 0x%x,ChipAddress 0x%x\n",mode,ChipAddress);
    if(Data == 0 || NbData == 0)
    {
        pr_err("cdx2834 read register parameter error !!\n");
        return 0;
    }
    
    if(mode == 0) // read mode
    { 
        struct i2c_msg msg;
        memset(&msg, 0, sizeof(msg));
        msg.addr = ChipAddress;
        msg.flags |=  I2C_M_RD;  //write  I2C_M_RD=0x01
        msg.len = NbData;
        msg.buf = Data;
        ret = i2c_transfer(state->i2c,  &msg, 1);
    }
    
    else if(mode == 3)
    {
        struct i2c_msg msg; 
        cdx2834_set_addr(pstate,ChipAddress, Data[0]) ; //set reg address first
        //read real data 
        memset(&msg, 0, sizeof(msg));
        msg.addr = ChipAddress;
        msg.flags |=  I2C_M_RD;  //write  I2C_M_RD=0x01
        msg.len = NbData;
        msg.buf = Data;
        ret = i2c_transfer(state->i2c,  &msg, 1);
    
    }
    else
    {
      
      pr_dbg("cdx2834 i2c_transfe11111r\n");
        struct i2c_msg msg[2];
        pr_dbg("cdx2834 i2c_transfe2222 Data 0x%xr\n",Data);
        /*construct 2 msgs, 1 for reg addr, 1 for reg value, send together*/
        regbuf[0] = Data[0] & 0xff;
        
        pr_dbg("cdx2834 i2c_transfe33333r\n");
        memset(msg, 0, sizeof(msg));
    
    pr_dbg("cdx2834 i2c_transfe44444r\n");
        /*write value*/
        msg[0].addr = ChipAddress;
        msg[0].flags &= ~I2C_M_RD;   /*i2c_transfer will emit a stop flag, so we should send 2 msg together,
                                                                     * and the second msg's flag=I2C_M_NOSTART, to get the right timing*/
                                                                         
        msg[0].buf = Data;
        msg[0].len = NbData;
        
        if((*msg[0].buf)== 0x09)
        {
           // dump_stack();
        }
        //pr_dbg("msg[0].buf 0x%x,msg[0].addr[0] 0x%x,msg[1].buf 0x%x,msg[1].addr 0x%x,",*msg[0].buf,msg[0].addr,*msg[1].buf,msg[1].addr);
       // pr_dbg("cdx2834 i2c_transfer\n");
        ret = i2c_transfer(state->i2c, msg, 1);

        
        
    }
    
    if(ret<0){
        pr_err (" %s: writereg error, errno is %d \n", __FUNCTION__, ret);
        return -1;
    }
    else
        return 0;
}

sony_result_t dvbtest2_i2c_Write(void *pstate,struct sony_i2c_t * pI2c, 
								uint8_t deviceAddress, 
								const uint8_t * pData, 
								uint32_t size,
								uint8_t mode)
{
    pstate = cdx2834;
 	return (sony_result_t) I2cReadWrite(pstate,1, deviceAddress, (uint8_t *)pData, size);
} 

sony_result_t dvbtest2_i2c_Read(void *pstate,struct sony_i2c_t * pI2c, 
							   uint8_t deviceAddress, 
							   uint8_t * pData, 
							   uint32_t size,
							   uint8_t mode)
{
    pstate = cdx2834;
    //printk("dvbtest2_i2c_Read\n");
	return (sony_result_t) I2cReadWrite(pstate,0,deviceAddress, (uint8_t *)pData, size);
}
sony_result_t dvbtest2_i2c_WriteGw(void *pstate,struct sony_i2c_t * pI2c, 
							   uint8_t deviceAddress, 
							   const uint8_t * pData, 
							   uint32_t size,
							   uint8_t mode)
{
 	uint8_t data[MAX_I2C_WRITE];
    struct i2c_msg msg[3];
        uint8_t rData[MAX_I2C_WRITE];
	//pstate = cdx2834;
	    int ret = 0;
	   // printk("dvbtest2_i2c_WriteGw\n");
	if ((!pI2c) || (!pData)) {
		   return SONY_RESULT_ERROR_ARG;
	   }
	
	if (size > (MAX_I2C_WRITE - (1 + 1))) {
			return SONY_RESULT_ERROR_ARG;
		}
	
 		memset(data,0,255);
        memset(rData,0,255);
        memset(msg,0,sizeof(msg));
        memcpy (rData, pData, size);
		data[0] = pI2c->gwSub;
		//data[1]= deviceAddress;

         msg[0].addr = pI2c->gwAddress;
         msg[0].flags = 0;   /*i2c_transfer will emit a stop flag, so we should send 2 msg together,
                                                                          * and the second msg's flag=I2C_M_NOSTART, to get the right timing*/                                                                     
         msg[0].buf = data;
         msg[0].len = 1;

         //rData[0]=deviceAddress;
         //memcpy (&rData[1], pData, size);
         
         msg[1].addr = 0x60;//pI2c->gwAddress;
         msg[1].flags = 0;  //write  I2C_M_RD=0x01
         msg[1].len = size;
         msg[1].buf = rData;

         ret = i2c_transfer(cdx2834->i2c,&msg, 2);
        
           //memcpy(pData,rData,size); 
          // printk("ret %d\n",ret);
         if(ret != 2)
         	{
         		return (sony_result_t)ret ;
         	}
         return SONY_RESULT_OK ;





                
		//memcpy (&data[2], pData, size);
		
	//return (sony_result_t) TunerI2cReadWrite(pstate,1,pI2c->gwAddress, data, (size+2 ));
 }	//L

sony_result_t dvbtest2_i2c_ReadGw(void *pstate,struct sony_i2c_t * pI2c, 
							   uint8_t deviceAddress, 
							   uint8_t * pData, 
							   uint32_t size,
							   uint8_t mode)
{	
	uint8_t data[50];
    uint8_t rData[50];
    
    int ret = 0;
    struct i2c_msg msg[4];
	int error,i;
    pstate = cdx2834;
    //printk("dvbtest2_i2c_ReadGw\n");
	if ((!pI2c) || (!pData)) {
		return SONY_RESULT_ERROR_ARG;
	}
	memset(msg,0,sizeof(msg));

    data[0] = pI2c->gwSub;
    msg[0].addr = pI2c->gwAddress;
    msg[0].flags = 0;   /*i2c_transfer will emit a stop flag, so we should send 2 msg together,
                                                                     * and the second msg's flag=I2C_M_NOSTART, to get the right timing*/                                                                     
    msg[0].buf = data;
    msg[0].len = 1;

    
    msg[1].addr = 0x60;//pI2c->gwAddress;
    msg[1].flags = I2C_M_RD;  //write  I2C_M_RD=0x01
    msg[1].len = 1;
    msg[1].buf = rData;

    msg[2].addr = 0x6C;//pI2c->gwAddress;
    msg[2].flags =  (I2C_M_RD);  //write  I2C_M_RD=0x01
    msg[2].len = size;
    msg[2].buf = (uint8_t *)rData;
    ret = i2c_transfer(cdx2834->i2c,&msg, 3);
    data[0] = pData[0];
  	memcpy(pData,rData,size); 
    //printk("ReadGW reg 0x%x, pData 0x%x\n",data[0],pData[0]);
		if(ret != 3)
		{
			return (sony_result_t)ret ;
		}
		 return SONY_RESULT_OK ;

	//return (sony_result_t)ret ;
}	//LGI



unsigned char user_I2CReadArray(unsigned char SubAddress, short int nBytes, unsigned char *ReadData)
{
	ReadData[0]=SubAddress;
	//return dvbtest2_i2c_ReadGw(cdx2834,&cdx2834->tunerI2C,TUNER_IIC_DEV_ADR,ReadData,nBytes,0);

}
unsigned char user_I2CWriteArray(unsigned char SubAddress,short int nBytes, unsigned char *WriteData)
{
	unsigned char i=0;
	unsigned char u8Buff[20];

	if (nBytes > 20) return 1;
	u8Buff[0] = SubAddress;
	for(i=0; i<nBytes; i++)
	{
		u8Buff[1+i] = *(WriteData+i);
	}

	//return dvbtest2_i2c_WriteGw(cdx2834,&cdx2834->tunerI2C,TUNER_IIC_DEV_ADR,u8Buff,nBytes+1,0);
	
}

/*------------------------------------------------------------------------------
 Function: Example Scanning Callback
 ------------------------------------------------------------------------------*/

/**
 @brief Example scanning call-back function that implements 
        carrier offset compensation for DVB-T/T2/C.
        It also provides an example scan progress indicator.

        Carrier offset compensation allows the demodulator 
        to lock to signals that have large enough carrier offsets
        as to prevent TS lock.
        Carrier offset compensation is important to real world tuning
        and scanning and is highly recommended.

 @param pDriver The driver instance.
 @param pResult The scan result.

*/
/////////////////for tuner ///////////////
sony_result_t CXD2831tuner_Init(struct sony_dvb_tuner_t * pTuner)
{
	//initialize all static variables
	//g_prev_BWKHz=-1;
	//tuner_Init();
	//tuner_SetHLMode( TUNER_HP_DECT_ENABLE);
	//tuner_SetModes( TUNER_STBY_MODE, TUNER_GAIN_MODE, TUNER_INV_MODE, TUNER_IF_DC_MODE);
	printk("init ascot2d tuner!!!\n");
	//sony_dvb_ascot2d_Initialize (pTuner);
	return SONY_RESULT_OK;
}

sony_result_t CXD2831SetTuner(struct sony_dvb_tuner_t * pTuner,
                               uint32_t frequency, sony_dvb_system_t system, uint8_t bandWidth)
{	
	unsigned int inNZfreq =4570;//4800;
    unsigned int bandWidthKhz = 0;
    printk("CXD2831SetTuner bandWidth %d \n",bandWidth);
    //dump_stack();
	switch(bandWidth)
	{
		case 8:
			inNZfreq = 4570;//3600;//4800;
			break;
		case 7:
			inNZfreq = 4200;
			break;
		case 6:
			inNZfreq = 4570;
			break;
		default:
			inNZfreq = 4800;
			break;
	}
	bandWidthKhz =bandWidth*1000;
    	//tuner_SetFreq( frequency, TUNER_XTAL_FREQ, bandWidthKhz, inNZfreq, TUNER_RF_IN, TUNER_IF_MODE);
	//tuner_Optimize(frequency, TUNER_XTAL_FREQ, bandWidthKhz, inNZfreq, TUNER_IF_MODE, TUNER_BB_DB_OFFSET);
	sony_dvb_ascot2d_Tune(pTuner,frequency, system, bandWidth);
    return SONY_RESULT_OK;
}



void CXD2831_CDX2834_init(struct cdx2834_state *state)
{
	sony_result_t result = SONY_RESULT_OK;
	sony_dvb_scan_params_t scanParams;
	int i ;
	cdx2834 = state;
	state->tunerI2C.gwAddress = SONY_DVB_DEMOD_ADDRESS;
	state->tunerI2C.gwSub = 0x09;	   /* Connected via demod I2C gateway function. */
	state->tunerI2C.Read = dvbtest2_i2c_ReadGw;
	state->tunerI2C.Write = dvbtest2_i2c_WriteGw;
	state->tunerI2C.ReadRegister = sony_i2c_CommonReadRegister;
	state->tunerI2C.WriteRegister = sony_i2c_CommonWriteRegister;
	state->tunerI2C.WriteOneRegister = sony_i2c_CommonWriteOneRegister;
	//state->ascot2d.pI2c = &(state->tunerI2C);                        
	state->demodI2C.gwAddress = 0x00; 
	state->demodI2C.gwSub = 0x00;	   /* N/A */
	state->demodI2C.Read = dvbtest2_i2c_Read;
	state->demodI2C.Write = dvbtest2_i2c_Write;
	state->demodI2C.ReadRegister = sony_i2c_CommonReadRegister;
	state->demodI2C.WriteRegister = sony_i2c_CommonWriteRegister;
	state->demodI2C.WriteOneRegister = sony_i2c_CommonWriteOneRegister;
 //printk("state->demodI2C.ReadRegister 0x%x\n",state->demodI2C.ReadRegister);
	state->demod.system=SONY_DVB_SYSTEM_DVBT;


    result = sony_dvb_ascot2d_Create (SONY_ASCOT2D_ADDRESS, 41, &(state->tunerI2C), SONY_ASCOT2D_CONFIG_EXT_REF, &(state->ascot2d), &(state->tuner));
    if (result != SONY_RESULT_OK) {
        printk("Error: Unable to create Sony ASCOT2D tuner driver. (status=%d, %s)\r\n", result, FormatResult (result));
        return -1;
    }
	/* Setup demodulator. */
	result = sony_dvb_integration_Create (SONY_DVB_41000kHz,SONY_DVB_DEMOD_ADDRESS, &(state->demodI2C), \
	&(state->demod), &(state->tuner), &(state->device));
#if 1
	    /* Configure demod driver for Sony ASCOT2D tuner IF. */
	    //state->demod.iffreq_config.config_DVBT5 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
	    //state->demod.iffreq_config.config_DVBT6 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
	   // state->demod.iffreq_config.config_DVBT7 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.20);
	   // state->demod.iffreq_config.config_DVBT8 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.80);
		state->demod.iffreq_config.config_DVBT5 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
		state->demod.iffreq_config.config_DVBT6 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
		state->demod.iffreq_config.config_DVBT7 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.20);
		state->demod.iffreq_config.config_DVBT8 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.80);
	    state->demod.iffreq_config.config_DVBC = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.90);

	/* T2 ITB setup. */
	state->demod.iffreq_config.config_DVBT2_5 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
	state->demod.iffreq_config.config_DVBT2_6 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
	state->demod.iffreq_config.config_DVBT2_7 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.20);
	state->demod.iffreq_config.config_DVBT2_8 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.80);



#else
    /* Configure demod driver for Sony ASCOT2D tuner IF. */
    state->demod.iffreq_config.config_DVBT6 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);//DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);
    state->demod.iffreq_config.config_DVBT7 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);
    state->demod.iffreq_config.config_DVBT8 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);
    state->demod.iffreq_config.config_DVBC = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);
    
    /* T2 ITB setup. */
    state->demod.iffreq_config.config_DVBT2_5 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);
    state->demod.iffreq_config.config_DVBT2_6 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);
    state->demod.iffreq_config.config_DVBT2_7 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);
    state->demod.iffreq_config.config_DVBT2_8 = DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57);
#endif
	/* Initialize the I2C */


	/* Initialize the system. */
	result = sony_dvb_integration_Initialize (&(state->device));
	if (result != SONY_RESULT_OK) {
	pr_err ("Error: Unable to initialise the driver. (status=%d, %s)\r\n", result, FormatResult (result));
	return ;
	}


	result = sony_dvb_demod_SetConfig (&(state->demod), DEMOD_CONFIG_IFAGCNEG, 1);
	if (result != SONY_RESULT_OK) {
	pr_err ("Error: Unable to configure IFAGCNEG. (status=%d, %s)\r\n", result, FormatResult (result));
	return ;
	}
#if 0 //def TUNER_SPECTRUM_INV
        result = sony_dvb_demod_SetConfig (&(state->demod), DEMOD_CONFIG_SPECTRUM_INV, 1);
        if (result != SONY_RESULT_OK) {
            pr_err ("Error: Unable to configure SPECTRUM_INV. (status=%d, %s)\r\n", result, FormatResult (result));
            return -1;
        }
#endif

	sony_dvb_demod_SetConfig(&(state->demod), DEMOD_CONFIG_LATCH_ON_POSEDGE, 0);    //  1
#ifdef TUNER_RFLVLMON_DISABLE
	result = sony_dvb_demod_SetConfig (&(state->demod), DEMOD_CONFIG_RFLVMON_ENABLE, 0);
	if (result != SONY_RESULT_OK) {
	pr_err ("Error: Unable to configure RFLVMON_ENABLE. (status=%d, %s)\r\n", result, FormatResult (result));
	return ;
}
#endif
	state->demod.is_ascot =DVB_TUNER_UNKNOWN;
	/* Sony CXD2831 tuner setup. Set to 1 ONLY if Sony CXD2831 tuner is connected. */
#ifdef TUNER_SONY_ASCOT
	state->demod.is_ascot = DVB_TUNER_ASCOT2D;
#endif	

}
uint32_t opfreq;
uint8_t opbwMHz;

sony_result_t CXD2831_CDX2834_Scan(struct cdx2834_state *state,uint32_t startFreqkHz,uint8_t bwMHz)
{
  	unsigned int modulation_mode=state->mode;
	sony_dvb_tune_params_t scanParams;
	sony_result_t result = SONY_RESULT_OK;
   	 int i;
    
	/*------------------------------------------------------------------------------
	   Scan - DVB-T/T2 multiple standard scan.
	  ------------------------------------------------------------------------------*/
	printk("jay modulation_mode %d",modulation_mode);
	 //modulation_mode=0;
	 sony_atomic_set (&(state->device.cancel), 0);
	if(modulation_mode==0)
		{
	         //result = sony_dvb_integration_BlindTune (&device, SONY_DVB_SYSTEM_DVBT, startFreqkHz, bwMHz);
	        scanParams.system = SONY_DVB_SYSTEM_DVBT;   /* First attempted system is DVB-T. */
	 	 scanParams.centreFreqkHz = startFreqkHz/1000;
	  	 scanParams.bwMHz = bwMHz;
	  	 scanParams.tParams.usePresets = 0;
		scanParams.tParams.profile = SONY_DVBT_PROFILE_HP;
		}
	else  {
		 scanParams.system = SONY_DVB_SYSTEM_DVBT2;   /* First attempted system is DVB-T. */
	  	scanParams.centreFreqkHz = startFreqkHz/1000;
	   	scanParams.bwMHz = bwMHz;
		scanParams.t2Params.dataPLPId =0;
		scanParams.t2Params.tuneResult =0;
		}
    // TS Parallel, MSB DATA[7], TS_VALID active high, TS_SYNC active high, TS_CLK falling edge, TS_CLK output continue
    //sony_dvb_demod_SetConfig( state->device.pDemod, DEMOD_CONFIG_PARALLEL_SEL, 1); // TS output, 0: Serial, 1:Parallel (default)
    //sony_dvb_demod_SetConfig( state->device.pDemod, DEMOD_CONFIG_SER_DATA_ON_MSB, 1); // TS data output pin. 0: TSDATA0, 1: TSDATA7 (dafault)
    //sony_dvb_demod_SetConfig( state->device.pDemod, DEMOD_CONFIG_OUTPUT_SEL_MSB, 1); // TS Parallel/Serial output bit order. (Parallel): 0: MSB TSDATA[0] 1: MSB TSDATA[7] (Default). (Serial): 0: LSB first 1: MSB first (Default).
    //sony_dvb_demod_SetConfig( state->device.pDemod, DEMOD_CONFIG_TSVALID_ACTIVE_HI, 0); // TSVALID 0: active low. 1: active high (Default).
    //sony_dvb_demod_SetConfig( state->device.pDemod, DEMOD_CONFIG_TSSYNC_ACTIVE_HI, 1); //TSSYNC 0: active low. 1: active high (Default). @note Not available in DVB-T2 mode.
    pr_dbg("Jay CXD2831_CDX2834_Scan startFreqkHz %d,bwMHz %d ,modulation_mode %d!!\n",scanParams.centreFreqkHz,scanParams.bwMHz,modulation_mode);
    sony_dvb_demod_SetConfig (&(state->demod), DEMOD_CONFIG_IFAGCNEG, 1);
    
   // sony_dvb_demod_SetConfig (&(state->demod), DEMOD_CONFIG_RFLVMON_ENABLE, 0);
    sony_dvb_demod_SetConfig( state->device.pDemod, DEMOD_CONFIG_TSERR_ACTIVE_HI, 0); // TSERR 0: active low. 1: active high (Default). @note Not available in DVB-T2 mode.
    sony_dvb_demod_SetConfig( state->device.pDemod, DEMOD_CONFIG_LATCH_ON_POSEDGE, 0); // TSCLK 0: Falling/Negative edge. 1: Rising/Positive edge (Default).
    //sony_dvb_demod_SetConfig( state->device.pDemod, DEMOD_CONFIG_TSCLK_CONT, 1); // TSCLK output 0: Gated (Default) 1: Continuous
    // Tuner IFAGC positive, Spectrum Normal, RF_AGC disable
    //sony_dvb_demod_SetConfig( &demod, DEMOD_CONFIG_IFAGCNEG, 0);// IFAGC sense 0: Positive IFAGC. 1: Inverted IFAGC (Default)
    //sony_dvb_demod_SetConfig( &demod, DEMOD_CONFIG_SPECTRUM_INV, 0); //Spectrum inversion 0: Normal (Default). 1: Inverted.
    //sony_dvb_demod_SetConfig( &demod, DEMOD_CONFIG_RFAGC_ENABLE, 0); // RF AGC PWM enable/disable, 0: Disable RF AGC PWM output (Default). 1: Enable RF AGC PWM output.
    
    //enable TS_ERROR output    
	result = sony_dvb_integration_Tune (&(state->device), &scanParams);
    result = sony_dvb_demod_TuneEnd(state->device.pDemod);

	result = sony_dvb_demod_TuneFinalize (state->device.pDemod, &scanParams, result);

    sony_i2c_CommonWriteOneRegister(&(state->demodI2C), 0x6C, 0x00, 0x00);
    sony_i2c_SetRegisterBits (&(state->demodI2C), 0x6C, 0xFF, 0x00, 0x01);
    sony_i2c_SetRegisterBits (&(state->demodI2C), 0x6C, 0x8B, 0x40, 0x40);
    
    //change IFAGC ADC range to 0.7V
    sony_i2c_CommonWriteOneRegister(&(state->demodI2C), 0x6C, 0x00, 0x00);
    sony_i2c_SetRegisterBits (&(state->demodI2C), 0x6C, 0x82, 0x00, 0x20);
    sony_dvb_demod_TuneFinalize (state->device.pDemod, &scanParams, result);

    //msleep(3000);

    #if 0
    if ((result == SONY_RESULT_ERROR_TIMEOUT) || (result == SONY_RESULT_OK)) {
        
        #define SONY_ASCOT2D_OFFSET_CUTOFF_HZ       50000
        int32_t offsetkHz = 0;
        uint32_t stepkHz = (SONY_ASCOT2D_OFFSET_CUTOFF_HZ + 500) / 1000;
    
        /* Monitor carrier offset. */
        result = sony_dvb_demod_monitor_CarrierOffset (state->device.pDemod, &offsetkHz);
        if (result != SONY_RESULT_OK) {
            printk ("Error: Unable to monitor T carrier offset. (status=%d, %s)\r\n", result, FormatResult (result));
            return -1;
        }
    
        printk ("DVB-T carrier offset detected. Centre=%lukHz, offset=%ldkHz, step=%lukHz.\r\n", scanParams.centreFreqkHz,
                offsetkHz, stepkHz);
    
        /* Carrier recovery loop locked (demod locked), compensate for the offset and retry tuning. */
        stepkHz = (stepkHz + 1) / 2;
        if ((uint32_t) abs (offsetkHz) > stepkHz) {
            /* Tuners have only a fixed frequency step size (stepkHz), */
            /* therefore we must query the tuner driver to get the actual */
            /* centre frequency set by the tuner. */
            scanParams.centreFreqkHz = (uint32_t) ((int32_t) state->device.pTuner->frequency + offsetkHz);
            scanParams.t2Params.tuneResult = 0; /* Clear result. */
    
            printk ("Re-tuning to compensate offset. New centre=%lukHz.\r\n", scanParams.centreFreqkHz);
            result = sony_dvb_integration_Tune (&(state->device), &scanParams);
        }
    }
    if (result != SONY_RESULT_OK) {
        printk ("Error: Unable to get TS lock DVB-T signal at %lukHz. (status=%d, %s)\r\n", scanParams.centreFreqkHz,
                result, FormatResult (result));
        return -1;
    }
    printk ("TS locked to DVB-T signal at %lukHz.\r\n", scanParams.centreFreqkHz);

    for(i=0;i<10;i++)
    {

            sony_dvb_demod_lock_result_t tsLock = DEMOD_LOCK_RESULT_NOTDETECT;
            result = sony_dvb_demod_monitor_TSLock (state->device.pDemod, &tsLock);
            if (result == SONY_RESULT_OK) {
                printk ("DVB-T TS lock=%d\r\n", tsLock);

            }
        SONY_SLEEP (1000);


    }
    #endif
	  return result;
}
static const char *FormatResult (sony_result_t result)
{
    char *pErrorName = "UNKNOWN";
    switch (result) {
    case SONY_RESULT_OK:
        pErrorName = "OK";
        break;
    case SONY_RESULT_ERROR_TIMEOUT:
        pErrorName = "ERROR_TIMEOUT";
        break;
    case SONY_RESULT_ERROR_UNLOCK:
        pErrorName = "ERROR_UNLOCK";
        break;
    case SONY_RESULT_ERROR_CANCEL:
        pErrorName = "ERROR_CANCEL";
        break;
    case SONY_RESULT_ERROR_ARG:
        pErrorName = "ERROR_ARG";
        break;
    case SONY_RESULT_ERROR_I2C:
        pErrorName = "ERROR_I2C";
        break;
    case SONY_RESULT_ERROR_SW_STATE:
        pErrorName = "ERROR_SW_STATE";
        break;
    case SONY_RESULT_ERROR_HW_STATE:
        pErrorName = "ERROR_HW_STATE";
        break;
    case SONY_RESULT_ERROR_RANGE:
        pErrorName = "ERROR_RANGE";
        break;
    case SONY_RESULT_ERROR_NOSUPPORT:
        pErrorName = "ERROR_NOSUPPORT";
        break;
    case SONY_RESULT_ERROR_OTHER:
        pErrorName = "ERROR_OTHER";
        break;
    default:
        pErrorName = "ERROR_UNKNOWN";
        break;
    }
    return pErrorName;
}

static const char *FormatSystem (sony_dvb_system_t sys)
{
    char *pName = "Unknown";
    switch (sys) {
    case SONY_DVB_SYSTEM_DVBC:
        pName = "DVB-C";
        break;
    case SONY_DVB_SYSTEM_DVBT:
        pName = "DVB-T";
        break;
    case SONY_DVB_SYSTEM_DVBT2:
        pName = "DVB-T2";
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        pName = "DVB-C2";
        break;
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        break;
    }
    return pName;
}

sony_result_t CXD2831_CDX2834_GetTPLock(struct cdx2834_state *state)
{	
	sony_result_t result = SONY_RESULT_OK;
	printk("CXD2831_CDX2834_GetTPLock\n");
	 //  result = WaitDemodLock(&device);sony_dvb_integration_WaitTSLock
	sony_dvb_demod_lock_result_t tsLock = DEMOD_LOCK_RESULT_NOTDETECT;
	if (state->mode == 1)
	{
		result = WaitDemodLock (&(state->device));
		printk("T2 wait demod lock\n");
	}
	else
	{
		printk("T2&T WaitTSLock\n");
       result = sony_dvb_integration_WaitTSLock (&(state->device));
	}
	   if (result != SONY_RESULT_OK) {
		   return SONY_RESULT_ERROR_UNLOCK;
	   }
	   
	  return SONY_RESULT_OK;
}

void demod_reset(struct cdx2834_state *state)
{
	gpio_request(state->config.reset_pin, "cdx2834:RESET");
    pr_dbg("the reset pin is %x\n",state->config.reset_pin);
    //state->config.reset_pin = 6;
    
	gpio_direction_output(state->config.reset_pin, 0);
    
    pr_dbg("Jay reset pin \n");
	msleep(600);
	gpio_direction_output(state->config.reset_pin, 1);
	msleep(200);
}


int  demod_init(struct cdx2834_state *state)
{
	demod_reset(state);						// need reset demod on our board		
	
	state->i2c = i2c_get_adapter(state->config.i2c_id);
	if (!state->i2c) {
		pr_error("cannot get i2c adaptor\n");
	}
	CXD2831_CDX2834_init(state);
				
	return 0;
}
int demod_check_locked(unsigned char* lock)
{
	if(CXD2831_CDX2834_GetTPLock(cdx2834)==SONY_RESULT_OK)
	{
		pr_dbg("demod_check_locked !!\n");
		*lock =1;
		return 1;
	}
	*lock =0;
	return 0;

}
int demod_connect(struct cdx2834_state *state,unsigned int freq_khz, unsigned char bandwidth)
{
	unsigned char wbMHz;

	switch(bandwidth)
		{
			case 0:
				wbMHz = 8;
				break;
			case 1: 
				wbMHz = 7;
				break;
			case 2: 
				wbMHz = 6;
				break;				
				
			default:
				wbMHz = 8;
				break;	
		}
	CXD2831_CDX2834_Scan(state,freq_khz,wbMHz);
	return 0;
}
int demod_disconnect(void)
{
	return 0;
}
int demod_get_signal_strength(struct cdx2834_state* state,unsigned int* strength)
{
	int32_t pSNR;
	uint32_t uiPostBer,uiPreBer;
	//SNR
	unsigned int modulation_mode=state->mode;
	if(modulation_mode==0){
			sony_dvb_demod_monitorT_PreViterbiBER(&(state->demod), &uiPreBer);
		
			sony_dvb_demod_monitorT_PreRSBER(&(state->demod), &uiPostBer);
			
			sony_dvb_demod_monitorT_SNR(&(state->demod), &pSNR);
		}
	else
	{
		sony_dvb_demod_monitorT2_SNR(&(state->demod), &pSNR);
	}
	if(pSNR<0)
		*strength =5;
	else
		*strength =pSNR/1000;
	return 0;
}
int demod_get_signal_quality(struct cdx2834_state *state,unsigned int* quality)
{
	uint8_t pQuality;
	unsigned int modulation_mode=state->mode;
	if(modulation_mode==0){
	sony_dvb_demod_monitorT_Quality(&(state->demod), &pQuality);
		}
	else
	{
		sony_dvb_demod_monitorT2_Quality(&(state->demod), &pQuality);
	}
		*quality =pQuality;
	
	return 0;
}
int demod_get_signal_errorate(struct cdx2834_state *state,unsigned int* errorrate)
{
	uint32_t uiPostBer,uiPreBer;
	sony_dvb_demod_monitorT_PreRSBER(&(state->demod), &uiPostBer);
	*errorrate = uiPostBer;
	return 0;
}

int demod_set_data_plp(struct cdx2834_state *state, uint8_t plp_id)
{
	sony_result_t ret;
	
	printk("Sony DVBT2 cdx2834 set plp, id=%d\n", plp_id);
	ret = sony_dvb_demodT2_SetPLPConfig(&(state->demod), 0x1, plp_id);
	if (ret != SONY_RESULT_OK) {
		printk("Sony DVBT2 cdx2834 set plp config failed, ret=0x%x\n", ret);
		return 1;
	}
	printk("Sony DVBT2 cdx2834 set plp config ok\n");
	return 0;
}

int demod_get_active_data_plp(struct cdx2834_state *state, sony_dvbt2_plp_t *plp_info)
{
	sony_result_t ret;
	
	ret = sony_dvb_demod_monitorT2_ActivePLP(&(state->demod), SONY_DVBT2_PLP_DATA, plp_info);
	if (ret != SONY_RESULT_OK) {
		printk("Sony DVBT2 get active data plp failed, ret=0x%x\n", ret);
		return 1;
	}

	return 0;
}

int demod_get_data_plps(struct cdx2834_state *state, uint8_t *plp_ids, uint8_t *plp_num)
{
	sony_result_t ret;
	int wait_time = 0;

	*plp_num = 0;
	for(;;) {
    	ret = sony_dvb_demod_monitorT2_DataPLPs(&(state->demod), plp_ids, plp_num);
        if (ret == SONY_RESULT_OK) {
            break;
        } else if (ret == SONY_RESULT_ERROR_HW_STATE) {
            if (wait_time >= DVB_DEMOD_TUNE_T2_L1POST_TIMEOUT) {
				printk("Sony DVBT2 get data plps timeout\n");
				return 0;
            } else {
                msleep (DEMOD_TUNE_POLL_INTERVAL);
                wait_time += DEMOD_TUNE_POLL_INTERVAL;
            }
        } else {
        	printk("Sony DVBT2 get data plps failed, ret=0x%x\n", ret);
           return 1;
        }
    }

	printk("Sony DVBT2 cdx2834 get data plps: num=%d\n", *plp_num);
	return 0;
}

int demod_deinit(struct cdx2834_state *state)
{
	if(state->i2c)
	{
		i2c_put_adapter(state->i2c);
	}

	return 0;
}



