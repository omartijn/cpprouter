#include "catch2.hpp"
#include <router/table.h>
#include <iostream>

static bool free_callback_invoked{ false };

void free_callback()
{
    free_callback_invoked = true;
};


TEST_CASE("paths can be matched", "[path]")
{
    // an empty table to test with
    router::table<void()>   table;

    SECTION("never route on an empty table") {
        // the table is empty, so all attempts to route
        // will yield a out of range condition
        REQUIRE_THROWS_AS(table.route("/test"), std::out_of_range);
        REQUIRE(table.routable("/test") == false);
    }

    SECTION("404 handler") {
        struct not_found_handler
        {
            void handle_404() { handler_invoked = true; }
            bool handler_invoked{ false };
        };

        not_found_handler tester;

        table.set_not_found<&not_found_handler::handle_404>(&tester);
        table.route("/wherever/not/found");
        REQUIRE(table.routable("/wherever/not/found") == false);

        REQUIRE(tester.handler_invoked == true);
    }

    SECTION("invoking a free function") {
        table.add<&free_callback>("/callback");

        REQUIRE(free_callback_invoked == false);

        table.route("/callback");
        REQUIRE(table.routable("/callback") == true);
        REQUIRE(table.routable("/some/other/path") == false);

        REQUIRE(free_callback_invoked == true);
    }

    SECTION("invoking a member function") {
        struct callback_tester
        {
            void callback1() { callback1_invoked = true; }
            void callback2() noexcept { callback2_invoked = true; }

            bool callback1_invoked { false };
            bool callback2_invoked { false };
        };

        callback_tester tester;

        table.add<&callback_tester::callback1>("/callback/1", &tester);
        table.add<&callback_tester::callback2>("/callback/2", &tester);

        REQUIRE(tester.callback1_invoked == false);
        REQUIRE(tester.callback2_invoked == false);

        table.route("/callback/1");

        REQUIRE(tester.callback1_invoked == true);
        REQUIRE(tester.callback2_invoked == false);

        table.route("/callback/2");

        REQUIRE(tester.callback1_invoked == true);
        REQUIRE(tester.callback2_invoked == true);
    }

    SECTION("invoking a member function with slug") {
        struct slug_data
        {
            std::size_t number;
            std::string slug;

            using dto = router::dto<slug_data>
                ::bind<&slug_data::number>
                ::bind<&slug_data::slug>;
        };

        struct callback_tester
        {
            void callback1(std::size_t number, std::string&& slug)
            {
                _number = number;
                _slug = std::move(slug);
            }

            void callback2(std::tuple<std::size_t, std::string>&& slugs)
            {
                _number = std::get<0>(slugs);
                _slug = std::move(std::get<1>(slugs));
            }

            void callback3(slug_data&& slugs)
            {
                _number = slugs.number;
                _slug = std::move(slugs.slug);
            }

            std::size_t number() const noexcept { return _number; }
            const std::string& slug() const noexcept { return _slug; }

            std::size_t _number;
            std::string _slug;
        };

        callback_tester tester;

        table.add<&callback_tester::callback1>("/callback1/{\\d+}/{\\w+}", &tester);
        table.add<&callback_tester::callback2>("/callback2/{\\d+}/{\\w+}", &tester);
        table.add<&callback_tester::callback3>("/callback3/{\\d+}/{\\w+}", &tester);

        table.route("/callback1/100/hello_world");

        REQUIRE(tester.number() == 100);
        REQUIRE(tester.slug() == "hello_world");

        table.route("/callback2/200/hello_callback");

        REQUIRE(tester.number() == 200);
        REQUIRE(tester.slug() == "hello_callback");
    }
}
