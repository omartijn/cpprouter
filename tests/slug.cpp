#include <router/slug.h>

#include <catch2/catch_all.hpp>

TEST_CASE("slugs should correctly detect start and end", "[slug]") {
    SECTION("slug with invalid starting character") {
        std::string_view pattern{"\\d}"};
        REQUIRE_THROWS_AS(router::slug{pattern}, std::logic_error);
    }

    SECTION("unterminated slug data") {
        std::string_view pattern{"{\\d"};
        REQUIRE_THROWS_AS(router::slug{pattern}, std::logic_error);
    }

    SECTION("slug-only data") {
        std::string_view pattern{"{\\d}"};
        router::slug slug{pattern};

        REQUIRE(pattern.empty());
    }

    SECTION("nested curly braces") {
        std::string_view pattern{"{\\d{2}}"};
        router::slug slug{pattern};

        REQUIRE(pattern.empty());
    }

    SECTION("slug with trailing suffix") {
        std::string_view pattern{"{\\d}/test"};
        router::slug slug{pattern};

        REQUIRE(pattern == "/test");
    }
}
