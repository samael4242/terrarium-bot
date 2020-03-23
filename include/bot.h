#ifndef BOT_H
#define BOT_H

struct bot_handle {
	void *private_data;
};

struct arguments {
	char *token_path;
};

int bot_init(struct bot_handle *handle, struct arguments *bot_args);
int bot_close(struct bot_handle *handle);

int bot_process_message(struct bot_handle *handle);

#endif /* BOT_H */
