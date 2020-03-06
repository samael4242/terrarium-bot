#include <wiringPi.h>
#include <log.h>
#include "driverAPI.h"

int init_HW()
{
	wiringPiSetup();
	wiringPiSetupGpio();
	return 0;
}

int close_HW()
{
	return 0;
}

static int drv2wpi_mode(enum driver_mod mode)
{
	switch (mode) {
		case DRIVER_INPUT:
			return INPUT;
		case DRIVER_OUTPUT:
			return OUTPUT;
		default:
			return DRIVER_UNDEFINED;
	};
}

static int drv2wpi_level(enum driver_level level)
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

static enum driver_level wpi2drv_level(int level)
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

void driver_delay(int ms)
{
	delay(ms);
}

void set_pin_mode(int pin, enum  driver_mod mode)
{
	int __mode = drv2wpi_mode(mode);
	if (__mode == DRIVER_UNDEFINE)
		return;

	pinMode(pin, __mode);
}

 void write_pin_level(int pin, int level)
{
	int __level = drv2wpi_level(level);
	if (__level == DRIVER_UNDEFINE)
		return;

	digitalWrite(pin, __level);
}

enum driver_level read_pin_level(int pin)
{
	pr_debug("%s\n", __func__);
	return wpi2drv_level(digitalRead(pin));
}
