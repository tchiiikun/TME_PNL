.PHONY : all clean check
.ONESHELL:

CHECK_PATCH=../../../../biblio/eudyptula/torvalds/scripts/checkpatch.pl

all : cron_func

%.o : %.c
	gcc -Wall -c $<
handle.o : handle.c
	gcc -Wall -c -ldl $<
cron_func : cron_func.o libfunc.so
	gcc $(PWD)/libfunc.so -Wall $^ -o $@
libfunc.so : nothing.o
	gcc -Wall -shared -Wl,-soname,$@ \
		-o $@ $<
libread.so : tchao.o
	gcc -Wall -shared -Wl,-soname,$@ \
		-o $@ $<
libsym.so : handle.o
	gcc -Wall -ldl -shared -Wl,-soname,$@ \
		-o $@ $<
check :
	for f in *.c *.h ; do
		$(CHECK_PATCH) -f $$f
	done

clean :
	rm -rf *.o cron_func libfunc.so
