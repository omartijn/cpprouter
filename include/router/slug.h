#pragma once

#include <string_view>
#include <stdexcept>
#include <regex>


namespace router {

    /**
     *  Class holding the data for a slug,
     *  allowing to extract the data.
     */
    class slug
    {
        public:
            /**
             *  Constructor
             *
             *  @param  pattern The slug pattern, the pattern is consumed from the input
             */
            slug(std::string_view& pattern)
            {
                // the pattern must include the slug opening character
                if (pattern.empty() || pattern[0] != '{') {
                    // not a valid slug pattern, does not start with an opening brace
                    throw std::logic_error{ "Missing slug opening character" };
                }

                // track whether the last character was an escape character
                // and how many levels of curly braces we have entered
                bool        last_char_was_escape    { false };
                std::size_t brace_nesting_level     { 0     };

                // read all characters until we hit the slug end
                for (std::size_t i{ 0 }; i < pattern.size(); ++i) {
                    // did we find an escape character?
                    if (pattern[i] == '\\') {
                        // if the previous character was also an escape character
                        // then we escaped the escape character (to get a literal
                        // backslash), so we need to flip the state back to false
                        // otherwise it's set to true.
                        last_char_was_escape = !last_char_was_escape;
                        continue;
                    }

                    // escaped characters aren't checked, they do not change the
                    // brace nesting level, so we can skip further processing
                    if (last_char_was_escape) {
                        last_char_was_escape = false;
                        continue;
                    }

                    // did we find another curly brace character?
                    if (pattern[i] == '{') {
                        // increase nesting level
                        ++brace_nesting_level;
                    } else if (pattern[i] == '}') {
                        // decrease nesting level
                        --brace_nesting_level;
                    }

                    // did we find the final, closing slug character?
                    if (brace_nesting_level == 0) {
                        // to get the regular expression we ignore the first
                        // character from the pattern (the opening {) and the
                        // final closing character (the })
                        auto expression = pattern.substr(1, i - 1);

                        // initialize the regex and consume the data
                        _pattern.assign(begin(expression), end(expression));
                        pattern.remove_prefix(i + 1);

                        // the slug is complete, stop processing data
                        break;
                    }
                }

                // check if the slug was properly closed
                if (brace_nesting_level > 0) {
                    // there are more opening braces than closing
                    // braces, which means the pattern does not contain
                    // the closing brace, we cannot create the regex
                    throw std::logic_error{ "Unterminated slug, missing closing curly brace" };
                }
            }

            /**
             *  Match the slug against the given input
             *
             *  @param  input   The input to test
             *  @param  output  Set to the matched data
             *  @return Whether the input matched the slug pattern
             */
            bool match(std::string_view& input, std::string_view& output) const
            {
                // the match results to use
                using match_results = std::match_results<std::string_view::const_iterator>;

                // search the input and get the results
                match_results   matches {                                                                   };
                bool            matched { std::regex_search(begin(input), end(input), matches, _pattern)    };

                // did we find a match and did it occur at the start of the input?
                if (matched && matches.position(0) == 0) {
                    // store the match in the output
                    output = input.substr(0, matches.length(0));

                    // consume the matched input
                    input.remove_prefix(output.size());
                    return true;
                }

                // no match found, or the match was not at the start of the input
                return false;
            }

            /**
             *  Find the beginning of slug data
             *  in a given (sub)path.
             *
             *  @param  path    The path to search in
             *  @return index inside the path, or std::string_view::npos
             */
            constexpr static std::size_t find_start(std::string_view path) noexcept
            {
                // the iterator to the slug inside the path, and the position to continue our search
                // find the slug character inside the path
                auto position = path.find_first_of('{');

                // keep going until we find a match or run out of data
                while (position != std::string_view::npos) {
                    // if the match is the first character in the path
                    // there can be no escape character before it
                    if (position == 0) {
                        break;
                    }

                    // check whether the match is not escaped by a backslash
                    if (path[position - 1] != '\\') {
                        break;
                    }

                    // the slug point we found was escaped by a backslash,
                    // so we search the remainder of the path
                    position = path.find_first_of('{', position + 1);
                }

                // return the position of the slug character
                return position;
            }
        private:
            std::regex  _pattern;   // the pattern to match for this slug
    };

}
