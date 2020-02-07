#ifndef BOT_H
#define BOT_H

struct bot_handle {
	void *private_data;
};

int bot_init(struct bot_handle *handle);
int bot_close(struct bot_handle *handle);

int bot_process_message(struct bot_handle *handle);

#endif /* BOT_H */
