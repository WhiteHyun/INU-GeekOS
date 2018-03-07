#!/bin/bash

# for generating the cycstubs.cyc, cstubs.c, and attendant cyclone
# header files.  Will place the former two in the proper source
# directories, and the latter in the include/libc/cyclone directory.

if [ $# -lt 1 ]; then
  echo "usage: dobuildlib <project-home> [buildlib-executable]"
  exit 1
elif [ $# = 2 ]; then
  BUILDLIB=$2
else
  BUILDLIB=buildlib
fi
PROJHOME=$1

CYS=$PROJHOME/include/libc/cyclone/cyclib.cys
INC=$PROJHOME/include
OUT=/tmp/buildlib.out

TGT_H=$PROJHOME/include/libc/cyclone
TGT_C=$PROJHOME/src/common
TGT_CYC=$PROJHOME/src/common/cyclone

$BUILDLIB -d $OUT -I$INC $CYS
if [ $? != 0 ]; then
  echo "Buildlib failed!"
  exit 1
fi

cp $OUT/libc/*.h $TGT_H
sed -e 's#libc/#cyclone/#' $OUT/cycstubs.cyc > $TGT_CYC/cycstubs.cyc
cp $OUT/cstubs.c $TGT_C

rm -rf $OUT

exit 0
