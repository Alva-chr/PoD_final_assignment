CC = mpicc
CFLAGS = -Wall -O3 -g
BINS = malaria

all: $(BINS)

malaria: malaria.c prop.h
	$(CC) $(CFLAGS) -o $@ malaria.c

clean:
	$(RM) $(BINS)

