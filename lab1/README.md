# Lab 1

## Overview
Three operations are commonly used to program CPU and I/O registers: Test, Set, and Clear.

## Assignment
You will program the following functions:

```cpp
bool Test(const volatile uint32_t *bitset, int bit_index);
void Set(volatile uint32_t *bitset, int bit_index);
void Clear(volatile uint32_t *bitset, int bit_index);
constexpr uint32_t Mask(int bit_start, int bit_end);
```

Do not use any library functions when programming these functions. All of these functions must be standalone, as it would be in an operating system environment.

### Bit Indices
The bit index is either negative or positive. If the bit index is positive, it will start from the least significant bit (bit index #0). If the bit index is negative, it will be from the most significant bit (bit index #32). For example, bit -1 will be 32 - 1 = bit 31. Notice that 0 is considered positive. So, the first negative bit that is possible is -1.

### Test
Test will take a 32-bit bitset and a single bit index. Test will return false if that given bit is 0 or true if that given bit is 1.

### Set
Set will take a 32-bit bitset and a single bit index. Set will set the given bit index to 1 of the original bitset.

### Clear
Clear will take a 32-bit bitset and a single bit index. Clear will clear the given bit index to 0 of the original bitset.

### Mask
Mask will generate a "mask" of binary 1s from bit_start to bit_end. bit_start and bit_end are relative to little-endian byte order. You must handle the condition where positive bit_start is less than positive bit_end or the opposite if the indices are negative. If bit_start is equal to bit_end, then it will only set a single bit. You may NOT use any loops to generate the pattern. The bit index bit_start down to the bit index of bit_end will be binary 1s. Everything else will be binary 0s. All bit indices are inclusive.

For example, Mask(10, 2) will return the following 32-bit mask:
```
0b0000_0000_0000_0000_0000_0111_1111_1100   (0x0000_07fc)
```
You must ensure that Mask(10, 2), Mask(2, 10), Mask(-22, -30), Mask(-30, -22), Mask(-22, 2), and Mask(-30, 10) all return the same bit pattern shown above.

## Examples
```cpp
uint32_t bitset = 0;
Set(&bitset, 2);
//Bitset is now the value 4
Set(&bitset, 1)
//Bitset is now the value 6
Clear(&bitset, 2);
//Bitset is now the value 2

//The following prints 1 (true)
cout << Test(&bitset, 2) << '\n';
//The following prints 0 (false)
cout << Test(&bitset, 7) << '\n';
//The following prints 0x000007fc
cout << hex << Mask(-22, 2) << '\n';
//The following prints 0xfffffff0
cout << hex << Mask(31, 4) << '\n';
```

## Compiling
To compile your lab, you must compile with a newer C++ toolchain:
```bash
/home/smarz1/Programs/x86_64/bin/g++ -Wall -std=c++17 -g -O0 -o lab1 lab1.cpp
```

## Submission
Do NOT submit your int main(). If you have one that you've tested your code with, comment it out or delete it.

Submit your .cpp file.
