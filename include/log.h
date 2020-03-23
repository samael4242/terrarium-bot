#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifndef DEBUG
#define pr_fmt(fmt) fmt
#else
#define pr_fmt(fmt) "%s:%d: " fmt, __FILE__, __LINE__
#endif

#define LOG_ERR "E: "     /* error conditions */
#define LOG_WARNING "W: " /* warning conditions */
#define LOG_INFO "I: "    /* informational */
#define LOG_DEBUG "D: "   /* debugging */

#define log(prefix, fmt, ...) \
	(void)(fprintf(stderr, prefix pr_fmt(fmt), ##__VA_ARGS__))

#define pr_err(...) __extension__(log(LOG_ERR, ##__VA_ARGS__))
#define pr_warn(...) __extension__(log(LOG_WARNING, ##__VA_ARGS__))
#define pr_info(...) __extension__(log(LOG_INFO, ##__VA_ARGS__))

#ifdef DEBUG
#define pr_debug(...) __extension__(log(LOG_DEBUG, ##__VA_ARGS__))
#else
#define pr_debug(...)
#endif

#endif /* LOG_H */
