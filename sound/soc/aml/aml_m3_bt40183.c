
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/jack.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>

#include "aml_dai.h"
#include "aml_platform.h"

#include "../codecs/bt40183.h"

static int bt40183_hw_params(struct snd_pcm_substream *substream,
                    struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
    int ret = 0;

	printk(KERN_DEBUG "%s: codec_dai=%p cpu_dai=%p rate: %d\n", __func__, codec_dai, cpu_dai, params_rate(params));

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_DSP_B|SND_SOC_DAIFMT_NB_IF|SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "%s: set codec dai fmt failed!\n", __func__);
		return ret;
    }

	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_DSP_B|SND_SOC_DAIFMT_NB_IF|SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "%s: set cpu dai fmt failed!\n", __func__);
		return ret;
    }

	/* set the codec system clock for DAC and ADC to xtal value */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, params_rate(params) * 256, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec dai system clock\n");
		return ret;
	}

	/* set the codec system clock for DAC and ADC to xtal value */
	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, params_rate(params) * 256, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu dai system clock\n");
		//return ret;
	}

	return 0;
}

static struct snd_soc_ops bt40183_ops = {
    .hw_params = bt40183_hw_params,
};


static int bt40183_codec_init(struct snd_soc_codec *codec)
{
    return 0;
}

static int bt40183_set_bias_level(struct snd_soc_card *card,
                    enum snd_soc_bias_level level)
{
    return 0;
}

static struct snd_soc_dai_link bt40183_dai_link[] = {
    [0] = {
    .name = "BT40183",
    .stream_name = "BT40183",//"BT40183 PCM",
    .cpu_dai = &aml_dai[1],
    .codec_dai = &bt40183_dai,
    .init = bt40183_codec_init,
    .ops = &bt40183_ops,
}
};

static struct snd_soc_card snd_soc_bt40183 = {
    .name = "AML-BT40183",
    .platform = &aml_soc_platform,
    .dai_link = &bt40183_dai_link[0],
    .num_links = 1,
    .set_bias_level = bt40183_set_bias_level,
};

static struct snd_soc_device bt40183_snd_devdata = {
    .card = &snd_soc_bt40183,
    .codec_dev = &soc_codec_dev_bt40183,
};

static struct platform_device *bt40183_snd_device = NULL;

static int bt40183_audio_probe(struct platform_device *pdev)
{
    int ret = 0;
    printk("bt40183_audio_probe\n");
    printk(KERN_INFO "enter %s id:%d\n ", __func__, pdev->id);
	
    bt40183_snd_device = platform_device_alloc("soc-audio", pdev->id);
    if (!bt40183_snd_device) {
        printk(KERN_ERR "ASoC: Platform device alloc failed\n");
        ret = -ENOMEM;
    }

    platform_set_drvdata(bt40183_snd_device, &bt40183_snd_devdata);
    bt40183_snd_devdata.dev = &bt40183_snd_device->dev;

    ret = platform_device_add(bt40183_snd_device);
    if (ret) {
        printk(KERN_ERR "ASoC: Platform device add failed\n");
        goto error;
    }

    return ret;

error:

    platform_device_put(bt40183_snd_device);

    return ret;
}

static int bt40183_audio_remove(struct platform_device *pdev)
{
    if (bt40183_snd_device) {
        platform_device_unregister(bt40183_snd_device);
        bt40183_snd_device = NULL;
    }

    return 0;
}


static struct platform_driver bt40183_audio_driver = {
    .probe  = bt40183_audio_probe,
    .remove = bt40183_audio_remove,
    .driver = {
        .name = "bt40183_audio",
        .owner = THIS_MODULE,
    },
};

static int __init bt40183_audio_init(void)
{
    return platform_driver_register(&bt40183_audio_driver);
}

static void __exit bt40183_audio_exit(void)
{
    platform_driver_unregister(&bt40183_audio_driver);
}

module_init(bt40183_audio_init);
module_exit(bt40183_audio_init);

/* Module information */
MODULE_AUTHOR("AMLogic, Inc.");
MODULE_DESCRIPTION("ALSA SoC BT40183 audio driver");
MODULE_LICENSE("GPL");


