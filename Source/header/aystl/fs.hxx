#pragma once

#include <string>
#include <vector>
#include <optional>

#include <dirent.h>
#include <sys/stat.h>

#include <header/aystl/string.hxx>
#include <header/aystl/algorithm.hxx>

namespace aystl::fs {

    enum class pathtype
    {   any       = -0x1
      , file      = DT_REG
      , directory = DT_DIR
    };

    std::optional<std::vector<std::string>> ListDirFiles(const std::string_view dirPath);

    std::optional<std::vector<aystl::string>> ListDirItems(aystl::string const& dirPath, aystl::fs::pathtype const& filter );

    std::optional<bool> PathIsDir(const std::string_view path);
}