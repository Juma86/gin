#include <optional>
#include <string>
#include <iostream>
#include <memory>

std::optional<std::string> ReadSubFile(const std::string& packagePath, const std::string& subfileName, const bool logErrors);