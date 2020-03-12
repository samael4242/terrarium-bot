#ifndef DEVICE_H
#define DEVICE_H

enum relay_channel {
	RELAY_1 = 1,
	RELAY_2,
	RELAY_3,

	RELAY_INVALID = -1,
};

struct dev_temp {
	double ch1;
	double ch2;
};

struct dev_hum {
	double ch1;
	double ch2;
};

struct instance {
	struct dev_temp temperature;
	struct dev_hum humidity;

	enum relay_channel pin;

	void *priv_data;
};


int device_init(struct instance *inst);
int device_close(struct instance *inst);

void device_get_temperature(struct instance *inst);
void device_get_humidity(struct instance *inst);

int device_relay_off(struct instance *inst);
int device_relay_on(struct instance *inst);
#endif /* DEVICE_H */
