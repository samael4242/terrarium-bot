#include <bcm2835.h>
#include "driverAPI.h"
#include <log.h>

int init_HW()
{
	pr_debug("%s\n", __func__);
#ifdef BCM_DEBUG
	bcm2835_set_debug(1);
#endif
	return bcm2835_init();
	return 0;
}

int close_HW()
{
	return bcm2835_close();
}

static int drv2bcm_mode(enum driver_mod mode)
{
	switch (mode) {
		case DRIVER_INPUT:
			return BCM2835_GPIO_FSEL_INPT;
		case DRIVER_OUTPUT:
			return BCM2835_GPIO_FSEL_OUTP;
		default:
			return DRIVER_UNDEFINED;
	};
}

static int drv2bcm_level(enum driver_level level)
{
	switch (level) {
		case DRIVER_HIGH:
			return HIGH;
		case DRIVER_LOW:
			return LOW;
		default:
			return DRIVER_UNDEFINED;
	};
}

static enum driver_level bcm2drv_level(int level)
{
	switch (level) {
		case HIGH:
			return DRIVER_HIGH;
		case LOW:
			return DRIVER_LOW;
		default:
			return DRIVER_UNDEFINED;
	};
}

void set_pin_mode(int pin, enum  driver_mod mode)
{
	pr_debug("%s\n", __func__);
	mode = drv2bcm_mode(mode);
	if (mode == DRIVER_UNDEFINE)
		return;

	bcm2835_gpio_fsel(pin, mode);
}

void write_pin_level(int pin, int level)
{
	pr_debug("%s\n", __func__);
	level = drv2bcm_level(level);
	if (level == DRIVER_UNDEFINE)
		return;

	bcm2835_gpio_write(pin, level);
}

enum driver_level read_pin_level(int pin)
{
	pr_debug("%s\n", __func__);
	return bcm2drv_level(bcm2835_gpio_lev(pin));
}

void driver_delay(int ms)
{
	pr_debug("%s\n", __func__);
	delay(ms);
}
