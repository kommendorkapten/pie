#!/bin/sh

find . -name '*.c' | xargs grep malloc | grep -v testp | grep -v "/dm/" | grep -v "/lib/"
printf "%s" "Total number of mallocs:"
find . -name '*.c' | xargs grep malloc | grep -v testp | grep -v "/dm/" | grep -v "/lib/" | wc -l
