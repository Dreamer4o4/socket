CC := gcc -g
exe := myhttp
obj := myhttp.o

all:$(exe)

$(exe):$(obj)
	$(CC) $^ -o $@ -lpthread

myhttp.o:myhttp.h

clean:
	rm *.o $(exe)