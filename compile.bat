gcc -g -Wall MasterSystem/emu.c BITS/bitUtils.c Z80/z80.c TMS9918a/tms.c sfml\*.a *.dll -o Gui0x52 -lmingw32 -lcomdlg32 -static-libgcc -static-libstdc++
pause
