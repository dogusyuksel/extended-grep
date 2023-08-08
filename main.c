#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/queue.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <libgen.h>

#include "main.h"

struct entry
{
	char *word;
	TAILQ_ENTRY(entry) entries;
};
TAILQ_HEAD(tailq, entry);

struct image_data_entry
{
	long data;
	TAILQ_ENTRY(image_data_entry) entries;
};
TAILQ_HEAD(image_data_tailq, image_data_entry);

struct parser
{
	bool add_line_no;
	bool only_show_changes;
	bool show_line;
	bool image_created;
	bool take_as_string;
	unsigned int from;
	unsigned int select;
	unsigned int element_at;
	unsigned int line_below;
	unsigned long line_cnt;
	unsigned long total_found_cnt;
	unsigned long total_showed_cnt;
	unsigned long start_lineno;
	unsigned long end_lineno;
	long max_thres;
	long min_thres;
	long max_value;
	long min_value;
	long base;
	char *keyword_list;
	char *file_path;
	char *dname;
	char *tag;
	char seperator[2];
	char commandline[ONE_LINE_MAX_LEN];
	FILE *fp;
	struct tailq keyword_list_tq;
	struct image_data_tailq image_data_tailq;
};

static struct parser parser;

static struct option parameters[] = {
	{ "logfilepath",		required_argument,	0,		'f'		},
	{ "help",				no_argument,		0,		'h'		},
	{ "keywords",			required_argument,	0,		'k'		},
	{ "seperator",			required_argument,	0,		's'		},
	{ "elementat",			required_argument,	0,		'e'		},
	{ "linebelow",			required_argument,	0,		'b'		},
	{ "select",				required_argument,	0,		't'		},
	{ "from",				required_argument,	0,		'r'		},
	{ "showlineno",			no_argument,		0,		'l'		},
	{ "onlyshowchanges",	no_argument,		0,		'c'		},
	{ "maxthres",			required_argument,	0,		'x'		},
	{ "minthres",			required_argument,	0,		'm'		},
	{ "version",			no_argument,		0,		'v'		},
	{ "drawgraph",			no_argument,		0,		'g'		},
	{ "startlineno",		required_argument,	0,		'i'		},
	{ "endlineno",			required_argument,	0,		'j'		},
	{ "showline",			no_argument,		0,		'q'		},
	{ "tag",				required_argument,	0,		'a'		},
	{ "takeasstring",		no_argument,		0,		'd'		},
	{ "base",				required_argument,	0,		'n'		},
	{ NULL,					0,					0,		0 		},
};

static void print_help_exit (const char *name)
{
	if (!name) {
		errorf("name is null\n");
		exit(NOK);
	}
	debugf("\n%s - ver: %s\n", name, VERSION);
	debugf("This application is used to parse big and complex log files.\nIt extracts necessary data for you to observe changes on them affectively.\n");

	debugf("\nparameters;\n\n");
	debugf("\t%s--logfilepath%s     \t(-f): log file path\n\t\tused to specify the log file's directory path.\n\t\tThis is %sMANDATORY%s field\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET, ANSI_COLOR_RED, ANSI_COLOR_RESET);
	debugf("\t%s--keywords%s        \t(-k): keyword list\n\t\tused to specify special keywords to pick a line. You can use multiple keywords seperated by comma without empty space.\n\t\tThis is %sMANDATORY%s field\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET, ANSI_COLOR_RED, ANSI_COLOR_RESET);
	debugf("\t%s--seperator%s       \t(-s): seperator\n\t\tused to specify special character to split the picked line.\n\t\tTAB is used if it is not set\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--elementat%s       \t(-e): element at\n\t\tused to specify which element you want to extract after splitting the line.\n\t\tIf this is not used, then whole line will be filtered.\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--linebelow%s       \t(-b): line below\n\t\tused to select a new line that is number of lines below the picket line before\n\t\tThis is usefull when there is no constant string specifier to pick the line that we want to examine\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--showlineno%s      \t(-l): add line no\n\t\tused to specify real line no in the log doc in the new generated file\n\t\tNo parameter required\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--onlyshowchanges%s \t(-c): show only changes\n\t\tused to parameter changes, like \"watch\" property\n\t\tNo parameter required\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--maxthres%s        \t(-x): max threshold\n\t\tused to filter numeric values\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--minthres%s        \t(-m): min threshold\n\t\tused to filter numeric values\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--select%s          \t(-t): select\n\t\tused to pick \"select-th\" line from \"from\" lines\n\t\tfrom arg is must\n\t\tusefull to remove duplicated log lines\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--from%s            \t(-r): from\n\t\tused to pick \"select-th\" line from \"from\" lines\n\t\tselect arg is must\n\t\tusefull to remove duplicated log lines\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--version%s         \t(-v): show version\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--drawgraph%s       \t(-g): draw graph\n\t\tcreates graph from the extracted data\n\t\tuseful when to visualize the data\n\t\tnewly created image can be seen on current dir\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--startlineno%s     \t(-i): show results after the line given as argument\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--endlineno%s       \t(-j): show results before the line given as argument\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--showline%s        \t(-q): show directly the lines that contains kewords. Other filters cannot be used except 'startlineno' and 'endlineno'\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--tag%s             \t(-a): TAG the line saved in command history database\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--takeasstring%s    \t(-d): for complex lines, the user could work and save some filtered result first, to rework on it later.\n\t\tSo this option can be used to threat the selected word as string\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
	debugf("\t%s--base%s            \t(-n): indicates filtered value's base. Eg: 10, 16 (for hex)\n\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);

	debugf("\n");

	exit(NOK);
}

static int create_keyword_list_list(struct parser *parser)
{
	struct entry *entry;
	char *rest = NULL;
	char *token;
	char *delim = DELIM;

	if (!parser || !parser->keyword_list || !strlen(parser->keyword_list) || !delim || !strlen(delim)) {
		errorf("wrong prm\n");
		return NOK;
	}

	for (token = strtok_r(parser->keyword_list, delim, &rest);
		token != NULL;
		token = strtok_r(NULL, delim, &rest)) {
	
		entry = calloc(1, sizeof(struct entry));
		if (!entry) {
			errorf("calloc failed\n");
			return NOK;
		}

		entry->word = strdup(token);
		if (!entry->word) {
			errorf("strdup failed\n");
			return NOK;
		}

		TAILQ_INSERT_TAIL(&parser->keyword_list_tq, entry, entries);
	}

	return OK;
}

static void destroy_data_list(struct parser *parser)
{
	struct image_data_entry *entry;
	struct image_data_entry *entry_next;

	entry = TAILQ_FIRST(&parser->image_data_tailq);
	while (entry != NULL) {
		entry_next = TAILQ_NEXT(entry, entries);
		FREE(entry);
		entry = entry_next;
	}
}

static void destroy_keyword_list_list(struct parser *parser)
{
	struct entry *entry;
	struct entry *entry_next;

	entry = TAILQ_FIRST(&parser->keyword_list_tq);
	while (entry != NULL) {
		entry_next = TAILQ_NEXT(entry, entries);
		FREE(entry->word);
		FREE(entry);
		entry = entry_next;
	}
}

static int contains_necessary_keywords(struct  parser *parser, char *line)
{
	struct entry *entry;

	if (!parser || !line) {
		errorf("wrong prm\n");
		return NOK;
	}

	TAILQ_FOREACH(entry, &parser->keyword_list_tq, entries) {
		if (!strstr(line, entry->word)) {
			return NOK;
		}
	}

	return OK;
}

static void data_mapping_process(struct image_data_tailq *image_data_tailq, long max_val, long min_val, long *total_data)
{
	long i = 0;
	long new_max_value = max_val;
	double dividor = ceil((double)((double)max_val / (double)IMAGE_HEIGHT));
	double multiplier = round((double)((double)IMAGE_HEIGHT / (double)max_val));
	struct image_data_entry *entry = NULL;

	if (!image_data_tailq) {
		errorf("arg is NULL\n");
		return;
	}

	if (min_val < 0) {
		new_max_value += ((-1) * min_val);
		dividor = ceil((double)((double)new_max_value / (double)IMAGE_HEIGHT));
		multiplier = round((double)((double)IMAGE_HEIGHT / (double)new_max_value));
	}

	TAILQ_FOREACH(entry, image_data_tailq, entries) {
		if (min_val < 0) {
			entry->data += (-1) * min_val;
		}

		if (new_max_value < IMAGE_HEIGHT) {
			entry->data = (long)(entry->data * multiplier);
		} else {
			entry->data = (long)(entry->data / dividor);
		}

		i++;
	}

	*total_data = i;
}

static int create_image(struct parser *parser, long max_val, long min_val, char *datetimestring)
{
	FILE *pgmimg = NULL;
	int i = 0, j = 0;
	long total_data = 0;
	char filepath[ONE_LINE_MAX_LEN] = {0};
	struct image_data_entry *entry = NULL;
	int image_height = IMAGE_HEIGHT;
	struct entry *kw_entry = NULL;

	UNUSED(min_val);

	if (!parser || !datetimestring) {
		errorf("arg is NULL\n");
		return NOK;
	}

	data_mapping_process(&(parser->image_data_tailq), max_val, min_val, &total_data);

	if ((int)(total_data * (EMPTY_WIDTH + PIXEL_WIDTH)) > IMAGE_MAX_WIDTH) {
		errorf("data is too long, please shorten it to draw graph\n");
		return OK;
	}

	image_height = (int)((double)image_height * 1.2);

	snprintf(filepath, sizeof(filepath), "%s/%s%s%s.pgm",
		parser->dname, (!parser->tag) ? "" : parser->tag, (!parser->tag) ? "" : "-", datetimestring);

	pgmimg = fopen(filepath, "wb");
	if (!pgmimg) {
		errorf("fopen failed filepath: %s\n", filepath);
		return NOK;
	}

	fprintf(pgmimg, "P2\n");
	fprintf(pgmimg, "%d %d\n", (int)(total_data * (EMPTY_WIDTH + PIXEL_WIDTH)), (int)image_height);

	for (i = image_height - 1; i >= 0; i--) {
		TAILQ_FOREACH(entry, &(parser->image_data_tailq), entries) {
			for (j = 0; j < PIXEL_WIDTH; j++) {
				if (entry->data < i) {
					fprintf(pgmimg, "255 ");
				} else {
					fprintf(pgmimg, "0 ");
				}
			}
			for (j = 0; j < EMPTY_WIDTH; j++) {
				fprintf(pgmimg, "255 ");
			}
		}
		fprintf(pgmimg, "\n");
	}

	fclose(pgmimg);

	return OK;
}

static int insert_data_to_data_queue(long data, struct image_data_tailq *image_data_tailq)
{
	struct image_data_entry *entry = NULL;

	if (!image_data_tailq) {
		errorf("arg is NULL\n");
		return NOK;
	}

	entry = calloc(1, sizeof(struct image_data_entry));
	if (!entry) {
		errorf("calloc failed\n");
		return NOK;
	}

	entry->data = data;

	TAILQ_INSERT_TAIL(image_data_tailq, entry, entries);

	return OK;
}

static long take_clean_value_hex(char *token)
{
	int i = 0;
	long value = 0;
	char *ptr = NULL;
	unsigned int starting_pos = 0;
	unsigned int ending_pos = 0;
	char *dup_token = NULL;

	if (!token) {
		return 0;
	}
	if (strlen(token) > 2 && token[0] == '0' &&
		(token[1] == 'x' || token[1] == 'X')) {
		dup_token = strdup(&token[2]);
	} else {
		dup_token = strdup(token);
	}
	if (!dup_token) {
		return 0;
	}

	for (i = 0; i < (int)strlen(dup_token); i++) {
		if ((dup_token[i] >= 0x30 && dup_token[i] <= 0x39) ||
			(dup_token[i] == '-') ||
			(dup_token[i] >= 'a' && dup_token[i] <= 'f') ||
			(dup_token[i] >= 'A' && dup_token[i] <= 'F')) {
			starting_pos = i;
			break;
		}
	}

	ending_pos = strlen(dup_token);
	for (i = 0; i < (int)strlen(dup_token); i++) {
		if ((!(dup_token[i] >= 0x30 && dup_token[i] <= 0x39) &&
			!(dup_token[i] >= 'a' && dup_token[i] <= 'f') &&
			!(dup_token[i] >= 'A' && dup_token[i] <= 'F')) || (dup_token[i] == '-') &&
			i >= 1) {
			ending_pos = i - 1;
			break;
		}
	}

	if (starting_pos < strlen(dup_token) && ending_pos >= starting_pos) {
		dup_token[ending_pos + 1] = '\0';
		value = strtol(&dup_token[starting_pos], &ptr, 16);

		if (ptr && strlen(ptr) > 0) {
			FREE(dup_token);
			return 0;
		}
	}

	FREE(dup_token);

	return value;
}

static long take_clean_value(char *token)
{
	int i = 0;
	long value = 0;
	char *ptr = NULL;
	unsigned int starting_pos = 0;
	unsigned int ending_pos = 0;
	char *dup_token = NULL;

	if (!token) {
		return 0;
	}
	dup_token = strdup(token);
	if (!dup_token) {
		return 0;
	}

	for (i = 0; i < (int)strlen(dup_token); i++) {
		if ((dup_token[i] >= 0x30 && dup_token[i] <= 0x39) || (dup_token[i] == '-')) {
			starting_pos = i;
			break;
		}
	}

	ending_pos = strlen(dup_token);
	for (i = 0; i < (int)strlen(dup_token); i++) {
		if (!(dup_token[i] >= 0x30 && dup_token[i] <= 0x39) || (dup_token[i] == '-') &&
			i >= 1) {
			ending_pos = i - 1;
			break;
		}
	}

	if (starting_pos < strlen(dup_token) && ending_pos >= starting_pos) {
		dup_token[ending_pos + 1] = '\0';
		value = strtol(&dup_token[starting_pos], &ptr, 10);

		if (ptr && strlen(ptr) > 0) {
			FREE(dup_token);
			return 0;
		}
	}

	FREE(dup_token);

	return value;
}

static long clean_value_mux(char *token, long base)
{
	switch (base)
	{
	case 10:
		return take_clean_value(token);
		break;
	case 16:
		return take_clean_value_hex(token);
		break;
	default:
		break;
	}

	return 0;
}

static int fgets_custom(char *s, int n, FILE *stream)
{
	int read = EOF;
	int read_count = 0;

	if (!s || !n || !stream) {
		return NOK;
	}

	while(true) {
		read = fgetc(stream);
		if (read == EOF) {
			return NOK;
		}
		if (read_count >= n || (char)read == '\n') {
			break;
		}
		if ((char)read < ' ' || (char)read > '~') {
			continue;
		}
		s[read_count++] = (char)read;
	}

	s[read_count] = 0;
	return OK;
}

static int extract_data(struct parser *parser)
{
	unsigned int element_cnt = 0;
	char buffer[ONE_LINE_MAX_LEN] = {0};
	char *rest = NULL;
	char *token = NULL;
	char temp[ONE_LINE_MAX_LEN] = {0};
	char datetimestring[ONE_LINE_MAX_LEN] = {0};
	char commandhistorypath[ONE_LINE_MAX_LEN] = {0};
	long value_priv = 0;
	long value = 0;
	struct timespec ts;
	struct tm *brokentime = NULL;
	FILE *commandhistoryfs = NULL;

	if (!parser) {
		errorf("wrong prm\n");
		return NOK;
	}

	parser->fp = fopen(parser->file_path, "r");
	if (!parser->fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	if (parser->image_created) {
		TAILQ_INIT(&(parser->image_data_tailq));
	}

	memset(buffer, 0, sizeof(buffer));

	while (fgets_custom(buffer, sizeof(buffer) - 1, parser->fp) == OK) {
		memset(temp, 0, sizeof(temp));

		buffer[strcspn(buffer, "\n")] = 0;

		++parser->line_cnt;

		if (contains_necessary_keywords(parser, buffer) == OK) {
			element_cnt = 0;

			if (parser->line_below != 0) {
				unsigned int i = 0;

				for (i = 0; i < parser->line_below; i++) {
					memset(buffer, 0, sizeof(buffer));
					if (fgets_custom(buffer, sizeof(buffer) - 1, parser->fp) != OK) {
						errorf("fget fails\n");
						goto out;
					}
					++parser->line_cnt;
				}
			}

			if (parser->start_lineno != UINT_MAX && parser->line_cnt < parser->start_lineno) {
				continue;
			}

			if (parser->end_lineno != UINT_MAX && parser->line_cnt > parser->end_lineno) {
				continue;
			}

			if (parser->show_line) {
				debugf("%s\n", buffer);
				continue;
			}

			if (parser->element_at == UINT_MAX) {
				token = buffer;
				element_cnt++;
				goto skip_seperator;
			}

			for (token = strtok_r(buffer, parser->seperator, &rest);
				token != NULL;
				token = strtok_r(NULL, parser->seperator, &rest)) {

				if (!strstr(token, parser->seperator)) {
					element_cnt++;
				}

				if (element_cnt == parser->element_at) {
					break;
				}
			}

skip_seperator:
			if (!token) {
				continue;
			}

			if (strlen(token) > ONE_LINE_MAX_LEN) {
				continue;
			}

			if (element_cnt) {
				parser->total_found_cnt++;
			}
			value = clean_value_mux(token, parser->base);

			if (parser->min_thres != LONG_MAX && value < parser->min_thres) {
				continue;
			}
			if (parser->max_thres != LONG_MAX && value > parser->max_thres) {
				continue;
			}

			if (parser->only_show_changes && value == value_priv) {
				continue;
			}

			value_priv = value;


			if (parser->take_as_string) {
				snprintf(temp, sizeof(temp), "%s", token);
				goto add_line_no;
			}
			snprintf(temp, sizeof(temp), "%ld", value);

			if (!((parser->total_found_cnt - 1) % parser->from == parser->select)) {
				continue;
			}

			if (value > parser->max_value) {
				parser->max_value = value;
			}
			if (value < parser->min_value) {
				parser->min_value = value;
			}

			if (parser->image_created) {
				if (insert_data_to_data_queue(value, &(parser->image_data_tailq)) == NOK) {
					errorf("insert failed\n");
				}
			}

add_line_no:
			if (parser->add_line_no) {
				int len = strlen(temp);
				snprintf(&temp[len], sizeof(temp) - len, "\t%ld", parser->line_cnt);
			}

			debugf("%s\n", temp);

			parser->total_showed_cnt++;
		}
	}

out:
	FCLOSE(parser->fp);

	if (!parser->show_line) {
		errorf("\n**********************************************************\n");
		errorf("TOTAL PROGRESSED LINES: %ld\n", parser->line_cnt);
		errorf("TOTAL EXTRACTED LINES: %ld\n", parser->total_found_cnt);
		errorf("TOTAL SHOWED LINES: %ld\n", parser->total_showed_cnt);
		if (parser->max_value != LONG_MIN || parser->min_value != LONG_MAX) {
			errorf("MAX: %ld\tMIN: %ld\n", parser->max_value, parser->min_value);
		}
		errorf("\n**********************************************************\n");


		if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
			errorf("clock_gettime");
			return NOK;
		}
		brokentime = localtime(&(ts.tv_sec));

		if(strftime(datetimestring, sizeof(datetimestring), "%Y-%m-%d %H:%M:%S", brokentime) == 0) {
			errorf("strftime failed\n");
			return NOK;
		}

		if (parser->image_created) {
			if (create_image(parser, parser->max_value, parser->min_value, datetimestring) == NOK) {
				errorf("create_image() failed\n");
			}
		}

		snprintf(commandhistorypath, sizeof(commandhistorypath), "%s/%s", parser->dname, COMMAND_HISTORY);
		commandhistoryfs = fopen(commandhistorypath, "a+");
		if (!commandhistoryfs) {
			errorf("fopen failed\n");
			return NOK;
		}
		fputc('[', commandhistoryfs);
		fputs(datetimestring, commandhistoryfs);
		fputs("]\t", commandhistoryfs);
		if (parser->tag) {
			fputc('[', commandhistoryfs);
			fputs(parser->tag, commandhistoryfs);
			fputs("]\t", commandhistoryfs);
		}
		fputs(parser->commandline, commandhistoryfs);
		fclose(commandhistoryfs);
	}

	return OK;
}

static void init_parser(struct parser *parser)
{
	if (!parser) {
		errorf("arg is null\n");
		return;
	}

	parser->fp = NULL;
	parser->add_line_no = false;
	parser->only_show_changes = false;
	parser->image_created = false;
	parser->select = 0;
	parser->from = 1;
	parser->line_below = 0;
	memset(parser->seperator, 0, sizeof(parser->seperator));
	parser->seperator[0] = '\t';
	parser->element_at = UINT_MAX;
	parser->min_thres = LONG_MAX;
	parser->max_thres = LONG_MAX;
	parser->max_value = LONG_MIN;
	parser->min_value = LONG_MAX;
	parser->start_lineno = UINT_MAX;
	parser->end_lineno = UINT_MAX;
	parser->show_line = false;
	parser->base = 10;
	memset(parser->commandline, 0, sizeof(parser->commandline));
}

static void deinit_parser(struct parser *parser)
{
	if (!parser) {
		errorf("arg is null\n");
		return;
	}

	FREE(parser->file_path);
	FREE(parser->dname);
	FREE(parser->tag);
	FREE(parser->keyword_list);

	FCLOSE(parser->fp);
}

static void sigint_handler(__attribute__((unused)) int sig_num)
{
	if (!parser.show_line) {
		errorf("\n**********************************************************\n");
		errorf("TOTAL PROGRESSED LINES: %ld\n", parser.line_cnt);
		errorf("TOTAL EXTRACTED LINES: %ld\n", parser.total_found_cnt);
		errorf("TOTAL SHOWED LINES: %ld\n", (parser.total_found_cnt / parser.from));
		if (parser.max_value != LONG_MIN || parser.min_value != LONG_MAX) {
			errorf("MAX: %ld\tMIN: %ld\n", parser.max_value, parser.min_value);
		}
		errorf("\n**********************************************************\n");
	}

	destroy_keyword_list_list(&parser);
	destroy_data_list(&parser);
	deinit_parser(&parser);

	exit(NOK);
}

int main(int argc, char **argv)
{
	int ret = OK;
	int c, o, i;
	char *ptr = NULL;
	char file_path_dup[ONE_LINE_MAX_LEN] = {0};
	char file_dup_command[ONE_LINE_MAX_LEN] = {0};

	signal(SIGINT, sigint_handler);

	init_parser(&parser);

	for (i = 0; i < argc; i++) {
		strncat(parser.commandline, argv[i], sizeof(parser.commandline) - strlen(parser.commandline) - 1);
		strncat(parser.commandline, " ", sizeof(parser.commandline) - strlen(parser.commandline) - 1);
	}
	strncat(parser.commandline, "\n", sizeof(parser.commandline) - strlen(parser.commandline) - 1);

	while ((c = getopt_long(argc, argv, "hnlcvqgf:k:s:e:b:t:r:x:m:i:j:a:", parameters, &o)) != -1) {
		switch (c) {
			case 'f':
				snprintf(file_path_dup, sizeof(file_path_dup), "%s.dup", optarg);
				parser.file_path = strdup(file_path_dup);
				parser.dname = strdup(optarg);
				if (!parser.file_path || !parser.dname) {
					errorf("strdup failed\n");
					return NOK;
				}

				dirname(parser.dname);
				if (!strlen(parser.file_path) || !strlen(parser.dname)) {
					errorf("file_path seems wrong\n");
					return NOK;
				}

				snprintf(file_dup_command, sizeof(file_path_dup), "cp -f %s %s", optarg, parser.file_path);
				system(file_dup_command);
				break;
			case 'h':
				print_help_exit(argv[0]);
				break;
			case 'k':
				parser.keyword_list = strdup(optarg);
				if (!parser.keyword_list) {
					errorf("strdup failed\n");
					return NOK;
				}
				break;
			case 'a':
				parser.tag = strdup(optarg);
				if (!parser.tag) {
					errorf("strdup failed\n");
					return NOK;
				}
				break;
			case 's':
				if (optarg && strlen(optarg) > 0) {
					parser.seperator[0] = optarg[0];
					parser.seperator[1] = '\0';
				} else {
					errorf("arg is wrong\n");
					return NOK;
				}
				break;
			case 'e':
				parser.element_at = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'l':
				parser.add_line_no = true;
				break;
			case 'd':
				parser.take_as_string = true;
				break;
			case 'c':
				parser.only_show_changes = true;
				break;
			case 't':
				parser.select = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'r':
				parser.from = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'b':
				parser.line_below = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'x':
				parser.max_thres = strtol(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'n':
				parser.base = strtol(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'm':
				parser.min_thres = strtol(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'v':
				errorf("\n%s - ver: %s\n", argv[0], VERSION);
				return OK;
			case 'g':
				parser.image_created = true;
				break;
			case 'i':
				parser.start_lineno = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'j':
				parser.end_lineno = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'q':
				parser.show_line = true;
				break;
			default:
				errorf("unknown argument\n");
				return NOK;
		}
	}

	if (!parser.file_path) {
		errorf("'file' argument is mandatory\n");
		print_help_exit(argv[0]);
	}

	if (!parser.keyword_list) {
		errorf("keyword_list field is mandatory\n");
		print_help_exit(argv[0]);
	}

	if (parser.from == 0 ||
		(parser.select >= parser.from)) {
		errorf("wronf select/from usage\n");
		print_help_exit(argv[0]);
	}

	if (parser.min_thres != LONG_MAX || parser.max_thres != LONG_MAX) {
		if (parser.min_thres >= parser.max_thres - 1 && parser.min_thres != LONG_MAX) {
			errorf("wrong threshold usage\n");
			print_help_exit(argv[0]);
		}
	}

	TAILQ_INIT(&parser.keyword_list_tq);

	if (create_keyword_list_list(&parser) != OK) {
		errorf("tailq fail\n");
		goto fail;
	}

	if (extract_data(&parser) != OK) {
		errorf("extraction failed\n");
		goto fail;
	}

	goto out;

fail:
	ret = NOK;

out:
	destroy_keyword_list_list(&parser);
	destroy_data_list(&parser);
	deinit_parser(&parser);

	return ret;
}
