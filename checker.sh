#!/bin/bash

BUILD_DIR=./build
OUTPUT_LEXER_DIR=./build/output/lexer

lexical_analyzer() {
    echo "Testing the lexical analyzer"
    mkdir -p $OUTPUT_LEXER_DIR

    passed=0
    for file_path in $(ls ./tests/lexer/*.cl); do
        ref_path=./tests/lexer/$(basename $file_path .cl).ref
        out_path=$OUTPUT_LEXER_DIR/$(basename $file_path .cl).out

        file_name=$(basename $file_path)
        echo -en "Testing $file_name ... "

        $BUILD_DIR/main --lex $file_path > $out_path 2>&1
        diff $out_path $ref_path > /dev/null 2>&1

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
if [ "$ARG1" == "--lexer" ]; then
    lexical_analyzer
elif [ -z "$ARG1" ]; then
    lexical_analyzer
else
    echo "Invalid argument"
fi
