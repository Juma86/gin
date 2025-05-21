#pragma once

#ifndef ABC
#define ABC

#include <string>
#include <optional>
#include <unordered_map>
#include <iostream>

namespace aystl::net {

    struct host {
        struct sftp_data {
            std::string user
                    , pass;
        };

        std::string name
                , port
                , prot;
        
        std::optional<sftp_data> sftp = std::nullopt;
    };

    
    aystl::net::host ParseHost(const std::string& hostLiteral);

}

#endif