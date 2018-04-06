.PHONY : all clean check
.ONESHELL:

HIS=../EXO-03/




all : testVersion testOffset testHistory

%.o : %.c
	gcc -Wall -c $< 

testVersion : testVersion.c version.o
	gcc -Wall -g $^ -o $@

testOffset : testOffset.c version.o
	gcc -Wall -g $^ -o $@

testHistory : $(HIS)testHistory.c history.o commit.o
	gcc -Wall -g $^ -o $@

history.o: $(HIS)history.c
	gcc -Wall -g -c $<

commit.o: $(HIS)commit.c
	gcc -Wall -g -c $<


clean :
	rm -rf *.o testVersion testOffset testHistory
	rm ../EXO-03/*.o
