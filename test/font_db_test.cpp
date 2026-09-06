#include <catch2/catch_test_macros.hpp>
#include "font_db.h"
#include "config.h"

TEST_CASE("FontDb::Get returns the same options pointer twice", "[font_db]") {
    Config::Settings cfg;
    cfg.load_system_fonts = false;  // skip costly system font load in unit test
    auto* a = FontDb::Get(cfg);
    auto* b = FontDb::Get(cfg);
    REQUIRE(a != nullptr);
    REQUIRE(a == b);  // singleton
}
