#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <telebot/telebot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <log.h>
#include <DHT.h>
#include <bcm2835.h>

#define SIZE_OF_ARRAY(array) (sizeof(array)/sizeof(array[0]))
#define MAX_STRING_SIZE 4096
#define TOKEN_PATH ".token"

#define GPROUP_CAHT_ID -1001316572508

int main(int argc, char *argv[])
{
	int index, count, offset = 1;
	char str[MAX_STRING_SIZE] = {0,};
	char token[MAX_STRING_SIZE] = {0,};
	FILE *fp = NULL;
	struct DHTdata data1;
	struct DHTdata data2;
	telebot_user_t *me;
	telebot_handler_t handle;
	telebot_error_e ret;
	telebot_message_t message;
	telebot_update_t *updates;

	pr_info("Welcome to Terrarium bot\n");

	fp = fopen(".token", "r");
	if (fp == NULL) {
		pr_err("Failed to open token file\n");
		goto err;
	}

	if (fscanf(fp, "%s", token) == 0) {
		pr_err("Reading token failed");
		goto close_fd;
	}

	pr_debug("Token: %s\n", token);

	fclose(fp);

	if (telebot_create(&handle, token) != TELEBOT_ERROR_NONE) {
		pr_err("Telebot create failed\n");
		goto err;
	}


	if (telebot_get_me(handle, &me) != TELEBOT_ERROR_NONE) {
		pr_err("Failed to get bot information\n");
		telebot_destroy(handle);
		goto err;
	}

	pr_debug("ID: %d\n", me->id);
	pr_debug("First Name: %s\n", me->first_name);
	pr_debug("User Name: %s\n", me->username);

	telebot_free_me(me);

	bcm2835_init();

	while (1) {
		/* busy wait */
		sleep(1);

		ret = telebot_get_updates(handle, offset, 20, 0, NULL, 0, &updates, &count);
		if (ret != TELEBOT_ERROR_NONE)
			continue;

		pr_info("Number of updates: %d\n", count);

		for (index = 0;index < count; index++) {
			message = updates[index].message;
			pr_info("=======================================================\n");
			pr_info("%s: %s \n", message.from->first_name, message.text);
			pr_info("=======================================================\n");\
			if (message.text) {
				if (strstr(message.text, "/start")) {
					snprintf(str, SIZE_OF_ARRAY(str), "Hello %s",
						 message.from->first_name);
				} else  if (strstr(message.text, "/temperature")) {
temp_retry1:
					if (!getDHTdata(4, &data1))
						goto temp_retry1;
temp_retry2:
					if (!getDHTdata(16, &data2))
						goto temp_retry2;
					snprintf(str, SIZE_OF_ARRAY(str), "T: %.1lf *C T: %.1lf *C",
							data1.temerature,
							data2.temerature);
				} else  if (strstr(message.text, "/humidity")) {
hum_retry1:
					if (!getDHTdata(4, &data1))
						goto hum_retry1;
hum_retry2:
					if (!getDHTdata(16, &data2))
						goto hum_retry2;
					snprintf(str, SIZE_OF_ARRAY(str), "H: %.1lf%% H: %.1lf%%",
							data1.humidity,
							data2.humidity);
				} else {
					snprintf(str, SIZE_OF_ARRAY(str), "RE:%s", message.text);
				}
			} else {
				snprintf(str, SIZE_OF_ARRAY(str), "Could not get message");
			}
			ret = telebot_send_message(handle, message.chat->id, str, "",
						   false, false, 0, "");
			if (ret != TELEBOT_ERROR_NONE) {
				pr_err("Failed to send message: %d \n", ret);
			}
			offset = updates[index].update_id + 1;
		}

		telebot_free_updates(updates, count);
	}

	telebot_destroy(handle);

	return 0;

close_fd:
	fclose(fp);
err:
	return -1;
}
