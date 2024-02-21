#!/bin/bash

lexical_analyzer() {
    echo "Testing the lexical analyzer"

    passed=0
    for file_path in $(ls tests/lexer/*.cl); do
        ref_path=./tests/lexer/$(basename $file_path .cl).ref

        file_name=$(basename $file_path)
        echo -en "Testing $file_name ... "

        make run -si ARGS="--lex $file_path" | diff - $ref_path > /dev/null 2>&1

        if [ $? -eq 0 ]; then
            echo -e "\e[32mPASSED\e[0m"
            passed=$((passed + 1))
        else
            echo -e "\e[31mFAILED\e[0m"
        fi
    done

    total=$(ls ./tests/lexer/*.cl | wc -l)
    echo "Passed $passed/$total tests"
}

make clean && make

ARG1=$1
if [ "$ARG1" == "--lex" ]; then
    lexical_analyzer
elif [ -z "$ARG1" ]; then
    lexical_analyzer
else
    echo "Invalid argument"
fi
