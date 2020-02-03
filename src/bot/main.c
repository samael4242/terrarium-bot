#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <telebot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define SIZE_OF_ARRAY(array) (sizeof(array)/sizeof(array[0]))
#define MAX_STRING_SIZE 4096
#define TOKEN_PATH ".token"

int main(int argc, char *argv[])
{
	int index, count, offset = 1;
	char str[MAX_STRING_SIZE] = {0,};
	char token[MAX_STRING_SIZE] = {0,};
	FILE *fp = NULL;
	telebot_user_t *me;
	telebot_handler_t handle;
	telebot_error_e ret;
	telebot_message_t message;
	telebot_update_t *updates;

	printf("Welcome to Terrarium bot\n");

	fp = fopen(".token", "r");
	if (fp == NULL) {
		printf("Failed to open token file\n");
		goto err;
	}

	if (fscanf(fp, "%s", token) == 0) {
		printf("Reading token failed");
		goto close_fd;
	}

	printf("Token: %s\n", token);

	fclose(fp);

	if (telebot_create(&handle, token) != TELEBOT_ERROR_NONE) {
		printf("Telebot create failed\n");
		goto err;
	}


	if (telebot_get_me(handle, &me) != TELEBOT_ERROR_NONE) {
		printf("Failed to get bot information\n");
		telebot_destroy(handle);
		goto err;
	}

	printf("ID: %d\n", me->id);
	printf("First Name: %s\n", me->first_name);
	printf("User Name: %s\n", me->username);

	telebot_free_me(me);

	while (1) {
		/* busy wait */
		sleep(1);

		ret = telebot_get_updates(handle, offset, 20, 0, NULL, 0, &updates, &count);
		if (ret != TELEBOT_ERROR_NONE)
			continue;

		printf("Number of updates: %d\n", count);

		for (index = 0;index < count; index++) {
			message = updates[index].message;
			printf("=======================================================\n");
			printf("%s: %s \n", message.from->first_name, message.text);
			printf("=======================================================\n");\
			if (message.text) {
				if (strstr(message.text, "/start")) {
					snprintf(str, SIZE_OF_ARRAY(str), "Hello %s",
						 message.from->first_name);
				}
				else {

						snprintf(str, SIZE_OF_ARRAY(str), "RE:%s", message.text);
				}
			} else {
				snprintf(str, SIZE_OF_ARRAY(str), "Could not get message");
			}
			ret = telebot_send_message(handle, message.chat->id, str, "",
						   false, false, 0, "");
			if (ret != TELEBOT_ERROR_NONE) {
				printf("Failed to send message: %d \n", ret);
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
