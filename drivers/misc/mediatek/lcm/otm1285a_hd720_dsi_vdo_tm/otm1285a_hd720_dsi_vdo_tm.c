/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <mt-plat/mt_gpio.h>
#include <mach/gpio_const.h>
#include <linux/string.h>
#include "lcm_drv.h"

#define LCM_ID_OTM1285						0x40

#define FRAME_WIDTH						(720)
#define FRAME_HEIGHT						(1280)

#define REGFLAG_DELAY						0xFFFF
#define REGFLAG_END_OF_TABLE					0xFFFA

#define GPIO_LCM_PWR_EN						(GPIO119 | 0x80000000)

#define SET_RESET_PIN(v)					(lcm_util.set_reset_pin((v)))
#define MDELAY(n) 						(lcm_util.mdelay(n))

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define read_reg_v2(cmd, buffer, buffer_size)			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define __SAME_IC_COMPATIBLE__

static LCM_UTIL_FUNCS lcm_util;

struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x00, 1, {0x00}},
	{0xFF, 3, {0x12, 0x85, 0x01}},
	{0x00, 1, {0x80}},
	{0xFF, 2, {0x12, 0x85}},
	{0x00, 1, {0xC1}},
	{0xC5, 1, {0x33}},
	{0x00, 1, {0x93}},
	{0xC5, 1, {0x17}},
	{0x00, 1, {0x95}},
	{0xC5, 1, {0x10}},
	{0x00, 1, {0x96}},
	{0xC5, 1, {0x80}},
	{0x00, 1, {0x97}},
	{0xC5, 1, {0x16}},
	{0x00, 1, {0x98}},
	{0xC5, 2, {0x80, 0x0F}},
	{0x00, 1, {0x00}},
	{0xD8, 2, {0x32, 0x32}},
	{0x00, 1, {0x80}},
	{0xC5, 1, {0x34}},
	{0x00, 1, {0x90}},
	{0xF5, 14, {0x03, 0x15, 0x09, 0x15, 0x07, 0x15, 0x0C, 0x15, 0x0A, 0x15, 0x09, 0x15, 0x0A, 0x15}},
	{0x00, 1, {0xA0}},
	{0xF5, 14, {0x12, 0x11, 0x03, 0x15, 0x09, 0x15, 0x11, 0x15, 0x08, 0x15, 0x07, 0x15, 0x09, 0x15}},
	{0x00, 1, {0xC0}},
	{0xF5, 14, {0x0E, 0x15, 0x0E, 0x15, 0x00, 0x15, 0x00, 0x15, 0x2E, 0x15, 0x14, 0x11, 0x00, 0x25}},
	{0x00, 1, {0xD0}},
	{0xF5, 15, {0x07, 0x15, 0x0A, 0x15, 0x10, 0x11, 0x00, 0x10, 0x90, 0x90, 0x90, 0x02, 0x90, 0x00, 0x00}},
	{0x00, 1, {0x82}},
	{0xC0, 3, {0x00, 0x0E, 0x0C}},
	{0x00, 1, {0x80}},
	{0xC0, 2, {0x00, 0x70}},
	{0x00, 1, {0x80}},
	{0xC1, 4, {0x19, 0x19, 0x19, 0x19}},
	{0x00, 1, {0x90}},
	{0xC1, 1, {0x77}},
	{0x00, 1, {0x80}},
	{0xC2, 8, {0x84, 0x02, 0x7B, 0x12, 0x85, 0x01, 0x5D, 0xF0}},
	{0x00, 1, {0xF0}},
	{0xC2, 2, {0x00, 0x00}},
	{0x00, 1, {0x90}},
	{0xC2, 15, {0x82, 0x02, 0x01, 0x0D, 0x05, 0x81, 0x02, 0x01, 0x0D, 0x05, 0x80, 0x02, 0x01, 0x0D, 0x05}},
	{0x00, 1, {0xA0}},
	{0xC2, 5, {0x83, 0x02, 0x01, 0x0D, 0x05}},
	{0x00, 1, {0xEC}},
	{0xC2, 2, {0x10, 0x00}},
	{0x00, 1, {0xA0}},
	{0xC0, 1, {0x0B}},
	{0x00, 1, {0xA3}},
	{0xC0, 1, {0x00}},
	{0x00, 1, {0xA5}},
	{0xC0, 2, {0x18, 0x05}},
	{0x00, 1, {0x80}},
	{0xCC, 12, {0x03, 0x04, 0x05, 0x06, 0x11, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B}},
	{0x00, 1, {0xB0}},
	{0xCC, 12, {0x06, 0x05, 0x04, 0x03, 0x0C, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B}},
	{0x00, 1, {0x80}},
	{0xCD, 15, {0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x01, 0x02, 0x14, 0x13, 0x12, 0x0B, 0x0B, 0x0B}},
	{0x00, 1, {0x80}},
	{0xCB, 7, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xF0}},
	{0xCB, 7, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF}},
	{0x00, 1, {0xC0}},
	{0xCB, 15, {0x05, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xD0}},
	{0xCB, 12, {0x00, 0x00, 0x00, 0x00, 0xFD, 0x00, 0xFD, 0xFD, 0x05, 0x00, 0x00, 0x00}},
	{0x00, 1, {0x90}},
	{0xCB, 15, {0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xA0}},
	{0xCB, 12, {0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xE0}},
	{0xCC, 4, {0x80, 0xF0, 0x00, 0x00}},
	{0x00, 1, {0xD0}},
	{0xCD, 15, {0x01, 0x00, 0x03, 0x00, 0x13, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}},
	{0x00, 1, {0xE0}},
	{0xCD, 12, {0x10, 0x11, 0x12, 0x05, 0x14, 0x15, 0x08, 0x08, 0x08, 0x19, 0x1A, 0x1B}},
	{0x00, 1, {0xA0}},
	{0xCD, 15, {0x00, 0x02, 0x00, 0x04, 0x13, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}},
	{0x00, 1, {0xB0}},
	{0xCD, 12, {0x10, 0x11, 0x12, 0x05, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B}},
	{0x00, 1, {0xA2}},
	{0xC1, 1, {0x00}},
	{0x00, 1, {0xB4}},
	{0xC0, 1, {0x10}},
	{0x00, 1, {0xB3}},
	{0xC0, 1, {0x33}},
	{0x00, 1, {0x80}},
	{0xC4, 1, {0x04}},
	{0x00, 1, {0x83}},
	{0xC4, 1, {0x02}},
	{0x00, 1, {0xF8}},
	{0xC2, 1, {0x40}},
	{0x00, 1, {0x80}},
	{0xA5, 1, {0x0C}},
	{0x00, 1, {0x81}},
	{0xA5, 1, {0x04}},
	{0x00, 1, {0xE6}},
	{0xCD, 3, {0x08, 0x08, 0x08}},
	{0x00, 1, {0xF2}},
	{0xC2, 2, {0x04, 0x04}},
	{0x00, 1, {0xFD}},
	{0xC2, 3, {0xFF, 0xFF, 0xFF}},
	{0x00, 1, {0x9A}},
	{0xC5, 3, {0x22, 0x22, 0x20}},
	{0x00, 1, {0x00}},
	{0xE1, 24, {0x0A, 0x1A, 0x24, 0x33, 0x3C, 0x44, 0x51, 0x63, 0x6D, 0x7F, 0x8A, 0x92, 0x67, 0x61, 0x5D, 0x4F, 0x3E, 0x30, 0x26, 0x20, 0x1A, 0x14, 0x12, 0x11}},
	{0x00, 1, {0x00}},
	{0xE2, 24, {0x0A, 0x1A, 0x24, 0x33, 0x3C, 0x44, 0x51, 0x63, 0x6D, 0x7F, 0x8A, 0x92, 0x67, 0x61, 0x5D, 0x4F, 0x3E, 0x30, 0x26, 0x20, 0x1A, 0x14, 0x12, 0x11}},
	{0x00, 1, {0x00}},
	{0xE3, 24, {0x0A, 0x1A, 0x24, 0x33, 0x3C, 0x44, 0x51, 0x63, 0x6D, 0x7F, 0x8A, 0x92, 0x67, 0x61, 0x5D, 0x4F, 0x3E, 0x30, 0x26, 0x20, 0x1A, 0x14, 0x12, 0x11}},
	{0x00, 1, {0x00}},
	{0xE4, 24, {0x0A, 0x1A, 0x24, 0x33, 0x3C, 0x44, 0x51, 0x63, 0x6D, 0x7F, 0x8A, 0x92, 0x67, 0x61, 0x5D, 0x4F, 0x3E, 0x30, 0x26, 0x20, 0x1A, 0x14, 0x12, 0x11}},
	{0x00, 1, {0x00}},
	{0xE5, 24, {0x0A, 0x1A, 0x24, 0x33, 0x3C, 0x44, 0x51, 0x63, 0x6D, 0x7F, 0x8A, 0x92, 0x67, 0x61, 0x5D, 0x4F, 0x3E, 0x30, 0x26, 0x20, 0x1A, 0x14, 0x12, 0x11}},
	{0x00, 1, {0x00}},
	{0xE6, 24, {0x0A, 0x1A, 0x24, 0x33, 0x3C, 0x44, 0x51, 0x63, 0x6D, 0x7F, 0x8A, 0x92, 0x67, 0x61, 0x5D, 0x4F, 0x3E, 0x30, 0x26, 0x20, 0x1A, 0x14, 0x12, 0x11}},
	{0x00, 1, {0x00}},
	{0xFF, 3, {0xFF, 0xFF, 0xFF}},
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 30, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Sleep mode off
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},

	// Sleep mode on
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for(i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {
			case REGFLAG_DELAY:
				MDELAY(table[i].count);
				break;
			case REGFLAG_END_OF_TABLE:
				break;
			default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));
	params->type = LCM_TYPE_DSI;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;
	params->dsi.noncont_clock = 1;
	params->dsi.noncont_clock_period = 2;
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.packet_size = 256;
	params->dsi.vertical_sync_active = 6;
	params->dsi.vertical_backporch = 20;
	params->dsi.vertical_frontporch = 14;
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.horizontal_sync_active = 60;
	params->dsi.horizontal_backporch = 100;
	params->dsi.horizontal_frontporch = 100;
	params->dsi.horizontal_blanking_pixel = 60;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.PLL_CLOCK = 239;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
}

static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[2];
	unsigned int array[16];

	mt_set_gpio_mode(GPIO_LCM_PWR_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_PWR_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_PWR_EN, GPIO_OUT_ONE);

	MDELAY(10);
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(150);

	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xDA, buffer, 1);

	id = buffer[0];
	pr_info("%s, id of otm1285a = 0x%08x\n", __func__, id);

	if(id == LCM_ID_OTM1285) {
		return 1;
	} else {
		return 0;
	}
}


static void lcm_init(void)
{
	mt_set_gpio_mode(GPIO_LCM_PWR_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_PWR_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_PWR_EN, GPIO_OUT_ONE);
	
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

	mt_set_gpio_mode(GPIO_LCM_PWR_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_PWR_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_PWR_EN, GPIO_OUT_ZERO);
	MDELAY(10);
}

static void lcm_resume(void)
{
	lcm_init();
}

LCM_DRIVER otm1285a_hd720_dsi_vdo_tm_lcm_drv = {
	.name = "otm1285a_dsi_vdo_tm",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
};
