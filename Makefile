SRC = $(wildcard kernel/*.c drivers/*.c cpu/*.c mem/*.c Lib/*.c fs/*.c user/*.c)
OBJ = $(SRC:.c=.o cpu/hdlr_stub.o)

all: run clean

run: vhd
	qemu-system-i386 -no-reboot $^

debug16: vhd boot_sect.elf kernel.elf
	qemu-system-i386 -s -S $< &
	i386-elf-gdb -ix ".gdb_conf/asm16.gdb" \
				 -ex "set tdesc filename .gdb_conf/target.xml" \
				 -ex "target extended-remote localhost:1234" \
				 -ex "symbol-file boot_sect.elf" \
				 -ex "symbol-file kernel.elf" \

debug32asm: vhd boot_sect.elf kernel.elf
	qemu-system-i386 -s -S $< &
	i386-elf-gdb -ix ".gdb_conf/asm32.gdb" \
			     -ex "target extended-remote localhost:1234" \
				 -ex "symbol-file kernel.elf" \
				 -ex "br *0x40000000+init32" \

debug32c: vhd boot_sect.elf kernel.elf
	qemu-system-i386 -s -S $< &
	i386-elf-gdb -ex "target extended-remote localhost:1234" \
				 -ex "symbol-file kernel.elf" \

vhd: boot_sect.bin kernel.bin
	dd bs=1M if=/dev/zero of=$@ count=512
	dd bs=512 if=$< of=$@ count=1 conv=notrunc
	dd bs=512 if=kernel.bin of=$@ seek=1 count=1024 conv=notrunc
	dd bs=4K if=./testprog/shell.bin of=vhd seek=263 count=1 conv=notrunc


kernel.elf: kernel_entry.o $(OBJ)
	i386-elf-ld -Ttext 0xC0000000 -o $@  $^

kernel.bin: kernel_entry.o $(OBJ)
	i386-elf-ld -Ttext 0xC0000000 -o $@ --oformat binary $^

%.o: %.c
	i386-elf-gcc -ffreestanding -g -c $< -o $@

cpu/hdlr_stub.o: cpu/hdlr_stub.s
	i386-elf-as -c $< -o $@

boot_sect.elf: boot_sect.o
	i386-elf-ld -Ttext 0x0 -o $@ -e init $^

boot_sect.bin: boot_sect.o
	i386-elf-ld -Ttext 0x0 -o $@ -e init --oformat binary $^

boot_sect.o: boot/boot_sect.s
	i386-elf-as $^ -o $@

kernel_entry.o: boot/kernel_entry.s
	i386-elf-as $^ -o $@

clean:
	rm -rf *.bin *.elf *.o vhd kernel/*.o cpu/*.o drivers/*.o mem/*.o Lib/*.o fs/*.o user/*.o testprog/*.o testprog/*.bin