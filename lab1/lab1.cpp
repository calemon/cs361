#include <cstdint>

bool Test(const volatile uint32_t *bitset, int bit_index){
    return false;
}

void Set(volatile uint32_t *bitset, int bit_index){
    return;
}

void Clear(volatile uint32_t *bitset, int bit_index){
    return;
}

constexpr uint32_t Mask(int bit_start, int bit_end){
    return 0;
}

int compTest(){
    return 55;
}