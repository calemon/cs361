/*
*    Lab 1
*    Casey Lemon
*    January 17, 2019
*
*    Program needs to implement four functions (Test, Set, Clear, Mask), that
*    manipulate the bits of a 32-bit unsigner number.
*/

#include "lab1.h"
// #include <stdint.h>

bool Test(const volatile uint32_t *bitset, int bit_index){
    /* Check if the given bit_index is out of bounds */
    if(bit_index > 31 || bit_index < -32) return false;
    /* If a given bit_index is negative, translate it to the correct positive index */
    if(bit_index < 0) bit_index += 32;

    /* Right shift bitset by given index to get desired bit in first index.
    Then AND by 1 to test if a 1 or 0 is at the shifted index */
    return (((*bitset >> bit_index) & 1) == 1) ? true : false; 
}

void Set(volatile uint32_t *bitset, int bit_index){
    /* Check if the given bit_index is out of bounds */
    if(bit_index > 31 || bit_index < -32) return;
    /* If a given bit_index is negative, translate it to the correct positive index */
    if(bit_index < 0) bit_index += 32;

    /* Left shift 1 by the given bit_index then OR the given bitset with the shifted bits */
    *bitset |= 1 << bit_index;

    return;
}

void Clear(volatile uint32_t *bitset, int bit_index){
    /* Check if the given bit_index is out of bounds */
    if(bit_index > 31 || bit_index < -32) return;
    /* If a given bit_index is negative, translate it to the correct positive index */
    if(bit_index < 0) bit_index += 32;

    /* Left shift 1 by the given bit_index, and flip the result. Then AND the given bitset with the shifted bits to clear */
    *bitset &= ~(1 << bit_index);

    return;
}

constexpr uint32_t Mask(int bit_start, int bit_end){
    uint32_t mask = 0;
    /* Check if the given bit_index is out of bounds */
    if(bit_start > 31 || bit_start < -32 || bit_end > 31 || bit_end < -32) return mask;
    /* If a given bit_index is negative, translate it to the correct positive index */
    if(bit_start < 0) bit_start += 32;
    if(bit_end < 0) bit_end += 32;
    /* Swap if the given start is greater than the given end */
    if(bit_start > bit_end){
        int temp = bit_start;
        bit_start = bit_end;
        bit_end = temp;
    }

    /* Left shift 1 by the difference between bit_end and bit_start, and subtract the result by 1 .
    Then left shift the result of the subtraction by bit_start and OR with 1 left shifted by bit_end */
    mask = (((1 << (bit_end - bit_start)) - 1) << bit_start) | (1 << bit_end);

    return mask;
}
