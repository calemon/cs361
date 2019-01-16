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
    uint32_t mask = 0;
    if(bit_start > 31 || bit_start < -32 || bit_end > 31 || bit_end < -32) return mask;
    if(bit_start < 0) bit_start += 32;
    if(bit_end < 0) bit_end += 32;
    if(bit_start > bit_end){
        int temp = bit_start;
        bit_start = bit_end;
        bit_end = temp;
    }

    for(int i = bit_start; i <= bit_end; i++) mask |= 1 << i;

    return mask;
}