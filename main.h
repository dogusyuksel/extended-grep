#ifndef _MAIN_H___
#define _MAIN_H___

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define VERSION		"01.00"

#define OK			0
#define NOK			1

#define DELIM 		","

#define ONE_LINE_MAX_LEN	(1024 + 1)
#define BUFF_SIZE			256

#define debugf(...)		fprintf(stdout, __VA_ARGS__)
#define errorf(...)		fprintf(stderr, __VA_ARGS__)

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
