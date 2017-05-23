#!/bin/bash

echo $* changed
echo testing ...
c++ test-Log-YAML.cpp && ./a.out
