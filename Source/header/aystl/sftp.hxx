#pragma once

#include <header/aystl/host.hxx>

#include <optional>
#include <memory>
#include <string>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <fcntl.h>

namespace aystl::net::sftp {

    std::optional<std::unique_ptr<char[]>> ReadRemoteFile (aystl::net::host const& host, std::string const& filePath );

}