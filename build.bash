#!/bin/bash
/usr/bin/g++ -fdiagnostics-color=always -g \
$(find ${PWD} -name "*.cpp" ! -path "*/elf_writer/*") \
-o main -std=c++17
