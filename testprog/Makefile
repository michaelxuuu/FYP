shell.bin : shell.o
	i386-elf-ld shell.o -o shell.bin --oformat binary -Ttext 0x0 -e main

shell.o : shell.c
	i386-elf-gcc -c shell.c -o shell.o -ffreestanding

clean:
	rm shell.o shell.bin