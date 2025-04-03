# CMSC216 Project 2 Makefile
AN = p2
CLASS = 216

# -Wno-comment: disable warnings for multi-line comments, present in some tests
# CFLAGS = -Wall -Wno-comment -Werror -g  -Wno-format-security 
CFLAGS = -Wall -Werror -g
CC     = gcc $(CFLAGS)
SHELL  = /bin/bash
CWD    = $(shell pwd | sed 's/.*\///g')

PROGRAMS = \
	scale_main \
	test_scale_update \
	puzzlebox   \
	hashset_main \
	test_hashset_verify \


export PARALLEL?=True		#enable parallel testing if not overridden

all : $(PROGRAMS) 

# cleaning target to remove compiled programs/objects
clean :
	rm -f $(PROGRAMS) *.o

help :
	@echo 'Typical usage is:'
	@echo '  > make                          # build all programs'
	@echo '  > make clean                    # remove all compiled items'
	@echo '  > make zip                      # create a zip file for submission'
	@echo '  > make prob1                    # built targets associated with problem 1'
	@echo '  > make test                     # run all tests'
	@echo '  > make test-prob2               # run test for problem 2'
	@echo '  > make test-prob2 testnum=5     # run problem 2 test #5 only'
	@echo '  > make update                   # download and install any updates to project files'


############################################################
# 'make zip' to create complete.zip for submission
ZIPNAME = $(AN)-complete.zip
zip : clean clean-tests
	rm -f $(ZIPNAME)
	cd .. && zip "$(CWD)/$(ZIPNAME)" -r "$(CWD)"
	@echo Zip created in $(ZIPNAME)
	@if (( $$(stat -c '%s' $(ZIPNAME)) > 10*(2**20) )); then echo "WARNING: $(ZIPNAME) seems REALLY big, check there are no abnormally large test files"; du -h $(ZIPNAME); fi
	@if (( $$(unzip -t $(ZIPNAME) | wc -l) > 256 )); then echo "WARNING: $(ZIPNAME) has 256 or more files in it which may cause submission problems"; fi

############################################################
# `make update` to get project updates
update :
	curl -s https://www.cs.umd.edu/~profk/216/$(AN)-update.sh | /bin/bash 


################################################################################
# scale problem
prob1 : scale_main test_scale_update

scale_main : scale_main.o scale_update.o scale_sim.o
	$(CC) -o $@ $^

scale_main.o : scale_main.c scale.h
	$(CC) -c $<

scale_sim.o : scale_sim.c scale.h
	$(CC) -c $<

scale_update.o : scale_update.c scale.h
	$(CC) -c $<

test_scale_update : test_scale_update.c scale_sim.o scale_update.o
	$(CC) -o $@ $^

test-prob1 : prob1 test_scale_update test-setup
	./testy -o md test_scale.org $(testnum)

################################################################################
# debugging problem
prob2 : puzzlebox

puzzlebox.o : puzzlebox.c
	$(CC) -c $<

puzzlebox : puzzlebox.o
	$(CC) -o $@ $^


test-prob2 : prob2 test-setup
	./puzzlebox input.txt

################################################################################
# hashset problem
prob3 : hashset_main

hashset_main : hashset_main.o hashset_funcs.o
	$(CC) -o $@ $^

hashset_main.o : hashset_main.c hashset.h
	$(CC) -c $<

hashset_funcs.o : hashset_funcs.c hashset.h
	$(CC) -c $<

test_hashset_verify : test_hashset_verify.c
	$(CC) -o $@ $^

test-prob3 : hashset_main test_hashset_verify
	./testy -o md test_hashset.org $(testnum)

################################################################################
# Testing Targets
test : test-prob1 test-prob2 test-prob3

test-setup:
	@chmod u+x testy

clean-tests :
	rm -rf test-results


