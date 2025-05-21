#include <header/uninstall.hxx>

#ifndef DEBUG
#define DEBUG false
#endif

size_t strlen ( char const * string );

AYSTL_CMD_METHOD(handleRemove,AYSTL_CMD_TOGGLETAGS_NAME,AYSTL_CMD_COLLECTIONTAGS_NAME,AYSTL_CMD_RESOURCES_NAME)
{

    auto const& collectionPackages { collectionTags.at("--packages") };

    auto const& toggleQuiet { toggleTags.at("--quiet") };
    auto const& toggleEmbed { toggleTags.at("--embed") };

    if ( collectionPackages.size() == 0 )
    {   if (! (toggleQuiet || toggleEmbed))
        {   std::cout << "No packages specified!" << std::endl;
            return true;
    }   }


    if(! (toggleQuiet || toggleEmbed))
    {   std::cout << "Attempting to remove the following packages:";
        for (auto const& packageName : collectionPackages)
        {   std::cout << "\n\r\t" << packageName;
    }   }
    std::cout << std::endl;

    // put in check to make sure same package isnt attempted to b removed tweice (remove duplacates in command line invocation, stops bugs!!!)
    std::vector<bool> results;
        for (const auto& packageName : collectionPackages)
        {   results.emplace_back( [packageName, &resources, &AYSTL_CMD_TOGGLETAGS_NAME] () -> bool {
                {
                    std::cout << "removing package name : " << packageName << std::endl;
                }
                std::string const& packageRegistryPath = std::any_cast<std::string>
                    ( resources.at("registry:packages") );
                /*std::unique_ptr<char[]> packageUninstallDataPath ( new char [
                    strlen(packageRegistryPath.data())
                  + strlen(packageName.data())
                  + strlen("/uninstall.muc")
                ]   );*/
                std::string const& packageRegistryDirectoryPath
                {   (   std::stringstream()
                            << std::any_cast<std::string>(resources.at("registry:packages"))
                            << "/" << packageName << "/"
                ).str() };
                if (! std::filesystem::exists( packageRegistryDirectoryPath ) )
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {   std::cout << packageName << " not found in package registry; are you sure it's installed?" << std::endl;
                    }
                    return false;
                }
                std::string const& packageRegistryMetaInfoFilePath
                {   (   std::stringstream()
                            <<  std::any_cast<std::string>(resources.at("registry:packages"))
                            << "/" << packageName << "/info.json"
                ).str() };
                FILE * packageMetaInfoHandle { fopen( packageRegistryMetaInfoFilePath.data(), "rb" ) };
                if(nullptr == packageMetaInfoHandle)
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {   std::cerr
                            << "Failed to open package "
                               "meta-info file: Fatal."
                            << std::endl;
                    }
                    return false;
                }
                if(0 != fseek(packageMetaInfoHandle,0,SEEK_END))
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {   std::cerr
                            << "Failed to seek to end of "
                               "package entry meta-info file: Fatal."
                            << std::endl;
                    }
                    return false;
                }
                decltype(ftell(packageMetaInfoHandle)) const fileLength { ftell(packageMetaInfoHandle) };
                if(fileLength<0)
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {   std::cerr
                            << "Failed to get file length "
                               "from package entry meta-info file: Fatal."
                        << std::endl;
                    }
                    if(0 != fclose(packageMetaInfoHandle))
                    {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                        {   std::cerr
                                << "Failed to close package "
                                   "registry meta-info file: Fatal."
                                << std::endl;

                        }
                        return false;
                    }
                    return false;
                }
                std::unique_ptr<muc::byte[]> packageMetaInfoContentsBuffer( new(std::nothrow) muc::byte [fileLength] );
                if(nullptr == packageMetaInfoContentsBuffer)
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {   std::cerr
                            << "Failed to allocated memory for "
                               "package entry meta-info file contents: Fatal."
                            << std::endl;
                    }
                    if(0 != fclose(packageMetaInfoHandle))
                    {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                        {   std::cerr
                                << "Failed to close package entry "
                                   "meta-info file handle while allocating "
                                   "memory for file contents: Fatal."
                            << std::endl;
                        }
                        return false;
                    }
                    return false;
                }
                if(0 != fseek(packageMetaInfoHandle, 0, SEEK_SET))
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {   std::cerr
                            << "Failed to seek to beginning of "
                               "package entry meta-info file: Fatal."
                            << std::endl;
                    }
                    if(0 != fclose(packageMetaInfoHandle))
                    {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                        {   std::cerr
                                << "Failed to close package "
                                   "entry meta-info file: Fatal."
                                << std::endl;
                        }
                        return false;
                    }
                    return false;
                }
                if( fileLength != (long) fread(packageMetaInfoContentsBuffer.get(), 1, fileLength, packageMetaInfoHandle))
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {   std::cerr
                            << "Failed to read contents "
                               "of package entry meta-info "
                               "file: Fatal."
                            << std::endl;
                    }
                    if(0 != fclose(packageMetaInfoHandle))
                    {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                        { std::cerr
                                << "Failed to close file handle of package "
                                   "entry meta-info file while trying to read "
                                   "it's contents: Fatal."
                                << std::endl;
                        }
                        return false;
                    }
                    return false;
                }
                if(0 != fclose(packageMetaInfoHandle))
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {   std::cerr
                            << "Failed to close file "
                               "handle for package entry meta-info: Fatal."
                            << std::endl;
                    }
                    return false;
                }
                simdjson::padded_string const& packageMetaInfoContentsPadded
                {   simdjson::padded_string(
                        std::string_view(
                            (const char *) packageMetaInfoContentsBuffer.get()
                          , fileLength
                )   )   };
                simdjson::ondemand::parser packageMetaInfoJsonParser;
                simdjson::ondemand::document packageMetaInfoDocument
                {   packageMetaInfoJsonParser.iterate(packageMetaInfoContentsPadded)
                };  
                std::string_view const uninstallSubPath
                {   packageMetaInfoDocument["package"]["uninstall"].get_string().take_value()
                };
                // Will change post-NEA to support a custom package entry file structure as defined by info.json within each package :) -- Halfway through :)
                std::string const& packageUninstallDataFilePath { ( std::stringstream()
                    << packageRegistryPath << "/"
                    << packageName         << "/"
                    << uninstallSubPath
                ).str() };
                if (DEBUG) {
                    std::cout << "package uninstall data path : " << packageUninstallDataFilePath << std::endl;
                }
                auto const& uninstallerMetafiles
                {   muc::GetMucSubFileNamesFromDisk(packageUninstallDataFilePath)
                };
                if(! uninstallerMetafiles.has_value())
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {
                        std::cerr
                            << "Could not read sub-file-names from uninstaller file:"
                               "\n\r\tCan not uninstall package -> "
                            <<  packageName
                            << std::endl;
                    }
                    return false;
                }
                std::set<std::string> expectedSubFileNames
                {   "/removelist.txt"
                  , "/runme.sh"
                };
                for (auto const& MUCSubFileName : uninstallerMetafiles.value())
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {
                        std::cout << "Processing muc sub file : " << MUCSubFileName << std::endl;
                    }
                    if( expectedSubFileNames.find(MUCSubFileName) != expectedSubFileNames.cend() )
                    {   expectedSubFileNames.erase(MUCSubFileName);
                    } else
                    {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                        {
                            std::cerr
                                << packageName
                                << ": Found unexpected "
                                   "file name: "
                                << MUCSubFileName
                                << ": Fatal."
                                << std::endl;
                        }
                        return false;
                    }
                }
                if(0 != expectedSubFileNames.size())
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {
                        std::cerr
                            << "The required files needed to "
                               "complete uninstallation were "
                               "not found inside uninstall.muc: Fatal."
                            << std::endl;
                    }
                    return false;
                }
                // All and only file names that are needed are present.
                constexpr auto fileCloser
                {   [](FILE* fileHandle) -> void { fclose(fileHandle); }
                };
                FILE * uninstallerFileHandle
                {   fopen(packageUninstallDataFilePath.data(),"rb")
                };
                if(nullptr == uninstallerFileHandle)
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {
                        std::cerr
                            << "Failed to open uninstallation "
                               "data file: Fatal."
                            << std::endl;
                    }
                    return false;
                }
                constexpr const char * removeListStringFileName { "/removelist.txt" };
                auto const& removeListString
                {   muc::DissolveMUCSubFileFromFileHandleToMemory(uninstallerFileHandle, removeListStringFileName)
                };
                constexpr const char *  postRemoveScriptFileName { "/runme.sh" };
                auto const& postRemoveScript
                {   muc::DissolveMUCSubFileFromFileHandleToMemory(uninstallerFileHandle, postRemoveScriptFileName)
                };
                if (DEBUG) {
                    std::cout << "[DEBUG] : removelist following |||\n\r"
                                 "                               vvv\n\r"
                              << (const char *) removeListString.value().get() << std::endl;
                    
                    std::cout << "[DEBUG] : runme following |||\n\r"
                                 "                          vvv\n\r"
                              << (char const *) postRemoveScript.value().get() << std::endl;
                }
                const auto& LogError{ [&AYSTL_CMD_TOGGLETAGS_NAME] (std::stringstream stream)
                {   if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {
                        std::cerr << stream.str() << std::endl;
                }   }   };
                if(0 != fclose(uninstallerFileHandle))
                {   LogError( std::stringstream()
                        << "Failed to close installation "
                           "data file: Fatal."
                    );

                    return false;
                }
                if(! (removeListString.has_value()))
                {   LogError( std::stringstream()
                        << "Failed to get contents of "
                           "remove list from uninstall.muc: "
                           "Fatal."
                   );

                   return false;
                }
                if(! (postRemoveScript.has_value()))
                {   LogError( std::stringstream()
                        << "Failed to get contents of "
                           "post remove script from uninstall.muc: "
                           "Fatal."
                    );

                    return false;
                }
                // All data needed is here from this point forward.
                const auto& ProcessRemovalListLine { [&AYSTL_CMD_TOGGLETAGS_NAME, &AYSTL_CMD_RESOURCES_NAME, &packageName]( const std::string_view line
                ) -> bool {
                    if(DEBUG)
                    {   std::cout << "[DEBUG] LENGTH ::: " << line.length() << std::endl;
                        std::cout << "[DEBUG] LINE ::: " << line << std::endl;
                    }
                    std::size_t lineClassNameSplitIndex = line.find_first_of(":");
                    std::string_view
                        lineClass
                    {   line.data()
                      , lineClassNameSplitIndex
                    } , lineName
                    {   line.data()
                          + lineClassNameSplitIndex + 1 // Bypass & Ignore the colon that splits them apart
                      , line.length()
                          - lineClassNameSplitIndex -1 // for the length of the colon delimiter.
                    };
                    {
                        if (DEBUG)
                        {   std::cout << "Parsed from removal list -> " << lineClass << " | " << lineName << std::endl;
                        }
                        if(! AYSTL_CMD_TOGGLETAGS_NAME.at("--no-ask"))
                        {   std::cout
                                << "\tPress <ENTER> to remove this "
                                << ( (lineClass=="f") ? "file"
                                   : (lineClass=="d") ? "directory"
                                   : (lineClass=="x") ? "wildcard"
                                   : "[UNKNOWN-TYPE]"
                                   )
                                << ".";
                            std::cin.get();
                        }
                    }
                    if( lineClass == "d" )
                    {   int rc = rmdir( lineName.data() ); // Directory must be empty
                        if (rc) return false; 
                    } else if ( lineClass == "f" )
                    {   int rc = unlink( lineName.data() );
                        if (rc) return false;
                    } else if ( lineClass == "x" ){
                        std::vector<std::string> const& processedLineNameVector {
                            aystl::algorithm::apply(
                                lineName
                              , [&AYSTL_CMD_RESOURCES_NAME,&packageName](char const& c
                                ) -> std::string {
                                    return
                                        (c == '$') ? packageName
                                      : (c == '^') ? std::any_cast<std::string>(AYSTL_CMD_RESOURCES_NAME.at("registry:packages"))
                                      : std::string(&c,1)
                                    ;
                        }   )   };
                        std::stringstream processedLineNameSStream;
                        for(std::string const& stringSegment : processedLineNameVector)
                        {   processedLineNameSStream << stringSegment;
                        }
                        std::string const& processedLineName
                        {   processedLineNameSStream.str()
                        };
                        //{   std::cout << "[DEBUG] : Expansion ::: " << processedLineName << std::endl;
                        //    std::cin.get();
                        //}
                        const auto& remove_item
                        {   [](const char * fpath
                             , const struct stat * // sb
                             , int // typeflag
                             , struct FTW * // ftwbuf
                            ) -> int {
                                //std::cout << "[DEBUG] :  FPATH ::: " << fpath << std::endl;
                                return remove(fpath);
                                //bool r = remove(fpath);
                                //std::cout << "Success: " << ( r==0 ? "yes" : "no" ) << std::endl;
                                //return r;
                        }   };
                        int rc = nftw(processedLineName.data(),remove_item,0xFF,FTW_DEPTH|FTW_PHYS);
                        if(rc) return false;
                    } else {
                        return false;
                    }
                    return true;
                }   };
                const auto& ConvertBufferToLineVector { []( std::optional<std::unique_ptr<muc::byte[]>> const& string
                ) -> std::vector<std::string> {
                    // Good example of a place to use a facebook stl small vector :)
                    if (DEBUG)
                    {   std::cout << "[DEBUG] : string -> " << ((char const *) string. value (). get ()) << std::endl;
                    }
                    std::vector<std::string> lineVector;
                    char currentCharacter;
                    std::size_t currentCharacterIndex = -1, currentLineStartIndex = 0, currentLineLength = 0;
                    while ( (currentCharacter = *(string->get()+ ++currentCharacterIndex)) != '\0' )
                    {   if( currentCharacter == '\n' )
                        {   std::string_view currentLineText
                            {   (char *) string->get() + currentLineStartIndex
                              , currentLineLength
                            };
    
                            lineVector.emplace_back(currentLineText);
    
                            currentLineStartIndex = currentCharacterIndex+1;
                            currentLineLength = -1;
                        }
                        currentLineLength +=1;
                    }
                    // Assume 1 more line, unless line length is 0
                    if(currentLineLength <= 1)
                    {   return lineVector;
                    }
                    {   std::string_view currentLineText
                        {   (char *) string->get() + currentLineStartIndex
                          , currentLineLength // 1 for null terminal, 1 for start, 1 for 1 excess (???)
                        };
    
                        lineVector.emplace_back(currentLineText);
                    }
                    return lineVector;
                }   };
                std::vector<std::string> const& removalListLines
                {   ConvertBufferToLineVector(removeListString)
                };
                std::vector<bool> removalListLineResults;
                for (auto const& removalListLine : removalListLines)
                {   removalListLineResults.push_back(
                        ProcessRemovalListLine(removalListLine)
                );  }
                bool const& removalListSuccessful
                {   aystl::algorithm::get_first_satisfier_or(removalListLineResults,true,[](bool const&i){return!i;})
                };
                if(! removalListSuccessful)
                {   LogError( std::stringstream()
                        << "Failed to remove all paths on the removal list"
                           ", due to this, running the uninstallation script "
                           "would not be safe: Fatal."
                    );
                    return false;
                }
                std::size_t const& postRemoveScriptLength { strlen( (const char *) postRemoveScript->get() ) };
                auto const& postRemoveScriptVBuffer
                {   aystl::algorithm::apply
                    (   std::vector<char>
                        (   postRemoveScript->get()
                        , postRemoveScript->get()
                            + postRemoveScriptLength)
                        , [](char i
                        ) -> char {
                            return (i != '\n') ? i : ';';
                }   )   };
                // Run post removal list scriupt here
                using process_t = FILE *;
                process_t postRemovalScriptProcess = popen( (const char *) postRemoveScript->get(),"r");
                if(! postRemovalScriptProcess) {
                    LogError ( std::stringstream()
                        << "Failed to open process for "
                           "post remove script: Fatal."
                    );
                    return false;
                }
                constexpr const std::size_t postRemoveScriptProcessReadBufferSize { 0x10000 };
                char postRemoveScriptProcessReadBuffer[postRemoveScriptProcessReadBufferSize];
                std::size_t postRemovalScriptProcessReadBufferBytesReadCount;
                while (0 != ( postRemovalScriptProcessReadBufferBytesReadCount = fread ( postRemoveScriptProcessReadBuffer
                                                                                       , 1
                                                                                       , postRemoveScriptProcessReadBufferSize
                                                                                       , postRemovalScriptProcess
                ))){
                    if(! (AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") || AYSTL_CMD_TOGGLETAGS_NAME.at("--embed")))
                    {
                        std::cout.write(postRemoveScriptProcessReadBuffer,postRemovalScriptProcessReadBufferBytesReadCount);
                    }
                }
                int postRemoveScriptExitCode;
                if(0 != (postRemoveScriptExitCode = pclose(postRemovalScriptProcess)))
                {   LogError( std::stringstream()
                        << "Removal script failed with the "
                           "following error code: "
                        << postRemoveScriptExitCode
                        << ": Fatal."
                    );

                    return false;
                }
                return postRemoveScriptExitCode == EXIT_SUCCESS;
    }()   );  }
    bool removalsFailed = false;
    std::size_t idx = 0;
    for (auto const& workerResult : results) {
        std::cout
            << collectionPackages[idx++]
            << ": "
            << ( !( removalsFailed |=! workerResult)
                ?"Removed successfully"
                :"Failed to remove!"
               )
            << std::endl;
    }
    return !removalsFailed;
}