#!/bin/bash

TESTS_DIR=tests

analyzer() {
    if [ "$#" -ne 2 ]; then
        echo "Usage: $0 <tests_dir> <exec_arg>"
        exit 1
    fi

    tests_dir=$TESTS_DIR/$1
    exec_arg=$2

    passed=0
    for file_path in $(ls $tests_dir/*.cl); do
        ref_path=$tests_dir/$(basename $file_path .cl).ref

        file_name=$(basename $file_path)
        echo -en "Testing $file_name ... "

        make run -si ARGS="$exec_arg $file_path" | diff - $ref_path > /dev/null 2>&1

        if [ $? -eq 0 ]; then
            echo -e "\e[32mPASSED\e[0m"
            passed=$((passed + 1))
        else
            echo -e "\e[31mFAILED\e[0m"
        fi
    done

    total=$(ls $tests_dir/*.cl | wc -l)
    echo "Passed $passed/$total tests"

}

lexical_analyzer() {
    echo "Testing the lexical analyzer"
    analyzer lexer --lex
}

syntax_analyzer() {
    echo "Testing the syntax analyzer"
    analyzer parser --syn
}

make clean && make

ARG1=$1
if [ "$ARG1" == "--lex" ]; then
    lexical_analyzer
elif [ "$ARG1" == "--syn" ]; then
    syntax_analyzer
elif [ -z "$ARG1" ]; then
    lexical_analyzer
    syntax_analyzer
else
    echo "Usage: $0 [--lex | --syn]"
    exit 1
fi
