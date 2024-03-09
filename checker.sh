#!/bin/bash

function usage {
    echo "Usage: $0 <project> [--lex | --syn | --sem | --tac]"
}

if [ "$1" == "--help" ]; then
    usage
    exit 0
fi

PROJECT=$1
if [ -z "$PROJECT" ]; then
    usage
    exit 1
fi

ARGS=${@:2}

docker build -t "cool-compiler-$PROJECT" ./$PROJECT \
    && docker run -v $(pwd)/checker/tests:/app/tests \
                  -v $(pwd)/checker/checker.sh:/app/checker.sh \
                  "cool-compiler-$PROJECT" sh /app/checker.sh $ARGS
