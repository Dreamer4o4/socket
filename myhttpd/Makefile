CC := gcc -g
exe := myhttp
obj := myhttp.o pth_pool.o

all:$(exe)

$(exe):$(obj)
	$(CC) $^ -o $@ -lpthread

myhttp.o:myhttp.h pth_pool.h
pth_pool.o:pth_pool.h


clean:
	rm *.o $(exe)