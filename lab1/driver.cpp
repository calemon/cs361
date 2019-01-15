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
}