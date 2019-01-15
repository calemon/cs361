#include <stdint.h>
bool Test(const volatile uint32_t *bitset, int bit_index);
void Set(volatile uint32_t *bitset, int bit_index);
void Clear(volatile uint32_t *bitset, int bit_index);
constexpr uint32_t Mask(int bit_start, int bit_end);
int compTest();