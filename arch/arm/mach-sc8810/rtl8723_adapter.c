#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <mach/regulator.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/types.h>

#ifdef CONFIG_WLAN_SDIO
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
extern void sdhci_bus_scan(void);
extern int sdhci_wifi_detect_isbusy(void);
extern int sdhci_device_attach(int on);
#endif
/*
 * Control BT/WIFI LDO always power on accoding to Realtek 8723as chip spec
 */

#ifdef CONFIG_WLAN_SDIO
static int check_probe_times = 0;
#define ADDR_MASK 0x10000
#define LOCAL_ADDR_MASK 0x00000
static u8 wifi_readb(struct sdio_func *func, u32 addr)
{
	int err;
	u8 ret = 0;

	ret = sdio_readb(func, ADDR_MASK | addr, &err);

	return ret;
}

static void wifi_writeb(struct sdio_func *func, u8 val, u32 addr)
{
	int err;

	sdio_writeb(func, val, ADDR_MASK | addr, &err);
}

static u8 wifi_readb_local(struct sdio_func *func, u32 addr)
{
	int err;
	u8 ret = 0;

	ret = sdio_readb(func, LOCAL_ADDR_MASK | addr, &err);

	return ret;
}

static void wifi_writeb_local(struct sdio_func *func, u8 val, u32 addr)
{
	int err;

	sdio_writeb(func, val, LOCAL_ADDR_MASK | addr, &err);
}

static int rtw_fake_driver_probe(
		struct sdio_func *func,
		const struct sdio_device_id *id)
{
	u8 value8 = 0;

	printk("RTL871X(adapter): %s enter\n", __func__);

	sdio_claim_host(func);

	/* unlock ISO/CLK/Power control register */
	//wifi_writeb(func, 0x00, 0x1C);

	/* we should leave suspend first, or all MAC io will fail */
	wifi_writeb_local(func, wifi_readb_local(func, 0x86) & (~(BIT(0))), 0x86);
	msleep(20);

	/* bt power leakage */
	wifi_writeb(func, 0, 0x93);

#ifdef CONFIG_RTL8723BS_MODULE
	//0x68[13]=1 set clk req high
	value8 = wifi_readb(func, 0x69);
	wifi_writeb(func, value8 | BIT(5), 0x69);

	//set clk 26M
	value8 = wifi_readb(func, 0x28);
	wifi_writeb(func, value8 | BIT(2), 0x28);

	//0x66[7]=1 set clk ext enable
	value8 = wifi_readb(func, 0x66);
	wifi_writeb(func, value8 | BIT(7), 0x66);

	value8 = wifi_readb(func, 0x78);
	wifi_writeb(func, (value8 & 0x7F), 0x78);

	value8 = wifi_readb(func, 0x79);
	wifi_writeb(func, (value8 & 0xFC) | BIT(1), 0x79);

	//0x64[12]=1 host wake pad pull enable (power leakage)
	value8 = wifi_readb(func, 0x65);
	wifi_writeb(func, value8 | BIT(4), 0x65);

	//0x04[7]=1 chip power down en  (power leakage)
	value8 = wifi_readb(func, 0x04);
	wifi_writeb(func, value8 | BIT(7), 0x04);

	//0x66[2:0]=0x07 PCM interface en (for SCO)
	value8 = wifi_readb(func, 0x66);
	wifi_writeb(func, value8 | BIT(0) | BIT(1) | BIT(2), 0x66);

	//Ant select BT_RFE_CTRL control enable
	//0x38[11]=1 0x4c[23-24]=0
	value8 = wifi_readb(func, 0x39);
	wifi_writeb(func, value8 | BIT(3), 0x39);
	value8 = wifi_readb(func, 0x4E);//0x4c bit23
	wifi_writeb(func, value8 & ~BIT(7), 0x4E);
	value8 = wifi_readb(func, 0x4F);//0x4c bit24
	wifi_writeb(func, value8 & ~BIT(0), 0x4F);
#endif

	//cardemue to suspend
	wifi_writeb(func, wifi_readb(func, 0x05) | (BIT(3)), 0x05);
	wifi_writeb(func, wifi_readb(func, 0x23) | (BIT(4)), 0x23);
	wifi_writeb(func, 0x20, 0x07);
	wifi_writeb_local(func, wifi_readb_local(func, 0x86) | (BIT(0)), 0x86);
	msleep(20);

	printk("RTL871X(adapter): write register ok:"
		"0x93 is 0x%x, 0x23 is 0x%x, 0x07 is 0x%x \n",
	 	wifi_readb(func, 0x93), wifi_readb(func, 0x23),
	 	wifi_readb(func, 0x07));

	sdio_release_host(func);

	check_probe_times = 1;
	return 0;
}

static void rtw_fake_driver_remove(struct sdio_func *func)
{
	check_probe_times = 0;
	printk("RTL871X(adapter): %s \n", __func__);
}

static const struct sdio_device_id sdio_ids_fake_driver[] = {
		{ SDIO_DEVICE(0x024c, 0x8723) },
		{ SDIO_DEVICE(0x024c, 0xb723) },
		{ SDIO_DEVICE(0x024c, 0x8703) },
};

static struct sdio_driver r871xs_fake_driver = {
	.probe = rtw_fake_driver_probe,
	.remove = rtw_fake_driver_remove,
	.name = "rtw_fake_driver",
	.id_table = sdio_ids_fake_driver,
};
#endif

static int wifi_bt_ldo_enable(void)
{
	int err;
	struct regulator *regulator_wifi_bt = NULL;

	printk("RTL871X(adapter): %s enter\n", __func__);

	/* make sure bt reset pin is low before power */
	/* on or BT disable power will high*/
	if (GPIO_BT_RESET > 0) {
		gpio_request(GPIO_BT_RESET , "bt_reset");
		gpio_direction_output(GPIO_BT_RESET , 0);
	}

#ifdef CONFIG_RTL8723BS_MODULE
	gpio_request(GPIO_WIFIBT_CHIPEN, "wifibt_en");
	gpio_direction_output(GPIO_WIFIBT_CHIPEN, 1);

	/* move it to FM driver, or 8mA will be cost */
	if(GPIO_FM_EN > 0){
		//gpio_request(GPIO_FM_EN, "fm_en");
		//gpio_direction_output(GPIO_FM_EN, 1);
	}

#endif
	if (GPIO_WIFI_POWERON > 0) {
		gpio_request(GPIO_WIFI_POWERON, "wifi_pwron");
		gpio_direction_output(GPIO_WIFI_POWERON, 1);
	}

	regulator_wifi_bt = regulator_get(NULL, "VDDWIF1");
	if (IS_ERR(regulator_wifi_bt)){
		pr_err("Can't get regulator for VDDWIF1\n");
		return -1;
	}

#ifdef CONFIG_RTL8723BS_MODULE
	err = regulator_set_voltage(regulator_wifi_bt, 3300000, 3300000);
#else
	err = regulator_set_voltage(regulator_wifi_bt, 1800000, 1800000);
#endif
	if (err){
		pr_err("RTL871X(adapter): Can't set regulator to voltage 3.3V\n");
		return -1;
	}

	//RTK_DEBUG
	regulator_set_mode(regulator_wifi_bt, REGULATOR_MODE_NORMAL);//REGULATOR_MODE_STANDBY);
	regulator_enable(regulator_wifi_bt);

#ifdef CONFIG_WLAN_SDIO
	regulator_wifi_bt = regulator_get(NULL, "VDDSD1");
	if (IS_ERR(regulator_wifi_bt)){
		pr_err("RTL871X(adapter): Can't get regulator for VDD_SDIO\n");
		return -1;
	}

	err = regulator_set_voltage(regulator_wifi_bt, 1800000, 1800000);
	if (err){
		pr_err("RTL871X(adapter): Can't set regulator to valtage 1.8V\n");
		return -1;
	}

	//RTK_DEBUG
	regulator_set_mode(regulator_wifi_bt, REGULATOR_MODE_NORMAL);//REGULATOR_MODE_STANDBY);
	regulator_enable(regulator_wifi_bt);

	if (GPIO_WIFI_RESET > 0) {
		gpio_request(GPIO_WIFI_RESET , "wifi_rst");
		gpio_direction_output(GPIO_WIFI_RESET , 1);
		msleep(5);
		sdhci_bus_scan();
		msleep(5);
		printk("RTL871X(adapter): %s after bus scan\n", __func__);
	}
#endif
	printk("RTL871X(adapter): %s exit\n", __func__);
	return 0;
}

static void wlan_bt_clk_init(void)
{
	struct clk *wlan_clk;
	struct clk *clk_parent;
	wlan_clk = clk_get(NULL, "clk_aux0");
	if (IS_ERR(wlan_clk)) {
		printk("clock: failed to get clk_aux0\n");
	}

	clk_parent = clk_get(NULL, "ext_32k");
	if (IS_ERR(clk_parent)) {
		printk("failed to get parent ext_32k\n");
	}

	clk_set_parent(wlan_clk, clk_parent);
	clk_set_rate(wlan_clk, 32000);
	clk_enable(wlan_clk);
}

static int __init wlan_bt_device_init(void)
{
	wlan_bt_clk_init();
	return wifi_bt_ldo_enable();
}

#ifdef CONFIG_WLAN_SDIO
static int __init wlan_bt_late_init(void)
{

	int i = 0;
	int err;

	printk("RTL871X(adapter): %s enter\n", __func__);

	/* sdhci_device_attach in wifi driver will fail
	 * because mmc->card is NULL in first wifi on
	 * after reboot, this will cause suspend/resume
	 * fail, so we add bus_scan here before first
	 * wifi on, but we should push down rst in late_init
	 * because bus_scan is in queue work */
	if (GPIO_WIFI_RESET > 0) {
		/* after sdhci_device_attached, there are some cmd for SDIO */
		/* sleep will make these cmd success and make sure new (high */
		/* speed SDIO card at address 0001) after sleep suspend/resume */
		/* will all ok after that, or suspend can't be call */
		for (i = 0; i <= 200; i++) {
			if(!sdhci_wifi_detect_isbusy())
				break;
			printk("RTL871X(adapter): %s sdhci_wifi_detect_isbusy %d times\n", __func__, i);
			msleep(100);
		}
		printk("RTL871X(adapter): %s after delay %d times (100ms)\n", __func__, i);
		msleep(20);
	}

	printk("RTL871X(adapter): %s sdio_register_driver for r871xs_fake_driver\n", __func__);
	err = sdio_register_driver(&r871xs_fake_driver);
	if (err){
		pr_err("RTL871X(adapter): register r871xs_fake_driver failed\n");
		return -1;
	}

	for (i = 0; i <= 20; i++) {
		msleep(10);
		if (check_probe_times)
			break;
	}

	printk("RTL871X(adapter): %s sdio_unregister_driver for r871xs_fake_driver %d\n", __func__, i);
	sdio_unregister_driver(&r871xs_fake_driver);
	printk("RTL871X(adapter): %s sdio_unregister_driver ok\n", __func__);

	/* dont set pwdn mode, because we bus scan once now */
	//if (GPIO_WIFI_RESET > 0)
	//	gpio_direction_output(GPIO_WIFI_RESET , 0);

	return 0;
}

late_initcall(wlan_bt_late_init);

#endif

device_initcall(wlan_bt_device_init);

MODULE_DESCRIPTION("Realtek wlan/bt init driver");
MODULE_LICENSE("GPL");

