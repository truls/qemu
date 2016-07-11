#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if ( argc != 3) {
        printf("Usage: %s <array_size_in_KB> <use_random_accesses: 1 or 0>\n", argv[0]);
        return 1;
    }

    uint64_t size = atoi(argv[1]) * 1024; // Size is size in Bytes
    int is_rand = atoi(argv[2]);

    uint64_t* arr = malloc(size * sizeof(uint8_t));

    uint64_t i = 0, tmp = 5;

    for (i = 0; i < size/sizeof(uint64_t); i++) {
        arr[i] = i;
    }
   
    printf("Array size is %llu bytes.\nInit done!\n", size);

    while(1) {
       __asm__ __volatile__(
                "VOODOO:\n\t"
                "ADD x0, %x[arr], %x[ind]\n\t"
                "LDR %x[tmp], [x0]\n\t"
                "ADD %x[ind], %x[ind], #64\n\t"
                "NO_MORE_MAGIC_PLS:\n\t"
                : [ind] "+r" (i), [tmp] "=r" (tmp)
                : [arr] "r" (arr)
                : "x0"
                );
        i = i % size;

    }
    return 0;
}

