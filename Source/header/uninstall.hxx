#pragma once

#include <header/aystl/cmd.hxx>
#include <header/aystl/algorithm.hxx>

#include <lib/muclib/muclib.hxx>
#include <lib/BS_thread_pool/BS_thread_pool.hpp>
#include <lib/simdjson/simdjson.h>

#include <iostream>
#include <string>
#include <string_view>
#include <future>
#include <mutex>
#include <filesystem>
#include <set>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <unistd.h>

AYSTL_CMD_METHOD(handleRemove,AYSTL_CMD_TOGGLETAGS_NAME,AYSTL_CMD_COLLECTIONTAGS_NAME,AYSTL_CMD_RESOURCES_NAME);