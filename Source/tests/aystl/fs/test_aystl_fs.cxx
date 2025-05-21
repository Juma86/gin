#include <header/tests/aystl/fs/test_aystl_fs.hxx>

#include <cstring>
#include <iostream>

bool test::aystl::fs::TestPathIsDir( test::aystl::fs::TestPathIsDirData const& data
                    , std::function<void(std::stringstream const&)> const& errorLogger
) { auto const& res
    {   ::aystl::fs::PathIsDir(data.input.fsPath)
    };
    if(! res.has_value())
    {   errorLogger(std::stringstream()
            << "::aystl::fs::PathIsDir : Returned std::nullopt"
               ", perhaps the path doesn't exist?\n\r\t"
               " path : "
            << data.input.fsPath
        );
        return false;
    }
    if(res != data.output.isDir)
    {   errorLogger(std::stringstream()
            << "::aystl::fs::PathIsDir : Mismatch between returned value and expected value" 
            << " : path = " << data.input.fsPath
            << " {" << res.value() << "(returned) vs " << data.output.isDir << "(expected)}"
        );
        return false;
    }
    return true;
}
bool test::aystl::fs::TestListDirFiles( test::aystl::fs::TestListDirFilesData const& data
                        , std::function<void(std::stringstream const&)> const& errorLogger
) { try {
        auto const& listDirFilesReturn
        {   ::aystl::fs::ListDirFiles ( data.input.dirPath )
        };
        if(! listDirFilesReturn.has_value())
        {   errorLogger( std::stringstream()
                << "::aystl::fs::ListDirFiles : Failed to return non-nullopt return value!\n\r\t"
                << "path = " << data.input.dirPath
            );
            return false;
        }
        if(listDirFilesReturn.value().size() != data.output.expectedFileNames.size())
        {   errorLogger( std::stringstream()
                << "aystl::fs::ListDirFiles : Different count of files have been returned than expected"
                << " : "
                << listDirFilesReturn.value().size()
                << " vs "
                << data.output.expectedFileNames.size()
            );
            return false;
        }
        std::size_t i = -1;
        for ( auto const& dirFile : listDirFilesReturn.value() )
        {   errorLogger(std::stringstream() << "dirFile = " << dirFile);
            if( std::strcmp(dirFile.data(), data.output.expectedFileNames.at(++i).data()))
            {   errorLogger( std::stringstream()
                    << "::aystl::fs::ListDirFiles : Mismatch between returned value and expected value"
                    << " : "
                    << dirFile.data()
                    << " vs "
                    << data.output.expectedFileNames.at(i).data()
                );
                std::cout << "Them" << std::endl;
                return false;
            }
        }
        std::cout << "Us" << std::endl;
        return true;
    } catch (std::exception& e) {
        errorLogger( std::stringstream()
            << "::aystl::fs::ListDirFiles : Caused an exception : "
            << e.what()
        );
        return false;
    }

    //Should be unreachable
    return false;
}
bool test::aystl::fs::TestListDirItems( test::aystl::fs::testListDirItemsData const& data
                        , std::function<void(std::stringstream const&)> const& errorLogger
) { auto const& listDirItemsReturn
    {   ::aystl::fs::ListDirItems( ::aystl::string {data.input.fsPath.data()}, data.input.filter)
    };
    if(! listDirItemsReturn.has_value())
    {   errorLogger( std::stringstream()
            << "::aystl::fs::ListDirItems : Failed to return non-nullopt return value!"
        );
        return false;
    }
    std::size_t i = -1;
    for(auto const& dirItem : listDirItemsReturn.value())
    {   if( std::strcmp(dirItem.data(), data.lesbo.expectedListing.at(++i).data()))
        {   errorLogger( std::stringstream()
                << "::aystl::fs::ListDirItems : Mismatch between returned value and expected value"
                << " : "
                << dirItem.data()
                << " vs "
                << data.lesbo.expectedListing.at(i).data()
            );
            return false;
        }
    }
    return true;
}
