#pragma once

//#include <EASTL/unordered_map.h>
//#include <EASTL/string.h>
//#include <EASTL/functional.h>
//#include <EASTL/any.h>
//#include <EASTL/vector.h>
//#include <EASTL/algorithm.h>
//#include <EASTL/optional.h>

#ifndef AYSTL_CMD_HXX
#define AYSTL_CMD_HXX

#include <unordered_map>
#include <string>
#include <functional>
#include <any>
#include <vector>
#include <algorithm>
#include <optional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <map>

#define AYSTL_CMD_TOGGLETAGS_NAME toggleTags
#define AYSTL_CMD_COLLECTIONTAGS_NAME collectionTags
#define AYSTL_CMD_RESOURCES_NAME resources
#define AYSTL_CMD_METHOD( METHOD_NAME, TOGGLETAGS_NAME, COLLECTIONTAGS_NAME, RESOURCES_NAME ) \
    bool METHOD_NAME ( const std::unordered_map<std::string, bool>& TOGGLETAGS_NAME,          \
        const std::unordered_map<std::string, std::vector<std::string>>& COLLECTIONTAGS_NAME, \
        const std::unordered_map<std::string, std::any>& RESOURCES_NAME                       \
    )

namespace aystl {
    class CommandLineProcessor {
    private:
        std::unordered_map<std::string, bool> m_toggleTags;
        std::unordered_map<std::string, std::vector<std::string>> m_collectionTags;
        std::unordered_map<std::string, std::any> m_resources;
        std::unordered_map<std::string, std::function<bool(const decltype(m_toggleTags)&,
                                                           const decltype(m_collectionTags)&,
                                                           const decltype(m_resources)&)>> m_functionTags;
        std::unordered_map<std::string, std::unordered_set<std::string>> m_alternativeTags;
        std::unordered_map<std::string, std::vector<std::string>> m_macros;
    public:
        CommandLineProcessor();
        void SetToggleTags      ( const decltype (m_toggleTags     )& toggleTags      );
        void SetFunctionTags    ( const decltype (m_functionTags   )& functionTags    );
        void SetCollectionTags  ( const decltype (m_collectionTags )& collectionTags  );
        void SetAlternativeTags ( const decltype (m_alternativeTags)& alternativeTags );
        void SetResources       ( const decltype (m_resources      )& resources       );
        void SetMacros          ( const decltype (m_macros         )& macros          );
        bool ParseCLIArgs ( int argc, char ** argv );
    };
}

#endif /* AYSTL_CMD_HXX */