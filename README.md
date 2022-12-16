# Parameter Details #

**egrep** application is used to parse big and complex log files.

It extracts necessary data for you to observe changes on them affectively.

## parameters ##
> **--file-path**: used to specify the log file's directory path.  
		This is **MANDATORY** field 

> **--keyword-list**: used to specify special keywords to pick a line. You can use multiple keywords seperated by comma without empty space.  
		This is **MANDATORY** field

> **--seperator**: used to specify special character to split the picked line.  
		TAB is used if it is not set

> **--element-at**: used to specify which element you want to extract after splitting the line.  
		This is **MANDATORY** field

> **--type-is-numeric**: used to specify the extracted element's type is numeric.  
		This is usefull when the extracted field contains numeric and alphanumeric characters together  
		No parameter required

> **--line-below**: used to select a new line that is number of lines below the picket line before  
		This is usefull when there is no constant string specifier to pick the line thatwe want to examine

> **--add-line-no**: used to specify real line no in the log doc in the new generated file  
		No parameter required

> **--only-show-changes**: used to parameter changes, like "watch" property  
		No parameter required

> **--max-threshold**: used to filter numeric values  
		type-is-numeric arg is used by default with this filter

> **--min-threshold**: used to filter numeric values  
		type-is-numeric arg is used by default with this filter

> **--select**: used to pick "select-th" line from "from" lines  
		from arg is must  
		usefull to remove duplicated log lines

> **--from**: used to pick "select-th" line from "from" lines  
		select arg is must  
		usefull to remove duplicated log lines

> **--sub-priv**: used to subtract previous data to the new one  
		useful when to show instant data ripples  
		ntype-is-numeric arg is used by default with this option


## Usage ##

A 'test.log' file is added to this repo. It includes 'ifconfig eth0' command's multiple output.
With the following command, we can filter out TX packet bytes.

> ./egrep --file-path ./test.log --keyword-list TX,packets,bytes --seperator " " --element-at 5

And the output will be as follows;

> 1184768451  
1184862475  
1184889445  
**********************************************************  
TOTAL PROGRESSED LINES: 30
TOTAL EXTRACTED LINES: 3  
TOTAL SHOWED LINES: 3  
**********************************************************


**Note** '>>' syntax can be used to save the outut to a file.
