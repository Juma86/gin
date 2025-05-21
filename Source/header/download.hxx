#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string_view>
#include <optional>
#include <functional>
#include <tuple>
#include <utility>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <filesystem>

#include <dirent.h>

#include <header/aystl/host.hxx>
#include <header/aystl/dbr.hxx>
#include <header/aystl/algorithm.hxx>
#include <header/aystl/cmd.hxx>
#include <header/aystl/fs.hxx>

AYSTL_CMD_METHOD(handleDownload,AYSTL_CMD_TOGGLETAGS_NAME,AYSTL_CMD_COLLECTIONTAGS_NAME,AYSTL_CMD_RESOURCES_NAME);