CC = mpicc
CFLAGS = -Wall -O3 -g -lm
BINS = malaria

all: $(BINS)

malaria: malaria.c prop.c prop.h
	$(CC) $(CFLAGS) -o $@ malaria.c prop.c -lm

clean:
	$(RM) $(BINS)

