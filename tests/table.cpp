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

        REQUIRE(tester.handler_invoked == true);
    }

    SECTION("invoking a free function") {
        table.add<free_callback>("/callback");

        REQUIRE(free_callback_invoked == false);

        table.route("/callback");

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
}
