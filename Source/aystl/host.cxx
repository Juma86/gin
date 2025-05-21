#include <header/aystl/host.hxx>

aystl::net::host aystl::net::ParseHost(const std::string& hostLiteral) {
  // const std::string& exampleLiteral { "sftp(1234)://username&password@test.someserver.tld" };

  //std::cout << "literal -> " << hostLiteral << std::endl;

  const std::string protocolSplitString = "://";

  const size_t protocolSplitStringLocation = hostLiteral.find(protocolSplitString);
  std::string protocolData = hostLiteral.substr( 0
                                               , protocolSplitStringLocation
                                               );
  std::string hostnameData = hostLiteral.substr( protocolSplitStringLocation
                                                  + protocolSplitString.length()
                                               );

  const std::unordered_map<std::string, std::string> protocolPortMap {
      {"sftp"  , "22"   }
    , {"http"  , "80"   }
    , {"https" , "443"  }
    , {"apnip" , "18945"}
  };

  std::string protocolName = protocolData.substr(0, protocolData.find("("));
  std::string protocolPort = (protocolData.find("(") != std::string::npos)
                              ? (protocolData.substr(protocolData.find("(")+1
                                                   , protocolData.find(")")-protocolData.find("(")-1))
                              : protocolPortMap.at(protocolData);
  std::string protocolAddr = (protocolName == "sftp")
                              ? (hostnameData.substr(hostnameData.find("@")+1))
                              : (hostnameData);
  std::string SFTPUsername = (protocolName == "sftp")
                              ? (hostnameData.substr(0
                                                   , hostnameData.find("&")))
                              : std::string();
  std::string SFTPPassword = (protocolName == "sftp")
                              ? (hostnameData.substr(hostnameData.find("&")+1
                                                   , hostnameData.find("@")-hostnameData.find("&")-1))
                              : std::string();

  return aystl::net::host {
      .name = protocolAddr
    , .port = protocolPort
    , .prot = protocolName
    , .sftp = (protocolName == "sftp")
              ? decltype(aystl::net::host::sftp) (aystl::net::host::sftp_data{ .user = SFTPUsername, .pass = SFTPPassword})
              : decltype(aystl::net::host::sftp) (std::nullopt                                                            )
  };
}