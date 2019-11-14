#!/bin/sh

rm bas cc

# bas.s
../build/pdp11 ../root /bin/as /usr/source/s1/bas.s; echo $?
../build/pdp11 ../root /bin/ld -s -n a.out -l; echo $?
../build/pdp11 ../root /bin/cmp a.out /bin/bas; echo $?
cp a.out bas

# cc.c
../build/pdp11 ../root /bin/cc -s -n -O /usr/source/s1/cc.c; echo $?
../build/pdp11 ../root /bin/cmp a.out /bin/cc; echo $?
cp a.out cc

rm a.out

md5sum bas ../root/bin/bas
md5sum cc ../root/bin/cc

# rebuild v6 kernel
echo 'rebuild v6 kernel'
md5sum ../root/hpunix
md5sum ../root/rkunix
md5sum ../root/rpunix
cd ../root/usr/sys
time ~/src/pdp11/build/pdp11 ~/src/UNIX_V6 /bin/sh run > /dev/null
cd ../../
md5sum hpunix
md5sum rkunix
md5sum rpunix
