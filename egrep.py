import sys
import argparse
import matplotlib.pyplot as plt

VERSION="0.0.1"

parser = argparse.ArgumentParser(
                    prog='ProgramName',
                    description='What the program does',
                    epilog='Text at the bottom of help')

parser.add_argument('-v', '--version',
                    action='store_true',
                    help='show version number and return')
parser.add_argument('-d', '--verbose',
                    action='store_true',
                    help='verbosity setting')
parser.add_argument('-f', '--filename',
                    action='store',
                    help='direct path to the log file')
################################################## FINDING LINE ##########################################################
parser.add_argument('-s', '--startline',
                    action='store',
                    help='starting line to run the query while line numbering ' \
                        'starts from 1 and the input numbered line will be included',
                    type=int)
parser.add_argument('-e', '--endline',
                    action='store',
                    help='ending line to run the query while the input numbered ' \
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
################################################## FINDING LINE ENDS ##########################################################
################################################## FINDING WORDS ##########################################################
parser.add_argument('-p', '--seperator',
                    action='store',
                    help='selected line will be splitted into words around this character, default is SPACE')
parser.add_argument('-m', '--elementat',
                    action='store',
                    help='the number of i\'th element in the line after splitting with respect to seperator, starting from 0',
                    type=int)
parser.add_argument('-i', '--typeint',
                    action='store_true',
                    help='treat the parsed word is int, assumed string by default')
################################################## FINDING WORDS ENDS ##########################################################
################################################## TREAT AS INT ##########################################################
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
################################################## TREAT AS INT ENDS ##########################################################
# use action="store_true" for flags
# type=int for max-min threshold

args = parser.parse_args()

def debug_log(message):
    if args.verbose == True:
        print(message)

if args.version == True:
    print('version', VERSION)
    sys.exit(0)

if args.filename == None:
    print('filename cannot be None')
    sys.exit(1)

if args.keywords == None:
    print('keywords cannot be None')
    sys.exit(1)

debug_log('filename: ' + args.filename)
debug_log('keywords: ' + args.keywords)

line_number = 0
total_found_line = 0
line_below = 0
final_line_no = 0
last_found = 0
x_coord = []
height = []

if args.linebelow != None:
    line_below = args.linebelow

 
# Opening file
try:
  with open(args.filename, 'r') as log_file_fp:
    
    # Using for loop
    for line in log_file_fp:
        line_number += 1

        if args.endline != None and line_number > args.endline:
            break
        if  args.startline != None and line_number < args.startline:
            continue

        is_line_picked = True
        keywords = (args.keywords).split(",")

        for keyword in keywords:
            lcl_keyword = keyword
            lcl_line = line.strip()
            if args.notkeysensitive == True:
                lcl_keyword = lcl_keyword.lower()
                lcl_line = lcl_line.lower()
            if lcl_keyword not in lcl_line:
                is_line_picked = False
                break
        

        if is_line_picked == True:
            final_line_no = line_number + line_below

        if final_line_no == line_number:
            # here, start to parse the line
            seperator = " "
            if args.seperator != None:
                seperator = args.seperator

            words = (line.strip()).split(seperator)
            if args.elementat != None:
                elementat = args.elementat
                for word in words:
                    if word == seperator or word.strip() == seperator or len(word.strip()) == 0:
                        words.remove(word)
                        continue

                if args.typeint == False:
                    total_found_line = total_found_line + 1
                    if args.showlineno == True:
                        print(str(line_number) + ' : ' + words[elementat])
                    else:
                        print(words[elementat])
                else:
                    # treat it is "int"
                    should_be_shown = True
                    base = 10
                    if args.base != None:
                        base = args.base
                    value = int(words[elementat], base)
                    if args.maxthreshold != None:
                        if value > args.maxthreshold:
                            should_be_shown = False
                    if args.minthreshold != None:
                        if value < args.minthreshold:
                            should_be_shown = False
                    if args.showifchanged == True:
                        if last_found == words[elementat]:
                            should_be_shown = False
                    
                    last_found = words[elementat]

                    if should_be_shown == True:
                        total_found_line = total_found_line + 1
                        if args.drawgraph:
                            height.append(value)

                        if args.showlineno == True:
                            print(str(line_number) + ' : ' + str(value))
                        else:
                            print(str(value))

            else:
                total_found_line = total_found_line + 1
                if args.showlineno == True:
                    print(str(line_number) + ' : ' + line.strip())
                else:
                    print(line.strip())

    # Closing files
    log_file_fp.close()
except IOError:
    print("IOError error occured while opening the file '", args.filename, "'")

if args.drawgraph:
    just_number = 0
    for h in height:
        just_number = just_number + 1
        x_coord.append(just_number)
    
    # plotting a bar chart
    plt.bar(x_coord, height, color = ['grey'])
    
    plt.xlabel('x - axis')
    plt.ylabel('y - axis')
    plt.title('output')
    
    plt.savefig('output.png')

print("############## SUMMARY ############")
print("    total processed line   ", line_number)
print("    total found line       ", total_found_line)
