#pragma once

#include <header/aystl/algorithm.hxx>

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <functional>
#include <sstream>

namespace test::aystl::algorithm {

    struct TestFind_nth_ofData
    {   struct input {
            std::vector<
                /* Test Parameters */ std::tuple<
                        /* Haystack    */ std::vector<std::string>
                      , /* Needle      */ std::string
                      , /* Nth Needle  */ std::size_t
                      , /* Start Index */ std::size_t
            >   > params;
        };
        struct output {
            std::vector
            <   /* Expected Found Index */ std::size_t
            > indices;
        };

        input input; output output;
    };

    bool TestFind_nth_of ( TestFind_nth_ofData const& data, std::function<void(std::stringstream const&)> const& errorLogger );

}