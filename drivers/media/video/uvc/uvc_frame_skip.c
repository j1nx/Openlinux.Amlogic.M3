#include <linux/uvcvideo.h>

static struct uvc_frame_skip frame_skip_tbl[] = {
	{
		.idVendor	= 0x1871,
		.idProduct	= 0x0306,
		.skip_num	= 15,
	}, 
};

int get_skip_num(__u16 idVendor, __u16 idProduct)
{
	int num = sizeof(frame_skip_tbl) / sizeof(struct uvc_frame_skip);
	int i, ret = 0;
	
	for (i = 0; i < num; i++) {
		if (frame_skip_tbl[i].idVendor == idVendor &&
				frame_skip_tbl[i].idProduct == idProduct) {
			ret = frame_skip_tbl[i].skip_num;
			break;
		}
	}
	
	return ret;
}