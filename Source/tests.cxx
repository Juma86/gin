#include <header/tests.hxx>

#include <simdutf.h>
#include <string.h>

AYSTL_CMD_METHOD(handleTests, AYSTL_CMD_TOGGLETAGS_NAME, AYSTL_CMD_COLLECTIONTAGS_NAME,)
{

    if(! AYSTL_CMD_TOGGLETAGS_NAME.at("--enable-testing"))
        { return false; }

    if( AYSTL_CMD_COLLECTIONTAGS_NAME.at("--test-resources-dir").size() != 1 )
    {   std::cerr << "A single test resources directory must be specified to perform upon: Fatal." << std::endl;

        return false;
    }

    auto const testResourcesDir { AYSTL_CMD_COLLECTIONTAGS_NAME.at("--test-resources-dir").at(0).c_str() };

    BS::thread_pool testerThreads ( std::thread::hardware_concurrency() );
    std::vector<std::future<std::pair<std::string,std::optional<std::string>>>> testerReturns; // std::nullopt on success, set on fail!

    std::mutex ConIO;

    auto const& el
    {   [&ConIO](std::stringstream const& errLog
        ) -> void {
            std::lock_guard<std::mutex>_{ConIO};
            std::cerr << errLog.str() << std::endl;
        }
    };

    testerReturns.emplace_back( testerThreads.submit_task( [&] () -> std::pair<std::string,std::optional<std::string>>
        {   bool const res
            {   test::aystl::dbr::TestDump( test::aystl::dbr::TestDumpData
                {   .input
                    {   .filePath {(std::stringstream() << testResourcesDir << "/aystl/dbr/WriteDisk/test.dbr").str()}
                      , .data     {"Hello, World! This is some test data"}
                } , .output
                    {   .data {"HyMA+B0JNq5wT0d6+XY1V21yGULG1oIMoskt2oZgPLqJO3V5rCaNAA=="}
            }   }   )   };
            constexpr const auto testFunctionName
            {   "::aystl::dbr::WriteDisk"
            };
            return res
              ? std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( std::nullopt ) }
              : std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( "Failed!"    ) }
            ;
    }   )   );

    testerReturns.emplace_back( testerThreads.submit_task( [&] () -> std::pair<std::string,std::optional<std::string>>
        {   bool const res
            {   test::aystl::dbr::TestLoad ( test::aystl::dbr::TestLoadData
                {   .input
                    {   .filePath {(std::stringstream() << testResourcesDir << "/aystl/dbr/ReadDisk/test.dbr").str()}
                } , .output
                    {   .data {"Hello world Hello world Hello world : Testing121212"}
            }   }, el )   };
            constexpr const auto testFunctionName
            {   "::aystl::dbr::ReadDisk"
            };
            return res
              ? std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( std::nullopt ) }
              : std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( "Failed!"    ) }
            ;
    }   )   );

    testerReturns.emplace_back( testerThreads.submit_task( [&] () -> std::pair<std::string,std::optional<std::string>>
        {   bool const res
            {   test::aystl::algorithm::TestFind_nth_of (test::aystl::algorithm::TestFind_nth_ofData
                    {   .input
                        {   .params
                            {   {{"a","e","i","o","u","e","e","b","e","p"},"e",0 ,3}
                              , {{"a","a","e","c","1","6","3","9","g","l"},"1",0 ,1}
                              , {{"a","a","e","c","1","6","3","1","g","l"},"1",5 ,1}
                              , {{"a","a","e","c","1","6","3","9","g","l"},"l",0 ,1}
                              , {{"a","a","e","c","1","6","3","9","g","l"},"a",0 ,1}
                              , {{"a","a","e","c","1","6","3","9","g","l"},"a",0 ,2}
                              , {{"a","a","e","c","1","6","3","9","g","l"},"a",1 ,1}
                              , {{"a","a","e","c","1","6","3","9","g","l"},"?",0 ,1}
                              , {{"a","a","e","c","1","6","3","9","g","l"},"l",15,1}
                              , {{"a","a","e","c","1","6","3","9","g","l"},"a",0 ,9}

                    }} , .output
                        {   .indices { 6, 4, 7, 9, 0, 1, 1, 10, 10, 10 }
            }   }, el )   };
            constexpr const auto testFunctionName
            {   "::aystl::algorithm::find_nth_of"
            };
            return res
              ? std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( std::nullopt ) }
              : std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( "Failed!"    ) }
            ;
    }   )   );


    testerReturns.emplace_back( testerThreads.submit_task( [&] () -> std::pair<std::string,std::optional<std::string>>
        {   bool res { true };
            auto testParameterCollection
            {   std::vector<::test::aystl::fs::TestPathIsDirData>{
                    ::test::aystl::fs::TestPathIsDirData
                    {   .input
                        {   .fsPath {std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/PathIsDir/A"}
                        },
                        .output
                        {   .isDir {true}
                    }   },
                    ::test::aystl::fs::TestPathIsDirData
                    {   .input
                        {   .fsPath {std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/PathIsDir/B"}
                        },
                        .output
                        {   .isDir {false}
                    }   },
                    ::test::aystl::fs::TestPathIsDirData
                    {   .input
                        {   .fsPath {std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/PathIsDir/C"}
                        },
                        .output
                        {   .isDir {true}
                    }   },
                    ::test::aystl::fs::TestPathIsDirData
                    {   .input
                        {   .fsPath {std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/PathIsDir/D"}
                        },
                        .output
                        {   .isDir {false}
                        }
                    }
        }   };
        for(auto const& item : testParameterCollection)
        {
            auto const& testResult = test::aystl::fs::TestPathIsDir( item, el );
            if(! testResult)
            {   res = false;
                break;
            }
        }
        constexpr const auto testFunctionName
        {   "::aystl::test::fs::PathIsDir"
        };
        return res
          ? std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( std::nullopt ) }
          : std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( "Failed!"    ) }
        ;
    }   )   );
/*
    testerReturns.emplace_back( testerThreads.submit_task( [&] () -> std::pair<std::string,std::optional<std::string>>
        {   bool res {true};
            std::vector<::test::aystl::fs::TestListDirFilesData> testsParams
            {   ::test::aystl::fs::TestListDirFilesData
                {   .input { .dirPath { std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/ListDirFiles/Case-1/" } }
                  , .output {  .expectedFileNames { "FileA", "FileB", "FileC" }}
                }
              , ::test::aystl::fs::TestListDirFilesData
                {   .input { .dirPath { std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/ListDirFiles/Case-2/" } }
                  , .output { .expectedFileNames { "SomeFile" } }
                }
              , ::test::aystl::fs::TestListDirFilesData
                {   .input { .dirPath { std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/ListDirFiles/Case-3" } }
                  , .output { .expectedFileNames {""} }
                }
              , ::test::aystl::fs::TestListDirFilesData
                {   .input { .dirPath { std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/ListDirFiles/Case-4" } }
                  , .output { .expectedFileNames {""} }
                }
              , ::test::aystl::fs::TestListDirFilesData
                {   .input { .dirPath { std::string(testResourcesDir,strlen(testResourcesDir)) + "/aystl/fs/ListDirFiles/Case-5" } }
                  , .output { .expectedFileNames { "filee" } }
                }
            };
            for (auto const& testParams : testsParams)
            {   el(std::stringstream()<<"scary -> "<<testParams.input.dirPath);
                auto const& testResult = test::aystl::fs::TestListDirFiles( testParams, el );
                if(! testResult)
                {   res = false;
                    break;
                }
            }
            constexpr const auto testFunctionName
            {   "::aystl::test::fs::ListDirFiles"
            };
            return res
              ? std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( std::nullopt ) }
              : std::pair<std::string,std::optional<std::string>> { testFunctionName, std::optional<std::string> ( "Failed!"    ) }
            ;
    }   )   );*/

    std::vector<std::string> failureMessages;
    std::size_t const desiredSuccesses
    {   testerReturns.size()
    };
    std::vector<bool> const testsIsSuccessful
    {   aystl::algorithm::apply_mutable(testerReturns, [&failureMessages](
            std::future<std::pair<std::string, std::optional<std::string>>>& testerIsSuccessFuture
            ) -> bool {
                auto const& testResult
                {   testerIsSuccessFuture.get()
                };
                bool const success
                {   ! testResult.second.has_value()
                };
                if(! success)
                {   failureMessages.emplace_back( ( std::stringstream()
                        << testResult.first
                        << " : " << testResult.second.value()
                    ) .str() );
                }
                return success;
    }   )   };
    for (auto const& failureMessage : failureMessages)
    {   std::cerr << failureMessage << std::endl;
    }
    std::size_t const successesCount { desiredSuccesses - failureMessages.size() };
    std::cout << successesCount << " of " << desiredSuccesses  << " tests passed!" << std::endl;

    return failureMessages.size() == 0;
}