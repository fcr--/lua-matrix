CFLAGS ?= `pkg-config --cflags lua5.3` -fpic -Wall -O2
MATRIX_SO ?= matrix.so

$(MATRIX_SO): matrix.o
	$(CC) -shared -o $@ $^ $(SOFLAGS) $(SOLIBS)

%.o: %.c matrix_config.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(MATRIX_SO) *.o