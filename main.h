#ifndef _MAIN_H___
#define _MAIN_H___

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define VERSION		"00.02"

#define OK			0
#define NOK			1

#define DELIM 		","

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define COMMAND_HISTORY	"command_history.txt"
#define PIXEL_WIDTH		5
#define EMPTY_WIDTH		2
#define IMAGE_HEIGHT	2048
#define IMAGE_MAX_WIDTH	64000

#define ONE_LINE_MAX_LEN	(2048 + 1)

#define debugf(...)		fprintf(stdout, __VA_ARGS__)
#define errorf(...)		fprintf(stderr, __VA_ARGS__)

#define UNUSED(__val__)		((void)__val__)

#define FREE(p)			{										\
							if (p) {							\
								free(p);						\
								p = NULL;						\
							}									\
						}

#define FCLOSE(p)		{										\
							if (p) {							\
								fclose(p);						\
								p = NULL;						\
							}									\
						}

enum element_type
{
	ALPHANUMERIC,
	NUMERIC,
	ELEMENT_TYPE_MAX
};

#endif //_MAIN_H___
