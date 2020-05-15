CC := gcc -g
exe := myhttp
obj := myhttp.o pth_pool.o log.o connect2socket.o server.o

all:$(exe)

$(exe):$(obj)
	$(CC) $^ -o $@ -lpthread

myhttp.o:myhttp.h pth_pool.h log.h server.h
pth_pool.o:pth_pool.h log.h
connect2socket.o:connect2socket.h
server.o:server.h myhttp.h


clean:
	rm *.o $(exe) *.txt