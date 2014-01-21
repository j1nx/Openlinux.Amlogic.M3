/*
 * aml_pcm.c  --  ALSA PCM interface for the AML Meson SoC
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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/soundcard.h>
#include <linux/timer.h>
#include <linux/debugfs.h>
#include <linux/major.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

#include "aml_pcm_bt.h"
#include "aml_pcm_bt_hw.h"
#include "aml_platform.h"

//#define PCM_DEBUG
#ifdef PCM_DEBUG
#define pcm_debug(fmt, args...)  printk (fmt, ## args)
#else
#define pcm_debug(fmt, args...)
#endif

static const struct snd_pcm_hardware aml_pcm_hardware = {  
    .info           = SNDRV_PCM_INFO_INTERLEAVED |
                            SNDRV_PCM_INFO_BLOCK_TRANSFER |
                            SNDRV_PCM_INFO_PAUSE,

	.formats        = SNDRV_PCM_FMTBIT_S16_LE,
	.period_bytes_min	= 32,
	.period_bytes_max	= 8 * 1024,
	.periods_min		= 2,
	.periods_max		= 1024,
	.buffer_bytes_max	= 64 * 1024,

    .rate_min           = 8000,
    .rate_max           = 8000,
    .channels_min       = 1,
    .channels_max       = 1,
};

struct aml_pcm_runtime_data {
	spinlock_t			lock;

    dma_addr_t          buffer_start;
    unsigned int        buffer_size;

    unsigned int        buffer_offset;

    unsigned int        data_size;

    unsigned int        running;
    unsigned int        timer_period;
    unsigned int        peroid_elapsed;

    struct timer_list   timer;
    struct snd_pcm_substream *substream;
};

static unsigned int period_sizes[] = { 64, 128, 256, 512, 1024, 2048, 4096, 8192 };

static struct snd_pcm_hw_constraint_list hw_constraints_period_sizes = {
	.count = ARRAY_SIZE(period_sizes),
	.list = period_sizes,
	.mask = 0
};

unsigned int aml_pcm_playback_buffer_addr = 0;
unsigned int aml_pcm_playback_buffer_size = 0;
unsigned int aml_pcm_capture_buffer_addr = 0;
unsigned int aml_pcm_capture_buffer_size = 0;

unsigned int aml_pcm_playback_phy_buffer_addr = 0;
unsigned int aml_pcm_playback_phy_buffer_size = 0;
unsigned int aml_pcm_capture_phy_buffer_addr = 0;
unsigned int aml_pcm_capture_phy_buffer_size = 0;

static void aml_pcm_config_tx(u32 addr, u32 size)
{
    pcm_debug(KERN_DEBUG "%s addr: 0x%08x size: 0x%x\n", __FUNCTION__, addr, size);
    pcm_out_set_buf(addr, size);
}

static void aml_pcm_config_rx(u32 addr, u32 size)
{
    pcm_debug(KERN_DEBUG "%s addr: 0x%08x size: 0x%x\n", __FUNCTION__, addr, size);
    pcm_in_set_buf(addr, size);
}

static void aml_pcm_start_tx(void)
{
    pcm_debug(KERN_DEBUG "%s", __FUNCTION__);
    pcm_out_enable(1);
}

static void aml_pcm_start_rx(void)
{
    pcm_debug(KERN_DEBUG "%s", __FUNCTION__);
    pcm_in_enable(1);
}

static void aml_pcm_stop_tx(void)
{
    pcm_debug(KERN_DEBUG "%s", __FUNCTION__);
    pcm_out_enable(0);
}

static void aml_pcm_stop_rx(void)
{
    pcm_debug(KERN_DEBUG "%s", __FUNCTION__);
    pcm_in_enable(0);
}

static unsigned int aml_pcm_offset_tx(struct aml_pcm_runtime_data *prtd)
{
    unsigned int value = 0;
    signed int diff = 0;

    value = pcm_out_rd_ptr();
    diff = value - prtd->buffer_start;
    if (diff < 0)
        diff = 0;
    else if (diff >= prtd->buffer_size)
        diff = prtd->buffer_size;

    //pcm_debug(KERN_DEBUG "%s value: 0x%08x offset: 0x%08x\n", __FUNCTION__, value, diff);
    return (unsigned int)diff;
}

static unsigned int aml_pcm_offset_rx(struct aml_pcm_runtime_data *prtd)
{
    unsigned int value = 0;
    signed int diff = 0;

    value = pcm_in_wr_ptr();
    diff = value - prtd->buffer_start;
    if (diff < 0)
        diff = 0;
    else if (diff >= prtd->buffer_size)
        diff = prtd->buffer_size;

    //pcm_debug(KERN_DEBUG "%s value: 0x%08x offset: 0x%08x\n", __FUNCTION__, value, diff);
    return (unsigned int)diff;
}

static void aml_pcm_timer_update(struct aml_pcm_runtime_data *prtd)
{
    struct snd_pcm_substream *substream = prtd->substream;
    struct snd_pcm_runtime *runtime = substream->runtime;
    unsigned int offset = 0;
    unsigned int size = 0;

    if (prtd->running && snd_pcm_running(substream)) {
    	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
    		offset = aml_pcm_offset_tx(prtd);
            if (offset < prtd->buffer_offset) {
                size = prtd->buffer_size + offset - prtd->buffer_offset;
            } else {
                size = offset - prtd->buffer_offset;
            }
    	} else {
            int rx_overflow = 0;
    		offset = aml_pcm_offset_rx(prtd);
            if (offset < prtd->buffer_offset) {
                size = prtd->buffer_size + offset - prtd->buffer_offset;
            } else {
                size = offset - prtd->buffer_offset;
            }
            rx_overflow = pcm_in_fifo_int() & (1 << 2);
            if (rx_overflow) {
                printk(KERN_WARNING "%s AUDIN_FIFO overflow !!\n", __FUNCTION__);
            }
    	}
    }

    prtd->buffer_offset = offset;
    prtd->data_size += size;
    if (prtd->data_size >= frames_to_bytes(runtime, runtime->period_size)) {
        prtd->peroid_elapsed++;
    }

    pcm_debug(KERN_DEBUG "%s buffer offset: %d data size: %d peroid size: %d peroid elapsed: %d\n",
                __FUNCTION__, prtd->buffer_offset, prtd->data_size, frames_to_bytes(runtime, runtime->period_size), prtd->peroid_elapsed);
}

static void aml_pcm_timer_rearm(struct aml_pcm_runtime_data *prtd)
{
    prtd->timer.expires = jiffies + prtd->timer_period;
	add_timer(&prtd->timer);
}

static int aml_pcm_timer_start(struct aml_pcm_runtime_data *prtd)
{
    pcm_debug(KERN_DEBUG "%s\n", __FUNCTION__);
	spin_lock(&prtd->lock);
	aml_pcm_timer_rearm(prtd);
    prtd->running = 1;
	spin_unlock(&prtd->lock);
	return 0;
}

static int aml_pcm_timer_stop(struct aml_pcm_runtime_data *prtd)
{
    pcm_debug(KERN_DEBUG "%s\n", __FUNCTION__);
	spin_lock(&prtd->lock);
    prtd->running = 0;
	del_timer(&prtd->timer);
	spin_unlock(&prtd->lock);
	return 0;
}


static void aml_pcm_timer_callback(unsigned long data)
{
    struct snd_pcm_substream *substream = (struct snd_pcm_substream *)data;
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct aml_pcm_runtime_data *prtd = runtime->private_data;
	unsigned long flags;
	unsigned int elapsed = 0;
    unsigned int datasize = 0;
	
	spin_lock_irqsave(&prtd->lock, flags);
    aml_pcm_timer_update(prtd);
    aml_pcm_timer_rearm(prtd);
	elapsed = prtd->peroid_elapsed;
    datasize = prtd->data_size;
    if (elapsed) {
        prtd->peroid_elapsed--;
        prtd->data_size -= frames_to_bytes(runtime, runtime->period_size);
    }
	spin_unlock_irqrestore(&prtd->lock, flags);
	if (elapsed) {
        if (elapsed > 1) {
            printk(KERN_WARNING "PCM timer callback not fast enough (elapsed periods: %d data_bytes: %d period_bytes: %d)!",
                    elapsed, datasize, frames_to_bytes(runtime, runtime->period_size));
        }
		snd_pcm_period_elapsed(prtd->substream);
    }
}

static int aml_pcm_timer_create(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct aml_pcm_runtime_data *prtd = runtime->private_data;

    pcm_debug(KERN_DEBUG "%s\n", __FUNCTION__);
	init_timer(&prtd->timer);
    prtd->timer_period = 1;
	prtd->timer.data = (unsigned long)substream;
	prtd->timer.function = aml_pcm_timer_callback;
    prtd->running = 0;
	return 0;
}

static int aml_pcm_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct aml_pcm_runtime_data *prtd = runtime->private_data;
	size_t size = params_buffer_bytes(params);
    int ret = 0;

    printk("enter %s stream: %s rate: %d format: %d size: %d\n",
                __FUNCTION__, (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) ? "playback" : "capture", params_rate(params), params_format(params), size);

	ret = snd_pcm_lib_malloc_pages(substream, size);
    if (ret < 0) {
        printk(KERN_ERR "%s snd_pcm_lib_malloc_pages return: %d\n", __FUNCTION__, ret);
    } else {
        prtd->buffer_start = runtime->dma_addr;
        prtd->buffer_size = runtime->dma_bytes;
        pcm_debug(KERN_DEBUG "%s dma_addr: 0x%08x dma_bytes: 0x%x\n", __FUNCTION__, runtime->dma_addr, runtime->dma_bytes);

        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
            aml_pcm_playback_phy_buffer_addr = runtime->dma_addr;
            aml_pcm_playback_phy_buffer_size = runtime->dma_bytes;
        } else {
            aml_pcm_capture_phy_buffer_addr = runtime->dma_addr;
            aml_pcm_capture_phy_buffer_size = runtime->dma_bytes;
        }
    }

    return ret;
}

static int aml_pcm_hw_free(struct snd_pcm_substream *substream)
{
    pcm_debug(KERN_DEBUG "enter %s\n", __FUNCTION__);
	snd_pcm_lib_free_pages(substream);

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        aml_pcm_playback_phy_buffer_addr = 0;
        aml_pcm_playback_phy_buffer_size = 0;
    } else {
        aml_pcm_capture_phy_buffer_addr = 0;
        aml_pcm_capture_phy_buffer_size = 0;
    }

    return 0;
}

static int aml_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct aml_pcm_runtime_data *prtd = runtime->private_data;

	printk(KERN_INFO "enter %s dma_addr: 0x%08x dma_bytes: 0x%x\n", __FUNCTION__, runtime->dma_addr, runtime->dma_bytes);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        pcm_debug(KERN_DEBUG "%s playback stream buffer start: 0x%08x size: 0x%x\n", __FUNCTION__, prtd->buffer_start, prtd->buffer_size);
        aml_pcm_config_tx(prtd->buffer_start, prtd->buffer_size);
        aml_pcm_playback_buffer_addr = (unsigned int)runtime->dma_area;
        aml_pcm_playback_buffer_size = runtime->dma_bytes;
	} else {
	    pcm_debug(KERN_DEBUG "%s capture stream buffer start: 0x%08x size: 0x%x\n", __FUNCTION__, prtd->buffer_start, prtd->buffer_size);
        aml_pcm_config_rx(prtd->buffer_start, prtd->buffer_size);
        aml_pcm_capture_buffer_addr = (unsigned int)runtime->dma_area;
        aml_pcm_capture_buffer_size = runtime->dma_bytes;
	}

    memset(runtime->dma_area, 0, runtime->dma_bytes);
    prtd->buffer_offset = 0;
    prtd->data_size = 0;
    prtd->peroid_elapsed = 0;

	return 0;
}

static int aml_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct aml_pcm_runtime_data *prtd = runtime->private_data;
	int ret = 0;

	printk(KERN_DEBUG "enter %s cmd: %d\n", __FUNCTION__, cmd);
	switch (cmd) {
    	case SNDRV_PCM_TRIGGER_START:
        case SNDRV_PCM_TRIGGER_RESUME:
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
    		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    			aml_pcm_start_tx();
    		else
    			aml_pcm_start_rx();
            aml_pcm_timer_start(prtd);
    		break;
    	case SNDRV_PCM_TRIGGER_STOP:
    	case SNDRV_PCM_TRIGGER_SUSPEND:
    	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            aml_pcm_timer_stop(prtd);
    		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    			aml_pcm_stop_tx();
    		else
    			aml_pcm_stop_rx();
    		break;
    	default:
    		ret = -EINVAL;
	}

	return ret;
}

static snd_pcm_uframes_t aml_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
    struct aml_pcm_runtime_data *prtd = runtime->private_data;
    snd_pcm_uframes_t frames;

	pcm_debug(KERN_DEBUG "enter %s\n", __FUNCTION__);
    //spin_lock(&prtd->lock);
    aml_pcm_timer_update(prtd);
    frames = bytes_to_frames(runtime, (ssize_t)prtd->buffer_offset);
    //spin_lock(&prtd->lock);

	return frames;
}

static int aml_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct aml_pcm_runtime_data *prtd;
	int ret;

    printk(KERN_INFO "enter %s type: %d\n", __FUNCTION__, substream->stream);
	snd_soc_set_runtime_hwparams(substream, &aml_pcm_hardware);

    /* Ensure that peroid size is a multiple of 32bytes */
	ret = snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, &hw_constraints_period_sizes);
	if (ret < 0) {
		printk(KERN_ERR "set period bytes constraint error\n");
		goto out;
	}

	/* Ensure that buffer size is a multiple of period size */
	ret = snd_pcm_hw_constraint_integer(runtime,
					    SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0) {
        printk(KERN_ERR "set periods constraint error\n");
		goto out;
    }

	prtd = kzalloc(sizeof(*prtd), GFP_KERNEL);
	if (prtd == NULL) {
        printk(KERN_ERR "out of memory\n");
		ret = -ENOMEM;
		goto out;
	}

    runtime->private_data = prtd;
    aml_pcm_timer_create(substream);
    prtd->substream = substream;
	spin_lock_init(&prtd->lock);

    return 0;
out:
	return ret;
}

static int aml_pcm_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
    struct aml_pcm_runtime_data *prtd = runtime->private_data;
    
    printk(KERN_INFO "enter %s type: %d\n", __FUNCTION__, substream->stream);
    if (prtd)
	    kfree(runtime->private_data);
	return 0;
}

static int aml_pcm_copy_playback(struct snd_pcm_runtime *runtime, int channel,
    snd_pcm_uframes_t pos,
    void __user *buf, snd_pcm_uframes_t count)
{
    struct aml_pcm_runtime_data *prtd = runtime->private_data;
    unsigned char* hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
    unsigned int wrptr = 0;
    int ret = 0;

    pcm_debug(KERN_DEBUG "enter %s channel: %d pos: %ld count: %ld\n", __FUNCTION__, channel, pos, count);

	if (copy_from_user(hwbuf, buf, frames_to_bytes(runtime, count))) {
        printk(KERN_ERR "%s copy from user failed!\n", __FUNCTION__);
		return -EFAULT;
    } else {
        wrptr = prtd->buffer_start + frames_to_bytes(runtime, pos) + frames_to_bytes(runtime, count);
        if (wrptr >= (prtd->buffer_start + prtd->buffer_size)) {
            wrptr = prtd->buffer_start + prtd->buffer_size;
        }
        pcm_out_set_wr_ptr(wrptr);
    }

    return ret;
}

static int aml_pcm_copy_capture(struct snd_pcm_runtime *runtime, int channel,
    snd_pcm_uframes_t pos,
    void __user *buf, snd_pcm_uframes_t count)
{
    struct aml_pcm_runtime_data *prtd = runtime->private_data;
    signed short *hwbuf = (signed short*)(runtime->dma_area + frames_to_bytes(runtime, pos));
    unsigned int rdptr = 0;
    int ret = 0;

    pcm_debug(KERN_DEBUG "enter %s channel: %d pos: %ld count: %ld\n", __FUNCTION__, channel, pos, count);

	if (copy_to_user(buf, hwbuf, frames_to_bytes(runtime, count))) {
        printk(KERN_ERR "%s copy to user failed!\n", __FUNCTION__);
		return -EFAULT;
    } else {
        //memset(hwbuf, 0xff, frames_to_bytes(runtime, count));
        rdptr = prtd->buffer_start + frames_to_bytes(runtime, pos) + frames_to_bytes(runtime, count);
        if (rdptr >= (prtd->buffer_start + prtd->buffer_size)) {
            rdptr = prtd->buffer_start + prtd->buffer_size;
        }
        pcm_in_set_rd_ptr(rdptr);
    }
    return ret;
}

static int aml_pcm_copy(struct snd_pcm_substream *substream, int channel,
    snd_pcm_uframes_t pos,
    void __user *buf, snd_pcm_uframes_t count)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    int ret = 0;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        ret = aml_pcm_copy_playback(runtime, channel, pos, buf, count);
    } else {
        ret = aml_pcm_copy_capture(runtime, channel, pos, buf, count);
    }

    return ret;
}

static int aml_pcm_silence(struct snd_pcm_substream *substream, int channel, 
    snd_pcm_uframes_t pos, snd_pcm_uframes_t count)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    unsigned char* ppos = NULL;
    ssize_t n;

    pcm_debug(KERN_DEBUG "enter %s\n", __FUNCTION__);
    n = frames_to_bytes(runtime, count);
    ppos = runtime->dma_area + frames_to_bytes(runtime, pos);
    memset(ppos, 0, n);

    return 0;
}

static struct snd_pcm_ops aml_pcm_ops = {
	.open		= aml_pcm_open,
	.close		= aml_pcm_close,
	.ioctl		= snd_pcm_lib_ioctl,
	.hw_params	= aml_pcm_hw_params,
	.hw_free	= aml_pcm_hw_free,
	.prepare	= aml_pcm_prepare,
	.trigger	= aml_pcm_trigger,
	.pointer	= aml_pcm_pointer,
	.copy       = aml_pcm_copy,
	.silence    = aml_pcm_silence,
};

static u64 aml_pcm_dmamask = DMA_BIT_MASK(32);

static int aml_pcm_preallocate_dma_buffer(struct snd_pcm *pcm,
	int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = aml_pcm_hardware.buffer_bytes_max;

    printk(KERN_DEBUG "enter %s stream: %d\n", __FUNCTION__, stream);
    
	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->area = dma_alloc_coherent(pcm->card->dev, size,
					   &buf->addr, GFP_KERNEL);
	if (!buf->area) {
        printk(KERN_ERR "%s dma_alloc_coherent failed (size: %d)!\n", __FUNCTION__, size);
		return -ENOMEM;
    }

	buf->bytes = size;
    printk(KERN_INFO "%s allcoate buf->area: %p buf->addr: 0x%x buf->bytes: %d\n",
                __FUNCTION__, buf->area, buf->addr, buf->bytes);
	return 0;
}

static int aml_pcm_new(struct snd_card *card, struct snd_soc_dai *dai,
		 struct snd_pcm *pcm)
{
	int ret = 0;

    printk("enter %s dai->name: %s dai->id: %d\n", __FUNCTION__, dai->name, dai->id);
    //pcm_debug(KERN_DEBUG "enter %s\n", __FUNCTION__);
	if (!card->dev->dma_mask)
		card->dev->dma_mask = &aml_pcm_dmamask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

	if (dai->playback.channels_min) {
		ret = aml_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto out;
	}

	if (dai->capture.channels_min) {
		ret = aml_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto out;
	}

out:
	return ret;
}

static void aml_pcm_free(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

    printk("enter %s\n", __FUNCTION__);
    //pcm_debug(KERN_DEBUG "enter %s\n", __FUNCTION__);
	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		dma_free_coherent(pcm->card->dev, buf->bytes,
				      buf->area, buf->addr);
		buf->area = NULL;
	}
}

struct aml_audio_interface aml_pcm_interface = {
    .id = AML_AUDIO_PCM,
    .name = "PCM",
    .pcm_ops = &aml_pcm_ops,
    .pcm_new = aml_pcm_new,
    .pcm_free =  aml_pcm_free,
};
