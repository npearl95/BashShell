
objects = utility.o myshell.o

test : $(objects)
	cc -o test $(objects)

main.o : utility.h myshell.h

.PHONY : clean
clean :
	rm $(objects)
