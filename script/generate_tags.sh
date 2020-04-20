#!/bin/bash
cat BUILD/GAP8/GCC_RISCV/*.d |\
sed -e 's/[\\ ]/\n/g' |	sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d' |\
ctags -V -L - --c++-kinds=+p --fields=+iaS --extra=+q
