EXTRA_INCLUDE=
EXTRA_LIBDIR=
ARGS=-Wall -O4 -std=c99


all:	rfm12

rfm12.o:	rfm12.c
	gcc -c $^ -lwiringPi -o rfm12.o $(ARGS) $(EXTRA_INCLUDE)

rfm12_communication.o:	rfm12_communication.c rfm12_communication.h config.h
	gcc -c $^ -lwiringPi  $(ARGS) $(EXTRA_INCLUDE)


rfm12:	rfm12.o rfm12_communication.o
		gcc $^ -lwiringPi -o rfm12 $(ARGS) $(EXTRA_LIBDIR)


run:	rfm12
	sudo ./rfm12

clean:
	rm -f *.o
	rm -f *.gch
	rm -f rfm12

init:
	sudo echo 25 > /sys/class/gpio/export
	sudo echo falling > /sys/class/gpio/gpio25/edge
