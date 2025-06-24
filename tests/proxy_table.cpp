#include <router/table.h>

#include <catch2/catch_all.hpp>

TEST_CASE("paths and methods can be matched", "[path-method]") {
    // the methods to support
    enum class method { get, put, post };

    // an empty table to test with
    router::table<router::proxy<void(), method::get, method::put, method::post>>
        table;

    SECTION("never route on an empty table") {
        // the table is empty, so all attempts to route
        // will yield a out of range condition
        REQUIRE_THROWS_AS(table.route("/test", method::get), std::out_of_range);
        REQUIRE(table.routable("/test") == false);
        REQUIRE(table.has_not_found_handler() == false);
    }

    SECTION("404 handler") {
        struct not_found_handler {
            void handle_404() { handler_invoked = true; }
            bool handler_invoked{false};
        };

        not_found_handler tester;

        table.set_not_found<&not_found_handler::handle_404>(&tester);
        table.route("/wherever/not/found", method::get);
        REQUIRE(table.routable("/wherever/not/found") == false);
        REQUIRE(table.has_not_found_handler() == true);

        REQUIRE(tester.handler_invoked == true);
    }

    SECTION("missing method on valid route") {
        struct callback_tester {
            void handle_404() { not_found_invoked = true; }
            bool not_found_invoked{false};

            void handle_405() { method_not_allowed = true; }
            bool method_not_allowed{false};

            void handle_get() { get_invoked = true; }
            bool get_invoked{false};
        };

        callback_tester tester;

        table.add("/callback")
            .set<method::get, &callback_tester::handle_get>(&tester);

        table.set_not_found<&callback_tester::handle_404>(&tester);
        table.route("/callback", method::post);
        REQUIRE(table.routable("/callback") == true);
        REQUIRE(table.has_not_found_handler() == true);

        REQUIRE(tester.not_found_invoked == true);

        // now set up a handler for missing method specifically
        table.set_not_proxied<&callback_tester::handle_405>(&tester);
        table.route("/callback", method::post);

        REQUIRE(tester.method_not_allowed == true);
    }

    SECTION("route call to correct method") {
        struct callback_tester {
            void handle_404() { not_found_invoked = true; }
            bool not_found_invoked{false};

            void handle_405() { method_not_allowed = true; }
            bool method_not_allowed{false};

            void handle_get() { get_invoked = true; }
            bool get_invoked{false};

            void handle_post() { post_invoked = true; }
            bool post_invoked{false};
        };

        callback_tester tester;

        table.add("/callback")
            .set<method::get, &callback_tester::handle_get>(&tester)
            .set<method::post, &callback_tester::handle_post>(&tester);

        table.set_not_found<&callback_tester::handle_404>(&tester);
        table.route("/callback", method::put);
        REQUIRE(table.routable("/callback") == true);
        REQUIRE(table.has_not_found_handler() == true);

        REQUIRE(tester.not_found_invoked == true);

        // now set up a handler for missing method specifically
        table.set_not_proxied<&callback_tester::handle_405>(&tester);
        table.route("/callback", method::put);

        REQUIRE(tester.method_not_allowed == true);

        table.route("/callback", method::post);
        REQUIRE(tester.post_invoked == true);
    }
}
