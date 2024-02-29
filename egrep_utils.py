class Utils:
    def __init__(self, args):
        self.args = args
        self.last_found = ""

    def debug_log(self, message):
        if self.args.verbose is True:
            print(message)

    def arg_check(self):
        if self.args.filename is None:
            print('filename cannot be None')
            return False

        if self.args.keywords is None:
            print('keywords cannot be None')
            return False

        return True

    def check_int_specific_args(self, words, elementat, line_number, total_found_line, height):
        ret_val = {}
        ret_val["height"] = height
        ret_val["total_found_line"] = total_found_line

        should_be_shown = True
        base = 10
        if self.args.base is not None:
            base = self.args.base
        value = int(words[elementat], base)
        if self.args.maxthreshold is not None:
            if value > self.args.maxthreshold:
                should_be_shown = False
        if self.args.minthreshold is not None:
            if value < self.args.minthreshold:
                should_be_shown = False
        if self.args.showifchanged is True:
            if self.last_found == words[elementat]:
                should_be_shown = False

        self.last_found = words[elementat]

        if should_be_shown is True:
            ret_val["total_found_line"] = ret_val["total_found_line"] + 1
            if self.args.drawgraph:
                ret_val["height"].append(value)

            if self.args.showlineno is True:
                print(str(line_number) + ' : ' + str(value))
            else:
                print(str(value))

        return ret_val

    def parse_line(self, line, line_number, total_found_line, height):
        ret_val = {}
        ret_val["height"] = height
        ret_val["total_found_line"] = total_found_line

        seperator = " "
        if self.args.seperator is not None:
            seperator = self.args.seperator

        words = line.split(seperator)
        if self.args.elementat is not None:
            elementat = self.args.elementat
            for word in words:
                if word == seperator or word.strip() == seperator or len(word.strip()) == 0:
                    words.remove(word)
                    continue

            if self.args.typeint is False:
                ret_val["total_found_line"] = ret_val["total_found_line"] + 1
                if self.args.showlineno is True:
                    print(str(line_number) + ' : ' + words[elementat])
                else:
                    print(words[elementat])
            else:
                ret_val = self.check_int_specific_args(words, elementat, line_number,
                                                       ret_val["total_found_line"], ret_val["height"])

        else:
            ret_val["total_found_line"] = ret_val["total_found_line"] + 1
            if self.args.showlineno is True:
                print(str(line_number) + ' : ' + line)
            else:
                print(line)

        return ret_val
