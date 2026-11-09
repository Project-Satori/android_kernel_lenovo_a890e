/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
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

#define OV5648_SENSOR_NAME "ov5648"
DEFINE_MSM_MUTEX(ov5648_mut);

#define OV5648_CDBG
#ifdef OV5648_CDBG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) pr_debug(fmt, ##args)
#endif
static struct msm_sensor_ctrl_t ov5648_s_ctrl;
int OTP_read = 0;
struct otp_struct {
     int customer_id;
     int module_integrator_id;
     int lens_id;
     int rg_ratio;
     int bg_ratio;
     int user_data[2];
     int light_rg;
     int light_bg;
}current_otp;
static int RG_Ratio_Typical = 0x15b;
static int BG_Ratio_Typical = 0x162;
static int R_gain_final, G_gain_final, B_gain_final;

static struct msm_sensor_power_setting ov5648_power_setting[] = {
/*
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VDIG,
		.config_val = GPIO_OUT_LOW,
		.delay = 10,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VDIG,
		.config_val = GPIO_OUT_HIGH,
		.delay = 10,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 10,
	},

	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 5,
	},
*/

	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VAF,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 30,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 30,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 30,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct v4l2_subdev_info ov5648_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id ov5648_i2c_id[] = {
	{OV5648_SENSOR_NAME,
		(kernel_ulong_t)&ov5648_s_ctrl},
	{ }
};

int ov5648_truly_cm8352_otp_write(struct msm_sensor_ctrl_t *s_ctrl,int addr, uint16_t data)
{
		//CDBG("%s addr[0x%x] data=0x%x \n",__func__, addr, data);
	return s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
			s_ctrl->sensor_i2c_client, addr, data, MSM_CAMERA_I2C_BYTE_DATA);
}
uint16_t ov5648_truly_cm8352_otp_read(struct msm_sensor_ctrl_t *s_ctrl,uint32_t addr)
{
	int rc;
	uint16_t temp;
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client, addr, &temp,MSM_CAMERA_I2C_BYTE_DATA);
		CDBG("read otp register addrees[0x%x] ret_val=0x%x rc=0x%x\n", addr, temp,rc);
	return temp;
}
// index: index of otp group. (1, 2, 3)
// return: 0, group index is empty
//         1, group index has invalid data
//         2, group index has valid data
int check_otp(struct msm_sensor_ctrl_t *s_ctrl,int index)
{
     int flag=0, i;
	 int rg=0, bg=0;
	 //uint16_t chipid = 0;
		CDBG("%s index=0x%x\n",__func__, index);

     if (index == 1)
     {
         // read otp --Bank 0
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x00);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x0f);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
         msleep(10);
         flag =ov5648_truly_cm8352_otp_read(s_ctrl,0x3d05);
         rg = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d07);
         bg = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d08);


      }
      else if (index == 2)
      {
                        // read otp --Bank 0
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x00);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x0f);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
         msleep(10);
         flag = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0e);
         // read otp --Bank 1
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x10);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x1f);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
         msleep(10);
         rg = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d00);
         bg = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d01);


    }
    else if (index == 3)
    {
              // read otp --Bank 1
        ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
        ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x10);
        ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x1f);
        ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
         msleep(10);
        flag = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d07);
        rg = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d09);
        bg = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0a);


    }
        //chipid =ov5648_truly_cm8352_otp_read(s_ctrl,0x300a);


  flag = flag & 0x80;
     // clear otp buffer
     for (i=0;i<16;i++) {
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d00 + i, 0x00);
     }
  if (flag) {
              return 1;
  }
  else
  {
              if (rg == 0 && bg == 0)
              {
                         return 0;
              }
              else
              {
                         return 2;
              }
  }
}



// index: index of otp group. (1, 2, 3)
// return: 0,
int read_otp(struct msm_sensor_ctrl_t *s_ctrl,int index, struct otp_struct *otp_ptr)
{
         int i;
         int address;
         index=index-1;
         // read OTP into buffer
         if (index == 0)
         {
                   // read OTP --Bank 0
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x00);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x0f);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
                   msleep(1);
                   address = 0x3d05 + index*9;
		           CDBG("%s address=0x%x\n",__func__, address);

                   (*otp_ptr).customer_id = (ov5648_truly_cm8352_otp_read(s_ctrl,address) & 0x7f);
                   (*otp_ptr).module_integrator_id = ov5648_truly_cm8352_otp_read(s_ctrl,address);
                   (*otp_ptr).lens_id = ov5648_truly_cm8352_otp_read(s_ctrl,address + 1);
                   (*otp_ptr).rg_ratio = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 2)<<2) + (ov5648_truly_cm8352_otp_read(s_ctrl,address + 6)>>6) ;
                   (*otp_ptr).bg_ratio = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 3)<<2) +((ov5648_truly_cm8352_otp_read(s_ctrl,address + 6)>>4)&(0x03));
                   (*otp_ptr).user_data[0] = ov5648_truly_cm8352_otp_read(s_ctrl,address + 4);
                   (*otp_ptr).user_data[1] = ov5648_truly_cm8352_otp_read(s_ctrl,address + 5);
                   (*otp_ptr).light_rg = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 7)<<2) +( (ov5648_truly_cm8352_otp_read(s_ctrl,address + 6)>>2)&(0x03));
                   (*otp_ptr).light_bg = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 8)<<2) + ((ov5648_truly_cm8352_otp_read(s_ctrl,address + 6))&(0x03));
                   // disable OTP read
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x00);
         }
         else if(index == 2)
         {
                   // read OTP --Bank 1
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x10);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x1f);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
                   msleep(1);
                   address = 0x3d05 + index*9-16;
		           CDBG("%s address=0x%x\n",__func__, address);

                   (*otp_ptr).customer_id = (ov5648_truly_cm8352_otp_read(s_ctrl,address) & 0x7f);
                   (*otp_ptr).module_integrator_id = ov5648_truly_cm8352_otp_read(s_ctrl,address);
                   (*otp_ptr).lens_id = ov5648_truly_cm8352_otp_read(s_ctrl,address + 1);
                   (*otp_ptr).rg_ratio = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 2)<<2) + (ov5648_truly_cm8352_otp_read(s_ctrl,address + 6)>>6) ;
                   (*otp_ptr).bg_ratio = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 3)<<2) +((ov5648_truly_cm8352_otp_read(s_ctrl,address + 6)>>4)&(0x03));
                   (*otp_ptr).user_data[0] = ov5648_truly_cm8352_otp_read(s_ctrl,address + 4);
                   (*otp_ptr).user_data[1] = ov5648_truly_cm8352_otp_read(s_ctrl,address + 5);
                   (*otp_ptr).light_rg = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 7)<<2) + ((ov5648_truly_cm8352_otp_read(s_ctrl,address + 6)>>2)&(0x03));
                   (*otp_ptr).light_bg = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 8)<<2) + ((ov5648_truly_cm8352_otp_read(s_ctrl,address + 6))&(0x03));
                   // disable OTP read
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x00);
         }
         else
         {
                   // read OTP --Bank 0
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x00);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x0f);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
                   msleep(1);
                   address = 0x3d05 + index*9;
		           CDBG("%s address=0x%x\n",__func__, address);

                   (*otp_ptr).customer_id = (ov5648_truly_cm8352_otp_read(s_ctrl,address) & 0x7f);
                   (*otp_ptr).module_integrator_id = ov5648_truly_cm8352_otp_read(s_ctrl,address);
                   (*otp_ptr).lens_id = ov5648_truly_cm8352_otp_read(s_ctrl,address + 1);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x00);
                   // read OTP --Bank 1
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x10);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x1f);
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
                   msleep(1);
                   address = 0x3d00;
                   (*otp_ptr).rg_ratio = (ov5648_truly_cm8352_otp_read(s_ctrl,address)<<2) + (ov5648_truly_cm8352_otp_read(s_ctrl,address + 4)>>6) ;
                   (*otp_ptr).bg_ratio = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 1)<<2) +((ov5648_truly_cm8352_otp_read(s_ctrl,address + 4)>>4)&(0x03));
                   (*otp_ptr).user_data[0] = ov5648_truly_cm8352_otp_read(s_ctrl,address + 2);
                   (*otp_ptr).user_data[1] = ov5648_truly_cm8352_otp_read(s_ctrl,address + 3);
                   (*otp_ptr).light_rg = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 5)<<2) + ((ov5648_truly_cm8352_otp_read(s_ctrl,address + 4)>>2)&(0x03));
                   (*otp_ptr).light_bg = (ov5648_truly_cm8352_otp_read(s_ctrl,address + 6)<<2) + ((ov5648_truly_cm8352_otp_read(s_ctrl,address + 4))&(0x03));
                   // disable OTP read
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x00);
         }

     CDBG("==========OV5648 OTP INFO==========\r\n");
     CDBG("%s-customer_id  = 0x%x\r\n", __func__, otp_ptr->customer_id);
     CDBG("%s-module_integrator_id      = 0x%x\r\n", __func__, otp_ptr->module_integrator_id);
     CDBG("%s-lens_id     = 0x%x\r\n", __func__, otp_ptr->lens_id);
     CDBG("%s-rg_ratio     = 0x%x\r\n", __func__, otp_ptr->rg_ratio);
     CDBG("%s-bg_ratio     = 0x%x\r\n", __func__, otp_ptr->bg_ratio);

     CDBG("%s-user_data[0] = 0x%x\r\n", __func__, otp_ptr->user_data[0]);
     CDBG("%s-user_data[1] = 0x%x\r\n", __func__, otp_ptr->user_data[1]);
     CDBG("%s-light_rg = 0x%x\r\n", __func__, otp_ptr->light_rg);
     CDBG("%s-light_bg = 0x%x\r\n", __func__, otp_ptr->light_bg);
     CDBG("%s-RG_Ratio_Typical    = 0x%x\r\n", __func__, RG_Ratio_Typical);
     CDBG("%s-BG_Ratio_Typical    = 0x%x\r\n", __func__, BG_Ratio_Typical);

         // clear OTP buffer
         for (i=0;i<16;i++)
         {
                   ov5648_truly_cm8352_otp_write(s_ctrl,0x3d00 + i, 0x00);
         }
     CDBG("%s-otp read over\r\n", __func__);

         return 0;
}
/*
{
//int i;
int temp;
// read otp into buffer
if (index == 1)
{
           // read otp --Bank 0
           ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
           ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x00);
           ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x0f);
           ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
           msleep(5);
            (*otp_ptr).module_integrator_id = (ov5648_truly_cm8352_otp_read(s_ctrl,0x3d05) & 0x7f);
            (*otp_ptr).lens_id = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d06);
            temp = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0b);
            (*otp_ptr).rg_ratio = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d07)<<2) + ((temp>>6) & 0x03));
            (*otp_ptr).bg_ratio = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d08)<<2) + ((temp>>4) & 0x03));
            (*otp_ptr).light_rg = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0c)<<2) + ((temp>>2) & 0x03));
            (*otp_ptr).light_bg = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0d)<<2) + (temp & 0x03));
            (*otp_ptr).user_data[0] = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d09);
            (*otp_ptr).user_data[1] = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0a);
}
else if (index == 2)
{
            // read otp --Bank 0
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x00);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x0f);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
           msleep(5);
            (*otp_ptr).module_integrator_id = (ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0e) & 0x7f);
            (*otp_ptr).lens_id = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0f);
            // read otp --Bank 1
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x10);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x1f);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
            msleep(5);
            temp = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d04);
            (*otp_ptr).rg_ratio = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d00)<<2) + ((temp>>6) & 0x03));
            (*otp_ptr).bg_ratio = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d01)<<2) + ((temp>>4) & 0x03));
            (*otp_ptr).light_rg = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d05)<<2) + ((temp>>2) & 0x03));
            (*otp_ptr).light_bg = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d06)<<2) + (temp & 0x03));
            (*otp_ptr).user_data[0] = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d02);
            (*otp_ptr).user_data[1] = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d03);
}
else if (index == 3)
{
            // read otp --Bank 1
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d84, 0xc0);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d85, 0x10);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d86, 0x1f);
            ov5648_truly_cm8352_otp_write(s_ctrl,0x3d81, 0x01);
            msleep(5);
            (*otp_ptr).module_integrator_id = (ov5648_truly_cm8352_otp_read(s_ctrl,0x3d07) & 0x7f);
            (*otp_ptr).lens_id = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d08);
            temp = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0d);
            (*otp_ptr).rg_ratio = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d09)<<2) + ((temp>>6) & 0x03));
            (*otp_ptr).bg_ratio = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0a)<<2) + ((temp>>4) & 0x03));
            (*otp_ptr).light_rg = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0e)<<2) + ((temp>>2) & 0x03));
            (*otp_ptr).light_bg = ((ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0f)<<2) + (temp & 0x03));
            (*otp_ptr).user_data[0] = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0b);
            (*otp_ptr).user_data[1] = ov5648_truly_cm8352_otp_read(s_ctrl,0x3d0c);
}

     CDBG("==========OV5648 OTP INFO==========\r\n");
     CDBG("%s-customer_id  = 0x%x\r\n", __func__, otp_ptr->customer_id);
     CDBG("%s-module_integrator_id      = 0x%x\r\n", __func__, otp_ptr->module_integrator_id);
     CDBG("%s-lens_id     = 0x%x\r\n", __func__, otp_ptr->lens_id);
     CDBG("%s-rg_ratio     = 0x%x\r\n", __func__, otp_ptr->rg_ratio);
     CDBG("%s-bg_ratio     = 0x%x\r\n", __func__, otp_ptr->bg_ratio);

     CDBG("%s-user_data[0] = 0x%x\r\n", __func__, otp_ptr->user_data[0]);
     CDBG("%s-user_data[1] = 0x%x\r\n", __func__, otp_ptr->user_data[1]);
     CDBG("%s-light_rg = 0x%x\r\n", __func__, otp_ptr->light_rg);
     CDBG("%s-light_bg = 0x%x\r\n", __func__, otp_ptr->light_bg);

     // clear otp buffer
     for (i=0;i<16;i++) {
         ov5648_truly_cm8352_otp_write(s_ctrl,0x3d00 + i, 0x00);
     }

     CDBG("%s-otp read over\r\n", __func__);

  return 0;
}
*/
// R_gain, sensor red gain of AWB, 0x400 =1
// G_gain, sensor green gain of AWB, 0x400 =1
// B_gain, sensor blue gain of AWB, 0x400 =1
// return 0;
int ov5648_truly_cm8352_update_awb_gain(struct msm_sensor_ctrl_t *s_ctrl,int R_gain, int G_gain, int B_gain)
{
         CDBG("%s R_gain=0x%x,G_gain=0x%x,B_gain=0x%x\n",__func__,R_gain,G_gain,B_gain);

     if (R_gain>0x400) {
         ov5648_truly_cm8352_otp_write(s_ctrl,0x5186, R_gain>>8);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x5187, R_gain & 0x00ff);
     }

     if (G_gain>0x400) {
         ov5648_truly_cm8352_otp_write(s_ctrl,0x5188, G_gain>>8);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x5189, G_gain & 0x00ff);
     }

     if (B_gain>0x400) {
         ov5648_truly_cm8352_otp_write(s_ctrl,0x518a, B_gain>>8);
         ov5648_truly_cm8352_otp_write(s_ctrl,0x518b, B_gain & 0x00ff);
     }
     return 0;
}
// call this function after OV5648 initialization
// return: 0 update success
//          1, no OTP
int ov5648_update_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
            int i;
            int otp_index;
            int temp;
            int R_gain, G_gain, B_gain, G_gain_R, G_gain_B;
            int rg,bg;
            // R/G and B/G of current camera module is read out from sensor OTP
            // check first OTP with valid data
         if(OTP_read == 0)
         {

            for(i=1;i<=3;i++) {
                       temp = check_otp(s_ctrl,i);
                      CDBG("%s: check_otp=%d\n", __func__,temp);
                       if (temp == 2) {
                                 otp_index = i;
                      CDBG("%s: otp_index=%d\n", __func__,i);

                                 break;
                       }
            }
            if (i>3) {
                      CDBG("%s: no valid wb OTP data\n", __func__);
                       // no valid wb OTP data
                       return -1;
            }
            read_otp(s_ctrl,otp_index, &current_otp);
            OTP_read = 1;
         }
        if(OTP_read != 0)
        {
         CDBG("%s-RG_Ratio_Typical    = 0x%x\r\n", __func__, RG_Ratio_Typical);
         CDBG("%s-BG_Ratio_Typical    = 0x%x\r\n", __func__, BG_Ratio_Typical);
             if(current_otp.light_rg==0) {
             // no light source information in OTP
             rg = current_otp.rg_ratio;
         }
         else {
             // light source information found in OTP
             rg = current_otp.rg_ratio * (current_otp.light_rg +512) / 1024;
         }

         if(current_otp.light_bg==0) {
             // no light source information in OTP
             bg = current_otp.bg_ratio;
         }
         else {
             // light source information found in OTP
             bg = current_otp.bg_ratio * (current_otp.light_bg +512) / 1024;
         }


         //calculate G gain
         //0x400 = 1x gain
         if(bg < BG_Ratio_Typical) {
             if (rg< RG_Ratio_Typical) {
                  // current_otp.bg_ratio < BG_Ratio_typical &&
                  // current_otp.rg_ratio < RG_Ratio_typical
                  G_gain = 0x400;
                  B_gain = 0x400 * BG_Ratio_Typical / bg;
                  R_gain = 0x400 * RG_Ratio_Typical / rg;
             }
             else {
                  // current_otp.bg_ratio < BG_Ratio_typical &&
                  // current_otp.rg_ratio >= RG_Ratio_typical
                  R_gain = 0x400;
                  G_gain = 0x400 * rg / RG_Ratio_Typical;
                  B_gain = G_gain * BG_Ratio_Typical /bg;
             }
         }
         else {
             if (rg < RG_Ratio_Typical) {
                  // current_otp.bg_ratio >= BG_Ratio_typical &&
                  // current_otp.rg_ratio < RG_Ratio_typical
                  B_gain = 0x400;
                  G_gain = 0x400 * bg / BG_Ratio_Typical;
                  R_gain = G_gain * RG_Ratio_Typical / rg;
             }
             else {
                  // current_otp.bg_ratio >= BG_Ratio_typical &&
                  // current_otp.rg_ratio >= RG_Ratio_typical
                  G_gain_B = 0x400 * bg / BG_Ratio_Typical;
                  G_gain_R = 0x400 * rg / RG_Ratio_Typical;

                  if(G_gain_B > G_gain_R ) {
                      B_gain = 0x400;
                      G_gain = G_gain_B;
                      R_gain = G_gain * RG_Ratio_Typical /rg;
                  }
                  else {
                       R_gain = 0x400;
                       G_gain = G_gain_R;
                       B_gain = G_gain * BG_Ratio_Typical / bg;
                  }
             }
         }

         ov5648_truly_cm8352_update_awb_gain(s_ctrl,R_gain, G_gain, B_gain);
         R_gain_final = R_gain;
         G_gain_final = G_gain;
         B_gain_final = B_gain;
         CDBG("%s R_gain=0x%x,G_gain=0x%x,B_gain=0x%x\n",__func__,R_gain,G_gain,B_gain);
        }
  return 0;
}

int ov5648_otp_proc(struct msm_sensor_ctrl_t *s_ctrl,struct msm_sensor_otp_params *sensor_otp_params_ptr)
{
    int rc=0;
	if(!s_ctrl->sensor_i2c_client)
	{
		printk("sensor_i2c_client is null fail \n");
		return 0;

	}
	if(!s_ctrl->sensor_i2c_client->i2c_func_tbl)
	{
		printk("i2c_func_tbl is null fail\n");
		return 0;

	}
	if(!s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read)
	{
		printk("i2c_read is null fail\n");
		return 0;

	}
	if(!s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write)
	{
		printk("i2c_write is null fail\n");
		return 0;

	}

    rc=ov5648_update_otp(s_ctrl);
    if(rc<0)
    {
       pr_err("%s  ljk ov5648_update_otp fail",__func__);
    }
    else
    {
       (* sensor_otp_params_ptr).module_integrator_id=current_otp.module_integrator_id;
       pr_err("%s  ljk ov5648_update_otp MID=%d",__func__,(* sensor_otp_params_ptr).module_integrator_id);

    }
    return rc;
};

static int32_t msm_ov5648_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &ov5648_s_ctrl);
}

static struct i2c_driver ov5648_i2c_driver = {
	.id_table = ov5648_i2c_id,
	.probe  = msm_ov5648_i2c_probe,
	.driver = {
		.name = OV5648_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov5648_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static struct msm_sensor_ctrl_t ov5648_s_ctrl = {
	.sensor_i2c_client = &ov5648_sensor_i2c_client,
	.power_setting_array.power_setting = ov5648_power_setting,
	.power_setting_array.size =
			ARRAY_SIZE(ov5648_power_setting),
	.msm_sensor_mutex = &ov5648_mut,
	.sensor_v4l2_subdev_info = ov5648_subdev_info,
	.sensor_v4l2_subdev_info_size =
			ARRAY_SIZE(ov5648_subdev_info),
	.sensor_otp_proc = ov5648_otp_proc,
};

static const struct of_device_id ov5648_dt_match[] = {
	{
		.compatible = "qcom,ov5648",
		.data = &ov5648_s_ctrl
	},
	{}
};

MODULE_DEVICE_TABLE(of, ov5648_dt_match);

static struct platform_driver ov5648_platform_driver = {
	.driver = {
		.name = "qcom,ov5648",
		.owner = THIS_MODULE,
		.of_match_table = ov5648_dt_match,
	},
};

static int32_t ov5648_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
    pr_err("%s  ljk here",__func__);

	match = of_match_device(ov5648_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init ov5648_init_module(void)
{
	int32_t rc = 0;
    pr_err("%s  ljk here",__func__);
	rc = platform_driver_probe(&ov5648_platform_driver,
		ov5648_platform_probe);
	if (!rc)
		return rc;
	return i2c_add_driver(&ov5648_i2c_driver);
}

static void __exit ov5648_exit_module(void)
{
	if (ov5648_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&ov5648_s_ctrl);
		platform_driver_unregister(&ov5648_platform_driver);
	} else
		i2c_del_driver(&ov5648_i2c_driver);
	return;
}

module_init(ov5648_init_module);
module_exit(ov5648_exit_module);
MODULE_DESCRIPTION("ov5648");
MODULE_LICENSE("GPL v2");
