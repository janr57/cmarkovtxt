# Makefile for 'cmarkovtxt.c'


cmarkovtxt: cmarkovtxt.c
	gcc -O3 $< -o $@

all: cmarkovtxt

.PHONY: clean

clean:
	rm -rf cmarkovtxt *~ 

