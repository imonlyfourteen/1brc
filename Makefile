
CFLAGS += -O0 -g -march=native

all: 1brc  generate  generate_stations_rand_h

1brc:
	echo '1brc has been built:)'

GEN_OBJS    = rnd.o  funcs.o  get_time.o
generate: generate_stations_rand_h $(GEN_OBJS) generate.c
	./generate_stations_rand_h
	$(CC) $(CFLAGS) generate.c $(GEN_OBJS) -lm -o generate

GEN_SN_OBJS = rnd.o  funcs.o
generate_stations_rand_h: $(GEN_SN_OBJS) generate_stations_rand_h.c
	$(CC) $(CFLAGS) generate_stations_rand_h.c $(GEN_SN_OBJS) -lm -o generate_stations_rand_h

rnd.o: rnd.h
funcs.o: funcs.h
get_time.o: get_time.h

clean:
	rm *.o  generate  generate_stations_rand_h  1brc
