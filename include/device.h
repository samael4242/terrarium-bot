#ifndef DEVICE_H
#define DEVICE_H

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

	void *priv_data;
};


int device_init(struct instance *inst);
int device_close(struct instance *inst);
void device_get_temperature(struct instance *inst);
void device_get_humidity(struct instance *inst);

#endif /* DEVICE_H */
