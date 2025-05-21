#include <header/main.hxx>

constexpr static const std::string_view GIN_CONFIG_PATH { "/etc/gin/gin.toml" };
constexpr static const std::string_view SOFTWARE_ARCHITECTURE_NAME { "x86_64" };
constexpr static const std::string_view SOFTWARE_VERSION_NAME { "0.6.0" };
constexpr static const std::string_view SOFTWARE_KERNEL_NAME { "linux" };
constexpr static const std::string_view SOFTWARE_OS_NAME { "ubuntu:24.04" };

 // Hopefully the new uninstaller works with the changes, where ti uses the meta file info to find the uninstall.muc :)

int main(int argc, char ** argv) {

    auto const& [ginConfig, ec] { LoadGinConfig(GIN_CONFIG_PATH, true) };

    if(ec != loadginconfig_parseerror::EC_OK) {
        std::cerr << "Failed to load configuration:\n\r\t" << (
            ec == loadginconfig_parseerror::EC_UNKNOWN_EXCEPTION ? "Unknown exception."
          : ec == loadginconfig_parseerror::EC_UNREACHABLE_AREA  ? "Unreachable area."
          : ec == loadginconfig_parseerror::EC_FILE_OPEN_FAIL    ? "Failed to open file."
          : ec == loadginconfig_parseerror::EC_FILE_SEEK_FAIL    ? "Failed to seek within file."
          : ec == loadginconfig_parseerror::EC_FILE_TELL_FAIL    ? "Failed to read current location in file"
          : ec == loadginconfig_parseerror::EC_FILE_READ_FAIL    ? "Failed to read data from file."
          : ec == loadginconfig_parseerror::EC_FILE_TOO_LARGE    ? "File is too large."
          : ec == loadginconfig_parseerror::EC_FILE_EXIT_FAIL    ? "Failed to close file."
          : ec == loadginconfig_parseerror::EC_MEM_ALLOC_FAIL    ? "Failed to allocate memory to store file data."
          : ec == loadginconfig_parseerror::EC_TOML_PARSE_FAIL   ? "Failed to parse toml data within file."
          : ec == loadginconfig_parseerror::EC_FILE_NOT_FOUND    ? "Configuration file not found."
          : "Error not supported."
        ) << std::endl;

        return EXIT_FAILURE;
    }

    aystl::CommandLineProcessor CLP;
    CLP.SetFunctionTags({
        { std::string("--download"      ), handleDownload      }
      , { std::string("--install"       ), handleInstall       }
      , { std::string("--clear-cache"   ), handleClearCache    }
      , { std::string("--manage-hosts"  ), handleManageHosts   }
      , { std::string("--search"        ), handleSearch        }
      , { std::string("--remove"        ), handleRemove        }
      , { std::string("--version"       ), handleVersion       }
      , { std::string("--list-installed"), handleListInstalled }
      , { std::string("--test"          ), handleTests         }
    });

    CLP.SetToggleTags({
        { std::string("--quiet"              ), false }
      , { std::string("--embed"              ), false }
   // , { std::string("--preserve-cache"     ), false } // NI
      , { std::string("--print-download-coms"), false }
      , { std::string("--no-ask"             ), false }
      , { std::string("--enable-testing"     ), false }
    });
    const auto& CACHE_PATH_DOWNLOADS { ginConfig.value()["cache"]["downloads"]. value_or ( std::string() ) };
    if(CACHE_PATH_DOWNLOADS == std::string())
      { std::cerr << "Download cache path could not be retrieved: fatal." << std::endl; return EXIT_FAILURE; }

    const auto& CONFIG_PATH_HOSTS { ginConfig.value()["config"]["hosts"]. value_or ( std::string() ) };
    if(CONFIG_PATH_HOSTS == std::string())
      { std::cerr << "Host config path could not be retrieved: fatal." << std::endl; return EXIT_FAILURE; }

    const auto& REGISTRY_PATH_PACKAGE { ginConfig.value()["persistent"]["package_registry"]. value_or( std::string() ) };
    if(REGISTRY_PATH_PACKAGE == std::string())
      { std::cerr << "Package registry path could not be retrieved: fatal." << std::endl; return EXIT_FAILURE; }

    CLP.SetCollectionTags({{ std::string("--packages"          ), std::vector<std::string>{                            } }
                         , { std::string("--from"              ), std::vector<std::string>{                            } }
                         , { std::string("--to"                ), std::vector<std::string>{CACHE_PATH_DOWNLOADS        } }
                         , { std::string("--hosts-config-paths"), std::vector<std::string>{CONFIG_PATH_HOSTS           } } // Should be checked in order, until existing file is found
                         , { std::string("--package-registries"), std::vector<std::string>{REGISTRY_PATH_PACKAGE       } }
                         , { std::string("--query"             ), std::vector<std::string>{                            } }
                         , { std::string("--tags"              ), std::vector<std::string>{                            } }
                         , { std::string("--filters"           ), std::vector<std::string>{                            } }
                         , { std::string("--test-resources-dir"), std::vector<std::string>{                            } }
                         , { std::string("--count"             ), std::vector<std::string>{ "3"                        } }
    }   );
    // ,                   { {"count", {"C"}}, {std::vector<std::size_t>(),ays::cmd::cvt::cst<std::size_t>([](auto const&in)->std::size_t{return(std::size_t)std::stoull(in);})} }
    // ,                   { {"count", {"C"}}, {std::vector<std::size_t>(),AYS_CMD_CVT_CST_FN((std::size_t) std::stoull(in))} }


    CLP.SetResources( {
        { "cache:downloads"        , CACHE_PATH_DOWNLOADS       }
      , { "config:hosts"           , CONFIG_PATH_HOSTS          }
      , { "registry:packages"      , REGISTRY_PATH_PACKAGE      }
      , { "software:version"       , SOFTWARE_VERSION_NAME      }
      , { "software:os"            , SOFTWARE_OS_NAME           }
      , { "software:kernel"        , SOFTWARE_KERNEL_NAME       }
      , { "software:architecture"  , SOFTWARE_ARCHITECTURE_NAME }
    } );

    CLP.SetAlternativeTags({
      // Functions
        {"--download"    , {"d"}}
      , {"--install"     , {"i"}}
      , {"--search"      , {"S"}}
      , {"--manage-hosts", {"H"}}
      , {"--clear-cache" , {"c"}}
      , {"--remove"      , {"r"}}
      // Collections
      , {"--packages"    , {"p"}}
      , {"--from"        , {"f"}}
      , {"--to"          , {"t"}}
      , {"--query"       , {"Q"}}
      , {"--filters"     , {"F"}}
      , {"--tags"        , {"T"}}
      // Toggles
      , {"--quiet"       , {"q"}}
      , {"--embed"       , {"e"}}
      , {"--no-ask"      , {"n"}}
    });

    CLP.SetMacros({

      // Full length macros

        { "install"  , { "-dicp"          } }
      , { "search"   , { "-SQ"            } }
      , { "purge"    , { "-rpn"           } }
      , { "download" , { "-dt", ".", "-p" } }

      // Shortened macros

      , { "ins"      , { "-dicp"          } }
      , { "sea"      , { "-SQ"            } }
      , { "pur"      , { "-rpn"           } }
      , { "dow"      , { "-dt", ".", "-p" } }

    });

    bool programExecutionResult;

    try{ programExecutionResult = CLP.ParseCLIArgs(argc, argv); }
    catch (const std::exception& e) {
        std::cout << "Application caused the following uncaught exception:\n\r\t"
            << e.what() << "\n\rThis error is not recoverable: fatal." << std::endl;

        programExecutionResult = false;
    }
    
    return programExecutionResult;

}