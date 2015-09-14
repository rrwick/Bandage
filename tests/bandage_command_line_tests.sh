#!/bin/bash

# Change this variable to point to the Bandage executable.
bandagepath="../../build-Bandage-Desktop_Qt_5_5_0_clang_64bit-Release/Bandage.app/Contents/MacOS/Bandage"

function test_all {
    command=$1
    expected_exit_code=$2
    expected_std_out=$3
    expected_std_err=$4

    $command 1> tmp/std_out 2> tmp/std_err
    exit_code=$?
    std_out="$(cat tmp/std_out)"
    std_err="$(cat tmp/std_err)"

    if [ $exit_code == $expected_exit_code ]; then correct_exit_code=true; else correct_exit_code=false; fi
    if [ "$std_out" == "$expected_std_out" ]; then correct_std_out=true; else correct_std_out=false; fi
    if [ "$std_err" == "$expected_std_err" ]; then correct_std_err=true; else correct_std_err=false; fi

    if $correct_exit_code && $correct_std_out && $correct_std_err;
        then echo "PASS";
    else
        echo "FAIL"
        echo "   command: $command"
        if ! $correct_exit_code; then echo "   expected exit code: $expected_exit_code"; echo "   actual exit code: $exit_code"; fi
        if ! $correct_std_out; then echo "   expected std out: $expected_std_out"; echo "   actual std out: $std_out"; fi
        if ! $correct_std_err; then echo "   expected std err: $expected_std_err"; echo "   actual std err: $std_err"; fi
    fi

    rm tmp/std_out
    rm tmp/std_err
}

function test_exit_code {
    command=$1
    expected_exit_code=$2

    $command 1> tmp/std_out 2> tmp/std_err
    exit_code=$?

    if [ $exit_code == $expected_exit_code ]; then correct_exit_code=true; else correct_exit_code=false; fi

    if $correct_exit_code && $correct_std_out && $correct_std_err;
        then echo "PASS";
    else
        echo "FAIL"
        echo "   command: $command"
        if ! $correct_exit_code; then echo "   expected exit code: $expected_exit_code"; echo "   actual exit code: $exit_code"; fi
    fi

    rm tmp/std_out
    rm tmp/std_err
}



mkdir tmp

test_all "$bandagepath image test.fastg test.png" 0 "" ""; rm test.png
test_all "$bandagepath image abc.fastg test.png" 1 "" "Bandage error: abc.fastg does not exist."



rmdir tmp