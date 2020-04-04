CC:=gcc
obj:=mywebbench.o bench.o connect2socket.o
exe:=mywebbench


all:$(exe)

$(exe):$(obj)
	$(CC) $(obj) -o $(exe) -lpthread

mywebbench.o:mywebbench.h
bench.o:bench.h
connect2socket.o:connect2socket.h

# %.o:%.c
# 	$(CC) -c $^ -o $@

clean:
	rm -rf $(obj) $(exe)

