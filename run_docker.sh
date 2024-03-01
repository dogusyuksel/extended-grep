#!/bin/bash

if [ "$#" -gt 0 ]; then
    argc=$#
    argv=("$@")
    full_string=""
    priv_string=""

    for (( j=0; j<argc; j++ )); do
        full_string="$priv_string${argv[j]} "
        priv_string=$full_string
    done
    # echo "->$full_string<-"
    docker run --rm -t --net=host -v $(pwd):/workspace --entrypoint=/bin/bash project_docker:master -c "$full_string"
else
    docker run --rm -it --net=host -v $(pwd):/workspace --entrypoint=/bin/bash project_docker:master
fi

