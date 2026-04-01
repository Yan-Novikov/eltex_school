all:
	gcc -fPIC phonebook.c -c
	gcc -shared phonebook.o -o libphonebook.so
	gcc main.c -o phonebook -L. -lphonebook
	LD_LIBRARY_PATH=. ./phonebook

clean:
	rm -f phonebook.o libphonebook.so phonebook