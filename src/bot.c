#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <bot.h>
#include <telebot/telebot.h>
#include <driver.h>
#include <common.h>
#include <log.h>

#define MAX_STRING_SIZE 4096
#define TOKEN_PATH ".token"

#define GPROUP_CAHT_ID -1001316572508

int bot_init(struct bot_handle *handle)
{
	telebot_user_t *me;
	telebot_handler_t *tb_handle = NULL;
	FILE *fp = NULL;
	char token[MAX_STRING_SIZE] = {0,};

	pr_info("Welcome to Terrarium bot\n");

	tb_handle = malloc(sizeof(telebot_handler_t));
	if (tb_handle == NULL)
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

	if (telebot_create(tb_handle, token) != TELEBOT_ERROR_NONE) {
		pr_err("Telebot create failed\n");
		goto err;
	}


	if (telebot_get_me(*tb_handle, &me) != TELEBOT_ERROR_NONE) {
		pr_err("Failed to get bot information\n");
		telebot_destroy(*tb_handle);
		goto err;
	}

	pr_debug("ID: %d\n", me->id);
	pr_debug("First Name: %s\n", me->first_name);
	pr_debug("User Name: %s\n", me->username);

	telebot_free_me(me);

	handle->private_data = tb_handle;
	driver_init();

	return 0;

close_fd:
	fclose(fp);
err:
	free(tb_handle);
	return -1;
}

int bot_close(struct bot_handle *handle)
{
	telebot_handler_t *tb_handle = 
		handle->private_data;

	telebot_destroy(*tb_handle);
	free(tb_handle);
	driver_close();
	return 0;
}

int bot_process_message(struct bot_handle *handle)
{
	int index, count, offset = 1;
	char str[MAX_STRING_SIZE] = {0,};
	telebot_error_e ret;
	telebot_message_t message;
	telebot_update_t *updates;
	telebot_handler_t *tb_handle = handle->private_data;
	struct DHTdata data1;
	struct DHTdata data2;

	while (1) {
		/* busy wait */
		sleep(1);

		ret = telebot_get_updates(*tb_handle, offset, 20, 0, NULL, 0, &updates, &count);
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
					if (!get_DHT_data(4, &data1))
						goto temp_retry1;
temp_retry2:
					if (!get_DHT_data(16, &data2))
						goto temp_retry2;
					snprintf(str, SIZE_OF_ARRAY(str), "T: %.1lf *C T: %.1lf *C",
							data1.temerature,
							data2.temerature);
				} else  if (strstr(message.text, "/humidity")) {
hum_retry1:
					if (!get_DHT_data(4, &data1))
						goto hum_retry1;
hum_retry2:
					if (!get_DHT_data(16, &data2))
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
			ret = telebot_send_message(*tb_handle, message.chat->id, str, "",
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
