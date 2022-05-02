SRC = $(wildcard ./*.c)
OBJ = $(SRC:.c=.o)
BIN = $(SRC:.c=.bin)
ELF = $(SRC:.c=.elf)

all : $(BIN) $(OBJ) $(ELF)

%.bin : %.o
	i386-elf-ld $< -o $@ --oformat binary -Ttext 0x1000 -e main -L. ./lib/libc.a

%.elf : %.o
	i386-elf-ld $< -o $@ -Ttext 0x1000 -e main -L. ./lib/libc.a

%.o : %.c
	i386-elf-gcc -c $< -o $@ -ffreestanding -g -e main -Ttext 0x1000

clean:
	rm *.o *.bin