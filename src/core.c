#include <bot.h>
#include <log.h>
#include <common.h>

int main(int argc, char *argv[])
{
	struct bot_handle handle;

	if(bot_init(&handle)) {
		pr_err("Bot init failed\n");
		return -1;
	}

	bot_process_message(&handle);

	bot_close(&handle);

	return 0;
}
