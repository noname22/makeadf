#!/bin/sh

cat >myctest.c <<END

#include<stdio.h>
#include<stdint.h>

union u{
    int32_t l;
    char c[4];
    };

int main(int argc, char *argv[])
{
    union u val;

    printf("%ld ",sizeof(int32_t));
    printf("%ld ",sizeof(int16_t));
    printf("%ld ",sizeof(int));
    
    val.l=1L;
    if (val.c[3]==1)
        puts("BIG");
    else
        puts("LITTLE");
}

END

gcc -std=gnu99 myctest.c -o myctest
rm myctest.c

if [ `./myctest |cut -d' ' -f 1` != 4 ]
    then echo "Error : sizeof(long)!=4"
fi
if [ `./myctest |cut -d' ' -f 2` != 2 ]
    then echo "Error : sizeof(short)!=2"
fi
if [ `./myctest |cut -d' ' -f 3` != 4 ]
    then echo "Error  :sizeof(int)!=4"
fi

if [ `./myctest |cut -d' ' -f 4` = LITTLE ]
    then 
    echo "#ifndef LITT_ENDIAN" >defendian.h
    echo "#define LITT_ENDIAN 1" >>defendian.h 
    echo "#endif /* LITT_ENDIAN */" >>defendian.h
    echo Little Endian machine detected 
else
    echo "#ifndef LITT_ENDIAN" >defendian.h
    echo "#endif /* LITT_ENDIAN */" >>defendian.h
    echo Big Endian machine detected
fi

if [ -f myctest.exe ]; then
	rm myctest.exe
fi

if [ -f myctest ]; then
	rm myctest
fi
