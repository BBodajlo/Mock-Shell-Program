CC = gcc

mysh: mysh.c
	$(CC) -o mysh mysh.c
clean:
	$(RM) -f *.o mysh mysh.exe