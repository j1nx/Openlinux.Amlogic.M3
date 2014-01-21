#include "Silabs_L0_API.h"


extern unsigned int jiffies_to_msecs(const unsigned long j);

long system_time()
{

  return jiffies_to_msecs(jiffies);
}

void system_wait(int time)
{
	msleep(time);
}


unsigned int L0_WriteCommandBytes(unsigned char slaveaddress, unsigned char length, unsigned char *pucDataBuffer)
{		
		slaveaddress = 0x64;
		I2CWrite(slaveaddress,pucDataBuffer,length);
		return length;

}
unsigned int L0_ReadCommandBytes(unsigned char slaveaddress, unsigned char length, unsigned char *pucDataBuffer)
{
		slaveaddress = 0x64;
		I2CRead(slaveaddress,pucDataBuffer,length);
		return length;
}


extern int si2168_get_fe_config(struct si2168_fe_config *cfg);

extern unsigned int I2CWrite(unsigned char I2CSlaveAddr, unsigned char *data, unsigned char length)
 {
//	printk("I2CSlaveAddr is %d\n",I2CSlaveAddr);
//	printk("I2CSlaveAddr is %d,length is %d,data is %x\n",I2CSlaveAddr,length,data[0]);
   /* I2C write, please port this function*/
	int ret = 0;
//	unsigned char regbuf[1];			/*8 bytes reg addr, regbuf 1 byte*/
	struct i2c_msg msg;			/*construct 2 msgs, 1 for reg addr, 1 for reg value, send together*/
	memset(&msg, 0, sizeof(msg));

	/*write reg address*/
/*	msg[0].addr = (state->config.demod_addr);					
	msg[0].flags = 0;
	msg[0].buf = regbuf;
	msg[0].len = 1;*/


	/*write value*/
	msg.addr = I2CSlaveAddr;
	msg.flags = 0;  //I2C_M_NOSTART;	/*i2c_transfer will emit a stop flag, so we should send 2 msg together,
																// * and the second msg's flag=I2C_M_NOSTART, to get the right timing*/
	msg.buf = data;
	msg.len = length;
#if 0

	/*write reg address*/
	msg[0].addr = 0x80;					
	msg[0].flags = 0;
	msg[0].buf = 0x7;
	msg[0].len = 1;

	/*write value*/
	msg[1].addr = 0x80;
	msg[1].flags = I2C_M_NOSTART;	/*i2c_transfer will emit a stop flag, so we should send 2 msg together,
																 * and the second msg's flag=I2C_M_NOSTART, to get the right timing*/
	msg[1].buf = 0x8;
	msg[1].len = 1;

#endif

	struct	si2168_fe_config cfg;

	si2168_get_fe_config(&cfg);
	ret = i2c_transfer((struct i2c_adapter *)cfg.i2c_adapter, &msg, 1);
	if(ret<0){
		printk(" %s: writereg error, errno is %d \n", __FUNCTION__, ret);
		return 0;
	}
	else{
		//printk(" %s:write success, errno is %d \n", __FUNCTION__, ret);
		return 1;
	}
      return 1;
 }
 
extern unsigned int  I2CRead(unsigned char I2CSlaveAddr, unsigned char *data, unsigned char length)
 {
     /* I2C read, please port this function*/
//		printk("I2CSlaveAddr is %d,length is %d\n",I2CSlaveAddr,length);
//		printk("I2CSlaveAddr is %d\n",I2CSlaveAddr);
		 int nRetCode = 0;
		 struct i2c_msg msg[1];
		 
		 if(data == 0 || length == 0)
		 {
			 printk("si2168 read register parameter error !!\n");
			 return 0;
		 }
	 
		 //read real data 
		 memset(msg, 0, sizeof(msg));
		 msg[0].addr = I2CSlaveAddr;
		 msg[0].flags |=  I2C_M_RD;  //write  I2C_M_RD=0x01
		 msg[0].len = length;
		 msg[0].buf = data;


		struct	si2168_fe_config cfg;
		si2168_get_fe_config(&cfg);
		 
		 nRetCode = i2c_transfer((struct i2c_adapter *)cfg.i2c_adapter, msg, 1);
	 
		 if(nRetCode != 1)
		 {
			 printk("si2168_readregister reg failure!\n");
			 return 0;
		 }
        return 1;
 }


