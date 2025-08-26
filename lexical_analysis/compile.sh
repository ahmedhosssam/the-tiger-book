#!/bin/bash
flex++ lexer.l \
&& g++ -std=c++17 main.cc lex.yy.cc \
&& ./a.out
