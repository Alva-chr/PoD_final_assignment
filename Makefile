CC = mpicc
CFLAGS = -Wall -O3 -g
BINS = malaria

all: $(BINS)

malaria: malaria.c
	$(CC) $(CFLAGS) -o $@ malaria.c

clean:
	$(RM) $(BINS)

