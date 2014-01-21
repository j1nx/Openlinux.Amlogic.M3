

#ifndef _CXD2831_CDX2834_API_H
#define _CXD2831_CDX2834_API_H
#include "sony_ascot2d.h"

struct cdx2834_fe_config {
	int                   i2c_id;
	int                 reset_pin;
	int                 demod_addr;
	int                 tuner_addr;
};


struct cdx2834_state {
	struct cdx2834_fe_config config;
	struct i2c_adapter *i2c;
	u32                 freq;
	u32 	bandwidth;
	u32 	mode;
	struct dvb_frontend fe;
	sony_dvb_integration_t device;
	struct sony_dvb_tuner_t tuner;
	sony_i2c_t demodI2C;
	sony_i2c_t tunerI2C;
	sony_dvb_demod_t demod;
	sony_ascot2d_t ascot2d;
};

/*****************************************************************************
 * Error code definitions
 *****************************************************************************/

#define DEMOD_ERROR_BASE                          (0x80000000+0x1000)
#define DEMOD_SUCCESS                             (AM_SUCCESS)
#define DEMOD_ERR_NO_MEMORY                       (DEMOD_ERROR_BASE+1)
#define DEMOD_ERR_DEVICE_NOT_EXIST                (DEMOD_ERROR_BASE+2)
#define DEMOD_ERR_HARDWARE_ERROR                  (DEMOD_ERROR_BASE+3)
#define DEMOD_ERR_BAD_PARAMETER                   (DEMOD_ERROR_BASE+4)
#define DEMOD_ERR_NOT_INITLIZED                   (DEMOD_ERROR_BASE+5)
#define DEMOD_ERR_DESTROY_FAILED                  (DEMOD_ERROR_BASE+6)
#define DEMOD_ERR_FEATURE_NOT_SUPPORT             (DEMOD_ERROR_BASE+7)
#define DEMOD_ERR_OTHER                           (DEMOD_ERROR_BASE+8)


/*****************************************************************************
 * Function prototypes	
 *****************************************************************************/

int  demod_init(struct cdx2834_state *state);
int demod_check_locked(unsigned char* lock);
int demod_connect(struct cdx2834_state *state,unsigned int freq_khz, unsigned char bandwidth);

int demod_disconnect(void);
int demod_get_signal_strength(struct cdx2834_state *state,unsigned int* strength);
int demod_get_signal_quality(struct cdx2834_state *state,unsigned int* quality);
int demod_get_signal_errorate(struct cdx2834_state *state,unsigned int* errorrate);
int demod_control(unsigned int cmd, void* param);
int demod_destroy(void* para);

int demod_deinit(struct cdx2834_state *state);
int demod_set_data_plp(struct cdx2834_state *state, uint8_t plp_id);
int demod_get_active_data_plp(struct cdx2834_state *state, sony_dvbt2_plp_t *plp_info);
int demod_get_data_plps(struct cdx2834_state *state, uint8_t *plp_ids, uint8_t *plp_num);

#endif
