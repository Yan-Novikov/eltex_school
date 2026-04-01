all:
	gcc -shared -fPIC -o libsum.so sum.c
	gcc -shared -fPIC -o libsubstract.so substract.c
	gcc -shared -fPIC -o libmultiply.so multiply.c
	gcc -shared -fPIC -o libdivide.so divide.c
	gcc -rdynamic -o calcul main.c -ldl

clean:
	rm -f calcul libsum.so libsubstract.so libmultiply.so libdivide.so