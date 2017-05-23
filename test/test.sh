#!/bin/bash

echo $* changed
echo testing ...
c++ -Wall -Wextra -pedantic test-Log-YAML.cpp && ./a.out
