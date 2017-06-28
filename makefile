objects=main.o simplehttp.o
myhttp:$(objects)
	gcc $(objects) -o myhttp -lpthread
main.o:main.c 
	gcc -c main.c 
simphttp.o:simplehttp.c
	gcc -c simplehttp.c 
clean :
	rm myhttp $(objects)

