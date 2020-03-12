#ifndef DHT_H
#define DHT_H

struct DHTdata {
	double temerature;
	double humidity;
	char val[5];
};

int driver_init();
int driver_close();

int get_DHT_data(int pin, struct DHTdata *ddata);

int relay_on(int pin);
int relay_off(int pin);

#endif /* DHGT_H */
