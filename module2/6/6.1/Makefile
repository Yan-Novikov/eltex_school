all:
	gcc -c phonebook.c -o phonebook.o
	ar rc libphonebook.a phonebook.o
	gcc main.c -L. -lphonebook -o phonebook

clean:
	rm -f phonebook.o libphonebook.a phonebook