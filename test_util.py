import argparse

import egrep_utils


def test_arg_check_filename():
    parser = argparse.ArgumentParser(
        prog='test',
        description='test desc',
        epilog='test epilog')
    parser.add_argument('-f', '--filename',
                        action='store',
                        help='direct path to the log file')
    parser.add_argument('-k', '--keywords',
                        action='store',
                        help='direct path to the log file')
    args = parser.parse_args(['--keywords', 'asd,asd'])
    egu = egrep_utils.Utils(args)

    assert egu.arg_check() is False


def test_arg_check_keywords():
    parser = argparse.ArgumentParser(
        prog='test',
        description='test desc',
        epilog='test epilog')
    parser.add_argument('-f', '--filename',
                        action='store',
                        help='direct path to the log file')
    parser.add_argument('-k', '--keywords',
                        action='store',
                        help='direct path to the log file')
    args = parser.parse_args(['--filename', 'asd'])
    egu = egrep_utils.Utils(args)

    assert egu.arg_check() is False


def test_int_specific_arg_check():
    total_found_line = 1
    ret_val = {}
    ret_val["height"] = {}
    ret_val["total_found_line"] = total_found_line

    parser = argparse.ArgumentParser(
        prog='test',
        description='test desc',
        epilog='test epilog')
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
    parser.add_argument('-l', '--showlineno',
                        action='store_true',
                        help='show line numbers')

    args = parser.parse_args()
    egu = egrep_utils.Utils(args)

    line = "alpha beta 3"
    seperator = " "
    elementat = 2
    line_number = 1

    words = line.split(seperator)

    ret_val = egu.check_int_specific_args(words, elementat, line_number,
                                          ret_val["total_found_line"], ret_val["height"])

    assert ret_val["total_found_line"] == (total_found_line + 1)
