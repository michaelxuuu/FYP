SRC = $(wildcard ./*.c)
OBJ = $(SRC:.c=.o)
BIN = $(SRC:.c=.bin)

all : $(BIN) $(OBJ)

%.bin : %.o
	i386-elf-ld $< -o $@ --oformat binary -Ttext 0x0 -e main -L. ./lib/libc.a

%.o : %.c
	i386-elf-gcc -c $< -o $@ -ffreestanding

clean:
	rm *.o *.bin