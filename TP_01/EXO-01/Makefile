.PHONY : all clean check
.ONESHELL:

HIS=../EXO-03/


CHECK_PATCH=./checkpatch.pl --no-tree


all : testVersion testOffset testHistory

%.o : %.c
	gcc -Wall -c $< 

testVersion : testVersion.c version.o
	gcc -Wall -g $^ -o $@

testOffset : testOffset.c version.o
	gcc -Wall -g $^ -o $@

testHistory : $(HIS)testHistory.c $(HIS)history.o $(HIS)commit.o
	gcc -Wall -g $^ -o $@


check : 
	for f in *.c *.h ; do
	    $(CHECK_PATCH) -f $$f
	done

clean :
	rm -rf *.o testVersion testOffset testHistory
