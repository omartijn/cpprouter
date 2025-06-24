#include <router/path.h>

#include <catch2/catch_all.hpp>

TEST_CASE("paths should correctly identify and parse slugs", "[path]") {
    // a container to hold slug matches
    std::vector<std::string_view> slugs;

    SECTION("path without any slugs") {
        // no slug start inside the part, this is a very simple path
        std::string_view slug_free{"/simple/path/without/slugs"};
        router::path path{slug_free};

        // since there are no slugs, both match_prefix
        // and match should behave exactly the same
        REQUIRE(path.match_prefix(slug_free) == true);
        REQUIRE(path.match(slug_free, slugs) == true);

        // popping a character off prevents matching, because
        // a path without slugs has the whole path as prefix
        slug_free.remove_suffix(1);

        // ensure the path no longer matches
        REQUIRE(path.match_prefix(slug_free) == false);
        REQUIRE(path.match(slug_free, slugs) == false);
    }

    SECTION("path with a simple slug") {
        // path with only a simple slug, with a very simple regex
        router::path path{"/test/{\\d+}/test"};

        // testing prefix only works when all data up to the first prefix is
        // available
        REQUIRE(path.match_prefix("/test/no-longer-inprefix") == true);
        REQUIRE(path.match_prefix("/testing/10/test") == false);

        // if the data _after_ the slug is invalid, the
        // match should still fail as well
        REQUIRE(path.match("/test/10/testing", slugs) == false);

        // check whether the regex is validated correctly
        REQUIRE(path.match("/test/ten/test", slugs) == false);
        REQUIRE(path.match("/test//test", slugs) == false);
        REQUIRE(path.match("/test/test", slugs) == false);

        // finally test correct input
        REQUIRE(path.match("/test/10/test", slugs) == true);

        // the slug should have been correctly parsed
        REQUIRE(slugs.size() == 1);
        REQUIRE(slugs[0] == "10");
    }

    SECTION("slug with embedded curly braces") {
        // this path has curly braces inside the slug regex
        router::path path{"/test/{\\d{2}}/test"};

        // this should only match on data with the correct number
        // of numbers inside the slug data
        REQUIRE(path.match("/test/10/test", slugs) == true);
        REQUIRE(path.match("/test/1/test", slugs) == false);
    }
}
