#!/bin/bash

echo $* changed
echo testing ...
c++ -Wall test-Log-YAML.cpp && ./a.out
