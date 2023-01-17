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
	bool type_is_numeric;
	bool add_line_no;
	bool only_show_changes;
	unsigned int from;
	unsigned int select;
	unsigned int element_at;
	unsigned int line_below;
	unsigned long line_cnt;
	unsigned long total_fount_cnt;
	long max_thres;
	long min_thres;
	char *keyword_list;
	char *file_path;
	char *image_file_path;
	char seperator[2];
	FILE *fp;
	struct tailq keyword_list_tq;
	struct image_data_tailq image_data_tailq;
};

static struct parser parser;

static struct option parameters[] = {
	{ "file",				required_argument,	0,	'o'	},
	{ "help",				no_argument,		0,	'h'	},
	{ "kwl",				required_argument,	0,	'k'	},
	{ "sep",				required_argument,	0,	'p'	},
	{ "ela",				required_argument,	0,	'e'	},
	{ "numeric",			no_argument,		0,	'n'	},
	{ "lb",					required_argument,	0,	'b'	},
	{ "select",				required_argument,	0,	's'	},
	{ "from",				required_argument,	0,	'f'	},
	{ "aln",				no_argument,		0,	'l'	},
	{ "soc",				no_argument,		0,	'c'	},
	{ "max",				required_argument,	0,	'x'	},
	{ "min",				required_argument,	0,	'm'	},
	{ "version",			no_argument,		0,	'v'	},
	{ "graph",				required_argument,	0,	'g'	},
	{ NULL,					0,					0, 	0 		},
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
	debugf("\t--file   \t(-o): log file path\n\t\tused to specify the log file's directory path.\n\t\tThis is MANDATORY field\n\n");
	debugf("\t--kwl    \t(-k): keyword list\n\t\tused to specify special keywords to pick a line. You can use multiple keywords seperated by comma without empty space.\n\t\tThis is MANDATORY field\n\n");
	debugf("\t--sep    \t(-p): seperator\n\t\tused to specify special character to split the picked line.\n\t\tTAB is used if it is not set\n\n");
	debugf("\t--ela    \t(-e): element at\n\t\tused to specify which element you want to extract after splitting the line.\n\t\tIf this is not used, then whole line will be filtered.\n\n");
	debugf("\t--numeric\t(-n): numeric type\n\t\tused to specify the extracted element's type is numeric.\n\t\tThis is usefull when the extracted field contains numeric and alphanumeric characters together\n\t\tNo parameter required\n\n");
	debugf("\t--lb     \t(-b): line below\n\t\tused to select a new line that is number of lines below the picket line before\n\t\tThis is usefull when there is no constant string specifier to pick the line that we want to examine\n\n");
	debugf("\t--aln    \t(-l): add line no\n\t\tused to specify real line no in the log doc in the new generated file\n\t\tNo parameter required\n\n");
	debugf("\t--soc    \t(-c): show only changes\n\t\tused to parameter changes, like \"watch\" property\n\t\tNo parameter required\n\n");
	debugf("\t--max    \t(-x): max threshold\n\t\tused to filter numeric values\n\t\ttype-is-numeric arg is used by default with this filter\n\n");
	debugf("\t--min    \t(-m): min threshold\n\t\tused to filter numeric values\n\t\ttype-is-numeric arg is used by default with this filter\n\n");
	debugf("\t--select \t(-s): select\n\t\tused to pick \"select-th\" line from \"from\" lines\n\t\tfrom arg is must\n\t\tusefull to remove duplicated log lines\n\n");
	debugf("\t--from   \t(-f): from\n\t\tused to pick \"select-th\" line from \"from\" lines\n\t\tselect arg is must\n\t\tusefull to remove duplicated log lines\n\n");
	debugf("\t--version\t(-v): show version\n\n");
	debugf("\t--graph  \t(-g): draw graph\n\t\tcreates graph from the extracted data\n\t\tuseful when to visualize the data\n\t\trequires argument which is file path for newly created image\n\t\ttype-is-numeric arg is used by default with this filter\n\n");

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

static void trim_empty_spaces(char *data, unsigned int max_len)
{
	unsigned int i = 0;
	unsigned int starting_idx = 0;
	unsigned int ending_idx = 0;
	unsigned int len = 0;

	if (!data) {
		return;
	}

	len = strlen(data);
	while(data[i++] == ' ');
	starting_idx = i - 1;

	i = len -1;
	while(data[i--] == ' ');
	ending_idx = i + 1;

	data[ending_idx + 1] = '\0';

	memmove(data, &data[starting_idx], max_len - starting_idx);
}

static void data_mapping_process(struct image_data_tailq *image_data_tailq, long max_val, long *total_data)
{
	long i = 0;
	double dividor = ceil((double)((double)max_val / (double)IMAGE_HEIGHT));
	double multiplier = round((double)((double)IMAGE_HEIGHT / (double)max_val));
	struct image_data_entry *entry = NULL;

	if (!image_data_tailq) {
		errorf("arg is NULL\n");
		return;
	}

	TAILQ_FOREACH(entry, image_data_tailq, entries) {
		if (max_val < IMAGE_HEIGHT) {
			entry->data = (long)(entry->data * multiplier);
		} else {
			entry->data = (long)(entry->data / dividor);
		}

		i++;
	}

	*total_data = i;
}

static int create_image(struct parser *parser, long max_val)
{
	FILE *pgmimg = NULL;
	int i = 0, j = 0;
	long total_data = 0;
	char filepath[ONE_LINE_MAX_LEN] = {0};
	struct image_data_entry *entry = NULL;
	int image_height = IMAGE_HEIGHT;

	if (!parser) {
		errorf("arg is NULL\n");
		return NOK;
	}

	data_mapping_process(&(parser->image_data_tailq), max_val, &total_data);

	if ((int)(total_data * (EMPTY_WIDTH + PIXEL_WIDTH)) > IMAGE_MAX_WIDTH) {
		errorf("data is too long, please shorten it to draw graph\n");
		return OK;
	}

	image_height = (int)((double)image_height * 1.2);

	snprintf(filepath, ONE_LINE_MAX_LEN, "%s/%s", parser->image_file_path, IMAGE_NAME);

	pgmimg = fopen(filepath, "wb");
	if (!pgmimg) {
		errorf("fopen failed\n");
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

static int extract_data(struct parser *parser)
{
	unsigned int element_cnt = 0;
	char buffer[ONE_LINE_MAX_LEN] = {0};
	char command[ONE_LINE_MAX_LEN] = {0};
	char *rest = NULL;
    char *token;
	char temp[ONE_LINE_MAX_LEN] = {0};
	char temp_priv[ONE_LINE_MAX_LEN] = {0};
	long max_value = LONG_MIN;

	memset(temp_priv, 0, sizeof(temp_priv));

	if (!parser) {
		errorf("wrong prm\n");
		return NOK;
	}

	parser->fp = fopen(parser->file_path, "r");
	if (!parser->fp) {
		errorf("fopen failed\n");
		return NOK;
	}

	if (parser->image_file_path) {
		TAILQ_INIT(&(parser->image_data_tailq));
	}

	memset(buffer, 0, sizeof(buffer));
	memset(command, 0, sizeof(command));

	while (fgets(buffer, sizeof(buffer) - 1, parser->fp))
	{
		memset(temp, 0, sizeof(temp));

		++parser->line_cnt;

		buffer[strcspn(buffer, "\n")] = 0;

		if (contains_necessary_keywords(parser, buffer) == OK) {
			element_cnt = 0;

			if (parser->line_below != 0) {
				unsigned int i = 0;

				for (i = 0; i < parser->line_below; i++) {
					if (!fgets(buffer, sizeof(buffer) - 1, parser->fp)) {
						errorf("fget fails\n");
						goto out;
					}
					++parser->line_cnt;
				}
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
				parser->total_fount_cnt++;
			}

			if ((parser->total_fount_cnt - 1) % parser->from == parser->select) {
				if (parser->type_is_numeric) {

					char *ptr = NULL;
					long value = 0;
					unsigned int i = 0;
					unsigned int starting_pos = 0;
					unsigned int ending_pos = 0;

					for (i = 0; i < strlen(token); i++) {
						if (token[i] >= 0x30 && token[i] <= 0x39) {
							starting_pos = i;
							break;
						}
					}

					for (i = strlen(token) - 1; i > 0; i--) {
						if (token[i] >= 0x30 && token[i] <= 0x39) {
							ending_pos = i;
							break;
						}
					}
					if (!starting_pos && !ending_pos) {
						continue;
					}

					if (starting_pos <= strlen(token) - 1 && ending_pos >= starting_pos) {
						token[ending_pos + 1] = '\0';
						value = strtol(&token[starting_pos], &ptr, 10);

						if (ptr && strlen(ptr) > 0) {
							continue;
						}
					}

					snprintf(temp, sizeof(temp), "%ld", value);

					if (value > max_value) {
						max_value = value;
					}

					if (parser->image_file_path) {
						if (insert_data_to_data_queue(value, &(parser->image_data_tailq)) == NOK) {
							errorf("insert failed\n");
						}
					}

					if (parser->min_thres != LONG_MAX && value < parser->min_thres) {
						continue;
					}
					if (parser->max_thres != LONG_MAX && value > parser->max_thres) {
						continue;
					}
				} else {
					snprintf(temp, sizeof(temp), "%s", token);
				}

				if (temp[strlen(temp) - 1] == '\n') {
					temp[strlen(temp) - 1] = '\0';
				}

				if (parser->only_show_changes && !strcmp(temp, temp_priv)) {
					continue;
				}

				snprintf(temp_priv, sizeof(temp_priv), "%s", temp);

				trim_empty_spaces(temp, sizeof(temp));

				if (parser->add_line_no) {
					debugf("%s\t%ld\n", temp, parser->line_cnt);
				} else {
					debugf("%s\n", temp);
				}
			}
		}
	}

out:
	FCLOSE(parser->fp);

	errorf("\n**********************************************************\n");
	errorf("TOTAL PROGRESSED LINES: %ld\n", parser->line_cnt);
	errorf("TOTAL EXTRACTED LINES: %ld\n", parser->total_fount_cnt);
	errorf("TOTAL SHOWED LINES: %ld\n", (parser->total_fount_cnt / parser->from));
	errorf("\n**********************************************************\n");

	if (parser->image_file_path) {
		if (create_image(parser, max_value) == NOK) {
			errorf("create_image() failed\n");
		}
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
	parser->type_is_numeric = false;
	parser->add_line_no = false;
	parser->only_show_changes = false;
	parser->select = 0;
	parser->from = 1;
	parser->line_below = 0;
	memset(parser->seperator, 0, sizeof(parser->seperator));
	parser->seperator[0] = '\t';
	parser->element_at = UINT_MAX;
	parser->min_thres = LONG_MAX;
	parser->max_thres = LONG_MAX;
}

static void deinit_parser(struct parser *parser)
{
	if (!parser) {
		errorf("arg is null\n");
		return;
	}

	FREE(parser->file_path);
	FREE(parser->keyword_list);
	FREE(parser->image_file_path);

	FCLOSE(parser->fp);
}

static void sigint_handler(__attribute__((unused)) int sig_num)
{
	destroy_keyword_list_list(&parser);
	destroy_data_list(&parser);
	deinit_parser(&parser);

	errorf("\n**********************************************************\n");
	errorf("TOTAL PROGRESSED LINES: %ld\n", parser.line_cnt);
	errorf("TOTAL EXTRACTED LINES: %ld\n", parser.total_fount_cnt);
	errorf("TOTAL SHOWED LINES: %ld\n", (parser.total_fount_cnt / parser.from));
	errorf("\n**********************************************************\n");

	exit(NOK);
}

int main(int argc, char **argv)
{
	int ret = OK;
	int c;
	int o;
	char *ptr = NULL;

	signal(SIGINT, sigint_handler);

	init_parser(&parser);

	while ((c = getopt_long(argc, argv, "h", parameters, &o)) != -1) {
		switch (c) {
			case 'o':
				parser.file_path = strdup(optarg);
				if (!parser.file_path) {
					errorf("strdup failed\n");
					return NOK;
				}

				if (!parser.file_path || !strlen(parser.file_path)) {
					errorf("file_path seems wrong\n");
					return NOK;
				}
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
			case 'p':
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
			case 'n':
				parser.type_is_numeric = true;
				break;
			case 'l':
				parser.add_line_no = true;
				break;
			case 'c':
				parser.only_show_changes = true;
				break;
			case 's':
				parser.select = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					errorf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 'f':
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
				parser.image_file_path = strdup(optarg);
				if (!parser.image_file_path) {
					errorf("strdup failed\n");
					return NOK;
				}

				if (!parser.image_file_path || !strlen(parser.image_file_path)) {
					errorf("image_file_path seems wrong\n");
					return NOK;
				}
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
		parser.type_is_numeric = true;
	}

	if (parser.image_file_path) {
		parser.type_is_numeric = true;
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
