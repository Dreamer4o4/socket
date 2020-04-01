CC:=gcc -g
obj:=mywebbench.o
exe:=mywebbench


all:$(exe)

$(exe):$(obj)
	$(CC) $(obj) -o $(exe) -lpthread

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	rm -rf $(obj) $(exe)

