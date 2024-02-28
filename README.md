<p align="center"> 
  <img src="pictures/egrep.png" width="300px" height="200px">
</p>
<h1 align="center"> Extended Grep </h1>
<h3 align="center"> A Tool to Extract and Visualize Data From a Log File </h3>  

</br>

<!-- TABLE OF CONTENTS -->
<h2 id="table-of-contents"> Table of Contents</h2>

<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#about-the-project"> ➤ About The Project</a></li>
    <li><a href="#prerequisites"> ➤ Prerequisites</a></li>
    <li><a href="#folder-structure"> ➤ Folder Structure</a></li>
    <li><a href="#arguments-details"> ➤ Arguments Details</a></li>
    <li><a href="#how-to-use"> ➤ How to Use</a></li>
    <li><a href="#license"> ➤ License</a></li>
  </ol>
</details>

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

<!-- ABOUT THE PROJECT -->
<h2 id="about-the-project"> About The Project</h2>

<p align="justify"> 
  This project's aim is to minimize the spent time to analyze a big and complex log files and visualize numeric type data fastly.
  As a developer, I generally spent my time to analyze logs gathered from the field. Some field problems are hard to reproduce. So, logging operations could take days sometimes. More logging day means big and complex log files. I dealt with 6GB txt file once before and my editor could not open it and crashed.

  So, I decided to write this application. Because with this application, I can do the followings;

  * extracting all the lines that includes special keywords given as argument
  * extracting all the words/data in a specified line, even the numeric data is hidden in a string
  * extracing data from a table. We are doing that with "--linebelow" argument because table content is changing and we cannot specify a keyword to point out the specific line
  * seeing the line number of a line thatwe interested
  * for numeric values, we can filter the data with maximum/minimum threshold
  * we can filter the data to see only when it is changed
  * graphs can be drawn for numeric data to see the trends fast
</p>

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

<!-- PREREQUISITES -->
<h2 id="prerequisites"> Prerequisites</h2>

![Python](https://img.shields.io/pypi/pyversions/3) <br>
![Ubuntu](https://img.shields.io/badge/Ubuntu-E95420?style=for-the-badge&logo=ubuntu&logoColor=white) <br>

No third party packets used in this project.

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

<!-- FOLDER STRUCTURE -->
<h2 id="folder-structure"> Folder Structure</h2>

    code
    .
    │
    ├── pictures
    │   ├── egrep.png
    │   ├── created_graph_sample.png
    │
    ├── LICENSE
    ├── egrep.py  
    ├── README.md
    ├── test.txt

* pictures folder: contains pictures used in the README.md file.
* LICENSE file: contains license information
* egrep.py file: is the source code
* README.md file: readme itselt
* test.txt file: a simple and sample log file to test the app quickly  

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

<!-- ARGUMENTS -->
<h2 id="arguments-details"> Arguments' Details</h2>
<p>

    usage: ProgramName [-h] [-v] [-d] [-f FILENAME] [-s STARTLINE] [-e ENDLINE] [-l] [-k KEYWORDS] [-t] [-b LINEBELOW] [-p SEPERATOR] [-m ELEMENTAT] [-i] [-a BASE] [-x MAXTHRESHOLD] [-n MINTHRESHOLD] [-c] [-w]
    
    What the program does
    
    optional arguments:
      -h, --help            show this help message and exit
      -v, --version         show version number and return
      -d, --verbose         verbosity setting
      -f FILENAME, --filename FILENAME
                            direct path to the log file
      -s STARTLINE, --startline STARTLINE
                            starting line to run the query while line numbering starts from 1 and the input numbered line will be included
      -e ENDLINE, --endline ENDLINE
                            ending line to run the query while the input numbered line will be included
      -l, --showlineno      show line numbers
      -k KEYWORDS, --keywords KEYWORDS
                            direct path to the log file
      -t, --notkeysensitive
                            are include keywords key sensitive
      -b LINEBELOW, --linebelow LINEBELOW
                            select the number of lines below of the keywords containing line
      -p SEPERATOR, --seperator SEPERATOR
                            selected line will be splitted into words around this character, default is SPACE
      -m ELEMENTAT, --elementat ELEMENTAT
                            the number of i'th element in the line after splitting with respect to seperator, starting from 0
      -i, --typeint         treat the parsed word is int, assumed string by default
      -a BASE, --base BASE  base of the found word, eg: 16, default is 10
      -x MAXTHRESHOLD, --maxthreshold MAXTHRESHOLD
                            maximum threshold to be shown
      -n MINTHRESHOLD, --minthreshold MINTHRESHOLD
                            minimum threshold to be shown
      -c, --showifchanged   shows the result only it is different than the previous
      -w, --drawgraph       draw graph and save it to the current directory with name 'output.png'

Text at the bottom of help



![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

<!-- HOWTO -->
<h2 id="how-to-use"> How to Use</h2>

1. Install an additional library for graph drawing
> pip install matplotlib
> pip install pytest
2. Then, you may use the executable on "test.txt" sample file like below
> python3 egrep.py -f test.txt --keywords RX,errors --notkeysensitive --elementat 2 --typeint --drawgraph

The above command's menaning is;
* use "test.txt" as the input log file
* first, extract the lines includes "RX and errors" keywords. (PS. more keywords canbe used here by seperating them with comma)
* keywords are not key sensitive
* after seperation, second word (starting from index 0) is the data that we interested
* since we know it is a numeric type, use "typeint" to draw fraph
* and "drawgraph" is used to draw graph

Here is the output of the execution;

    1
    2
    3
    4
    5
    6
    7
    8
    9
    10
    11
    12
    13
    14
    15
    16
    17
    18
    19
    20
    21
    22
    23
    24
    25
    26
    27
    28
    29
    30
    40
    50
    60
    70
    80
    90
    80
    70
    60
    50
    40
    30
    10
    20
    10
    10
    10
    10
    10
    10
    10
    20
    10
    50
    20
    20
    20
    20
    20
    100
    ############## SUMMARY ############
        total processed line    516
        total found line        60

Note that, we can use ">>" to save the output to a file.

Here is the output graph view;

<p align="left"> 
  <img src="pictures/created_graph_sample.png" width="140px" height="100px">
</p>

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

<!-- LIMITS -->
<h2 id="limitations"> Limitations</h2>

* PIXEL_WIDTH:      Used to determine one graph column's width
* EMPTY_WIDTH:        Used to determine empty pixels between each graph column
* IMAGE_HEIGHT:     Used to limit image height
* IMAGE_MAX_WIDTH:  Used to limit image width
* ONE_LINE_MAX_LEN: Used to limit one line lenght in the log file

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

<!-- LICENSE -->
<h2 id="license"> License</h2>

<h3 align="left"> This project is completely FREE </h3>
