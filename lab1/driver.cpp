#include "lab1.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

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
}

TEST_CASE( "Clear individual bit", "[bitClear]" ) {
    uint32_t bitset;
    SECTION("Setting bit at index 2") {
        bitset = 6;
        Clear(&bitset, 2);
        CHECK( bitset == 2 );
    }
}

TEST_CASE( "Get bit mask", "[bitMask]" ) {
}