#include "lab1.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

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

TEST_CASE( "Testing individual bit", "[bitTest]" ) {
    uint32_t bitset = 2;
    CHECK( Test(&bitset, 1) == true );
    CHECK( Test(&bitset, 7) == false );
    CHECK( Test(&bitset, -31) == true);
    
    bitset = 38;
    CHECK( Test(&bitset, 0) == false );
    CHECK( Test(&bitset, 1) == true );
    CHECK( Test(&bitset, 2) == true );
    CHECK( Test(&bitset, 3) == false );
    CHECK( Test(&bitset, 4) == false );
    CHECK( Test(&bitset, 5) == true );
    CHECK( Test(&bitset, -30) == true);
    CHECK( Test(&bitset, -1) == false);

    bitset = 13;
    CHECK( Test(&bitset, -32) == true );
    CHECK( Test(&bitset, -31) == false );
    CHECK( Test(&bitset, -30) == true );
    CHECK( Test(&bitset, -29) == true );
    CHECK( Test(&bitset, 35) == false );
    CHECK( Test(&bitset, 32) == false );
    CHECK( Test(&bitset, -33) == false );
    CHECK( Test(&bitset, -4353) == false );
    CHECK( Test(&bitset, 0) == true );
    CHECK( Test(&bitset, 1) == false );
    CHECK( Test(&bitset, 2) == true );
    CHECK( Test(&bitset, 3) == true );


}

TEST_CASE( "Set individual bit", "[bitSet]" ) {
    uint32_t bitset;

    bitset = 0;
    SECTION("bitset = 0, setting bit at index 2") {
        Set(&bitset, 2);
        CHECK( bitset == 4 );
    }

    bitset = 4;
    SECTION("bitset = 4, setting bit at index 1") {
        Set(&bitset, 1);
        CHECK( bitset == 6 );
    }

    bitset = 13;
    SECTION("bitset = 13, setting bit at index 1") {
        Set(&bitset, 1);
        CHECK( bitset == 15 );
    }

    bitset = 45;
    SECTION("bitset = 45, setting bit at index 4") {
        Set(&bitset, 4);
        CHECK( bitset == 61 );
    }

    bitset = 2147483648;
    SECTION("bitset = 2147483648, setting bit at index -31") {
        Set(&bitset, -32);
        CHECK( bitset == 2147483649 );
    }

    bitset = 2147483648;
    SECTION("bitset = 2147483648, setting bit at index -28") {
        Set(&bitset, -29);
        CHECK( bitset == 2147483656 );
    }

    bitset = 24;
    SECTION("bitset = 24, setting bit at index 35") {
        Set(&bitset, 35);
        CHECK( bitset == 24 );
    }

    bitset = 34;
    SECTION("bitset = 34, setting bit at index 32") {
        Set(&bitset, 32);
        CHECK( bitset == 34 );
    }

    bitset = 98;
    SECTION("bitset = 98, setting bit at index -128") {
        Set(&bitset, -128);
        CHECK( bitset == 98 );
    }
}

TEST_CASE( "Clear individual bit", "[bitClear]" ) {
    uint32_t bitset;

    bitset = 6;
    SECTION("bitset = 6, clearing bit at index 2") {
        Clear(&bitset, 2);
        CHECK( bitset == 2 );
    }

    bitset = 22;
    SECTION("bitset = 22, clearing bit at index 4") {
        Clear(&bitset, 4);
        CHECK( bitset == 6 );
    }

    bitset = 17;
    SECTION("bitset = 17, clearing bit at index -32") {
        Clear(&bitset, -32);
        CHECK( bitset == 16 );
    }

    bitset = 53452;
    SECTION("bitset = 53452, clearing bit at index -25") {
        Clear(&bitset, -25);
        CHECK( bitset == 53324 );
    }

    bitset = 24;
    SECTION("bitset = 24, clearing bit at index 35") {
        Clear(&bitset, 35);
        CHECK( bitset == 24 );
    }

    bitset = 34;
    SECTION("bitset = 34, clearing bit at index 32") {
        Clear(&bitset, 32);
        CHECK( bitset == 34 );
    }

    bitset = 98;
    SECTION("bitset = 98, clearing bit at index -128") {
        Clear(&bitset, -128);
        CHECK( bitset == 98 );
    }
}

TEST_CASE( "Get bit mask", "[bitMask]" ) {
    CHECK(Mask(2, 5) == 60);
    CHECK(Mask(5, 2) == 60);
    CHECK(Mask(-30, -27) == 60);
    CHECK(Mask(-27, -30) == 60);
    CHECK(Mask(2, -27) == 60);
    CHECK(Mask(5, -30) == 60);
    CHECK(Mask(10, 2) == 2044);
    CHECK(Mask(2, 10) == 2044);
    CHECK(Mask(-22, -30) == 2044);
    CHECK(Mask(-30, -22) == 2044);
    CHECK(Mask(-22, 2) == 2044);
    CHECK(Mask(-30, 10) == 2044);
}

TEST_CASE( "All function test", "[all]" ) {
    uint32_t bitset;

    SECTION("Test(), Test(), Clear(), Set()") {
        bitset = 39;
        CHECK(Test(&bitset, 2) == true);
        CHECK(Test(&bitset, 3) == false);
        Clear(&bitset, 5);
        CHECK(bitset == 7);
        Set(&bitset, 8);
        CHECK(bitset == 263);
    }

    SECTION("Mask(), Clear(), Clear(), Set(), Test()") {
        bitset = Mask(3,9);
        CHECK(bitset == 1016);
        Clear(&bitset, 5);
        CHECK(bitset == 984);
        Clear(&bitset, 7);
        CHECK(bitset == 856);
        Set(&bitset, 2);
        CHECK(bitset == 860);
        CHECK(Test(&bitset, 5) == false);
    }
}