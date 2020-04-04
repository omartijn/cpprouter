#pragma once

#include <string_view>
#include <algorithm>
#include <optional>
#include <ostream>
#include <vector>
#include <memory>
#include "slug.h"



namespace router {

    /**
     *  A path that can be routed
     */
    class path
    {
        public:
            /**
             *  Constructor
             *
             *  @param  path    The path to route
             */
            path(std::string_view path)
            {
                // find the first slug inside the given path
                std::size_t position = slug::find_start(path);

                // does the given path contain a slug?
                if (position == std::string_view::npos) {
                    // we use the whole path as prefix, since there are no slugs
                    _prefix.assign(path);
                    return;
                }

                // extract the prefix part from the path
                _prefix.assign(path, 0, position);
                path.remove_prefix(position);

                // process all slugs given in the path
                while (!path.empty()) {
                    // parse the slug at the front of the path
                    slug                slug    { path  };
                    std::string_view    suffix  {       };

                    // find the start of the next slug (if any)
                    position = slug::find_start(path);

                    // do we have another slug available
                    if (position == std::string_view::npos) {
                        // no more slugs, use the whole path
                        std::swap(suffix, path);
                    } else {
                        // extract the part up to the next slug from the path
                        suffix  = path.substr(0, position);
                        path    = path.substr(position);
                    }

                    // store the slug for later matching
                    _edges.emplace_back(std::move(slug), suffix);
                }
            }

            /**
             *  Check whether the path matches the given input
             *
             *  @param  input   The input to test
             *  @param  slugs   Output vector to be filled with slug data
             *  @return Whether the input matches the path
             */
            bool match(std::string_view input, std::vector<std::string_view>& slugs) const
            {
                // first we check whether the input correctly begins with the prefix
                if (input.substr(0, _prefix.size()) != _prefix) {
                    // input does not start with the prefix
                    return false;
                }

                // remove the prefix from the input
                input.remove_prefix(_prefix.size());

                // allocate memory for the matched slug data
                slugs.reserve(_edges.size());

                // go over all the edges
                for (auto& [slug, suffix] : _edges) {
                    // capture the slug data
                    std::string_view output;

                    // check the slug
                    if (!slug.match(input, output)) {
                        // the slug failed to match
                        return false;
                    }

                    // check whether the input now begins with
                    // the slug suffix (which comes after a slug)
                    if (input.substr(0, suffix.size()) != suffix) {
                        // the slug suffix did not match
                        return false;
                    }

                    // remove the suffix from the input
                    input.remove_prefix(suffix.size());

                    // add the slug data to the result
                    slugs.push_back(output);
                }

                // prefix and all sludges matched
                return true;
            }
        private:
            /**
             *  An edge consists of a slug and a
             *  literal trailing suffix
             */
            using edge = std::pair<slug, std::string>;

            std::string         _prefix;    // the part of the path up to the first slug
            std::vector<edge>   _edges;
    };

}
