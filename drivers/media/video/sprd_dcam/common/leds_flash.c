
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
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <media/t2m_leds_flash.h>

#define LED_TRIGGER_DEFAULT		"none"

static struct led_gpio_flash_data {
	flash_status f_status;
	int flash_gpio;
	flash_type leds_type;
	struct led_classdev cdev;
};
static void led_gpio_brightness_set(struct led_classdev *led_cdev,
				    enum led_brightness value)
{
	int rc = 0;
	struct led_gpio_flash_data *flash_led =
	    container_of(led_cdev, struct led_gpio_flash_data, cdev);

        switch (value) {
        case FLASH_STATUS_ON:         /*flash on */
        case FLASH_STATUS_LOW:         /*for torch */
                	/*low light */
		if(FLASH_BRIGHTLESS_VARIABLE)
		{
			//need implement yet!
			//break;
		}
        case FLASH_STATUS_HIGH:
                /*high light */
                gpio_set_value(flash_led->flash_gpio, 1);
                break;
        case FLASH_STATUS_CLOSE:              /*close flash */
        case FLASH_STATUS_OFF:
                /*close the light */
                gpio_set_value(flash_led->flash_gpio, 0);
                break;
        default:
                printk("Sensor_SetFlash unknow mode:flash_mode=%d .\n",
                       value);
                break;
        }

	flash_led->f_status = value;
	return;
}

static enum led_brightness led_gpio_brightness_get(struct led_classdev
						   *led_cdev)
{
	struct led_gpio_flash_data *flash_led =
	    container_of(led_cdev, struct led_gpio_flash_data, cdev);
	return flash_led->f_status;
}

int __devinit led_gpio_flash_probe(struct platform_device *pdev)
{
	int rc = 0;
	const char *temp_str;
	struct led_gpio_flash_data *flash_led = NULL;
	struct led_gpio_flash_platform_data *platform_data = pdev->dev.platform_data;

	flash_led = devm_kzalloc(&pdev->dev, sizeof(struct led_gpio_flash_data),
				 GFP_KERNEL);
	if (flash_led == NULL) {
		dev_err(&pdev->dev, "%s:%d Unable to allocate memory\n",
			__func__, __LINE__);
		return -ENOMEM;
	}

	flash_led->cdev.default_trigger = LED_TRIGGER_DEFAULT;
	flash_led->leds_type = platform_data->leds_type;
	flash_led->flash_gpio = platform_data->flash_gpio;

	rc = gpio_request(flash_led->flash_gpio, "FLASH_EN");
	if (rc) {
		dev_err(&pdev->dev,
			"%s: Failed to request gpio %d,rc = %d\n",
			__func__, flash_led->flash_gpio, rc);
			goto error;
		}

	gpio_direction_output(flash_led->flash_gpio, 1);
        gpio_set_value(flash_led->flash_gpio, 0);
	platform_set_drvdata(pdev, flash_led);
	flash_led->cdev.name = "flash-led";
	flash_led->cdev.max_brightness = LED_FULL;
	flash_led->cdev.brightness_set = led_gpio_brightness_set;
	flash_led->cdev.brightness_get = led_gpio_brightness_get;

	rc = led_classdev_register(&pdev->dev, &flash_led->cdev);
	if (rc) {
		dev_err(&pdev->dev, "%s: Failed to register led dev. rc = %d\n",
			__func__, rc);
		goto error;
	}
	return 0;

error:
	devm_kfree(&pdev->dev, flash_led);
	return rc;
}

int __devexit led_gpio_flash_remove(struct platform_device *pdev)
{
	struct led_gpio_flash_data *flash_led =
	    (struct led_gpio_flash_data *)platform_get_drvdata(pdev);

	led_classdev_unregister(&flash_led->cdev);
	devm_kfree(&pdev->dev, flash_led);
	return 0;
}

static struct platform_driver led_gpio_flash_driver = {
	.probe = led_gpio_flash_probe,
	.remove = __devexit_p(led_gpio_flash_remove),
	.driver = {
		   .name = LED_GPIO_FLASH_DRIVER_NAME,
		   .owner = THIS_MODULE,
		   }
};

static int __init led_gpio_flash_init(void)
{
	return platform_driver_register(&led_gpio_flash_driver);
}

static void __exit led_gpio_flash_exit(void)
{
	return platform_driver_unregister(&led_gpio_flash_driver);
}

late_initcall(led_gpio_flash_init);
module_exit(led_gpio_flash_exit);

MODULE_DESCRIPTION("T2M GPIO LEDs driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("leds:leds-flash");
