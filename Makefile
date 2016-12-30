EXTRA_INCLUDE=
EXTRA_LIBDIR=
ARGS=-Wall -O4 -std=c99

TMP=/dev/shm

all:	rfm69

$(TMP)/main.o:	main.c config.h
	gcc -c $< -lwiringPi -o $@ $(ARGS) $(EXTRA_INCLUDE)

$(TMP)/rfm12_communication.o:	rfm12_communication.c rfm12_communication.h config.h 
	gcc -c $< -o $@ -lwiringPi  $(ARGS) $(EXTRA_INCLUDE)


rfm69:	$(TMP)/main.o $(TMP)/rfm12_communication.o
		gcc $^ -lwiringPi -o $@ $(ARGS) $(EXTRA_LIBDIR)


run:	rfm69
	sudo ./rfm69

clean:
	rm -f *.o
	rm -f *.gch
	rm -f rfm69

init:
	sudo echo 25 > /sys/class/gpio/export
	sudo echo falling > /sys/class/gpio/gpio25/edge
