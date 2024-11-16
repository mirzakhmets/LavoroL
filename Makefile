# Project: Lavoro
# Makefile created by Embarcadero Dev-C++ 6.3

CPP      = g++
CC       = gcc
OBJ      = main.o src/gnu-efi-3.0.18/gnuefi/reloc_x86_64.o src/gnu-efi-3.0.18/lib/x86_64/callwrap.o src/gnu-efi-3.0.18/lib/x86_64/initplat.o src/gnu-efi-3.0.18/lib/x86_64/math.o src/gnu-efi-3.0.18/lib/boxdraw.o src/gnu-efi-3.0.18/lib/cmdline.o src/gnu-efi-3.0.18/lib/console.o src/gnu-efi-3.0.18/lib/crc.o src/gnu-efi-3.0.18/lib/data.o src/gnu-efi-3.0.18/lib/debug.o src/gnu-efi-3.0.18/lib/dpath.o src/gnu-efi-3.0.18/lib/entry.o src/gnu-efi-3.0.18/lib/error.o src/gnu-efi-3.0.18/lib/event.o src/gnu-efi-3.0.18/lib/exit.o src/gnu-efi-3.0.18/lib/guid.o src/gnu-efi-3.0.18/lib/hand.o src/gnu-efi-3.0.18/lib/hw.o src/gnu-efi-3.0.18/lib/init.o src/gnu-efi-3.0.18/lib/lock.o src/gnu-efi-3.0.18/lib/misc.o src/gnu-efi-3.0.18/lib/pause.o src/gnu-efi-3.0.18/lib/print.o src/gnu-efi-3.0.18/lib/smbios.o src/gnu-efi-3.0.18/lib/sread.o src/gnu-efi-3.0.18/lib/str.o src/gnu-efi-3.0.18/lib/runtime/efirtlib.o src/gnu-efi-3.0.18/lib/runtime/rtdata.o src/gnu-efi-3.0.18/lib/runtime/rtlock.o src/gnu-efi-3.0.18/lib/runtime/rtstr.o src/gnu-efi-3.0.18/lib/runtime/vm.o src/gnu-efi-3.0.18/lib/x86_64/efi_stub.o src/gnu-efi-3.0.18/lib/x86_64/setjmp.o src/gnu-efi-3.0.18/lib/ctors.o src/gnu-efi-3.0.18/gnuefi/crt0-efi-x86_64.o src/packages/ACM/Box.o src/packages/ACM/Pyramids.o src/packages/AntTSP/AntTSP.o
LINKOBJ  = main.o src/gnu-efi-3.0.18/gnuefi/reloc_x86_64.o src/gnu-efi-3.0.18/lib/x86_64/callwrap.o src/gnu-efi-3.0.18/lib/x86_64/initplat.o src/gnu-efi-3.0.18/lib/x86_64/math.o src/gnu-efi-3.0.18/lib/boxdraw.o src/gnu-efi-3.0.18/lib/cmdline.o src/gnu-efi-3.0.18/lib/console.o src/gnu-efi-3.0.18/lib/crc.o src/gnu-efi-3.0.18/lib/data.o src/gnu-efi-3.0.18/lib/debug.o src/gnu-efi-3.0.18/lib/dpath.o src/gnu-efi-3.0.18/lib/entry.o src/gnu-efi-3.0.18/lib/error.o src/gnu-efi-3.0.18/lib/event.o src/gnu-efi-3.0.18/lib/exit.o src/gnu-efi-3.0.18/lib/guid.o src/gnu-efi-3.0.18/lib/hand.o src/gnu-efi-3.0.18/lib/hw.o src/gnu-efi-3.0.18/lib/init.o src/gnu-efi-3.0.18/lib/lock.o src/gnu-efi-3.0.18/lib/misc.o src/gnu-efi-3.0.18/lib/pause.o src/gnu-efi-3.0.18/lib/print.o src/gnu-efi-3.0.18/lib/smbios.o src/gnu-efi-3.0.18/lib/sread.o src/gnu-efi-3.0.18/lib/str.o src/gnu-efi-3.0.18/lib/runtime/efirtlib.o src/gnu-efi-3.0.18/lib/runtime/rtdata.o src/gnu-efi-3.0.18/lib/runtime/rtlock.o src/gnu-efi-3.0.18/lib/runtime/rtstr.o src/gnu-efi-3.0.18/lib/runtime/vm.o src/gnu-efi-3.0.18/lib/x86_64/efi_stub.o src/gnu-efi-3.0.18/lib/x86_64/setjmp.o src/gnu-efi-3.0.18/lib/ctors.o src/packages/ACM/Box.o src/packages/ACM/Pyramids.o src/packages/AntTSP/AntTSP.o
LIBS     = -L"/LavoroL/src/gnu-efi-3.0.18/gnuefi" -L"/LavoroL/src/gnu-efi-3.0.18/lib" -L"/LavoroL/src/gnu-efi-3.0.18/lib/x86_64"
INCS     = -I"/LavoroL/src" -I"/LavoroL/src/gnu-efi-3.0.18/inc" -I"/LavoroL/src/gnu-efi-3.0.18/inc/x86_64"
CXXINCS  = -I"/LavoroL/src" -I"/LavoroL/src/gnu-efi-3.0.18/inc" -I"/LavoroL/src/gnu-efi-3.0.18/inc/x86_64"
BIN      = LavoroL
CXXFLAGS = $(CXXINCS) -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -mno-avx -fPIE -g -O2 -funsigned-char -fno-strict-aliasing -fno-stack-check -fno-merge-all-constants -DCONFIG_X86_64 -maccumulate-outgoing-args -DGNU_EFI_USE_MS_ABI -fpermissive
CFLAGS   = $(INCS) 
DEL      = rm

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${DEL} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	ld $(LINKOBJ) -o $(BIN) $(LIBS) --no-undefined --build-id=sha1 -z nocombreloc -T "src/gnu-efi-3.0.18/gnuefi/elf_x86_64_efi.lds" -shared -Bsymbolic -nostdlib "src/gnu-efi-3.0.18/gnuefi/crt0-efi-x86_64.o"
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 $(BIN) LavoroL.efi

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)

src/gnu-efi-3.0.18/gnuefi/reloc_x86_64.o: src/gnu-efi-3.0.18/gnuefi/reloc_x86_64.c
	$(CPP) -c src/gnu-efi-3.0.18/gnuefi/reloc_x86_64.c -o src/gnu-efi-3.0.18/gnuefi/reloc_x86_64.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/x86_64/callwrap.o: src/gnu-efi-3.0.18/lib/x86_64/callwrap.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/x86_64/callwrap.c -o src/gnu-efi-3.0.18/lib/x86_64/callwrap.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/x86_64/initplat.o: src/gnu-efi-3.0.18/lib/x86_64/initplat.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/x86_64/initplat.c -o src/gnu-efi-3.0.18/lib/x86_64/initplat.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/x86_64/math.o: src/gnu-efi-3.0.18/lib/x86_64/math.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/x86_64/math.c -o src/gnu-efi-3.0.18/lib/x86_64/math.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/boxdraw.o: src/gnu-efi-3.0.18/lib/boxdraw.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/boxdraw.c -o src/gnu-efi-3.0.18/lib/boxdraw.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/cmdline.o: src/gnu-efi-3.0.18/lib/cmdline.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/cmdline.c -o src/gnu-efi-3.0.18/lib/cmdline.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/console.o: src/gnu-efi-3.0.18/lib/console.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/console.c -o src/gnu-efi-3.0.18/lib/console.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/crc.o: src/gnu-efi-3.0.18/lib/crc.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/crc.c -o src/gnu-efi-3.0.18/lib/crc.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/data.o: src/gnu-efi-3.0.18/lib/data.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/data.c -o src/gnu-efi-3.0.18/lib/data.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/debug.o: src/gnu-efi-3.0.18/lib/debug.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/debug.c -o src/gnu-efi-3.0.18/lib/debug.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/dpath.o: src/gnu-efi-3.0.18/lib/dpath.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/dpath.c -o src/gnu-efi-3.0.18/lib/dpath.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/entry.o: src/gnu-efi-3.0.18/lib/entry.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/entry.c -o src/gnu-efi-3.0.18/lib/entry.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/error.o: src/gnu-efi-3.0.18/lib/error.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/error.c -o src/gnu-efi-3.0.18/lib/error.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/event.o: src/gnu-efi-3.0.18/lib/event.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/event.c -o src/gnu-efi-3.0.18/lib/event.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/exit.o: src/gnu-efi-3.0.18/lib/exit.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/exit.c -o src/gnu-efi-3.0.18/lib/exit.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/guid.o: src/gnu-efi-3.0.18/lib/guid.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/guid.c -o src/gnu-efi-3.0.18/lib/guid.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/hand.o: src/gnu-efi-3.0.18/lib/hand.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/hand.c -o src/gnu-efi-3.0.18/lib/hand.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/hw.o: src/gnu-efi-3.0.18/lib/hw.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/hw.c -o src/gnu-efi-3.0.18/lib/hw.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/init.o: src/gnu-efi-3.0.18/lib/init.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/init.c -o src/gnu-efi-3.0.18/lib/init.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/lock.o: src/gnu-efi-3.0.18/lib/lock.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/lock.c -o src/gnu-efi-3.0.18/lib/lock.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/misc.o: src/gnu-efi-3.0.18/lib/misc.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/misc.c -o src/gnu-efi-3.0.18/lib/misc.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/pause.o: src/gnu-efi-3.0.18/lib/pause.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/pause.c -o src/gnu-efi-3.0.18/lib/pause.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/print.o: src/gnu-efi-3.0.18/lib/print.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/print.c -o src/gnu-efi-3.0.18/lib/print.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/smbios.o: src/gnu-efi-3.0.18/lib/smbios.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/smbios.c -o src/gnu-efi-3.0.18/lib/smbios.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/sread.o: src/gnu-efi-3.0.18/lib/sread.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/sread.c -o src/gnu-efi-3.0.18/lib/sread.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/str.o: src/gnu-efi-3.0.18/lib/str.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/str.c -o src/gnu-efi-3.0.18/lib/str.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/runtime/efirtlib.o: src/gnu-efi-3.0.18/lib/runtime/efirtlib.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/runtime/efirtlib.c -o src/gnu-efi-3.0.18/lib/runtime/efirtlib.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/runtime/rtdata.o: src/gnu-efi-3.0.18/lib/runtime/rtdata.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/runtime/rtdata.c -o src/gnu-efi-3.0.18/lib/runtime/rtdata.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/runtime/rtlock.o: src/gnu-efi-3.0.18/lib/runtime/rtlock.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/runtime/rtlock.c -o src/gnu-efi-3.0.18/lib/runtime/rtlock.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/runtime/rtstr.o: src/gnu-efi-3.0.18/lib/runtime/rtstr.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/runtime/rtstr.c -o src/gnu-efi-3.0.18/lib/runtime/rtstr.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/runtime/vm.o: src/gnu-efi-3.0.18/lib/runtime/vm.c
	$(CPP) -c src/gnu-efi-3.0.18/lib/runtime/vm.c -o src/gnu-efi-3.0.18/lib/runtime/vm.o $(CXXFLAGS)

# Continued

src/gnu-efi-3.0.18/gnuefi/crt0-efi-x86_64.o: src/gnu-efi-3.0.18/gnuefi/crt0-efi-x86_64.S
	$(CPP) -c src/gnu-efi-3.0.18/gnuefi/crt0-efi-x86_64.S -o src/gnu-efi-3.0.18/gnuefi/crt0-efi-x86_64.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/x86_64/efi_stub.o: src/gnu-efi-3.0.18/lib/x86_64/efi_stub.S
	$(CPP) -c src/gnu-efi-3.0.18/lib/x86_64/efi_stub.S -o src/gnu-efi-3.0.18/lib/x86_64/efi_stub.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/x86_64/setjmp.o: src/gnu-efi-3.0.18/lib/x86_64/setjmp.S
	$(CPP) -c src/gnu-efi-3.0.18/lib/x86_64/setjmp.S -o src/gnu-efi-3.0.18/lib/x86_64/setjmp.o $(CXXFLAGS)

src/gnu-efi-3.0.18/lib/ctors.o: src/gnu-efi-3.0.18/lib/ctors.S
	$(CPP) -c src/gnu-efi-3.0.18/lib/ctors.S -o src/gnu-efi-3.0.18/lib/ctors.o $(CXXFLAGS)

# Applications

src/packages/ACM/Box.o: src/packages/ACM/Box.cpp
	$(CPP) -c src/packages/ACM/Box.cpp -o src/packages/ACM/Box.o $(CXXFLAGS)

src/packages/ACM/Pyramids.o: src/packages/ACM/Pyramids.cpp
	$(CPP) -c src/packages/ACM/Pyramids.cpp -o src/packages/ACM/Pyramids.o $(CXXFLAGS)

src/packages/AntTSP/AntTSP.o: src/packages/AntTSP/AntTSP.cpp
	$(CPP) -c src/packages/AntTSP/AntTSP.cpp -o src/packages/AntTSP/AntTSP.o $(CXXFLAGS)
