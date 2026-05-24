# Makefile for 'cmarkovtxt*.c'


cmarkovtxt: cmarkovtxt.c
	gcc -O3 $< -o $@

cmarkovtxt2: cmarkovtxt2.c
	gcc -O3 $< -o $@

all: cmarkovtxt cmarkovtxt2

.PHONY: clean

clean:
	rm -rf cmarkovtxt cmarkovtxt2 *~ 

