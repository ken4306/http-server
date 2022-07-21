##
# Project Title
#
# @file
# @version 0.1
%.out: %.c
	gcc $< -o $@

all: server.out

run: server.out
	./server.out

.PHONY: run all
# end
