#include "catch2.hpp"
#include <router/variables.h>

TEST_CASE("slugs should parse correctly into structs")
{
    std::vector<std::string_view> slugs { "abc", "10", "def" };

    SECTION("parse slugs into struct") {
        struct dto_struct
        {
            std::string         field1;
            std::size_t         field2;
            std::string_view    field3;

            using dto = router::dto<dto_struct>
                ::bind<&dto_struct::field1>
                ::bind<&dto_struct::field2>
                ::bind<&dto_struct::field3>;
        };

        dto_struct output{};

        router::to_dto(slugs, output);

        REQUIRE(output.field1 == "abc");
        REQUIRE(output.field2 == 10);
        REQUIRE(output.field3 == "def");
    }

    SECTION("parse slugs into tuple") {
        std::tuple<std::string_view, std::size_t, std::string> output{};

        router::to_dto(slugs, output);

        REQUIRE(std::get<0>(output) == "abc");
        REQUIRE(std::get<1>(output) == 10);
        REQUIRE(std::get<2>(output) == "def");
    }
}
