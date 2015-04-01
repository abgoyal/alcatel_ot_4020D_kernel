/*
 * LED driver for Sprd charge led driven LEDS.
 *
 * Copyright (C) 2010 Spreadtrum 
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * 
 *  usage :
	  echo 100 > /sys/class/leds/charge-led/brightness
	  cat /sys/class/leds/charge-led/brightness
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/delay.h>

//#include <mach/adi_hal_internal.h>
//#include <mach/regs_ana.h>
#include <asm/io.h>
#include <mach/board.h>
#include <mach/gpio.h>

#include <linux/slab.h>

/* sprd keypad backlight */
struct sprd_charge_led {
	struct platform_device *pdev;
	struct mutex mutex;
	struct work_struct work;
	spinlock_t value_lock;
	enum led_brightness value;
	struct led_classdev cdev;
	int enabled;
};

#define to_sprd_led(led_cdev) \
	container_of(led_cdev, struct sprd_charge_led, cdev)

#define LED_GREEN 149

static void Flash_SetBrightness( unsigned long  value)
{
 	if (value == 0 || value == LED_GREEN)
	{
		gpio_direction_output(GPIO_CHARGE_LED,1);
		gpio_set_value(GPIO_CHARGE_LED,0);
		//__gpio_set_value(138,0);
	}
	else
	{
		gpio_direction_output(GPIO_CHARGE_LED,1);
		gpio_set_value(GPIO_CHARGE_LED,1);
		//__gpio_set_value(138,1);
	}
	
}

static void sprd_led_enable(struct sprd_charge_led *led)
{
	Flash_SetBrightness(led->value);
	led->enabled = 1;
}

static void sprd_led_disable(struct sprd_charge_led *led)
{
	Flash_SetBrightness(led->value);
	led->enabled = 0;
}

static void led_work(struct work_struct *work)
{
	struct sprd_charge_led *led = container_of(work, struct sprd_charge_led, work);
	unsigned long flags;
	mutex_lock(&led->mutex);
	spin_lock_irqsave(&led->value_lock, flags);
	if (led->value == 0) {
		spin_unlock_irqrestore(&led->value_lock, flags);
		sprd_led_disable(led);
		goto out;
	}
	spin_unlock_irqrestore(&led->value_lock, flags);
	sprd_led_enable(led);
out:
	mutex_unlock(&led->mutex);
}


static void sprd_led_set(struct led_classdev *led_cdev,
			   enum led_brightness value)
{
	struct sprd_charge_led *led = to_sprd_led(led_cdev);
	unsigned long flags;
	spin_lock_irqsave(&led->value_lock, flags);
	led->value = value;
	spin_unlock_irqrestore(&led->value_lock, flags);
	schedule_work(&led->work);	
}

static void sprd_charge_led_shutdown(struct platform_device *pdev)
{
	struct sprd_charge_led *led = platform_get_drvdata(pdev);
	mutex_lock(&led->mutex);
	led->value =0;
	led->enabled = 1;
	sprd_led_disable(led);
	mutex_unlock(&led->mutex);
}

static int sprd_charge_led_probe(struct platform_device *pdev)
{
	struct sprd_charge_led *led;
	int ret;
	
	led = kzalloc(sizeof(*led), GFP_KERNEL);
	if (led == NULL) {
		ret = -ENOMEM;
		goto err_led;
	}

	led->cdev.brightness_set = sprd_led_set;
	led->cdev.default_trigger = "heartbeat";
	led->cdev.name = "charge-led";
	led->cdev.brightness_get = NULL;
//	led->cdev.flags |= LED_CORE_SUSPENDRESUME;
	led->enabled = 0;
	spin_lock_init(&led->value_lock);
	mutex_init(&led->mutex);
	INIT_WORK(&led->work, led_work);
	led->value = 0;
	platform_set_drvdata(pdev, led);
	ret = led_classdev_register(&pdev->dev, &led->cdev);
	
	if (ret < 0)
		goto err_led;
		
	
	led->enabled = 0;
	schedule_work(&led->work);
	return 0;

 err_led:
	kfree(led);
	return ret;

}

static int sprd_charge_led_remove(struct platform_device *pdev)
{
	struct sprd_charge_led *led = platform_get_drvdata(pdev);
	led_classdev_unregister(&led->cdev);
	flush_scheduled_work();
	led->value = 0;
	led->enabled = 1;
	sprd_led_disable(led);
	kfree(led);

	return 0;
}

static struct platform_driver sprd_charge_led_driver = {
	.driver = {
			.name = "charge-led",
			.owner = THIS_MODULE,
		   },
	.probe = sprd_charge_led_probe,
	.remove = sprd_charge_led_remove,
	.shutdown = sprd_charge_led_shutdown,
};

static int __devinit sprd_charge_led_init(void)
{
	return platform_driver_register(&sprd_charge_led_driver);
}

static void sprd_charge_led_exit(void)
{
	platform_driver_unregister(&sprd_charge_led_driver);
}
module_init(sprd_charge_led_init);
module_exit(sprd_charge_led_exit);

MODULE_DESCRIPTION("Sprd SC8800G charge led driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:chargeled");
