/*
 * aml_platform.c  --  ALSA audio platform interface for the AML Meson SoC
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
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

#include "aml_pcm_bt.h"
#include "aml_pcm.h"
#include "aml_platform.h"
#include <mach/am_regs.h>
#include <mach/pinmux.h>
#include <linux/debugfs.h>

#ifndef ARRY_SIZE
#define ARRY_SIZE(A)    (sizeof(A) /sizeof(A[0]))
#endif
extern unsigned audioin_mode;

static LIST_HEAD(stream_list);
static DEFINE_SPINLOCK(platform_lock);
    
struct aml_platform_stream{
    struct list_head list;
    struct aml_audio_interface *interface;
    struct snd_pcm_substream *substream;
};

static struct aml_audio_interface *audio_interfaces[] = {
    &aml_i2s_interface,
    &aml_pcm_interface,
};

static inline struct aml_audio_interface *find_audio_interface(int id)
{
    struct aml_audio_interface *interface = NULL;
    int i = 0;

    for (i=0; i<ARRAY_SIZE(audio_interfaces); i++) {
        if (audio_interfaces[i]->id == id) {
            interface = audio_interfaces[i];
            break;
        }
    }

    return interface;
}

static inline struct aml_platform_stream *find_platform_stream(struct snd_pcm_substream *substream)
{
    struct aml_platform_stream *plat_stream = NULL;
    struct list_head *entry = NULL;

    list_for_each(entry, &stream_list) {
        plat_stream = list_entry(entry, struct aml_platform_stream, list);
        if (plat_stream->substream == substream) {
            return plat_stream;
        }
    }

    return NULL;
}

static void dump_platform_stream(void)
{
    struct aml_platform_stream *plat_stream = NULL;
    struct list_head *entry = NULL;
    int n = 0;

    printk("\naml-platform streams:\n");

    list_for_each(entry, &stream_list) {
        plat_stream = list_entry(entry, struct aml_platform_stream, list);
        printk(KERN_INFO "substream#%d ptr: %p type: %s name: %s interface: %s\n",
                        n,
                        plat_stream->substream,
                        (plat_stream->substream->stream == SNDRV_PCM_STREAM_PLAYBACK) ? "playback" : "capture",
                        plat_stream->substream->name,
                        plat_stream->interface->name);
        n++;
    }
}

static int aml_platform_open(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	//struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_soc_dai_link *machine = rtd->dai;
	struct snd_soc_dai *cpu_dai = machine->cpu_dai;
	struct snd_soc_dai *codec_dai = machine->codec_dai;
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    printk(KERN_INFO "enter %s (codec_dai: %s %d cpu_dai: %s %d)\n",
                    __FUNCTION__, codec_dai->name, codec_dai->id, cpu_dai->name, cpu_dai->id);

    aud_interface = find_audio_interface(cpu_dai->id);
    if (unlikely(NULL == aud_interface)) {
        printk(KERN_ERR "aml-platform: no such audio interface!");
        ret = -ENODEV;
        goto out;
    }

    BUG_ON(aud_interface->pcm_ops->open == NULL);

	plat_stream = kzalloc(sizeof(struct aml_platform_stream), GFP_KERNEL);
	if (unlikely(plat_stream == NULL)) {
        printk(KERN_ERR "aml-platform: out of memory!");
		ret = -ENOMEM;
		goto out;
	}

    ret = aud_interface->pcm_ops->open(substream);
    if (ret >= 0) {
        INIT_LIST_HEAD(&plat_stream->list);
        plat_stream->substream = substream;
        plat_stream->interface = aud_interface;

        spin_lock(&platform_lock);
        list_add_tail(&plat_stream->list, &stream_list);
        spin_unlock(&platform_lock);

        dump_platform_stream();
    } else {
        printk(KERN_ERR "aml-platform: open pcm substream failed ret: %d!", ret);
    }

out:
    return ret;
}

static int aml_platform_close(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	//struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_soc_dai_link *machine = rtd->dai;
	struct snd_soc_dai *cpu_dai = machine->cpu_dai;
	struct snd_soc_dai *codec_dai = machine->codec_dai;
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    printk(KERN_INFO "enter %s (codec_dai: %s %d cpu_dai: %s %d)\n",
                    __FUNCTION__, codec_dai->name, codec_dai->id, cpu_dai->name, cpu_dai->id);

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        BUG_ON(aud_interface->pcm_ops->close == NULL);

        ret = aud_interface->pcm_ops->close(substream);
        if (ret >= 0) {
            spin_lock(&platform_lock);
            list_del(&plat_stream->list);
            spin_unlock(&platform_lock);
            kfree(plat_stream);
        } else {
            printk(KERN_ERR "aml-platform: close pcm substream failed ret: %d!", ret);
        }
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
        ret = -EINVAL;
    }

    return ret;
}

static int aml_platform_ioctl(struct snd_pcm_substream * substream,
                unsigned int cmd, void *arg)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        if (aud_interface->pcm_ops->ioctl) {
            ret = aud_interface->pcm_ops->ioctl(substream, cmd, arg);
        } else {
            ret = snd_pcm_lib_ioctl(substream, cmd, arg);
        }
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
        ret = -EINVAL;
    }

    return ret;
}

static int aml_platform_hw_params(struct snd_pcm_substream *substream,
		 struct snd_pcm_hw_params *params)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        BUG_ON(aud_interface->pcm_ops->hw_params == NULL);

        ret = aud_interface->pcm_ops->hw_params(substream, params);
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
        ret = -EINVAL;
    }

    return ret;
}

static int aml_platform_hw_free(struct snd_pcm_substream *substream)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        BUG_ON(aud_interface->pcm_ops->hw_free == NULL);

        ret = aud_interface->pcm_ops->hw_free(substream);
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
        ret = -EINVAL;
    }

    return ret;
}

static int aml_platform_prepare(struct snd_pcm_substream *substream)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        BUG_ON(aud_interface->pcm_ops->prepare == NULL);

        ret = aud_interface->pcm_ops->prepare(substream);
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
        ret = -EINVAL;
    }

    return ret;
}

static int aml_platform_trigger(struct snd_pcm_substream *substream, int cmd)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        BUG_ON(aud_interface->pcm_ops->trigger == NULL);

        ret = aud_interface->pcm_ops->trigger(substream, cmd);
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
        ret = -EINVAL;
    }

    return ret;
}

static snd_pcm_uframes_t aml_platform_pointer(struct snd_pcm_substream *substream)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    snd_pcm_uframes_t ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        BUG_ON(aud_interface->pcm_ops->pointer == NULL);

        ret = aud_interface->pcm_ops->pointer(substream);
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
    }

    return ret;
}

static int aml_platform_copy(struct snd_pcm_substream *substream, int channel,
        	    snd_pcm_uframes_t pos,
        	    void __user *buf, snd_pcm_uframes_t count)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        BUG_ON(aud_interface->pcm_ops->copy == NULL);

        ret = aud_interface->pcm_ops->copy(substream, channel, pos, buf, count);
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
        ret = -EINVAL;
    }

    return ret;
}

static int aml_platform_silence(struct snd_pcm_substream *substream, int channel, 
                snd_pcm_uframes_t pos, snd_pcm_uframes_t count)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    unsigned char* ppos = NULL;
    ssize_t n = 0;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        if (aud_interface->pcm_ops->silence) {
            ret = aud_interface->pcm_ops->silence(substream, channel, pos, count);
        } else {
            n = frames_to_bytes(runtime, count);
            ppos = runtime->dma_area + frames_to_bytes(runtime, pos);
            memset(ppos, 0, n);
        }
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
        ret = -EINVAL;
    }

    return ret;
}

static struct page *aml_platform_page(struct snd_pcm_substream *substream,
                        unsigned long offset)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    struct page *ret = NULL;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        if (aud_interface->pcm_ops->page) {
            ret = aud_interface->pcm_ops->page(substream, offset);
        }
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
    }

    return ret;
}

static int aml_platform_mmap(struct snd_pcm_substream *substream, struct vm_area_struct *vma)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        if (aud_interface->pcm_ops->mmap) {
            ret = aud_interface->pcm_ops->mmap(substream, vma);
        }
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
    }

    return ret;
}

static int aml_platform_ack(struct snd_pcm_substream *substream)
{
    struct aml_audio_interface *aud_interface = NULL;
    struct aml_platform_stream *plat_stream = NULL;
    int ret = 0;

    plat_stream = find_platform_stream(substream);
    if (likely(plat_stream != NULL)) {
        aud_interface = plat_stream->interface;
        if (aud_interface->pcm_ops->ack) {
            ret = aud_interface->pcm_ops->ack(substream);
        }
    } else {
        printk(KERN_ERR "aml-platform: substream %p invalid!", substream);
        dump_platform_stream();
    }

    return ret;
}
    
static struct snd_pcm_ops aml_platform_ops = {
	.open		= aml_platform_open,
	.close		= aml_platform_close,
	.ioctl		= aml_platform_ioctl,
	.hw_params	= aml_platform_hw_params,
	.hw_free	= aml_platform_hw_free,
	.prepare	= aml_platform_prepare,
	.trigger	= aml_platform_trigger,
	.pointer	= aml_platform_pointer,
	.copy       = aml_platform_copy,
	.silence    = aml_platform_silence,
	.page       = aml_platform_page,
	.mmap       = aml_platform_mmap,
	.ack        = aml_platform_ack,
};

static int aml_platform_pcm_new(struct snd_card *card, struct snd_soc_dai *dai,
		 struct snd_pcm *pcm)
{
    struct snd_soc_pcm_runtime *rtd = pcm->private_data;
    struct snd_soc_dai_link *machine = rtd->dai;
    struct snd_soc_dai *codec_dai = machine->codec_dai;
    struct snd_soc_dai *cpu_dai = machine->cpu_dai;
    struct aml_audio_interface *aud_interface = NULL;
	int ret = 0;

    printk(KERN_INFO "enter %s (codec_dai: %s %d cpu_dai: %s %d)\n",
                    __FUNCTION__, codec_dai->name, codec_dai->id, cpu_dai->name, cpu_dai->id);

    aud_interface = find_audio_interface(cpu_dai->id);
    if (unlikely(NULL == aud_interface)) {
        printk(KERN_ERR "aml-platform: no such audio interface!");
        ret = -ENODEV;
        goto out;
    }

    BUG_ON(aud_interface->pcm_new == NULL);
    ret = aud_interface->pcm_new(card, dai, pcm);
out:
	return ret;
}

static void aml_platform_pcm_free(struct snd_pcm *pcm)
{
    struct snd_soc_pcm_runtime *rtd = pcm->private_data;
    struct snd_soc_dai_link *machine = rtd->dai;
    struct snd_soc_dai *codec_dai = machine->codec_dai;
    struct snd_soc_dai *cpu_dai = machine->cpu_dai;
    struct aml_audio_interface *aud_interface = NULL;

    printk(KERN_INFO "enter %s (codec_dai: %s %d cpu_dai: %s %d)\n",
                    __FUNCTION__, codec_dai->name, codec_dai->id, cpu_dai->name, cpu_dai->id);

    aud_interface = find_audio_interface(cpu_dai->id);
    if (unlikely(NULL == aud_interface)) {
        printk(KERN_ERR "aml-platform: no such audio interface!");
        return;
    }

    BUG_ON(aud_interface->pcm_free == NULL);
    aud_interface->pcm_free(pcm);

	return;
}

#ifdef CONFIG_PM
static int aml_platform_suspend(struct snd_soc_dai *dai)
{
    printk(KERN_INFO "enter %s (dai: %s id: %d)!\n",
            __FUNCTION__, dai->name, dai->id);

	return 0;
}

static int aml_platform_resume(struct snd_soc_dai *dai)
{
    printk(KERN_INFO "enter %s (dai: %s id: %d)!\n",
            __FUNCTION__, dai->name, dai->id);

	return 0;
}
#else
#define aml_platform_suspend	NULL
#define aml_platform_resume	NULL
#endif
#ifdef CONFIG_DEBUG_FS

static struct dentry *debugfs_root;
static struct dentry *debugfs_regs;
static struct dentry *debugfs_mems;

static int regs_open_file(struct inode *inode, struct file *file)
{
	return 0;
}

/**
 *	cat regs
 */
static ssize_t regs_read_file(struct file *file, char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	ssize_t ret;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
		
	ret = sprintf(buf, "Usage: \n"
										 "	echo base reg val >regs\t(set the register)\n"
										 "	echo base reg >regs\t(show the register)\n"
										 "	base -> c(cbus), x(aix), p(apb), h(ahb) \n"
									);
		
	if (ret >= 0)
		ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);
	kfree(buf);	
	
	return ret;
}

static int read_regs(char base, int reg)
{
	int val = 0;
	switch(base){
		case 'c':
			val = READ_CBUS_REG(reg);
			break;
		case 'x':
			val = READ_AXI_REG(reg);
			break;
		case 'p':
			val = READ_APB_REG(reg);
			break;
		case 'h':
			val = READ_AHB_REG(reg);
			break;
		default:
			break;
	};
	printk("\tReg %x = %x\n", reg, val);
	return val;
}

static void write_regs(char base, int reg, int val)
{
	switch(base){
		case 'c':
			WRITE_CBUS_REG(reg, val);
			break;
		case 'x':
			WRITE_AXI_REG(reg, val);
			break;
		case 'p':
			WRITE_APB_REG(reg, val);
			break;
		case 'h':
			WRITE_AHB_REG(reg, val);
			break;
		default:
			break;
	};
	printk("Write reg:%x = %x\n", reg, val);
}
static ssize_t regs_write_file(struct file *file,
		const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	int buf_size = 0;
	char *start = buf;
	unsigned long reg, value;
	char base;
	
	buf_size = min(count, (sizeof(buf)-1));
	
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;
	buf[buf_size] = 0;
	while (*start == ' ')
		start++;
		
	base = *start;
	start ++;
	if(!(base =='c' || base == 'x' || base == 'p' || base == 'h')){
		return -EINVAL;
	}
	
	while (*start == ' ')
		start++;
		
	reg = simple_strtoul(start, &start, 16);
	
	while (*start == ' ')
		start++;
		
	if (strict_strtoul(start, 16, &value))
	{
			read_regs(base, reg);
			return -EINVAL;
	}
	
	write_regs(base, reg, value);
	
	return buf_size;
}

static const struct file_operations regs_fops = {
	.open = regs_open_file,
	.read = regs_read_file,
	.write = regs_write_file,
};

static int mems_open_file(struct inode *inode, struct file *file)
{
	return 0;
}
static ssize_t mems_read_file(struct file *file, char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	ssize_t ret;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
		
	ret = sprintf(buf, "Usage: \n"
										 "	echo vmem >mems\t(read 64 bytes from vmem)\n"
										 "	echo vmem val >mems (write int value to vmem\n"
									);
		
	if (ret >= 0)
		ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);
	kfree(buf);	
	
	return ret;
}

static ssize_t mems_write_file(struct file *file,
		const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[256];
	int buf_size = 0;
	char *start = buf;
	unsigned long mem, value;
	int i=0;
	unsigned* addr = 0;
		
	buf_size = min(count, (sizeof(buf)-1));
	
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;
	buf[buf_size] = 0;
	
	while (*start == ' ')
		start++;
	
	mem = simple_strtoul(start, &start, 16);
	
	while (*start == ' ')
		start++;
		
	if (strict_strtoul(start, 16, &value))
	{
			addr = (unsigned*)mem;
			printk("%p: ", addr);
			for(i = 0; i< 8; i++){
				printk("%08x, ", addr[i]);
			}
			printk("\n");
			return -EINVAL;
	}
	addr = (unsigned*)mem;
	printk("%p: %08x\n", addr, *addr);
	*addr = value;
	printk("%p: %08x^\n", addr, *addr);
	
	return buf_size;
}
static const struct file_operations mems_fops={
	.open = mems_open_file,
	.read = mems_read_file,
	.write = mems_write_file,
};

static void aml_pcm_init_debugfs(void)
{
		debugfs_root = debugfs_create_dir("aml",NULL);
		if (IS_ERR(debugfs_root) || !debugfs_root) {
			printk("aml: Failed to create debugfs directory\n");
			debugfs_root = NULL;
		}
		
		debugfs_regs = debugfs_create_file("regs", 0644, debugfs_root, NULL, &regs_fops);
		if(!debugfs_regs){
			printk("aml: Failed to create debugfs file\n");
		}
		
		debugfs_mems = debugfs_create_file("mems", 0644, debugfs_root, NULL, &mems_fops);
		if(!debugfs_mems){
			printk("aml: Failed to create debugfs file\n");
		}
}
static void aml_pcm_cleanup_debugfs(void)
{
	debugfs_remove_recursive(debugfs_root);
}
#else
static void aml_pcm_init_debugfs()
{
}
static void aml_pcm_cleanup_debugfs()
{
}
#endif

struct snd_soc_platform aml_soc_platform = {
	.name		= "aml-audio",
	.pcm_ops 	= &aml_platform_ops,
	.pcm_new	= aml_platform_pcm_new,
	.pcm_free	= aml_platform_pcm_free,

	.suspend	= aml_platform_suspend,
	.resume		= aml_platform_resume,
};
static int  aml_soc_platform_init(void)
{
    int n = ARRY_SIZE(audio_interfaces);
    int i = 0;

    BUG_ON(n == 0);
    for (i=0; i<n; i++) {
        BUG_ON(audio_interfaces[i]->pcm_ops == NULL);
        BUG_ON(audio_interfaces[i]->pcm_new == NULL);
        BUG_ON(audio_interfaces[i]->pcm_free == NULL);
    }
#ifdef CONFIG_DEBUG_FS	
	aml_pcm_init_debugfs();
#endif	
    return snd_soc_register_platform(&aml_soc_platform);
}
static int __devinit aml_soc_platform_probe(struct platform_device *pdev)
{
	/* get audioin cfg data from board */
	if(pdev->dev.platform_data){
		audioin_mode = *(unsigned *)pdev->dev.platform_data;
		printk("AML soc audio in mode =============   %d \n",audioin_mode);
	}	
	return aml_soc_platform_init();
}

static int __devexit aml_soc_platform_remove(struct platform_device *pdev)
{
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}
static struct platform_driver aml_pcm_driver = {
	.driver = {
			.name = "aml-audio",
			.owner = THIS_MODULE,
	},

	.probe = aml_soc_platform_probe,
	.remove = __devexit_p(aml_soc_platform_remove),
};



static int __init aml_alsa_audio_init(void)
{
	aml_pcm_init_debugfs();
	return platform_driver_register(&aml_pcm_driver);
}

static void __exit aml_alsa_audio_exit(void)
{
    aml_pcm_cleanup_debugfs();
    platform_driver_unregister(&aml_pcm_driver);
}

module_init(aml_alsa_audio_init);


module_exit(aml_alsa_audio_exit);

MODULE_AUTHOR("Amlogic, Inc.");
MODULE_DESCRIPTION("Amlogic ASoC platform driver");
MODULE_LICENSE("GPL");

