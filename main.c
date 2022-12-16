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

#include "main.h"

struct entry
{
	char *word;
	TAILQ_ENTRY(entry) entries;
};
TAILQ_HEAD(tailq, entry);

struct parser
{
	bool type_is_numeric;
	bool add_line_no;
	bool only_show_changes;
	bool sub_priv;
	unsigned int from;
	unsigned int select;
	unsigned int element_at;
	unsigned int line_below;
	unsigned int line_cnt;
	unsigned int total_fount_cnt;
	long max_thres;
	long min_thres;
	char *keyword_list;
	char *file_path;
	char seperator[2];
	FILE *fp;
	struct tailq keyword_list_tq;
};

static struct parser parser;

static struct option parameters[] = {
	{ "file-path",			required_argument,	0,	0x0FF	},
	{ "help",				no_argument,		0,	0x100	},
	{ "keyword-list",		required_argument,	0,	0x101	},
	{ "seperator",			required_argument,	0,	0x102	},
	{ "element-at",			required_argument,	0,	0x103	},
	{ "type-is-numeric",	no_argument,		0,	0x104	},
	{ "line-below",			required_argument,	0,	0x106	},
	{ "select",				required_argument,	0,	0x10A	},
	{ "from",				required_argument,	0,	0x10B	},
	{ "add-line-no",		no_argument,		0,	0x107	},
	{ "only-show-changes",	no_argument,		0,	0x108	},
	{ "max-threshold",		required_argument,	0,	0x109	},
	{ "min-threshold",		required_argument,	0,	0x10C	},
	{ "sub-priv",			no_argument,		0,	0x10E	},
	{ "version",			no_argument,		0,	0x10F	},
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

	debugf("\nparameters;\n");
	debugf("\t--file-path: used to specify the log file's directory path.\n\t\tThis is MANDATORY field\n");
	debugf("\t--keyword-list: used to specify special keywords to pick a line. You can use multiple keywords seperated by comma without empty space.\n\t\tThis is MANDATORY field\n");
	debugf("\t--seperator: used to specify special character to split the picked line.\n\t\tTAB is used if it is not set\n");
	debugf("\t--element-at: used to specify which element you want to extract after splitting the line.\n\t\tThis is MANDATORY field\n");
	debugf("\t--type-is-numeric: used to specify the extracted element's type is numeric.\n\t\tThis is usefull when the extracted field contains numeric and alphanumeric characters together\n\t\tNo parameter required\n");
	debugf("\t--line-below: used to select a new line that is number of lines below the picket line before\n\t\tThis is usefull when there is no constant string specifier to pick the line thatwe want to examine\n");
	debugf("\t--add-line-no: used to specify real line no in the log doc in the new generated file\n\t\tNo parameter required\n");
	debugf("\t--only-show-changes: used to parameter changes, like \"watch\" property\n\t\tNo parameter required\n");
	debugf("\t--max-threshold: used to filter numeric values\n\t\ttype-is-numeric arg is used by default with this filter\n");
	debugf("\t--min-threshold: used to filter numeric values\n\t\ttype-is-numeric arg is used by default with this filter\n");
	debugf("\t--select: used to pick \"select-th\" line from \"from\" lines\n\t\tfrom arg is must\n\t\tusefull to remove duplicated log lines\n");
	debugf("\t--from: used to pick \"select-th\" line from \"from\" lines\n\t\tselect arg is must\n\t\tusefull to remove duplicated log lines\n");
	debugf("\t--sub-priv: used to subtract previous data to the new one\n\t\tuseful when to show instant data ripples\n\t\tntype-is-numeric arg is used by default with this option\n");

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
		errorf("[%d]: wrong prm\n", __LINE__);
		return NOK;
	}

	for (token = strtok_r(parser->keyword_list, delim, &rest);
		token != NULL;
		token = strtok_r(NULL, delim, &rest)) {
	
		entry = calloc(1, sizeof(struct entry));
		if (!entry) {
			errorf("[%d]: calloc failed\n", __LINE__);
			return NOK;
		}

		entry->word = strdup(token);
		if (!entry->word) {
			errorf("[%d]: strdup failed\n", __LINE__);
			return NOK;
		}

		TAILQ_INSERT_TAIL(&parser->keyword_list_tq, entry, entries);
	}

	return OK;
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
		errorf("[%d]: wrong prm\n", __LINE__);
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

static int extract_data(struct parser *parser)
{
	unsigned int element_cnt = 0;
	char buffer[ONE_LINE_MAX_LEN] = {0};
	char command[ONE_LINE_MAX_LEN] = {0};
	char *rest = NULL;
    char *token;
	char temp_priv[BUFF_SIZE] = {0};
	long value_priv = 0;

	memset(temp_priv, 0, sizeof(temp_priv));

	if (!parser) {
		errorf("[%d]: wrong prm\n", __LINE__);
		return NOK;
	}

	parser->fp = fopen(parser->file_path, "r");
	if (!parser->fp) {
		errorf("[%d]: fopen failed\n", __LINE__);
		return NOK;
	}

	memset(buffer, 0, sizeof(buffer));
	memset(command, 0, sizeof(command));

	while (fgets(buffer, sizeof(buffer) - 1, parser->fp))
	{
		char temp[BUFF_SIZE] = {0};

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
					long value_temp = 0;
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

					if (starting_pos < strlen(token) - 1 && ending_pos > starting_pos) {
						token[ending_pos + 1] = '\0';

						value = strtol(&token[starting_pos], &ptr, 10);

						if (ptr && strlen(ptr) > 0) {
							continue;
						}
					}

					value_temp = value;

					if (parser->sub_priv) {
						value = value - value_priv;
					}

					value_priv = value_temp;

					snprintf(temp, sizeof(temp), "%ld", value);

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
					debugf("%s\t%d\n", temp, parser->line_cnt);
				} else {
					debugf("%s\n", temp);
				}
			}
		}
	}

out:
	FCLOSE(parser->fp);

	errorf("\n**********************************************************\n");
	errorf("TOTAL PROGRESSED LINES: %d\n", parser->line_cnt);
	errorf("TOTAL EXTRACTED LINES: %d\n", parser->total_fount_cnt);
	errorf("TOTAL SHOWED LINES: %d\n", (parser->total_fount_cnt / parser->from));
	errorf("\n**********************************************************\n");

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
	parser->sub_priv = false;
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

	FCLOSE(parser->fp);
}

static void sigint_handler(__attribute__((unused)) int sig_num)
{
	destroy_keyword_list_list(&parser);
	deinit_parser(&parser);

	errorf("\n**********************************************************\n");
	errorf("TOTAL PROGRESSED LINES: %d\n", parser.line_cnt);
	errorf("TOTAL EXTRACTED LINES: %d\n", parser.total_fount_cnt);
	errorf("TOTAL SHOWED LINES: %d\n", (parser.total_fount_cnt / parser.from));
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
			case 0x0FF:
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
			case 0x100:
				print_help_exit(argv[0]);
				break;
			case 0x101:
				parser.keyword_list = strdup(optarg);
				if (!parser.keyword_list) {
					errorf("strdup failed\n");
					return NOK;
				}
				break;
			case 0x102:
				if (optarg && strlen(optarg) > 0) {
					parser.seperator[0] = optarg[0];
					parser.seperator[1] = '\0';
				} else {
					errorf("arg is wrong\n");
					return NOK;
				}
				break;
			case 0x103:
				parser.element_at = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					debugf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 0x104:
				parser.type_is_numeric = true;
				break;
			case 0x107:
				parser.add_line_no = true;
				break;
			case 0x108:
				parser.only_show_changes = true;
				break;
			case 0x10A:
				parser.select = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					debugf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 0x10B:
				parser.from = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					debugf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 0x106:
				parser.line_below = strtoul(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					debugf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 0x109:
				parser.max_thres = strtol(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					debugf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 0x10C:
				parser.min_thres = strtol(optarg, &ptr, 10);

				if (ptr && strlen(ptr) > 0) {
					debugf("unknown arg content: %s\n", ptr);
					return NOK;
				}
				break;
			case 0x10E:
				parser.sub_priv = true;
				break;
			case 0x10F:
				debugf("\n%s - ver: %s\n", argv[0], VERSION);
				return OK;
			default:
				debugf("unknown argument\n");
				return NOK;
		}
	}

	if (!parser.file_path) {
		errorf("file-path is mandatory\n");
		print_help_exit(argv[0]);
	}

	if (!parser.keyword_list) {
		errorf("keyword_list field is mandatory\n");
		print_help_exit(argv[0]);
	}

	if (parser.element_at == UINT_MAX) {
		errorf("element-at mandatory\n");
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

	if (parser.sub_priv) {
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
	deinit_parser(&parser);

	return ret;
}
