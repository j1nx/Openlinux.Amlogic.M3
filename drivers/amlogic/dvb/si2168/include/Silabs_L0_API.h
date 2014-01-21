
#ifndef    _Si2168_L0_API_H_
#define    _Si2168_L0_API_H_

/* _demod_defines_insertion_start */
#ifdef    DEMOD_Si2168
#ifndef   NO_SAT
 #define DEMOD_DVB_S_S2_DSS
#endif /* NO_SAT */
#endif /* DEMOD_Si2168 */
/* _demod_defines_insertion_point */

#include "Si2168_Commands.h"
#include "Si2168_Properties.h"
#include "Si2168_typedefs.h"
#include "Si2168_Commands_Prototypes.h"
#include "sifrontend.h"


void system_wait(int time);
long system_time(void);
unsigned int L0_Connect(L0_Context *WrapperI2C,CONNECTION_TYPE  connection_mode);
unsigned int L0_WriteCommandBytes(unsigned char slaveaddress, unsigned char length, unsigned char *pucDataBuffer);
unsigned int L0_ReadCommandBytes(unsigned char slaveaddress, unsigned char length, unsigned char *pucDataBuffer);
extern unsigned int I2CWrite(unsigned char I2CSlaveAddr, unsigned char *data, unsigned char length);
extern unsigned int  I2CRead(unsigned char I2CSlaveAddr, unsigned char *data, unsigned char length);




#endif /* _Si2168_L1_API_H_ */


