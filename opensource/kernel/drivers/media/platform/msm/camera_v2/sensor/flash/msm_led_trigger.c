/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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

#define pr_fmt(fmt) "%s:%d " fmt, __func__, __LINE__

#include <mach/gpiomux.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include "msm_led_flash.h"
#include "msm_camera_io_util.h"
#include "../msm_sensor.h"
#include "msm_led_flash.h"
#define FLASH_NAME "camera-led-flash"

#define CONFIG_MSMB_CAMERA_DEBUG
#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

extern int32_t msm_led_torch_create_classdev(
				struct platform_device *pdev, void *data);

static enum flash_type flashtype;
static struct msm_led_flash_ctrl_t fctrl;

static int32_t msm_led_trigger_get_subdev_id(struct msm_led_flash_ctrl_t *fctrl,
	void *arg)
{
	uint32_t *subdev_id = (uint32_t *)arg;
	if (!subdev_id) {
		pr_err("%s:%d failed\n", __func__, __LINE__);
		return -EINVAL;
	}
	*subdev_id = fctrl->pdev->id;
	CDBG("%s:%d subdev_id %d\n", __func__, __LINE__, *subdev_id);
	return 0;
}
static int32_t msm_flash_init_gpio_pin_tbl(struct device_node *of_node,
	struct msm_camera_gpio_conf *gconf, uint16_t *gpio_array,
	uint16_t gpio_array_size)
{
	int32_t rc = 0;
	int32_t val = 0;

	gconf->gpio_num_info = kzalloc(sizeof(struct msm_camera_gpio_num_info),
		GFP_KERNEL);
	if (!gconf->gpio_num_info) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		rc = -ENOMEM;
		return rc;
	}
	rc = of_property_read_u32(of_node, "qcom,gpio-flash-en", &val);
	if (rc < 0) {
		pr_err("%s:%d read qcom,gpio-flash-en failed rc %d\n",
			__func__, __LINE__, rc);
		goto ERROR;
	} else if (val >gpio_array_size) {
		pr_err("%s:%d qcom,gpio-flash-en invalid %d\n",
			__func__, __LINE__, val);
		goto ERROR;
	}
	gconf->gpio_num_info->gpio_num[0] =
		gpio_array[val];
	CDBG("%s qcom,gpio-flash-en %d\n", __func__,
		gconf->gpio_num_info->gpio_num[0]);
	return rc;
ERROR:
	kfree(gconf->gpio_num_info);
	gconf->gpio_num_info = NULL;
	return rc;
}
static int32_t msm_led_get_dt_data(struct device_node *of_node,
		struct msm_led_flash_ctrl_t *fctrl)
{
	int32_t rc = 0, i = 0;
	struct msm_camera_gpio_conf *gconf = NULL;
	struct device_node *flash_src_node = NULL;
	struct msm_camera_sensor_board_info *flashdata = NULL;
	uint32_t count = 0;
	uint16_t *gpio_array = NULL;
	uint16_t gpio_array_size = 0;
	CDBG("msm_led_get_dt_data called\n");
	if (!of_node) {
		pr_err("of_node NULL\n");
		return -EINVAL;
	}
	fctrl->flashdata = kzalloc(sizeof(
		struct msm_camera_sensor_board_info),
		GFP_KERNEL);
	if (!fctrl->flashdata) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	flashdata = fctrl->flashdata;
	flashdata->sensor_init_params = kzalloc(sizeof(
		struct msm_sensor_init_params), GFP_KERNEL);
	if (!flashdata->sensor_init_params) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	rc = of_property_read_u32(of_node, "cell-index", &fctrl->subdev_id);
	if (rc < 0) {
		pr_err("failed\n");
		return -EINVAL;
	}
	CDBG("subdev id %d\n", fctrl->subdev_id);
	rc = of_property_read_string(of_node, "qcom,flash-name",
		&flashdata->sensor_name);
	CDBG("%s qcom,flash-name %s, rc %d\n", __func__,
		flashdata->sensor_name, rc);
	if (rc < 0) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		goto ERROR1;
	}
	if (of_get_property(of_node, "qcom,flash-source", &count)) {
		count /= sizeof(uint32_t);
		CDBG("count %d\n", count);
		if (count > MAX_LED_TRIGGERS) {
			pr_err("failed\n");
			return -EINVAL;
		}
		for (i = 0; i < count; i++) {
			flash_src_node = of_parse_phandle(of_node,
				"qcom,flash-source", i);
			if (!flash_src_node) {
				pr_err("flash_src_node NULL\n");
				continue;
			}
			rc = of_property_read_string(flash_src_node,
				"linux,default-trigger",
				&fctrl->flash_trigger_name[i]);
			if (rc < 0) {
				pr_err("failed\n");
				of_node_put(flash_src_node);
				continue;
			}
			CDBG("default trigger %s\n",
				 fctrl->flash_trigger_name[i]);
			rc = of_property_read_u32(flash_src_node,
				"qcom,max-current",
				&fctrl->flash_op_current[i]);
			if (rc < 0) {
				pr_err("failed rc %d\n", rc);
				of_node_put(flash_src_node);
				continue;
			}
			of_node_put(flash_src_node);
			CDBG("max_current[%d] %d\n",
				i, fctrl->flash_op_current[i]);
			led_trigger_register_simple(
				fctrl->flash_trigger_name[i],
				&fctrl->flash_trigger[i]);
		}
	} else { /*Handle LED Flash Ctrl by GPIO*/
		flashdata->gpio_conf =
			 kzalloc(sizeof(struct msm_camera_gpio_conf),
				 GFP_KERNEL);
		if (!flashdata->gpio_conf) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			rc = -ENOMEM;
			return rc;
		}
		gconf = flashdata->gpio_conf;
		gpio_array_size = of_gpio_count(of_node);
		CDBG("%s gpio count %d\n", __func__, gpio_array_size);
		if (gpio_array_size) {
			gpio_array = kzalloc(sizeof(uint16_t) * gpio_array_size,
				GFP_KERNEL);
			if (!gpio_array) {
				pr_err("%s failed %d\n", __func__, __LINE__);
				rc = -ENOMEM;
				goto ERROR4;
			}
			for (i = 0; i < gpio_array_size; i++) {
				gpio_array[i] = of_get_gpio(of_node, i);
				CDBG("%s gpio_array[%d] = %d\n", __func__, i,
					gpio_array[i]);
			}
			rc = msm_sensor_get_dt_gpio_req_tbl(of_node, gconf,
				gpio_array, gpio_array_size);
			if (rc < 0) {
				pr_err("%s failed %d\n", __func__, __LINE__);
				goto ERROR4;
			}
			rc = msm_sensor_get_dt_gpio_set_tbl(of_node, gconf,
				gpio_array, gpio_array_size);
			if (rc < 0) {
				pr_err("%s failed %d\n", __func__, __LINE__);
				goto ERROR5;
			}
			rc = msm_flash_init_gpio_pin_tbl(of_node, gconf,
				gpio_array, gpio_array_size);
			if (rc < 0) {
				pr_err("%s failed %d\n", __func__, __LINE__);
				goto ERROR6;
			}
		}
		kfree(gpio_array);
		return rc;
ERROR6:
		kfree(gconf->cam_gpio_set_tbl);
ERROR5:
		kfree(gconf->cam_gpio_req_tbl);
ERROR4:
		kfree(gconf);
ERROR1:
		kfree(fctrl->flashdata);
		kfree(gpio_array);
	}
	return rc;
}
ssize_t proc_flash_led_write (struct file *file, const char __user *buf, size_t nbytes, loff_t *ppos)
{
    char string[nbytes];
	int rc = 0;
    sscanf(buf, "%s", string);
    if (!strcmp((const char *)string, (const char *)"on"))
    {
    		CDBG("MSM_CAMERA_LED_LOW called\n");
		if (fctrl.func_tbl->flash_led_low)
			rc = fctrl.func_tbl->flash_led_low(&fctrl);
    }
    else if (!strcmp((const char *)string, (const char *)"off"))
    {
        		CDBG("MSM_CAMERA_LED_LOW called\n");
		if (fctrl.func_tbl->flash_led_low)
			rc = fctrl.func_tbl->flash_led_off(&fctrl);
    }
    return nbytes;
}
EXPORT_SYMBOL(proc_flash_led_write);
static int32_t msm_led_trigger_config(struct msm_led_flash_ctrl_t *fctrl,
	void *data)
{
	int rc = 0;
	struct msm_camera_led_cfg_t *cfg = (struct msm_camera_led_cfg_t *)data;
//	uint32_t i;
	CDBG("called led_state %d\n", cfg->cfgtype);

	if (!fctrl) {
		pr_err("failed\n");
		return -EINVAL;
	}
#ifdef CONFIG_HW_AIRBUSE

	switch (cfg->cfgtype) {
	case MSM_CAMERA_LED_INIT:
		CDBG("MSM_CAMERA_LED_INIT called\n");
		if (fctrl->func_tbl->flash_led_init)
			rc = fctrl->func_tbl->flash_led_init(fctrl);
		break;
	case MSM_CAMERA_LED_RELEASE:
		CDBG("MSM_CAMERA_LED_RELEASE called\n");
		if (fctrl->func_tbl->flash_led_release)
			rc = fctrl->func_tbl->
				flash_led_release(fctrl);
		break;
	case MSM_CAMERA_LED_OFF:
		CDBG("MSM_CAMERA_LED_OFF called\n");
		if (fctrl->func_tbl->flash_led_off)
			rc = fctrl->func_tbl->flash_led_off(fctrl);
		break;
	case MSM_CAMERA_LED_LOW:
		CDBG("MSM_CAMERA_LED_LOW called\n");
		if (fctrl->func_tbl->flash_led_low)
			rc = fctrl->func_tbl->flash_led_low(fctrl);
		break;
	case MSM_CAMERA_LED_HIGH:
		CDBG("MSM_CAMERA_LED_HIGH called\n");
		if (fctrl->func_tbl->flash_led_high)
			rc = fctrl->func_tbl->flash_led_high(fctrl);
		break;
	default:
		rc = -EFAULT;
		break;
	}
#else
	switch (cfg->cfgtype) {
	case MSM_CAMERA_LED_OFF:
		for (i = 0; i < fctrl->num_sources; i++)
			if (fctrl->flash_trigger[i])
				led_trigger_event(fctrl->flash_trigger[i], 0);
		if (fctrl->torch_trigger)
			led_trigger_event(fctrl->torch_trigger, 0);
		break;

	case MSM_CAMERA_LED_LOW:
		if (fctrl->torch_trigger)
			led_trigger_event(fctrl->torch_trigger,
				fctrl->torch_op_current);
		break;

	case MSM_CAMERA_LED_HIGH:
		if (fctrl->torch_trigger)
			led_trigger_event(fctrl->torch_trigger, 0);
		for (i = 0; i < fctrl->num_sources; i++)
			if (fctrl->flash_trigger[i])
				led_trigger_event(fctrl->flash_trigger[i],
					fctrl->flash_op_current[i]);
		break;

	case MSM_CAMERA_LED_INIT:
	case MSM_CAMERA_LED_RELEASE:
		for (i = 0; i < fctrl->num_sources; i++)
			if (fctrl->flash_trigger[i])
				led_trigger_event(fctrl->flash_trigger[i], 0);
		if (fctrl->torch_trigger)
			led_trigger_event(fctrl->torch_trigger, 0);
		break;

	default:
		rc = -EFAULT;
		break;
	}
#endif
	CDBG("flash_set_led_state: return %d\n", rc);
	return rc;
}
int msm_flash_led_init(struct msm_led_flash_ctrl_t *fctrl)
{
	int rc = 0;
	struct msm_camera_sensor_board_info *flashdata = NULL;
	CDBG("%s:%d called\n", __func__, __LINE__);
	flashdata = fctrl->flashdata;
	if (flashdata->gpio_conf->cam_gpiomux_conf_tbl != NULL) {
		pr_err("%s:%d mux install\n", __func__, __LINE__);
		msm_gpiomux_install(
			(struct msm_gpiomux_config *)
			flashdata->gpio_conf->cam_gpiomux_conf_tbl,
			flashdata->gpio_conf->cam_gpiomux_conf_tbl_size);
	}
	rc = msm_camera_request_gpio_table(
		flashdata->gpio_conf->cam_gpio_req_tbl,
		flashdata->gpio_conf->cam_gpio_req_tbl_size, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		return rc;
	}
	msleep(20);
	gpio_set_value_cansleep(
		flashdata->gpio_conf->gpio_num_info->gpio_num[0],
		GPIO_OUT_LOW);
	return rc;
}
int msm_flash_led_release(struct msm_led_flash_ctrl_t *fctrl)
{
	int rc = 0;
	struct msm_camera_sensor_board_info *flashdata = NULL;
	flashdata = fctrl->flashdata;
	CDBG("%s:%d called\n", __func__, __LINE__);
	if (!fctrl) {
		pr_err("%s:%d fctrl NULL\n", __func__, __LINE__);
		return -EINVAL;
	}
	gpio_set_value_cansleep(
		flashdata->gpio_conf->gpio_num_info->gpio_num[0],
		GPIO_OUT_LOW);
	rc = msm_camera_request_gpio_table(
		flashdata->gpio_conf->cam_gpio_req_tbl,
		flashdata->gpio_conf->cam_gpio_req_tbl_size, 0);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		return rc;
	}
	return 0;
}
int msm_flash_led_off(struct msm_led_flash_ctrl_t *fctrl)
{
	int rc = 0;
	struct msm_camera_sensor_board_info *flashdata = NULL;
	flashdata = fctrl->flashdata;
	CDBG("%s:%d calle GPIO=%d\n", __func__, __LINE__,flashdata->gpio_conf->gpio_num_info->gpio_num[0]);
	if (!fctrl) {
		pr_err("%s:%d fctrl NULL\n", __func__, __LINE__);
		return -EINVAL;
	}
	gpio_set_value_cansleep(
		flashdata->gpio_conf->gpio_num_info->gpio_num[0],
		GPIO_OUT_LOW);
	return rc;
}
int msm_flash_led_low(struct msm_led_flash_ctrl_t *fctrl)
{
	int rc = 0;
	struct msm_camera_sensor_board_info *flashdata = NULL;
	flashdata = fctrl->flashdata;
	CDBG("%s:%d calle GPIO=%d\n", __func__, __LINE__,flashdata->gpio_conf->gpio_num_info->gpio_num[0]);
	gpio_set_value_cansleep(
		flashdata->gpio_conf->gpio_num_info->gpio_num[0],
		GPIO_OUT_HIGH);
	return rc;
}
int msm_flash_led_high(struct msm_led_flash_ctrl_t *fctrl)
{
	int rc = 0;
	struct msm_camera_sensor_board_info *flashdata = NULL;
	flashdata = fctrl->flashdata;
	CDBG("%s:%d calle GPIO=%d\n", __func__, __LINE__,flashdata->gpio_conf->gpio_num_info->gpio_num[0]);
	gpio_set_value_cansleep(
		flashdata->gpio_conf->gpio_num_info->gpio_num[0],
		GPIO_OUT_HIGH);
	return rc;
}

static const struct of_device_id msm_led_trigger_dt_match[] = {
	{.compatible = "qcom,camera-led-flash"},
	{}
};

MODULE_DEVICE_TABLE(of, msm_led_trigger_dt_match);

static struct platform_driver msm_led_trigger_driver = {
	.driver = {
		.name = FLASH_NAME,
		.owner = THIS_MODULE,
		.of_match_table = msm_led_trigger_dt_match,
	},
};

const struct file_operations proc_flash_led_operations = {
	.owner	= THIS_MODULE,
	.write	= proc_flash_led_write,
};
static int32_t msm_led_trigger_probe(struct platform_device *pdev)
{
	int32_t rc = 0, rc_1 = 0, i = 0;
	struct device_node *of_node = pdev->dev.of_node;
	struct device_node *flash_src_node = NULL;
	uint32_t count = 0;
	struct led_trigger *temp = NULL;
  struct proc_dir_entry * rcdir;
	CDBG("called\n");
	rcdir = proc_create_data("CTP_FLASH_CTRL", S_IFREG | S_IWUGO | S_IWUSR, NULL, &proc_flash_led_operations, NULL);
    if(rcdir==NULL)
    {
    	CDBG("proc_create_data fail\n");

    }
	if (!of_node) {
		pr_err("of_node NULL\n");
		return -EINVAL;
	}

	fctrl.pdev = pdev;
	fctrl.num_sources = 0;

	rc = of_property_read_u32(of_node, "cell-index", &pdev->id);
	if (rc < 0) {
		pr_err("failed\n");
		return -EINVAL;
	}
	CDBG("pdev id %d\n", pdev->id);

	rc = of_property_read_u32(of_node,
			"qcom,flash-type", &flashtype);
	if (rc < 0) {
		pr_err("flash-type: read failed\n");
		return -EINVAL;
	}

	if (of_get_property(of_node, "qcom,flash-source", &count)) {
		count /= sizeof(uint32_t);
		CDBG("count %d\n", count);
		if (count > MAX_LED_TRIGGERS) {
			pr_err("invalid count\n");
			return -EINVAL;
		}
		fctrl.num_sources = count;
		for (i = 0; i < count; i++) {
			flash_src_node = of_parse_phandle(of_node,
				"qcom,flash-source", i);
			if (!flash_src_node) {
				pr_err("flash_src_node NULL\n");
				continue;
			}

			rc = of_property_read_string(flash_src_node,
				"linux,default-trigger",
				&fctrl.flash_trigger_name[i]);
			if (rc < 0) {
				pr_err("default-trigger: read failed\n");
				of_node_put(flash_src_node);
				continue;
			}

			CDBG("default trigger %s\n",
				fctrl.flash_trigger_name[i]);

			if (flashtype == GPIO_FLASH) {
				/* use fake current */
				fctrl.flash_op_current[i] = LED_FULL;
			} else {
				rc = of_property_read_u32(flash_src_node,
					"qcom,current",
					&fctrl.flash_op_current[i]);
				rc_1 = of_property_read_u32(flash_src_node,
					"qcom,max-current",
					&fctrl.flash_max_current[i]);
				if ((rc < 0) || (rc_1 < 0)) {
					pr_err("current: read failed\n");
					of_node_put(flash_src_node);
					continue;
				}
			}

			of_node_put(flash_src_node);

			CDBG("max_current[%d] %d\n",
				i, fctrl.flash_op_current[i]);

			led_trigger_register_simple(fctrl.flash_trigger_name[i],
				&fctrl.flash_trigger[i]);

			if (flashtype == GPIO_FLASH)
				if (fctrl.flash_trigger[i])
					temp = fctrl.flash_trigger[i];
		}

		/* Torch source */
		flash_src_node = of_parse_phandle(of_node, "qcom,torch-source",
			0);
		if (flash_src_node) {
			rc = of_property_read_string(flash_src_node,
				"linux,default-trigger",
				&fctrl.torch_trigger_name);
			if (rc < 0) {
				pr_err("default-trigger: read failed\n");
				goto torch_failed;
			}

			CDBG("default trigger %s\n",
				fctrl.torch_trigger_name);

			if (flashtype == GPIO_FLASH) {
				/* use fake current */
				fctrl.torch_op_current = LED_FULL;
				if (temp)
					fctrl.torch_trigger = temp;
				else
					led_trigger_register_simple(
						fctrl.torch_trigger_name,
						&fctrl.torch_trigger);
			} else {
				rc = of_property_read_u32(flash_src_node,
					"qcom,current",
					&fctrl.torch_op_current);
				rc_1 = of_property_read_u32(flash_src_node,
					"qcom,max-current",
					&fctrl.torch_max_current);

				if ((rc < 0) || (rc_1 < 0)) {
					pr_err("current: read failed\n");
					goto torch_failed;
				}

				CDBG("torch max_current %d\n",
					fctrl.torch_op_current);

				led_trigger_register_simple(
					fctrl.torch_trigger_name,
					&fctrl.torch_trigger);
			}
torch_failed:
			of_node_put(flash_src_node);
		}
	}
      else{
       rc = msm_led_get_dt_data(of_node, &fctrl);
       if (rc < 0) {
               pr_err("%s failed line %d\n", __func__, __LINE__);
               return rc;
       }
       }

	rc = msm_led_flash_create_v4lsubdev(pdev, &fctrl);
//	if (!rc)
//		msm_led_torch_create_classdev(pdev, &fctrl);

	return rc;
}

static int __init msm_led_trigger_add_driver(void)
{
	CDBG("called\n");
	return platform_driver_probe(&msm_led_trigger_driver,
		msm_led_trigger_probe);
}

static struct msm_flash_fn_t msm_led_trigger_func_tbl = {
	.flash_get_subdev_id = msm_led_trigger_get_subdev_id,
	.flash_led_config = msm_led_trigger_config,
	.flash_led_init = msm_flash_led_init,
	.flash_led_release = msm_flash_led_release,
	.flash_led_off = msm_flash_led_off,
	.flash_led_low = msm_flash_led_low,
	.flash_led_high = msm_flash_led_high,
};

static struct msm_led_flash_ctrl_t fctrl = {
	.func_tbl = &msm_led_trigger_func_tbl,
};

module_init(msm_led_trigger_add_driver);
MODULE_DESCRIPTION("LED TRIGGER FLASH");
MODULE_LICENSE("GPL v2");
