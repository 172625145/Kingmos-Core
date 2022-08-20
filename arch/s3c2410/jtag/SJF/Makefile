CC    = gcc
#CFLAGS = -O2 -s -DDEBUG -D__linux__ -g
CFLAGS = -DDEBUG -D__linux__ -g

all:SJF2410 

SJF2410: jtag.o strata32.o pin2410.o ppt.o sjf.o mem_rdwr.o strata32.o k9s1208.o
	$(CC) $(CFLAGS) -o $@ jtag.o strata32.o pin2410.o ppt.o sjf.o mem_rdwr.o k9s1208.o 
	cp SJF2410 ../Images/
clean:
	rm -rf *.o SJF2410
