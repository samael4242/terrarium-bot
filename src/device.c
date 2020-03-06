#include <device.h>
#include <driver.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#define DEBUG
#include <log.h>

#define DHT_CH1 4
#define DHT_CH2 16

#define SIZE_OF_POOL 5

#define MAX_RETRY_NUM 100

struct dev_handler {
	bool terminate;

	pthread_mutex_t lock;
	pthread_t mitering_thread;

	struct dev_temp temp_pool[SIZE_OF_POOL];
	struct dev_hum hum_pool[SIZE_OF_POOL];

	double temp_ch1;
	double temp_ch2;

	double hum_ch1;
	double hum_ch2;
};

static void get_mitering(struct dev_temp *temp, struct dev_hum *hum)
{
	struct DHTdata ddata;
	int ret, counter = 0;

	do {
		ret = get_DHT_data(DHT_CH1, &ddata);

		if (ddata.temerature >= 80.1 ||
		    ddata.temerature <= -40.1)
			ret = 1;
		if (ddata.humidity >= 100. ||
		    ddata.humidity <= 0.)
			ret = 1;
		if (ddata.temerature == 0. &&
		    ddata.humidity == 0.)
			ret = 1;
		if (counter > MAX_RETRY_NUM) {
			pr_warn("Could not get sensor data\n");
			break;
		}

		counter++;
	} while (ret);

	temp->ch1 = ddata.temerature;
	hum->ch1 = ddata.humidity;
	counter = 0;

	do {
		ret = get_DHT_data(DHT_CH2, &ddata);

		if (ddata.temerature > 80.0 ||
		    ddata.temerature < -40.0)
			ret = 1;
		if (ddata.humidity > 99.9 ||
		    ddata.humidity < 0.)
			ret = 1;
		if (ddata.temerature == 0. &&
		    ddata.humidity == 0.)
			ret = 1;
		if (counter > MAX_RETRY_NUM) {
			pr_warn("Could not get sensor data\n");
			break;
		}

		counter++;
	} while (ret);

	temp->ch2 = ddata.temerature;
	hum->ch2 = ddata.humidity;
}

static int init_mitering(struct dev_handler *handle)
{
	int i;

	for (i = 0; i < SIZE_OF_POOL; i++) {
		get_mitering(&handle->temp_pool[i],
				&handle->hum_pool[i]);
	}

	return 0;
}

static void *mitering_thread(void *args)
{
	struct dev_handler *handle = args;
	bool terminate = false;
	int counter = 0;
	int i;
	double temp_ch1 = 0, temp_ch2 = 0;
	double hum_ch1 = 0, hum_ch2 = 0;

	pr_debug("mitering_thread init\n");

	do {
		get_mitering(&handle->temp_pool[counter],
				&handle->hum_pool[counter]);

		counter++;
		counter %= SIZE_OF_POOL;

		for (i = 0; i < SIZE_OF_POOL; i++) {
			temp_ch1 += handle->temp_pool[i].ch1;
			temp_ch2 += handle->temp_pool[i].ch2;
			hum_ch1 += handle->hum_pool[i].ch1;
			hum_ch2 += handle->hum_pool[i].ch2;
		}

		temp_ch1 /= SIZE_OF_POOL;
		temp_ch2 /= SIZE_OF_POOL;
		hum_ch1 /= SIZE_OF_POOL;
		hum_ch2 /= SIZE_OF_POOL;

		pthread_mutex_lock(&handle->lock);
		handle->temp_ch1 = temp_ch1;
		handle->temp_ch2 = temp_ch2;

		handle->hum_ch1 = hum_ch1;
		handle->hum_ch2 = hum_ch2;

		pr_debug("T: %.1lf *C, T: %.1lf *C\n",
				temp_ch1,
				temp_ch2);
		pr_debug("H: %.1lf %%, H: %.1lf %%\n",
				hum_ch1,
				hum_ch2);
		pr_debug("=====\n");
		pr_debug("T: %.1lf *C, T: %.1lf *C\n",
				handle->temp_pool[counter].ch1,
				handle->temp_pool[counter].ch2);
		pr_debug("H: %.1lf %%, H: %.1lf %%\n",
				handle->hum_pool[counter].ch1,
				handle->hum_pool[counter].ch2);
		pr_debug("=====================================\n");

		terminate = handle->terminate;
		pthread_mutex_unlock(&handle->lock);

	sleep(10);

		temp_ch1 = temp_ch2 = 0;
		hum_ch1 = hum_ch2 = 0;
	} while (!terminate);

	return NULL;
}

int device_init(struct instance *inst)
{
	struct dev_handler *handle = NULL;

	handle = malloc(sizeof(*handle));
	if (handle == NULL)
		return -1;

	pthread_mutex_init(&handle->lock, 0);

	driver_init();

	handle->terminate = false;
	init_mitering(handle);
	pthread_create(&handle->mitering_thread, NULL,
			mitering_thread, handle);

	inst->priv_data = handle;

	return 0;
}

int device_close(struct instance *inst)
{
	struct dev_handler *handle = inst->priv_data;

	pthread_mutex_lock(&handle->lock);
	handle->terminate = true;
	pthread_mutex_unlock(&handle->lock);

	pthread_join(handle->mitering_thread, 0);

	pthread_mutex_destroy(&handle->lock);
	free(handle);

	driver_close();

	return 0;
}

void device_get_temperature(struct instance *inst)
{
	struct dev_handler *handle = inst->priv_data;

	pthread_mutex_lock(&handle->lock);
	inst->temperature.ch1 = handle->temp_ch1;
	inst->temperature.ch2 = handle->temp_ch2;
	pthread_mutex_unlock(&handle->lock);

	pr_debug("T: %.1lf *C, T: %.1lf *C\n",
			inst->temperature.ch1,
			inst->temperature.ch2);
	pr_debug("H: %.1lf %%, H: %.1lf %%\n",
			inst->humidity.ch1,
			inst->humidity.ch2);
}

void device_get_humidity(struct instance *inst)
{
	struct dev_handler *handle = inst->priv_data;

	pthread_mutex_lock(&handle->lock);
	inst->humidity.ch1 = handle->hum_ch1;
	inst->humidity.ch2 = handle->hum_ch2;
	pthread_mutex_unlock(&handle->lock);
}
