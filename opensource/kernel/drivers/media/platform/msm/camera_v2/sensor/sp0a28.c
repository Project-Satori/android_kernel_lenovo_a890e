/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"

#define CONFIG_MSMB_CAMERA_DEBUG
#define SP0A28_P0_0xdd	0x88
#define SP0A28_P0_0xde	0xb0 //80

#define SP0A28_P1_0xe8         0x28 //0x10//10//;sharp_fac_pos_outdoor
#define SP0A28_P1_0xec         0x28 //0x20//20//;sharp_fac_neg_outdoor
#define SP0A28_P1_0xe9         0x20 //0x0a//0a//;sharp_fac_pos_nr
#define SP0A28_P1_0xed         0x20 //0x20//20//;sharp_fac_neg_nr
#define SP0A28_P1_0xea         0x20 //0x08//08//;sharp_fac_pos_dummy
#define SP0A28_P1_0xef	0x20//18//;sharp_fac_neg_dummy
#define SP0A28_P1_0xeb	0x10//08//;sharp_fac_pos_low
#define SP0A28_P1_0xf0	0x10//18//;sharp_fac_neg_low
//saturation
#define SP0A28_P0_0xd3	0x84
#define SP0A28_P0_0xd4	0x84
#define SP0A28_P0_0xd6	0x70
#define SP0A28_P0_0xd7  0x50
#define SP0A28_P0_0xd8	0x84
#define SP0A28_P0_0xd9	0x84
#define SP0A28_P0_0xda	0x70
#define SP0A28_P0_0xdb   0x50

//Ae target
#define SP0A28_P0_0xf7	0x78//0x80
#define SP0A28_P0_0xf8	0x70//0x78
#define SP0A28_P0_0xf9	0x8e//0x8c//0x80
#define SP0A28_P0_0xfa	0x86//0x84//0x78

#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

#define SP0a28_SENSOR_NAME "sp0a28"
DEFINE_MSM_MUTEX(sp0a28_mut);

static struct msm_sensor_ctrl_t sp0a28_s_ctrl;

static struct msm_sensor_power_setting sp0a28_power_setting[] = {
	/*{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 5,
	},*/
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 0,
		.delay = 1,
	},


	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 10,
	},


	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 10,
	},


	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 10,
	},





	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 5,
	},
};

static struct msm_camera_i2c_reg_conf sp0a28_start_settings[] = {
	{0xfd, 0x00},
	{0x0e, 0x00},
	{0x2f, 0x01},
	{0x36, 0x00},
};

static struct msm_camera_i2c_reg_conf sp0a28_stop_settings[] = {
	{0xfd, 0x00},
	{0x2f, 0x00},
	{0x0e, 0x01},
	{0x36, 0x20},
};

static struct msm_camera_i2c_reg_conf sp0a28_recommend_settings[] = {
	{0xfd,0x01},
	{0x7c,0x6c},
//	{0x68,0x02},
//	{0x6f,0x02},
	{0xfd,0x00},
    {0x1c,0x00},
	{0x32,0x00},
    {0xe7,0x03},
    {0xe7,0x00},
	{0x0e,0x00},
	{0x0f,0x40},
	{0x10,0x40},
	{0x11,0x10},
	{0x12,0xa0},
    {0x13,0xff},
    {0x14,0x18},
	{0x15,0x00},
	{0x16,0x08},
    {0x1a,0x37},
    {0x1b,0x17},
    {0x1c,0x2f},
	{0x1d,0x00},
    {0x1e,0x57},
	{0x21,0x34},//f
	{0x22,0x12},
	{0x24,0x80},
	{0x25,0x02},
	{0x26,0x03},
	{0x27,0xeb},
	{0x28,0x5f},
	{0x2f,0x01},
	{0x5f,0x02},
	{0xfb,0x33},

       //blacklevel
    {0x65,0x18},//18
    {0x66,0x18},//18
    {0x67,0x18},//18
    {0x68,0x18},//18
	//ae setting


	//Status

	//DP
	{0xfd,0x00},
	{0x45,0x00},
	{0x46,0x99},
	{0x79,0xff},
	{0x7a,0xff},
	{0x7b,0x10},
	{0x7c,0x10},

	//lsc  for SX5044module
	{0xfd,0x01},
        {0x35,0x30},//2e 24
        {0x36,0x30},//27 24
        {0x37,0x30},//27 24
        {0x38,0x30},//22 24
        {0x39,0x30},//30 24
        {0x3a,0x30},//28 24
        {0x3b,0x30},//2c 24
        {0x3c,0x30},//20 24
        {0x3d,0x30},//30 24
        {0x3e,0x30},//29 24
        {0x3f,0x30},//21 24
        {0x40,0x30},//27 24
        {0x41,0x00},//0x00
	{0x42,0x00},//0x18
	{0x43,0x00},//0x02
	{0x44,0x00},
	{0x45,0x00},
	{0x46,0x04},//0x14
	{0x47,0x00},
	{0x48,0x00},
    {0x49,0xfd},//00
	{0x4a,0x04},//0x12
	{0x4b,0x00},
	{0x4c,0x00},
	{0xfd,0x00},
	{0xa1,0x20},
	{0xa2,0x20},
	{0xa3,0x20},
	{0xa4,0xff},
	//smooth
	{0xfd,0x01},
	{0xde,0x0f},
	{0xfd,0x00},
	//\u951f\u65a4\u62f7\u901a\u951f\u65a4\u62f7\u951f\u65a4\u62f7\u5e73\u951f\u65a4\u62f7\u951f\u65a4\u62f7\u503c
    {0x57,0x04},   ///raw_dif_thr_outdoor
    {0x58,0x08},// ;raw_dif_thr_normal
    {0x56,0x12},// ;raw_dif_thr_dummy
    {0x59,0x12},// ;raw_dif_thr_lowlight
    //GrGb\u5e73\u951f\u65a4\u62f7\u951f\u65a4\u62f7\u503c
    {0x89,0x04},   ///raw_grgb_thr_outdoor
	{0x8a,0x0a}, //;raw_grgb_thr_normal
	{0x9c,0x10}, //;raw_grgb_thr_dummy
    {0x9d,0x15}, ///raw_grgb_thr_lowlight
	//Gr\Gb\u4e4b\u951f\u65a4\u62f7\u5e73\u951f\u65a4\u62f7\u5f3a\u951f\u65a4\u62f7
	{0x81,0xe0}, //   ;raw_gflt_fac_outdoor
	{0x82,0xa0}, //;80;raw_gflt_fac_normal
	{0x83,0x80}, //   ;raw_gflt_fac_dummy
	{0x84,0x80}, //   ;raw_gflt_fac_lowlight
	//Gr\u951f\u65a4\u62f7Gb\u951f\u65a4\u62f7\u901a\u951f\u65a4\u62f7\u951f\u65a4\u62f7\u5e73\u951f\u65a4\u62f7\u5f3a\u951f\u65a4\u62f7
    {0x85,0xe0}, ///raw_gf_fac_outdoor
	{0x86,0xa0}, //;raw_gf_fac_normal
	{0x87,0x80}, //;raw_gf_fac_dummy
	{0x88,0x80}, //;raw_gf_fac_lowlight
	//R\u951f\u65a4\u62f7B\u5e73\u951f\u65a4\u62f7\u5f3a\u951f\u65a4\u62f7
	{0x5a,0xff},		//;raw_rb_fac_outdoor
	{0x5b,0xc0}, 		//;raw_rb_fac_normal
	{0x5c,0xa0}, 	  //;raw_rb_fac_dummy
    {0x5d,0x80},            ///raw_rb_fac_lowlight
	//adt \u5e73\u951f\u65a4\u62f7\u951f\u65a4\u62f7\u503c\u951f\u65a4\u62f7\u951f\u65a4\u62f7\u5e94
	{0xa7,0xff},
	{0xa8,0xff},	//;0x2f
	{0xa9,0xff},	//;0x2f
	{0xaa,0xff},	//;0x2f
	//dem_morie_thr \u53bb\u6469\u951f\u65a4\u62f7\u951f\u65a4\u62f7
	{0x9e,0x10},
	//sharpen

	{0xfd,0x01},	//
	{0xe2,0x30},	//	;sharpen_y_base
	{0xe4,0xa0},	//	;sharpen_y_max

    {0xe5,0x04},// ;rangek_neg_outdoor
    {0xd3,0x08},// ;rangek_pos_outdoor
    {0xd7,0x04},// ;range_base_outdoor

	{0xe6,0x08},//0x08	// ;rangek_neg_normal
	{0xd4,0x10},//0x10	// ;rangek_pos_normal
	{0xd8,0x08},	// ;range_base_normal

    {0xe7,0x08},// ;rangek_neg_dummy
	{0xd5,0x10},	// ;rangek_pos_dummy
    {0xd9,0x08},// ;range_base_dummy

	{0xd2,0x10},	// ;rangek_neg_lowlight
	{0xd6,0x10},	// ;rangek_pos_lowlight
	{0xda,0x10},	// ;range_base_lowlight

	{0xe8,SP0A28_P1_0xe8},//0x20	//;sharp_fac_pos_outdoor
	{0xec,SP0A28_P1_0xec},//0x30	//;sharp_fac_neg_outdoor
	{0xe9,SP0A28_P1_0xe9},//0x10	//;sharp_fac_pos_nr
	{0xed,SP0A28_P1_0xed},//0x30	//;sharp_fac_neg_nr
	{0xea,SP0A28_P1_0xea},//0x10	//;sharp_fac_pos_dummy
	{0xef,SP0A28_P1_0xef},//0x20	//;sharp_fac_neg_dummy
	{0xeb,SP0A28_P1_0xeb},//0x10	//;sharp_fac_pos_low
	{0xf0,SP0A28_P1_0xf0},//0x20	//;sharp_fac_neg_low

	//CCM
#if 0
	{0xfd,0x01},	//
	//{0xa0,0x80},	//;8c;80;80;80(\u951f\u65a4\u62f7\u8272\u951f\u63a5\u65a4\u62f7\u951f\u7f34\ue0ac\u62f7\u951f\u65a4\u62f7\u951f\u65a4\u62f7\u951f?
        {0xa0,0x8c},	//;8c;80;80;80
	{0xa1,0x06},	//;0c;00;0 ;0
	//{0xa2,0x00},	//;e8;00;0 ;0
        {0xa2,0xed},	//;e8;00;0 ;0
	{0xa3,0xe7},	//;ec;ff;f2;f3;f0
	{0xa4,0xb3},	//;99;9a;8e;a6
	//{0xa5,0xf2},	//;fb;e7;0 ;ea
        {0xa5,0xe7},	//;fb;e7;0 ;ea
	{0xa6,0x13},	//;0d;0c;0 ;0
	{0xa7,0xc0},	//;da;da;e6;e6
	{0xa8,0xac},	//;98;9a;9a;9a
	{0xa9,0x30},	//;30;00;0 ;0
	{0xaa,0x33},	//;33;33;3 ;33
	{0xab,0x0c},	//;0c;0c;c ;c
	{0xfd,0x00},	//;00
#else
    {0xfd,0x01},	//01
    {0xa0,0xa6},	//81
    {0xa1,0x00},	//f4
    {0xa2,0xda},	//0c
    {0xa3,0x0c},	//0c
    {0xa4,0xa6},	//a6
    {0xa5,0xcd},	//cd
    {0xa6,0x0c},	//0c
    {0xa7,0xe0},	//e0
    {0xa8,0xb3},	//b3
    {0xa9,0x30},	//0c
    {0xaa,0x30},	//30
    {0xab,0x0c},	//0c
    {0xfd,0x00},	//00
#endif
	//gamma
	{0xfd,0x00},	//00
	{0x8b,0x00},	//0
	{0x8c,0x0f},	//12
	{0x8d,0x21},	//f
	{0x8e,0x3b},	//31
	{0x8f,0x64},	//c
	{0x90,0x84},	//62
	{0x91,0xa0},	//77
	{0x92,0xb6},	//89
	{0x93,0xc7},	//9b
	{0x94,0xd2},	//a8
	{0x95,0xda},	//b5
	{0x96,0xe0},	//c0
	{0x97,0xe6},	//ca
	{0x98,0xea},	//d4
	{0x99,0xef},	//dd
	{0x9a,0xf4},	//e6
	{0x9b,0xf7},	//ef
	{0xfd,0x01},	//01
	{0x8d,0xfb},	//f7
	{0x8e,0xff},	//ff



	/*//gamma \u951f\u63ed\u9636\u5206\u8be7\u62f7\u951f\u65a4\u62f7
	
	{0xfd,0x00},
	{0x8b,0x00},
	{0x8c,0x11},
	{0x8d,0x24},
	{0x8e,0x3f},
	{0x8f,0x64},
	{0x90,0x7f},
	{0x91,0x93},
	{0x92,0xa4},
	{0x93,0xb2},
	{0x94,0xbb},
	{0x95,0xc4},
	{0x96,0xcb},
	{0x97,0xd1},
	{0x98,0xd5},
	{0x99,0xdc},
	{0x9a,0xe2},
	{0x9b,0xe9},
	{0xfd,0x01},
	{0x8d,0xf2},
	{0x8e,0xff},
	{0xfd,0x00},
	*/
	//awb for \u951f\u65a4\u62f7\u951f\u65a4\u62f7
	{0xfd,0x01},
	//{0x21,0x68},//r bottom
	//{0x22,0xff},
    {0x28,0xa0},//c4 //6d
    {0x29,0x80},//9e //e0
	//{0x2b,0x20},
	{0x11,0x13},
	//{0x12,0x18},//13
    {0x12,0x13},//13
	{0x2e,0x13},
        //{0x2e,0x13},	// 2.25
	{0x2f,0x13},
       // {0x2f,0x13},
	{0x16,0x1c},
	{0x17,0x1a},
	{0x18,0x1a},
	{0x19,0x54},
//	{0x1a,0xa8},//a5
	{0x1a,0x96},//a5
//	{0x1b,0x8f},//9a
	{0x1b,0x8d},//9a
	{0x2a,0xef},

	/*// awb for \u524d\u951f\u65a4\u62f7
	{0xfd,0x01},
	{0x11,0x08},
	{0x12,0x08},
	{0x2e,0x04},
	{0x2f,0x04},
	{0x16,0x1c},
	{0x17,0x1a},
	{0x18,0x16},
	{0x19,0x54},
	{0x1a,0x90},
	{0x1b,0x9b},
	{0x2a,0xef},
	{0x2b,0x30},
	{0x21,0x96},
	{0x22,0x9a},
	*/
	//AE;rpc
	{0xfd,0x00},
	{0xe0,0x3a},//
	{0xe1,0x2c},//24,
	{0xe2,0x26},//
	{0xe3,0x22},//
	{0xe4,0x22},//
	{0xe5,0x20},//
	{0xe6,0x20},//
	{0xe8,0x20},//19,
	{0xe9,0x20},//19,
	{0xea,0x20},//19,
	{0xeb,0x1e},//18,
	{0xf5,0x1e},//18,
	{0xf6,0x1e},//18,
	//ae min gain
	{0xfd,0x01},
{0x94,0x60},///rpc_max_indr 60
	{0x95,0x1e},//0x18
{0x9c,0x60},///rpc_max_outdr 60
	{0x9d,0x1e},//0x18
	//ae target
	{0xfd,0x00},
	{0xed,SP0A28_P0_0xf7 + 0x04},//0x84
	{0xf7,SP0A28_P0_0xf7},		//0x80
	{0xf8,SP0A28_P0_0xf8},		//0x78
	{0xec,SP0A28_P0_0xf8 - 0x04},//0x74
	{0xef,SP0A28_P0_0xf9 + 0x04},//0x84
	{0xf9,SP0A28_P0_0xf9},		//0x80
	{0xfa,SP0A28_P0_0xfa},		//0x78
	{0xee,SP0A28_P0_0xfa - 0x04},//0x74
	//gray detect
	{0xfd,0x01},
	{0x30,0x40},
	{0x31,0x70},
	{0x32,0x20},
	{0x33,0xef},
	{0x34,0x02},
	{0x4d,0x40},
	{0x4e,0x25},
	{0x4f,0x13},
	//saturation
	{0xfd,0x00},
	{0xbe,0xaa},
	{0xc0,0xff},
	{0xc1,0xff},
	{0xd3,SP0A28_P0_0xd3},
	{0xd4,SP0A28_P0_0xd4},
	{0xd6,SP0A28_P0_0xd6},
	{0xd7,SP0A28_P0_0xd7},
	{0xd8,SP0A28_P0_0xd8},
	{0xd9,SP0A28_P0_0xd9},
	{0xda,SP0A28_P0_0xda},
	{0xdb,SP0A28_P0_0xdb},
	//heq
	{0xfd,0x00},
	{0xdc,0x00},	//;heq_offset
	{0xdd,SP0A28_P0_0xdd},	//;ku
	{0xde,SP0A28_P0_0xde},	//;90;kl
	{0xdf,0x80},	//;heq_mean
	//YCnr
	{0xfd,0x00},
	{0xc2,0x08},	//Ynr_thr_outdoor
	{0xc3,0x08},	//Ynr_thr_normal
	{0xc4,0x08},	//Ynr_thr_dummy
	{0xc5,0x10},	//Ynr_thr_lowlight
	{0xc6,0x80},	//cnr_thr_outdoor
	{0xc7,0x80},	//cnr_thr_normal
	{0xc8,0x80},	//cnr_thr_dummy
	{0xc9,0x80},	//cnr_thr_lowlight
	//auto lum
	{0xfd,0x00},
	{0xb2,0x18},
    {0xb3,0x0f},
    {0xb4,0x20},///30
    {0xb5,0x35},///45
    //ae setting 24MHz 12~10fps
    {0xfd,0x00},
    {0x03,0x00},
    {0x04,0xb4},
    {0x05,0x00},
    {0x06,0x00},
    {0x09,0x04},
    {0x0a,0x74},
    {0xf0,0x3c},
    {0xf1,0x00},
    {0xfd,0x01},
    {0x90,0x0a},
    {0x92,0x01},
    {0x98,0x3c},
    {0x99,0x00},
    {0x9a,0x03},
    {0x9b,0x00},
    {0xfd,0x01},
    {0xce,0x58},
    {0xcf,0x02},           
    {0xd0,0x58},
    {0xd1,0x02},
    {0xfd,0x00},
    
    {0xfd,0x01},
    {0xc4,0x6c},
    {0xc5,0x7c},
    {0xca,0x30},
    {0xcb,0x45},
    {0xcc,0x60},
    {0xcd,0x60},
	//func enable
	{0xfd,0x00},
    {0x31,0x70},
	{0x32,0x0d},
	{0x34,0x7e},
	{0x33,0xef},//ef
	{0x35,0x00},
    {0xf4,0x09},
	//{0xe7,0x00},//

//color bar
//{0xfd, 0x00},
//{0x0d, 0x14},
};

static struct msm_camera_i2c_reg_conf sp0a28_saturation[][9] = {
	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 - 0x80},
		{0xd4, SP0A28_P0_0xd4 - 0x80},
		{0xd6, SP0A28_P0_0xd6 - 0x80},
		{0xd7, SP0A28_P0_0xd7 - 0x80},
		{0xd8, SP0A28_P0_0xd8 - 0x80},
		{0xd9, SP0A28_P0_0xd9 - 0x80},
		{0xda, SP0A28_P0_0xda - 0x80},
		{0xdb, SP0A28_P0_0xdb - 0x80},}, /* SATURATION LEVEL0*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 - 0x70},
		{0xd4, SP0A28_P0_0xd4 - 0x70},
		{0xd6, SP0A28_P0_0xd6 - 0x70},
		{0xd7, SP0A28_P0_0xd7 - 0x70},
		{0xd8, SP0A28_P0_0xd8 - 0x70},
		{0xd9, SP0A28_P0_0xd9 - 0x70},
		{0xda, SP0A28_P0_0xda - 0x70},
		{0xdb, SP0A28_P0_0xdb - 0x70},}, /* SATURATION LEVEL1*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 - 0x60},
		{0xd4, SP0A28_P0_0xd4 - 0x60},
		{0xd6, SP0A28_P0_0xd6 - 0x60},
		{0xd7, SP0A28_P0_0xd7 - 0x60},
		{0xd8, SP0A28_P0_0xd8 - 0x60},
		{0xd9, SP0A28_P0_0xd9 - 0x60},
		{0xda, SP0A28_P0_0xda - 0x60},
		{0xdb, SP0A28_P0_0xdb - 0x60},}, /* SATURATION LEVEL2*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 - 0x40},
		{0xd4, SP0A28_P0_0xd4 - 0x40},
		{0xd6, SP0A28_P0_0xd6 - 0x40},
		{0xd7, SP0A28_P0_0xd7 - 0x40},
		{0xd8, SP0A28_P0_0xd8 - 0x40},
		{0xd9, SP0A28_P0_0xd9 - 0x40},
		{0xda, SP0A28_P0_0xda - 0x40},
		{0xdb, SP0A28_P0_0xdb - 0x40},}, /* SATURATION LEVEL3*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 - 0x20},
		{0xd4, SP0A28_P0_0xd4 - 0x20},
		{0xd6, SP0A28_P0_0xd6 - 0x20},
		{0xd7, SP0A28_P0_0xd7 - 0x20},
		{0xd8, SP0A28_P0_0xd8 - 0x20},
		{0xd9, SP0A28_P0_0xd9 - 0x20},
		{0xda, SP0A28_P0_0xda - 0x20},
		{0xdb, SP0A28_P0_0xdb - 0x20},}, /* SATURATION LEVEL4*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3},
		{0xd4, SP0A28_P0_0xd4},
		{0xd6, SP0A28_P0_0xd6},
		{0xd7, SP0A28_P0_0xd7},
		{0xd8, SP0A28_P0_0xd8},
		{0xd9, SP0A28_P0_0xd9},
		{0xda, SP0A28_P0_0xda},
		{0xdb, SP0A28_P0_0xdb},}, /* SATURATION LEVEL5*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 + 0x18},
		{0xd4, SP0A28_P0_0xd4 + 0x18},
		{0xd6, SP0A28_P0_0xd6 + 0x18},
		{0xd7, SP0A28_P0_0xd7 + 0x18},
		{0xd8, SP0A28_P0_0xd8 + 0x18},
		{0xd9, SP0A28_P0_0xd9 + 0x18},
		{0xda, SP0A28_P0_0xda + 0x18},
		{0xdb, SP0A28_P0_0xdb + 0x18},}, /* SATURATION LEVEL6*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 + 0x30},
		{0xd4, SP0A28_P0_0xd4 + 0x30},
		{0xd6, SP0A28_P0_0xd6 + 0x30},
		{0xd7, SP0A28_P0_0xd7 + 0x30},
		{0xd8, SP0A28_P0_0xd8 + 0x30},
		{0xd9, SP0A28_P0_0xd9 + 0x30},
		{0xda, SP0A28_P0_0xda + 0x30},
		{0xdb, SP0A28_P0_0xdb + 0x30},}, /* SATURATION LEVEL7*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 + 0x48},
		{0xd4, SP0A28_P0_0xd4 + 0x48},
		{0xd6, SP0A28_P0_0xd6 + 0x48},
		{0xd7, SP0A28_P0_0xd7 + 0x48},
		{0xd8, SP0A28_P0_0xd8 + 0x48},
		{0xd9, SP0A28_P0_0xd9 + 0x48},
		{0xda, SP0A28_P0_0xda + 0x48},
		{0xdb, SP0A28_P0_0xdb + 0x48},}, /* SATURATION LEVEL8*/

	{
		{0xfd, 0x00},
		{0xd3, SP0A28_P0_0xd3 + 0x60},
		{0xd4, SP0A28_P0_0xd4 + 0x60},
		{0xd6, SP0A28_P0_0xd6 + 0x60},
		{0xd7, SP0A28_P0_0xd7 + 0x60},
		{0xd8, SP0A28_P0_0xd8 + 0x60},
		{0xd9, SP0A28_P0_0xd9 + 0x60},
		{0xda, SP0A28_P0_0xda + 0x60},
		{0xdb, SP0A28_P0_0xdb + 0x60},}, /* SATURATION LEVEL9*/
};

static struct msm_camera_i2c_reg_conf sp0a28_contrast[][3] = {
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd - 0x28},
	{0xde, SP0A28_P0_0xde - 0x28},
	}, /* CONTRAST L0*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd - 0x20},
	{0xde, SP0A28_P0_0xde - 0x20},
	}, /* CONTRAST L1*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd - 0x18},
	{0xde, SP0A28_P0_0xde - 0x18},
	}, /* CONTRAST L2*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd - 0x10},
	{0xde, SP0A28_P0_0xde - 0x10},
	}, /* CONTRAST L3*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd - 0x08},
	{0xde, SP0A28_P0_0xde - 0x08},
	}, /* CONTRAST L4*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd},
	{0xde, SP0A28_P0_0xde},
	}, /* CONTRAST L5*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd + 0x08},
	{0xde, SP0A28_P0_0xde + 0x08},
	}, /* CONTRAST L6*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd + 0x10},
	{0xde, SP0A28_P0_0xde + 0x10},
	}, /* CONTRAST L7*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd + 0x18},
	{0xde, SP0A28_P0_0xde + 0x18},
	}, /* CONTRAST L8*/
	{
	{0xfd, 0x00},
	{0xdd, SP0A28_P0_0xdd + 0x20},
	{0xde, SP0A28_P0_0xde + 0x20},
	}, /* CONTRAST L9*/
};


static struct msm_camera_i2c_reg_conf sp0a28_sharpness[][9] = {
	{
	{0xfd, 0x01},
	{0xe8, SP0A28_P1_0xe8 - 0x10},
	{0xec, SP0A28_P1_0xec - 0x10},
	{0xe9, SP0A28_P1_0xe9 - 0x10},
	{0xed, SP0A28_P1_0xed - 0x10},
	{0xea, SP0A28_P1_0xea - 0x10},
	{0xef, SP0A28_P1_0xef - 0x10},
	{0xeb, SP0A28_P1_0xeb - 0x10},
	{0xf0, SP0A28_P1_0xf0 - 0x10},
	}, /* SHARPNESS LEVEL 0*/
	{
	{0xfd, 0x01},
	{0xe8, SP0A28_P1_0xe8 - 0x08},
	{0xec, SP0A28_P1_0xec - 0x08},
	{0xe9, SP0A28_P1_0xe9 - 0x08},
	{0xed, SP0A28_P1_0xed - 0x08},
	{0xea, SP0A28_P1_0xea - 0x08},
	{0xef, SP0A28_P1_0xef - 0x08},
	{0xeb, SP0A28_P1_0xeb - 0x08},
	{0xf0, SP0A28_P1_0xf0 - 0x08},
	}, /* SHARPNESS LEVEL 1*/
	{
	{0xfd, 0x01},
	{0xe8, SP0A28_P1_0xe8},
	{0xec, SP0A28_P1_0xec},
	{0xe9, SP0A28_P1_0xe9},
	{0xed, SP0A28_P1_0xed},
	{0xea, SP0A28_P1_0xea},
	{0xef, SP0A28_P1_0xef},
	{0xeb, SP0A28_P1_0xeb},
	{0xf0, SP0A28_P1_0xf0},
	}, /* SHARPNESS LEVEL 2*/
	{
	{0xfd, 0x01},
	{0xe8, SP0A28_P1_0xe8 + 0x08},
	{0xec, SP0A28_P1_0xec + 0x08},
	{0xe9, SP0A28_P1_0xe9 + 0x08},
	{0xed, SP0A28_P1_0xed + 0x08},
	{0xea, SP0A28_P1_0xea + 0x08},
	{0xef, SP0A28_P1_0xef + 0x08},
	{0xeb, SP0A28_P1_0xeb + 0x08},
	{0xf0, SP0A28_P1_0xf0 + 0x08},
	}, /* SHARPNESS LEVEL 3*/
	{
	{0xfd, 0x01},
	{0xe8, SP0A28_P1_0xe8 + 0x18},
	{0xec, SP0A28_P1_0xec + 0x18},
	{0xe9, SP0A28_P1_0xe9 + 0x18},
	{0xed, SP0A28_P1_0xed + 0x18},
	{0xea, SP0A28_P1_0xea + 0x18},
	{0xef, SP0A28_P1_0xef + 0x18},
	{0xeb, SP0A28_P1_0xeb + 0x18},
	{0xf0, SP0A28_P1_0xf0 + 0x18},
	}, /* SHARPNESS LEVEL 4*/
	{
	{0xfd, 0x01},
	{0xe8, SP0A28_P1_0xe8 + 0x28},
	{0xec, SP0A28_P1_0xec + 0x28},
	{0xe9, SP0A28_P1_0xe9 + 0x28},
	{0xed, SP0A28_P1_0xed + 0x28},
	{0xea, SP0A28_P1_0xea + 0x28},
	{0xef, SP0A28_P1_0xef + 0x28},
	{0xeb, SP0A28_P1_0xeb + 0x28},
	{0xf0, SP0A28_P1_0xf0 + 0x28},
	}, /* SHARPNESS LEVEL 5*/
	{
	{0xfd, 0x01},
	{0xe8, SP0A28_P1_0xe8 + 0x48},
	{0xec, SP0A28_P1_0xec + 0x48},
	{0xe9, SP0A28_P1_0xe9 + 0x48},
	{0xed, SP0A28_P1_0xed + 0x48},
	{0xea, SP0A28_P1_0xea + 0x48},
	{0xef, SP0A28_P1_0xef + 0x48},
	{0xeb, SP0A28_P1_0xeb + 0x48},
	{0xf0, SP0A28_P1_0xf0 + 0x48},
	}, /* SHARPNESS LEVEL 6*/
};

static struct msm_camera_i2c_reg_conf sp0a28_exposure[][2] = {
	{{0xfd,0x00},{0xdc,0xe0},}, /*EXPOSURECOMPENSATIONN2*/
	{{0xfd,0x00},{0xdc,0xf0},}, /*EXPOSURECOMPENSATIONN1*/
	{{0xfd,0x00},{0xdc,0x00},}, /*EXPOSURECOMPENSATIOND*/
	{{0xfd,0x00},{0xdc,0x10},}, /*EXPOSURECOMPENSATIONP1*/
	{{0xfd,0x00},{0xdc,0x20},}, /*EXPOSURECOMPENSATIONP2*/
};

// sp0a28 not suport!
static struct msm_camera_i2c_reg_conf sp0a28_iso[][2] = {
	{{0xfd, 0x00},{0x24, 0x80},},/*ISO_AUTO*/
	{{0xfd, 0x00},{0x24, 0x80},},/*ISO_DEBLUR*/
	{{0xfd, 0x00},{0x24, 0x20},},/*ISO_100*/
	{{0xfd, 0x00},{0x24, 0x30},},/*ISO_200*/
	{{0xfd, 0x00},{0x24, 0x40},},/*ISO_400*/
	{{0xfd, 0x00},{0x24, 0x50},},/*ISO_800*/
	{{0xfd, 0x00},{0x24, 0x60},},/*ISO_1600*/
};





static struct msm_camera_i2c_reg_conf sp0a28_special_effect[][4] = {
	{{0xfd, 0x00},{0x62, 0x00},{0x63, 0x80},{0x64, 0x80},}, /*for special effect OFF*/
	{{0xfd, 0x00},{0x62, 0x20},{0x63, 0x80},{0x64, 0x80},}, /*for special effect MONO*/
	{{0xfd, 0x00},{0x62, 0x04},{0x63, 0x80},{0x64, 0x80},}, /*for special efefct Negative*/
	{{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1},}, /*Solarize is not supported by sensor*/
	{{0xfd, 0x00},{0x62, 0x10},{0x63, 0x8c},{0x64, 0x70},}, /*for sepia*/
	{{0xfd, 0x00},{0x62, 0x00},{0x63, 0x80},{0x64, 0x80},}, /*Posteraize*/
	{{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1},}, /* White board not supported*/
	{{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1},}, /*Blackboard not supported*/
	{{0xfd, 0x00},{0x62, 0x10},{0x63, 0x20},{0x64, 0xb0},}, /*Aqua */
	{{0xfd, 0x00},{0x62, 0x01},{0x63, 0x80},{0x64, 0x80},}, /*Emboss */
	{{0xfd, 0x00},{0x62, 0x02},{0x63, 0x80},{0x64, 0x80},}, /*sketch */
	{{0xfd, 0x00},{0x62, 0x10},{0x63, 0xc0},{0x64, 0x38},}, /*Neno*/
	{{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1},}, /*MAX not supported*/
};




static struct msm_camera_i2c_reg_conf sp0a28_antibanding[][22] = {
	{
{0xfd,0x00},
{0x03,0x00},
{0x04,0x96},
{0x05,0x00},
{0x06,0x00},
{0x09,0x04},
{0x0a,0x74},
{0xf0,0x32},
{0xf1,0x00},
{0xfd,0x01},
{0x90,0x0c},
{0x92,0x01},
{0x98,0x32},
{0x99,0x00},
{0x9a,0x03},
{0x9b,0x00},
{0xfd,0x01},
{0xce,0x58},
{0xcf,0x02},
{0xd0,0x58},
{0xd1,0x02},
{0xfd,0x00},
	}, /*ANTIBANDING 60HZ*/
	{
//ae setting fix 10-12fps
	{0xfd,0x00},
	{0x03,0x00},
	{0x04,0xb4},
	{0x05,0x00},
	{0x06,0x00},
	{0x09,0x04},
	{0x0a,0x74},
	{0xf0,0x3c},
	{0xf1,0x00},
	{0xfd,0x01},
	{0x90,0x0a},
	{0x92,0x01},
	{0x98,0x3c},
	{0x99,0x00},
	{0x9a,0x03},
	{0x9b,0x00},
	{0xfd,0x01},
	{0xce,0x58},
	{0xcf,0x02},
	{0xd0,0x58},
	{0xd1,0x02},
	{0xfd,0x00},
	}, /*ANTIBANDING 50HZ*/
	{
//ae setting fix 10-12fps
	{0xfd,0x00},
	{0x03,0x00},
	{0x04,0xb4},
	{0x05,0x00},
	{0x06,0x00},
	{0x09,0x04},
	{0x0a,0x74},
	{0xf0,0x3c},
	{0xf1,0x00},
	{0xfd,0x01},
	{0x90,0x0a},
	{0x92,0x01},
	{0x98,0x3c},
	{0x99,0x00},
	{0x9a,0x03},
	{0x9b,0x00},
	{0xfd,0x01},
	{0xce,0x58},
	{0xcf,0x02},
	{0xd0,0x58},
	{0xd1,0x02},
	{0xfd,0x00},
	}, /*ANTIBANDING AUTO*/
};

static struct msm_camera_i2c_reg_conf sp0a28_wb_oem[][5] = {
	{{0xfd, 0x01},{0x28, 0xa0},{0x29, 0x80},{0xfd, 0x00},{0xfd, 0x00},}, /*WHITEBALNACE AUTO*/
	{{0xfd, 0x01},{0x28, 0xa0},{0x29, 0x80},{0xfd, 0x00},{0xfd, 0x00},}, /*WHITEBALNACE CUSTOM*/
	{{0xfd, 0x00},{0xfd, 0x01},{0x28, 0x68},{0x29, 0xa0},{0xfd, 0x00},}, /*INCANDISCENT*/
	{{0xfd, 0x00},{0xfd, 0x01},{0x28, 0x70},{0x29, 0x9c},{0xfd, 0x00},}, /*FLOURESECT*/
	{{0xfd, 0x00},{0xfd, 0x01},{0x28, 0x88},{0x29, 0x80},{0xfd, 0x00},}, /*DAYLIGHT*/
	{{0xfd, 0x00},{0xfd, 0x01},{0x28, 0x90},{0x29, 0x6c},{0xfd, 0x00},}, /*CLOUDY*/
	{{0xfd, 0x00},{0xfd, 0x00},{0xfd, 0x00},{0xfd, 0x00},{0xfd, 0x00},}, /*WHITEBALNACE OFF*/
};




static struct v4l2_subdev_info sp0a28_subdev_info[] = {
	{
		//.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.code   = V4L2_MBUS_FMT_UYVY8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order  = 0,
	},
};

static const struct i2c_device_id sp0a28_i2c_id[] = {
	{SP0a28_SENSOR_NAME, (kernel_ulong_t)&sp0a28_s_ctrl},
	{ }
};

static int32_t msm_sp0a28_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	CDBG("%s, E. ", __func__);

	return msm_sensor_i2c_probe(client, id, &sp0a28_s_ctrl);
}

static struct i2c_driver sp0a28_i2c_driver = {
	.id_table = sp0a28_i2c_id,
	.probe  = msm_sp0a28_i2c_probe,
	.driver = {
		.name = SP0a28_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client sp0a28_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

static const struct of_device_id sp0a28_dt_match[] = {
	{.compatible = "qcom,sp0a28", .data = &sp0a28_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, sp0a28_dt_match);

static struct platform_driver sp0a28_platform_driver = {
	.driver = {
		.name = "qcom,sp0a28",
		.owner = THIS_MODULE,
		.of_match_table = sp0a28_dt_match,
	},
};

static int32_t sp0a28_platform_probe(struct platform_device *pdev)
{
	int32_t rc;
	const struct of_device_id *match;
	CDBG("%s, E.", __func__);
	match = of_match_device(sp0a28_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init sp0a28_init_module(void)
{
	int32_t rc;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&sp0a28_platform_driver,
		sp0a28_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&sp0a28_i2c_driver);
}

static void __exit sp0a28_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (sp0a28_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&sp0a28_s_ctrl);
		platform_driver_unregister(&sp0a28_platform_driver);
	} else
		i2c_del_driver(&sp0a28_i2c_driver);
	return;
}

int32_t sp0a28_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	CDBG("%s:%d %s cfgtype = %d\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++)
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
		CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		CDBG("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);

		break;
	case CFG_SET_INIT_SETTING:
		/* Write Recommend settings */
		pr_err("%s, sensor write init setting!!", __func__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_recommend_settings,
			ARRAY_SIZE(sp0a28_recommend_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_RESOLUTION:
		break;
	case CFG_SET_STOP_STREAM:
		pr_err("%s, sensor stop stream!!", __func__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_stop_settings,
			ARRAY_SIZE(sp0a28_stop_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_START_STREAM:
		pr_err("%s, sensor start stream!!", __func__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_start_settings,
			ARRAY_SIZE(sp0a28_start_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		cdata->cfg.sensor_init_params =
			*s_ctrl->sensordata->sensor_init_params;
		CDBG("%s:%d init params mode %d pos %d mount %d\n", __func__,
			__LINE__,
			cdata->cfg.sensor_init_params.modes_supported,
			cdata->cfg.sensor_init_params.position,
			cdata->cfg.sensor_init_params.sensor_mount_angle);
		break;
	case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info sensor_slave_info;
		struct msm_sensor_power_setting_array *power_setting_array;
		int slave_index = 0;
		if (copy_from_user(&sensor_slave_info,
		    (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_sensor_slave_info))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		/* Update sensor slave address */
		if (sensor_slave_info.slave_addr) {
			s_ctrl->sensor_i2c_client->cci_client->sid =
				sensor_slave_info.slave_addr >> 1;
		}

		/* Update sensor address type */
		s_ctrl->sensor_i2c_client->addr_type =
			sensor_slave_info.addr_type;

		/* Update power up / down sequence */
		s_ctrl->power_setting_array =
			sensor_slave_info.power_setting_array;
		power_setting_array = &s_ctrl->power_setting_array;
		power_setting_array->power_setting = kzalloc(
			power_setting_array->size *
			sizeof(struct msm_sensor_power_setting), GFP_KERNEL);
		if (!power_setting_array->power_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(power_setting_array->power_setting,
		    (void *)sensor_slave_info.power_setting_array.power_setting,
		    power_setting_array->size *
		    sizeof(struct msm_sensor_power_setting))) {
			kfree(power_setting_array->power_setting);
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		s_ctrl->free_power_setting = true;
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.slave_addr);
		CDBG("%s sensor addr type %d\n", __func__,
			sensor_slave_info.addr_type);
		CDBG("%s sensor reg %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id_reg_addr);
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id);
		for (slave_index = 0; slave_index <
			power_setting_array->size; slave_index++) {
			CDBG("%s i %d power setting %d %d %ld %d\n", __func__,
				slave_index,
				power_setting_array->power_setting[slave_index].
				seq_type,
				power_setting_array->power_setting[slave_index].
				seq_val,
				power_setting_array->power_setting[slave_index].
				config_val,
				power_setting_array->power_setting[slave_index].
				delay);
		}
		kfree(power_setting_array->power_setting);
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_POWER_DOWN:
		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_SET_STOP_STREAM_SETTING: {
		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;
		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
		    (void *)reg_setting, stop_setting->size *
		    sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
	}

	case CFG_SET_SATURATION: {
		int32_t sat_lev;
		if (copy_from_user(&sat_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Saturation Value is %d", __func__, sat_lev);

        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_saturation[sat_lev],
			ARRAY_SIZE(sp0a28_saturation[sat_lev]),
			MSM_CAMERA_I2C_BYTE_DATA);

		break;
	}
	case CFG_SET_CONTRAST: {
		int32_t con_lev;
		if (copy_from_user(&con_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Contrast Value is %d", __func__, con_lev);
        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_contrast[con_lev],
			ARRAY_SIZE(sp0a28_contrast[con_lev]),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	}
	case CFG_SET_SHARPNESS: {
		int32_t shp_lev;
		if (copy_from_user(&shp_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Sharpness Value is %d", __func__, shp_lev);
        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_sharpness[shp_lev],
			ARRAY_SIZE(sp0a28_sharpness[shp_lev]),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	}
	case CFG_SET_ISO: {
		int32_t iso_lev;
		if (copy_from_user(&iso_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: ISO Value is %d", __func__, iso_lev);
        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_iso[iso_lev],
			ARRAY_SIZE(sp0a28_iso[iso_lev]),
			MSM_CAMERA_I2C_BYTE_DATA);
			break;
	}
	case CFG_SET_EXPOSURE_COMPENSATION: {
		int32_t ec_lev;
		if (copy_from_user(&ec_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Exposure compensation Value is %d",
			__func__, ec_lev);
        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_exposure[ec_lev],
			ARRAY_SIZE(sp0a28_exposure[ec_lev]),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	}
	case CFG_SET_EFFECT: {
		int32_t effect_mode;
		if (copy_from_user(&effect_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Effect mode is %d", __func__, effect_mode);
        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_special_effect[effect_mode],
			ARRAY_SIZE(sp0a28_special_effect[effect_mode]),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	}
	case CFG_SET_ANTIBANDING: {
		int32_t antibanding_mode;
		if (copy_from_user(&antibanding_mode,
			(void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: anti-banding mode is %d", __func__,
			antibanding_mode);
        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_antibanding[antibanding_mode],
			ARRAY_SIZE(sp0a28_antibanding[antibanding_mode]),
			MSM_CAMERA_I2C_BYTE_DATA);

		break;
	}
	/*
	case CFG_SET_BESTSHOT_MODE: {
		int32_t bs_mode;
		if (copy_from_user(&bs_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: best shot mode is %d", __func__, bs_mode);

		break;
	}
	*/
	case CFG_SET_WHITE_BALANCE: {
		int32_t wb_mode;
    	int rc = 0;
    	unsigned short temp = 0;

		if (copy_from_user(&wb_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write(s_ctrl->sensor_i2c_client,
			0xfd,
			0x00,
			MSM_CAMERA_I2C_BYTE_DATA);

        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_read(s_ctrl->sensor_i2c_client,
			0x32,
			&temp,
			MSM_CAMERA_I2C_BYTE_DATA);

	switch (wb_mode)
	{
	case MSM_CAMERA_WB_MODE_AUTO:
		temp |= 0x08;
		break;
	case MSM_CAMERA_WB_MODE_CUSTOM:
		temp |= 0x08;
		break;

	case MSM_CAMERA_WB_MODE_OFF:
	case MSM_CAMERA_WB_MODE_INCANDESCENT:
		temp &= (~(0x08));
		break;
	case MSM_CAMERA_WB_MODE_WARM_FLUORESCENT:
		temp &= (~(0x08));
		break;
	case MSM_CAMERA_WB_MODE_DAYLIGHT:
		temp &= (~(0x08));
		break;
	case MSM_CAMERA_WB_MODE_CLOUDY_DAYLIGHT:
		temp &= (~(0x08));
		break;
	}

        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write(s_ctrl->sensor_i2c_client,
			0x32,
			temp,
			MSM_CAMERA_I2C_BYTE_DATA);


		CDBG("%s: white balance is %d", __func__, wb_mode);
        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			sp0a28_wb_oem[wb_mode],
			ARRAY_SIZE(sp0a28_wb_oem[wb_mode]),
			MSM_CAMERA_I2C_BYTE_DATA);
			break;
	}
	default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}

int32_t sp0a28_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	uint16_t chipid = 0;

	CDBG("%s,  E. calling i2c_read:, i2c_addr:%d, id_reg_addr:%d",
		__func__,
		s_ctrl->sensordata->slave_info->sensor_slave_addr,
		s_ctrl->sensordata->slave_info->sensor_id_reg_addr);

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			0x02,
			&chipid, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
		return rc;
	}

	CDBG("%s: read  id: %x ; expected id: 0xa2\n", __func__, chipid);
	if (chipid != 0xa2) {
		pr_err("msm_sensor_match_id chip id doesnot match \n");
		return -ENODEV;
	}

	return rc;
}


static struct msm_sensor_fn_t sp0a28_sensor_func_tbl = {
	.sensor_config = sp0a28_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = sp0a28_match_id,
};

static struct msm_sensor_ctrl_t sp0a28_s_ctrl = {
	.sensor_i2c_client = &sp0a28_sensor_i2c_client,
	.power_setting_array.power_setting = sp0a28_power_setting,
	.power_setting_array.size = ARRAY_SIZE(sp0a28_power_setting),
	.msm_sensor_mutex = &sp0a28_mut,
	.sensor_v4l2_subdev_info = sp0a28_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(sp0a28_subdev_info),
	.func_tbl = &sp0a28_sensor_func_tbl,
};

module_init(sp0a28_init_module);
module_exit(sp0a28_exit_module);
MODULE_DESCRIPTION("Aptina 1.26MP YUV sensor driver");
MODULE_LICENSE("GPL v2");
