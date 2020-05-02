CC := gcc -g
exe := myhttp
obj := myhttp.o pth_pool.o log.o

all:$(exe)

$(exe):$(obj)
	$(CC) $^ -o $@ -lpthread

myhttp.o:myhttp.h pth_pool.h log.h
pth_pool.o:pth_pool.h log.h


clean:
	rm *.o $(exe)