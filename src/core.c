#include <bot.h>
#include <log.h>
#include <common.h>
#include <argp.h>

static struct argp_option bot_options[] = {
	{"token", 't', "TOKEN", 0, "Path to .token file. Default path - current folder"},
	{0}
};

static char args_bot_doc[] = "TOKEN";
static char bot_doc[] = "Terrarium-bot -- A program to controle terrarium using telegram bot";

static error_t parse_bot_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key) {
		case 't':
			arguments->token_path = arg;
			break;
		case ARGP_KEY_ARG:
			break;
		case ARGP_KEY_END:
			if (arguments->token_path == NULL) {
				argp_usage (state);
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	};

	return 0;
}

int main(int argc, char *argv[])
{
	struct bot_handle handle;
	struct argp bot_argp = {bot_options, parse_bot_opt,
		args_bot_doc, bot_doc};
	struct arguments bot_arguments = {
		.token_path = NULL 
	};

	argp_parse(&bot_argp, argc, argv, 0, 0, &bot_arguments);

	if(bot_init(&handle, &bot_arguments)) {
		pr_err("Bot init failed\n");
		return -1;
	}

	bot_process_message(&handle);

	bot_close(&handle);

	return 0;
}
