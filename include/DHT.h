#ifndef DHT_H
#define DHT_H

struct DHTdata {
	double temerature;
	double humidity;
	char val[5];
};

int getDHTdata(int pin, struct DHTdata *ddata);

#endif /* DHGT_H */
