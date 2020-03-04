#ifndef DRIVERAPI_H
#define DRIVERAPI_H

enum driver_mod {
	DRIVER_INPUT = 0,
	DRIVER_OUTPUT,

	DRIVER_UNDEFINE = -1,
};

enum driver_level {
	DRIVER_LOW = 0,
	DRIVER_HIGH,

	DRIVER_UNDEFINED = -1,
};

int init_HW();
int close_HW();

void driver_delay(int ms);

void set_pin_mode(int pin, enum  driver_mod mode);

void write_pin_level(int pin, int level);
enum driver_level read_pin_level(int pin);

#endif /* DRIVERAPI_H */
