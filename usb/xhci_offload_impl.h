/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Google Corp.
 *
 * Author:
 *  Howard.Yen <howardyen@google.com>
 */

#ifndef __XHCI_OFFLOAD_IMPL_H
#define __XHCI_OFFLOAD_IMPL_H

enum usb_offload_op_mode {
	USB_OFFLOAD_STOP,
	USB_OFFLOAD_DRAM
};

struct xhci_offload_data {
	struct xhci_hcd *xhci;

	bool usb_audio_offload;
	bool dt_direct_usb_access;
	bool offload_state;

	enum usb_offload_op_mode op_mode;
};

struct xhci_offload_data *xhci_get_offload_data(void);
int xhci_offload_helper_init(void);

extern int xhci_exynos_register_offload_ops(struct xhci_exynos_ops *offload_ops);

#endif /* __XHCI_OFFLOAD_IMPL_H */
