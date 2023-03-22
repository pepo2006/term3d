#!/bin/bash

if ! ls bin; then mkdir bin; fi 2>/dev/null

gcc -lm -Wall -Wextra -ggdb -o ./bin/demo main.c
