#!/bin/bash

PROJECT=$1
if [ -z "$PROJECT" ]; then
    echo "Usage: $0 <project> [args]"
    exit 1
fi

ARGS=${@:2}

docker build -t "cool-compiler-$PROJECT" ./$PROJECT \
    && docker run -v $(pwd)/checker/tests:/app/tests \
                  -v $(pwd)/checker/checker.sh:/app/checker.sh \
                  "cool-compiler-$PROJECT" sh /app/checker.sh $ARGS
