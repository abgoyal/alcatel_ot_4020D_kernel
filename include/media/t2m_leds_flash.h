#ifndef __T2M_LEDS_FLASH__
#define __T2M_LEDS_FLASH__

#define LED_GPIO_FLASH_DRIVER_NAME      "leds-gpio-flash"
#define LED_GPIO_FLASH_DEVICE_NAME      "leds-gpio-flash"

typedef enum {
        FLASH_NONE,
        FLASH_BRIGHTLESS_FIXED,
        FLASH_BRIGHTLESS_VARIABLE,
        FLASH_MAX,
}flash_type;

typedef enum {
        FLASH_STATUS_OFF = 0,
        FLASH_STATUS_ON  = 1,
        FLASH_STATUS_LOW = 2,
        FLASH_STATUS_CLOSE = 0X10,
        FLASH_STATUS_HIGH = 0x11,
        FLASH_STATUS_MAX,
}flash_status;

struct led_gpio_flash_platform_data{
	flash_type leds_type;
	int flash_gpio;
};

#endif
