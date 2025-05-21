#pragma once

#include <optional>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>

namespace aystl::dbr {

    std::optional<std::string> ReadDisk (const std::string& filePath);
    bool WriteDisk(const std::string& filePath, const std::string& dbrFile );

}