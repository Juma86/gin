#include <toml++/toml.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <memory>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

enum class loadginconfig_parseerror {
    EC_OK = NULL

        // Misc
  , EC_UNKNOWN_EXCEPTION
  , EC_UNREACHABLE_AREA

        // File Associated Errors
  , EC_FILE_NOT_FOUND
  , EC_FILE_TOO_LARGE
  , EC_FILE_OPEN_FAIL
  , EC_FILE_SEEK_FAIL
  , EC_FILE_TELL_FAIL
  , EC_FILE_READ_FAIL
  , EC_FILE_EXIT_FAIL
  , EC_MEM_ALLOC_FAIL

        // Toml Associated Errors
  , EC_TOML_PARSE_FAIL

};

std::pair<std::optional<toml::table>, loadginconfig_parseerror> LoadGinConfig ( std::string_view const& a_svGinConfigPath, bool const a_bLogError );