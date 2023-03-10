// SPDX-License-Identifier: GPL-2.0-only
/*
 * Google Whitechapel AoC ALSA  Driver on PCM
 *
 * Copyright (c) 2023 Google LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <sound/core.h>
#include <linux/usb.h>

#include "aoc_alsa.h"
#include "card.h"
#include "usbaudio.h"

struct uaudio_dev {
	int card_num;
	struct snd_usb_audio *chip;
};

static struct uaudio_dev uadev[SNDRV_CARDS];

void (*cb_func)(struct usb_device*, struct usb_host_endpoint*);

static struct snd_usb_substream *find_substream(unsigned int card_num,
	unsigned int device, unsigned int direction)
{
	struct snd_usb_stream *as;
	struct snd_usb_substream *subs = NULL;
	struct snd_usb_audio *chip;

	chip = uadev[card_num].chip;

	if (!chip || atomic_read(&chip->shutdown))
		goto done;

	if (device >= chip->pcm_devs)
		goto done;

	if (direction > SNDRV_PCM_STREAM_CAPTURE)
		goto done;

	list_for_each_entry(as, &chip->pcm_list, list) {
		if (as->pcm_index == device) {
			subs = &as->substream[direction];
			goto done;
		}
	}

done:
	return subs;
}

void usb_audio_offload_connect(struct snd_usb_audio *chip)
{
	int card_num;

	if (!chip)
		return;

	card_num = chip->card->number;
	if (card_num >= SNDRV_CARDS)
		return;

	pr_info("%s card%d connected", __func__, card_num);
	mutex_lock(&chip->mutex);
	uadev[card_num].chip = chip;
	uadev[card_num].card_num = card_num;
	mutex_unlock(&chip->mutex);
}

void usb_audio_offload_disconnect(struct snd_usb_audio *chip)
{
	int card_num;

	if (!chip)
		return;

	card_num = chip->card->number;
	if (card_num >= SNDRV_CARDS)
		return;

	mutex_lock(&chip->mutex);
	pr_info("%s card%d disconnected", __func__, card_num);
	uadev[card_num].chip = NULL;
	uadev[card_num].card_num = 0;
	mutex_unlock(&chip->mutex);
}

void usb_audio_offload_suspend(struct usb_interface *intf, pm_message_t message)
{
	pr_debug("%s", __func__);
}

int aoc_set_usb_mem_config(struct aoc_chip *achip)
{
	struct usb_host_endpoint *ep;
	struct snd_usb_substream *subs;
	struct snd_usb_audio *chip;
	int card_num;
	int device;
	int direction;

	if (!achip)
		return -ENODEV;

	card_num = achip->usb_card;
	device = achip->usb_device;
	direction = achip->usb_direction;
	chip = uadev[card_num].chip;

	pr_info("%s card %d device %d, direction %d", __func__,
			card_num, device, direction);
	if ((card_num >= 0 && card_num < SNDRV_CARDS) &&
		(device >= 0) &&
		(direction >= 0 && direction <= SNDRV_PCM_STREAM_CAPTURE)) {
		mutex_lock(&chip->mutex);
		subs = find_substream(card_num, device, direction);
		if (!subs) {
			pr_err("%s subs not found", __func__);
			mutex_unlock(&chip->mutex);
			return -ENODEV;
		}
		ep = usb_pipe_endpoint(subs->dev, subs->data_endpoint->pipe);
		if (!ep) {
			pr_err("%s data ep # %d context is null\n",
					__func__, subs->data_endpoint->ep_num);
			mutex_unlock(&chip->mutex);
			return -ENODEV;
		}

		if (cb_func) {
			cb_func(subs->dev, ep);
		} else {
			pr_info("%s call aoc_alsa_usb_callback_register() first, skip", __func__);
		}
		mutex_unlock(&chip->mutex);
	}

	return 0;
}

static struct snd_usb_platform_ops ops = {
	.connect_cb = usb_audio_offload_connect,
	.disconnect_cb = usb_audio_offload_disconnect,
	.suspend_cb = usb_audio_offload_suspend,
};

bool aoc_alsa_usb_callback_register(
			void (*callback)(struct usb_device*, struct usb_host_endpoint*))
{
	pr_info("%s usb callback register", __func__);
	if (callback) {
		cb_func = callback;
	} else {
		pr_err("%s: cb_func register fail", __func__);
		return false;
	}
	return true;
}
EXPORT_SYMBOL_GPL(aoc_alsa_usb_callback_register);

void aoc_alsa_usb_callback_unregister(void)
{
	pr_info("%s usb callback unregister", __func__);
	if (cb_func) {
		cb_func = NULL;
	}
}
EXPORT_SYMBOL_GPL(aoc_alsa_usb_callback_unregister);

int aoc_usb_init(void)
{
	int ret = 0;

	ret = snd_usb_register_platform_ops(&ops);
	if (ret < 0) {
		pr_err("%s snd_usb_register_platform_ops fail", __func__);
	} else {
		pr_info("%s registered usb callback", __func__);
	}

	return ret;
}

void aoc_usb_exit(void)
{
	pr_info("%s unregister usb callback", __func__);
	snd_usb_unregister_platform_ops();
}

