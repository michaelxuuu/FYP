set confirm off
set verbose off
set prompt \033[31mpm-mode-gdb > \033[0m

set output-radix 0d10
set input-radix 0d10

# These make gdb never pause in its output
set height 0
set width 0

# Intel syntax
set disassembly-flavor intel

set $state=1

define hook-stop
  if ($cr0 & 0x80000000)
    display_instr_pg
  else
    display_instr_seg
  end
end

set $CODE_SIZE = 10

# Use this function to print instructions in segmentated model
# The GDT code entry base address is 0x40000000, creating the illusion that the code is executed at 0xC0000000 and above
# Actually, the code is executed at 0x0 and above, so we subtract 0xC0000000 from eip every time we print instructions pointed to by eip
# Note that all the address get printed are phsycial address (including the addresses as immediate oprands, which is weird and I don't know why) !!!
define display_instr_seg
    set $_code_size = $CODE_SIZE
    if ($_code_size > 0)
        x /i $eip - 0xC0000000
        set $_code_size--
    end
    while ($_code_size > 0)
        x /i
        set $_code_size--
    end
end

# Use this function to print instructions when PG is enabled
# With PG, we can access the instructions by their logical (virtual) addresses
define display_instr_pg
    set $_code_size = $CODE_SIZE
    if ($_code_size > 0)
        x /i $eip
        set $_code_size--
    end
    while ($_code_size > 0)
        x /i
        set $_code_size--
    end
end

# Use this to print instruction at a specific address in segmentated model
# s - segmentated
# d - display
# i - instruction
define sdi
    x/2i 0x40000000+$arg0
end

# Use this to inspect memory data by address in segmentated model
define sdd
    x/2x 0x40000000+$arg0
end

# Use this to set breakpoints by address in segmentated model
# s - segmentated
# b - break
# a - address
define sba
    br $arg0+0x40000000
end

# Use this to set breakpoints by a symbol in segmentated model
define sbs
    br *0x40000000+$arg0
end

# Use this to set breakpoints by lines in segmentated model
define sbl
    py x = gdb.find_pc_line(gdb.decode_line("$arg0")[1][0].pc)
    py gdb.execute("sbs " + str(x.pc))
end