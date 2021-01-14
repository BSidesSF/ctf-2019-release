#!/bin/bash

if [ -z "`echo $1 | ./oracle | fgrep 'First byte of padding is not 0x00'`" ]; then
    echo 0
else
    echo 1
fi
