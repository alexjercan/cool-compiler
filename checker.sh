#!/bin/bash

TESTS_DIR=tests

analyzer() {
    if [ "$#" -ne 2 ]; then
        echo "Usage: $0 <tests_dir> <exec_arg>"
        exit 1
    fi

    tests_dir=$TESTS_DIR/$1
    exec_arg=$2

    echo "Running tests for $1"

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

runner() {
    if [ "$#" -ne 2 ]; then
        echo "Usage: $0 <tests_dir> <exec_arg>"
        exit 1
    fi

    tests_dir=$TESTS_DIR/$1
    exec_arg=$2

    echo "Running tests for $1"

    passed=0
    for file_path in $(ls $tests_dir/*.cl); do
        ref_path=$tests_dir/$(basename $file_path .cl).ref

        file_name=$(basename $file_path .cl)
        echo -en "Testing $file_name.cl ... "

        make run -si ARGS="$exec_arg $file_path" > /tmp/$file_name.s
        if [ $? -ne 0 ]; then
            echo -e "\e[31mFAILED\e[0m"
            continue
        fi

        fasm /tmp/$file_name.s 2>&1 > /dev/null
        if [ $? -ne 0 ]; then
            echo -e "\e[31mFAILED\e[0m"
            continue
        fi

        /tmp/$file_name | diff - $ref_path > /dev/null 2>&1

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

semantic_analyzer() {
    echo "Testing the semantic analyzer"
    analyzer semantic --sem
    analyzer semantic2 --sem
}

tac_generator() {
    echo "Testing the TAC generator"
    analyzer tac --tac
}

asm_generator() {
    echo "Testing the assembly generator"
    runner asm --asm
}

make clean && make

ARG1=$1
if [ "$ARG1" == "--lex" ]; then
    lexical_analyzer
elif [ "$ARG1" == "--syn" ]; then
    syntax_analyzer
elif [ "$ARG1" == "--sem" ]; then
    semantic_analyzer
elif [ "$ARG1" == "--tac" ]; then
    tac_generator
elif [ "$ARG1" == "--asm" ]; then
    asm_generator
elif [ -z "$ARG1" ]; then
    lexical_analyzer
    syntax_analyzer
    semantic_analyzer
    tac_generator
    asm_generator
else
    echo "Usage: $0 [--lex | --syn | --sem | --tac | --asm]"
    exit 1
fi
