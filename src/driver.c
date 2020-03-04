/* Implemet according to AM2302 datasheet.
 * See https://cdn-shop.adafruit.com/datasheets/Digital+humidity+and+temperature+sensor+AM2302.pdf
 * for more ditails
 * */

#include <driver.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <log.h>
#include <common.h>

#include "driverAPI.h"

#define DILAY_FOR_ONE 70
#define MAX_BIT_COUNTER 40 /* 16 bit RH data + 16 bit T data + 8 bit check sum */
#define MAX_TIKS_COUNT 1000 /* Timeout */
#define RSP_SIGNAL_COUNT 3
#define SIGN_BIT (1 << 7)
#define MAX_CHAR (1 << 8)
#define DEVIDER 10.

int driver_init()
{
	return init_HW();
}

int driver_close()
{
	return close_HW();
}

static void set_DHT_metering(struct DHTdata *ddata)
{
	double humidity = 0;
	double temperature = 0;
	char *data = ddata->val;

	humidity = data[0];
	humidity *= MAX_CHAR;
	humidity += data[1];
	/* get integer and fractional part */
	humidity /= DEVIDER;

	temperature = data[2] & ~SIGN_BIT;
	temperature *= MAX_CHAR;
	temperature += data[3];
	/* get integer and fractional part */
	temperature /= DEVIDER;

	if (data[2] & SIGN_BIT)
		temperature *= -1;

	ddata->temerature = temperature;
	ddata->humidity = humidity;
}

static unsigned long long current_timestamp() {
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return 1000000 * tv.tv_sec + tv.tv_usec;
}

int get_DHT_data(int pin, struct DHTdata *ddata)
{
	int tiks = 0;
	int cur_pin_level = DRIVER_HIGH;
	int bit_counter = 0;
	int i = 0;
	char checksum = 0;
	char *data = ddata->val;
	unsigned long long timestamp_start = 0;
	unsigned long long timestamp_end = 0;
	unsigned long long diff = 0;

	memset(ddata->val, 0, sizeof(ddata->val));

	set_pin_mode(pin, DRIVER_OUTPUT);

	write_pin_level(pin, DRIVER_HIGH);
	driver_delay(10);
	write_pin_level(pin, DRIVER_LOW);
	driver_delay(1);

	set_pin_mode(pin, DRIVER_INPUT);

	tiks = 0;

	/* Skip sensor`s response signal */
	for (i = 0; i < RSP_SIGNAL_COUNT; i++) {
		while (read_pin_level(pin) == cur_pin_level) {
			usleep(1);
			tiks++;

			if (tiks > MAX_TIKS_COUNT) {
				pr_debug("Could not setup sensor\n");
				return -1;
			}
		}

		cur_pin_level = read_pin_level(pin);
	}

	/*Data to read:
	 *
	 *+--------------------+-------------------+-------------+
	 *|   16bit RH data    |   16 bits T data  |  check sum  |
	 *+--------------------+-------------------+-------------+*/

	/* MAX_BIT_COUNTER * 2 = bit signal + separator */
	for (i = 0; i < MAX_BIT_COUNTER *2; i++) {
		tiks = 0;

		timestamp_start = current_timestamp();

		while (read_pin_level(pin) == cur_pin_level) {
			tiks++;

			/*  Data have been read */
			if (tiks == MAX_TIKS_COUNT)
				goto exit;
		};

		timestamp_end = current_timestamp();
		diff = timestamp_end - timestamp_start;

		/*  Skip bit separator */
		cur_pin_level = read_pin_level(pin);
		if (cur_pin_level == DRIVER_HIGH)
			continue;

		data[bit_counter >> 3] <<= 1;
		if (diff >= DILAY_FOR_ONE)
			data[bit_counter >> 3] |= 1;

		bit_counter++;
	}

exit:
#ifdef DEBUG
	for (int i = 0; i < SIZE_OF_ARRAY(ddata->val); i++) {
		pr_debug("data[%i]: %02X\n", i, data[i]);
	}
#endif

	checksum = data[0] + data[1] + data[2] + data[3];
	if (checksum != data[4] && bit_counter >= 39) {
		pr_debug("Checksum mismatch\n");
		return -1;
	}

	set_DHT_metering(ddata);

	return 0;
}

#ifdef TEST_DHT
int main()
{
	struct DHTdata metering;

	bcm2835_init();
	pr_info("GPIO4\n");
retry1:
	if (!get_DHT_data(4, &metering))
		pr_info("T: %.1lf *C, H: %.1lf\n", metering.temerature, metering.humidity);
	else
		goto retry1;
	pr_info("GPIO16\n");
retry2:
	if (!get_DHT_data(16, &metering))
		pr_info("T: %.1lf *C, H: %.1lf\n", metering.temerature, metering.humidity);
	else
		goto retry2;
	pr_info("=====================\n");

	return 0;
}
#endif
