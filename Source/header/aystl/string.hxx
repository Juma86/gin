#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace aystl {

    class string : public std::string {
    public:
        std::size_t damerau_levenshtein_distance( const std::string_view other );
    };

}