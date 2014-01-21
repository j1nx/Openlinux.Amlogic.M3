/*
 * bt40183.c  --  BT40183 ALSA SoC Audio driver
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include "bt40183.h"

#define AUDIO_NAME "BT40183"

struct bt40183_priv {
	struct snd_soc_codec codec;
};

static int bt40183_write(struct snd_soc_codec *codec, unsigned int reg, unsigned int val)
{
    /* call hw write */
    return 0;
}

static unsigned int bt40183_read(struct snd_soc_codec *codec, unsigned int reg)
{
    /* call hw read */
    return 0;
}

static int bt40183_hw_write(void *control_data, const char* buf, int count)
{
    return count;
}

static unsigned int bt40183_hw_read(struct snd_soc_codec *codec, unsigned int reg)
{
    return 0;
}

static int bt40183_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_A:
		break;
	case SND_SOC_DAIFMT_DSP_B:
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_IF:
		break;
	case SND_SOC_DAIFMT_IB_NF:
		break;
	case SND_SOC_DAIFMT_NB_IF:
		break;
	default:
		return -EINVAL;
	}

	/* set iface */
	return 0;
}

static int bt40183_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	/* bit size */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		break;
	case SNDRV_PCM_FORMAT_S8:
		break;
	}

	/* set iface */
	return 0;
}

static bt40183_set_sysclk(struct snd_soc_dai *dai,
		int clk_id, unsigned int freq, int dir)
{
    return 0;
}

static void bt40183_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
    return;
}

static int bt40183_set_bias_level(struct snd_soc_codec *codec,
				 enum snd_soc_bias_level level)
{

	switch (level) {
	case SND_SOC_BIAS_ON:
		break;

	case SND_SOC_BIAS_PREPARE:
		break;

	case SND_SOC_BIAS_STANDBY:
		break;

	case SND_SOC_BIAS_OFF:
		break;
	}

	codec->bias_level = level;

	return 0;
}

#define BT40183_RATES SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000

#define BT40183_FORMATS \
	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | \
	SNDRV_PCM_FMTBIT_S8)

static struct snd_soc_dai_ops bt40183_dai_ops = {
	.hw_params = bt40183_hw_params,
	.set_fmt = bt40183_set_dai_fmt,
	.set_sysclk = bt40183_set_sysclk,
	.shutdown = bt40183_shutdown,
};

struct snd_soc_dai bt40183_dai = {
	.name = "BT40183 Voice",
	.playback = {
		.stream_name = "Voice Playback",
		.channels_min = 1,
		.channels_max = 1,
		.rates = BT40183_RATES,
		.formats = BT40183_FORMATS,},
	.capture = {
		.stream_name = "Voice Capture",
		.channels_min = 1,
		.channels_max = 1,
		.rates = BT40183_RATES,
		.formats = BT40183_FORMATS,},
	.ops = &bt40183_dai_ops,
	.symmetric_rates = 1,
};
EXPORT_SYMBOL_GPL(bt40183_dai);

static int bt40183_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	bt40183_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int bt40183_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	bt40183_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	bt40183_set_bias_level(codec, codec->suspend_bias_level);
	return 0;
}

static struct snd_soc_codec *bt40183_codec = NULL;

static int bt40183_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;
	int ret = 0;
	printk("bt40183_probe\n");
	if (bt40183_codec == NULL) {
		dev_err(&pdev->dev, "Codec device not registered\n");
		return -ENODEV;
	}

	socdev->card->codec = bt40183_codec;
	codec = bt40183_codec;

	/* register pcms */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		dev_err(codec->dev, "failed to create pcms: %d\n", ret);
		goto pcm_err;
	}

	return ret;

pcm_err:
	return ret;
}

/* power down chip */
static int bt40183_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);

	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_bt40183 = {
	.probe = 	bt40183_probe,
	.remove = 	bt40183_remove,
	.suspend = 	bt40183_suspend,
	.resume =	bt40183_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_bt40183);


static int bt40183_register(struct bt40183_priv *bt40183,
			   enum snd_soc_control_type control)
{
	struct snd_soc_codec *codec = &bt40183->codec;
	int ret;
	printk("%s\n", __FUNCTION__);
	if (bt40183_codec) {
		dev_err(codec->dev, "Another BT40183 is registered\n");
		ret = -EINVAL;
		goto err;
	}

	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	codec->private_data = bt40183;
	codec->name = "BT40183";
	codec->owner = THIS_MODULE;
	codec->bias_level = SND_SOC_BIAS_OFF;
	codec->set_bias_level = bt40183_set_bias_level;
	codec->dai = &bt40183_dai;
	codec->num_dai = 1;
	codec->reg_cache_size = 0;
	codec->reg_cache = NULL;

#if 0
    codec->write = NULL;
    codec->read = NULL;
    codec->hw_write = NULL;
    codec->hw_read = NULL;
#else
    codec->write = bt40183_write;
    codec->read = bt40183_read;
    codec->hw_write = bt40183_hw_write;
    codec->hw_read = bt40183_hw_read;
#endif

	bt40183_dai.dev = codec->dev;
    bt40183_codec = codec;

	bt40183_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

	ret = snd_soc_register_codec(codec);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register codec: %d\n", ret);
		goto err;
	}

	ret = snd_soc_register_dai(&bt40183_dai);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register DAI: %d\n", ret);
		goto err_codec;
	}

	return 0;

err_codec:
	snd_soc_unregister_codec(codec);
err:
	kfree(bt40183);
	return ret;
}

static void bt40183_unregister(struct bt40183_priv *bt40183)
{
	bt40183_set_bias_level(&bt40183->codec, SND_SOC_BIAS_OFF);
	snd_soc_unregister_dai(&bt40183_dai);
	snd_soc_unregister_codec(&bt40183->codec);
	kfree(bt40183);
	bt40183_codec = NULL;
}

static int __devinit bt40183_platform_probe(struct platform_device *pdev)
{
	struct bt40183_priv *bt40183 = NULL;
	struct snd_soc_codec *codec = NULL;
	printk("%s\n", __FUNCTION__);
	bt40183 = kzalloc(sizeof(struct bt40183_priv), GFP_KERNEL);
	if (bt40183 == NULL)
		return -ENOMEM;

	codec = &bt40183->codec;

    dev_set_drvdata(&pdev->dev, bt40183);

    codec->control_data = NULL;
	codec->dev = &pdev->dev;

	return bt40183_register(bt40183, SND_SOC_CUSTOM);
}

static int __devexit bt40183_platform_remove(struct platform_device *pdev)
{
	struct bt40183_priv *bt40183 = dev_get_drvdata(&pdev->dev);
	bt40183_unregister(bt40183);
	return 0;
}

static struct platform_driver bt40183_platform_driver = {
	.driver = {
		.name = "BT40183",
		.owner = THIS_MODULE,
		},
	.probe = bt40183_platform_probe,
	.remove = __devexit_p(bt40183_platform_remove),
};

static int __init bt40183_init(void)
{
	return platform_driver_register(&bt40183_platform_driver);
}
module_init(bt40183_init);

static void __exit bt40183_exit(void)
{
	platform_driver_unregister(&bt40183_platform_driver);
}
module_exit(bt40183_exit);

MODULE_DESCRIPTION("ASoC BT40183 driver");
MODULE_AUTHOR("AMLogic, Inc.");
MODULE_LICENSE("GPL");
