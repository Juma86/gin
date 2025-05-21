#pragma once

#include <iostream>
#include <memory>
#include <filesystem>
#include <string>
#include <string_view>

#include <header/aystl/cmd.hxx>
#include <header/aystl/algorithm.hxx>
#include <header/aystl/fs.hxx>

#include <header/install/muc/read_subfile.hxx>

#include <lib/simdjson/simdjson.h>
#include <x86simdsort.h>
#include <lib/muclib/muclib.hxx>

#include <stdlib.h>
#include <dirent.h>

AYSTL_CMD_METHOD(handleInstall,AYSTL_CMD_TOGGLETAGS_NAME,AYSTL_CMD_COLLECTIONTAGS_NAME,AYSTL_CMD_RESOURCES_NAME);