#pragma once

#include <string_view>
#include <functional>
#include <sstream>

#include <header/aystl/dbr.hxx>

namespace test::aystl::dbr {

    struct TestDumpData {
        struct input  { std::string_view const& filePath; std::string_view const& data; };
        struct output { std::string_view const& data; };

        input input; output output;
    };

    struct TestLoadData {
        struct input  { std::string_view const& filePath; };
        struct output { std::string_view const& data; };

        input input; output output;
    };

    bool TestDump( TestDumpData const& data );
    bool TestLoad( TestLoadData const& data, std::function<void(std::stringstream const&)> const& errorLogger );

}