import argparse
import sys

import matplotlib.pyplot as plt

import egrep_utils

VERSION = "0.0.1"

parser = argparse.ArgumentParser(
    prog='egrep',
    description='an extended version of \'grep\'',
    epilog='egrep Help')
parser.add_argument('-v', '--version',
                    action='store_true',
                    help='show version number and return')
parser.add_argument('-d', '--verbose',
                    action='store_true',
                    help='verbosity setting')
parser.add_argument('-f', '--filename',
                    action='store',
                    help='direct path to the log file')
# FINDING LINE
parser.add_argument('-s', '--startline',
                    action='store',
                    help='starting line to run the query while line numbering '
                    'starts from 1 and the input numbered line will be included',
                    type=int)
parser.add_argument('-e', '--endline',
                    action='store',
                    help='ending line to run the query while the input numbered '
                    'line will be included',
                    type=int)
parser.add_argument('-l', '--showlineno',
                    action='store_true',
                    help='show line numbers')
parser.add_argument('-k', '--keywords',
                    action='store',
                    help='direct path to the log file')
parser.add_argument('-t', '--notkeysensitive',
                    action='store_true',
                    help='are include keywords key sensitive')
parser.add_argument('-b', '--linebelow',
                    action='store',
                    help='select the number of lines below of the keywords containing line',
                    type=int)
# FINDING LINE ENDS
# FINDING WORDS
parser.add_argument('-p', '--seperator',
                    action='store',
                    help='selected line will be splitted into words around this character, '
                    'default is SPACE')
parser.add_argument('-m', '--elementat',
                    action='store',
                    help='the number of i\'th element in the line after splitting with respect to seperator, '
                    'starting from 0',
                    type=int)
parser.add_argument('-i', '--typeint',
                    action='store_true',
                    help='treat the parsed word is int, assumed string by default')
# FINDING WORDS ENDS
# TREAT AS INT
parser.add_argument('-a', '--base',
                    action='store',
                    help='base of the found word, eg: 16, default is 10',
                    type=int)
parser.add_argument('-x', '--maxthreshold',
                    action='store',
                    help='maximum threshold to be shown',
                    type=int)
parser.add_argument('-n', '--minthreshold',
                    action='store',
                    help='minimum threshold to be shown',
                    type=int)
parser.add_argument('-c', '--showifchanged',
                    action='store_true',
                    help='shows the result only it is different than the previous')
parser.add_argument('-w', '--drawgraph',
                    action='store_true',
                    help='draw graph and save it to the current directory with name \'output.png\'')
# TREAT AS INT ENDS
args = parser.parse_args()

egu = egrep_utils.Utils(args)

if args.version is True:
    print('version', VERSION)
    sys.exit(0)

if egu.arg_check() is False:
    sys.exit(1)

egu.debug_log('filename: ' + args.filename)
egu.debug_log('keywords: ' + args.keywords)

final_line_no = 0
line_number = 0
line_below = 0
x_coord = []
global_return = {}

global_return["total_found_line"] = 0
global_return["height"] = []

if args.linebelow is not None:
    line_below = args.linebelow

# Opening file
try:
    with open(args.filename, 'r') as log_file_fp:
        # Using for loop
        for line in log_file_fp:
            line_number += 1

            if args.endline is not None and line_number > args.endline:
                break
            if args.startline is not None and line_number < args.startline:
                continue

            is_line_picked = True
            keywords = (args.keywords).split(",")

            for keyword in keywords:
                lcl_keyword = keyword
                lcl_line = line.strip()
                if args.notkeysensitive is True:
                    lcl_keyword = lcl_keyword.lower()
                    lcl_line = lcl_line.lower()
                if lcl_keyword not in lcl_line:
                    is_line_picked = False
                    break

            if is_line_picked is True:
                final_line_no = line_number + line_below

            if final_line_no == line_number:
                global_return = egu.parse_line(
                    line.strip(), line_number,
                    global_return["total_found_line"], global_return["height"])

        # Closing files
        log_file_fp.close()
except IOError:
    print("IOError error occured while opening the file '", args.filename, "'")

if args.drawgraph:
    just_number = 0
    for h in global_return["height"]:
        just_number = just_number + 1
        x_coord.append(just_number)

    # plotting a bar chart
    plt.bar(x_coord, global_return["height"], color=['grey'])

    plt.xlabel('x - axis')
    plt.ylabel('y - axis')
    plt.title('output')

    plt.savefig('output.png')

print("############## SUMMARY ############")
print("    total processed line   ", line_number)
print("    total found line       ", global_return["total_found_line"])
