CC:=gcc -g
obj:=myworkbench.o
exe:=myworkbench


all:$(exe)

$(exe):$(obj)
	$(CC) $(obj) -o $(exe)

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	rm -rf $(obj) $(exe)

