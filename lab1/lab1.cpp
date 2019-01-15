#include "lab1.h"
#include <iostream>
// #include <stdint.h>

bool Test(const volatile uint32_t *bitset, int bit_index){
    if(bit_index > 31 || bit_index < -32) return false;
    if(bit_index < 0) bit_index += 32;

    return (((*bitset >> bit_index) & 1) == 1) ? true : false; 
}

void Set(volatile uint32_t *bitset, int bit_index){
    if(bit_index > 31 || bit_index < -32) return;
    if(bit_index < 0) bit_index += 32;

    *bitset |= 1 << bit_index;

    return;
}

void Clear(volatile uint32_t *bitset, int bit_index){
    if(bit_index > 31 || bit_index < -32) return;
    if(bit_index < 0) bit_index += 32;

    *bitset &= ~(1 << bit_index);

    return;
}

constexpr uint32_t Mask(int bit_start, int bit_end){
    return 0;
}

int compTest(){
    return 55;
}