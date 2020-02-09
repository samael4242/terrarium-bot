#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <bot.h>
#include <telebot/telebot.h>
#include <device.h>
#include <common.h>
#include <log.h>

#define MAX_STRING_SIZE 4096
#define TOKEN_PATH ".token"

#define GPROUP_CAHT_ID -1001316572508

struct bot_data {
	struct instance dev_inst;
	telebot_handler_t tb_handle;
};

int bot_init(struct bot_handle *handle)
{
	telebot_user_t *me;
	struct bot_data *data = NULL;
	FILE *fp = NULL;
	char token[MAX_STRING_SIZE] = {0,};

	pr_info("Welcome to Terrarium bot\n");

	data = malloc(sizeof(*data));
	if (data == NULL)
		goto err;

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

	if (telebot_create(&data->tb_handle, token) != TELEBOT_ERROR_NONE) {
		pr_err("Telebot create failed\n");
		goto err;
	}

	if (telebot_get_me(data->tb_handle, &me) != TELEBOT_ERROR_NONE) {
		pr_err("Failed to get bot information\n");
		telebot_destroy(data->tb_handle);
		goto err;
	}

	pr_debug("ID: %d\n", me->id);
	pr_debug("First Name: %s\n", me->first_name);
	pr_debug("User Name: %s\n", me->username);

	telebot_free_me(me);
	device_init(&data->dev_inst);
	handle->private_data = data;

	return 0;

close_fd:
	fclose(fp);
err:
	free(data);
	return -1;
}

int bot_close(struct bot_handle *handle)
{
	struct bot_data *data =
		handle->private_data;

	telebot_destroy(data->tb_handle);
	device_close(&data->dev_inst);
	free(data);
	return 0;
}

int bot_process_message(struct bot_handle *handle)
{
	int index, count, offset = 1;
	char str[MAX_STRING_SIZE] = {0,};
	telebot_error_e ret;
	telebot_message_t message;
	telebot_update_t *updates;
	struct bot_data *data = handle->private_data;

	while (1) {
		/* busy wait */
		sleep(1);

		ret = telebot_get_updates(data->tb_handle, offset, 20, 0, NULL, 0, &updates, &count);
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
					device_get_temperature(&data->dev_inst);
					snprintf(str, SIZE_OF_ARRAY(str), "T: %.1lf *C T: %.1lf *C",
							data->dev_inst.temperature.ch1,
							data->dev_inst.temperature.ch2);
				} else  if (strstr(message.text, "/humidity")) {
					device_get_humidity(&data->dev_inst);
					snprintf(str, SIZE_OF_ARRAY(str), "H: %.1lf%% H: %.1lf%%",
							data->dev_inst.humidity.ch1,
							data->dev_inst.humidity.ch2);
				} else {
					snprintf(str, SIZE_OF_ARRAY(str), "RE:%s", message.text);
				}
			} else {
				snprintf(str, SIZE_OF_ARRAY(str), "Could not get message");
			}
			ret = telebot_send_message(data->tb_handle, message.chat->id, str, "",
						   false, false, 0, "");
			if (ret != TELEBOT_ERROR_NONE) {
				pr_err("Failed to send message: %d \n", ret);
			}
			offset = updates[index].update_id + 1;
		}

		telebot_free_updates(updates, count);
	}


	return 0;
}
