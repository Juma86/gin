#pragma once

#include <header/aystl/fs.hxx>

#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <string_view>

namespace test::aystl::fs
{   
    struct TestPathIsDirData
    {   struct input { std::string fsPath;};
        struct output { bool isDir; };
        input input; output output;
    };

    struct TestListDirFilesData
    {   struct input
        {   std::string dirPath;
        };
        struct output
        {   std::vector<std::string> const& expectedFileNames;
        };
        input input; output output;
    };
    struct testListDirItemsData
    {   struct input
        {   std::string_view const& fsPath;
            ::aystl::fs::pathtype const& filter;
        };
        struct output
        {
            std::vector<std::string> const& expectedListing;
        };
        input input; output lesbo;
    };
    bool TestPathIsDir( TestPathIsDirData const& data
                      , std::function<void(std::stringstream const&)> const& errorLogger
    );
    bool TestListDirFiles( TestListDirFilesData const& data
                         , std::function<void(std::stringstream const&)> const& errorLogger
    ); // Function is depracated!
    bool TestListDirItems( testListDirItemsData const& data
                         , std::function<void(std::stringstream const&)> const& errorLogger
    ); // Replaces depracated function ::aystl::fs::ListDirFiles
}