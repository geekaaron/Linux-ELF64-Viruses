
.PHONY: all clean

OBJ += extractor tvirus dvirus hello test

all: $(OBJ)

extractor: extractor.c
	@gcc -o $@ $<

tvirus: tvirus.c
	@gcc -o $@ $<

dvirus: dvirus.c
	@gcc -o $@ $<

hello: hello.s
	@as -o hello.o hello.s
	@ld -o hello hello.o

test: test.s
	@as -o test.o test.s
	@ld -o test test.o

clean:
	@rm *.o $(OBJ)
