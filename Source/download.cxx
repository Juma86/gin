#include <header/download.hxx>

constexpr const char * PACKAGE_EXTENSION { ".api" };

#ifndef DEBUG
#define DEBUG false
#endif

// This function has the following on-system program dependencies : bash, sshpass, sftp, ls, grep
AYSTL_CMD_METHOD(handleDownload,AYSTL_CMD_TOGGLETAGS_NAME,AYSTL_CMD_COLLECTIONTAGS_NAME,AYSTL_CMD_RESOURCES_NAME) {

    using returncode_t = int;
    using filetransform_t = std::pair<std::string, std::string>;
    using download_function    = std::function<returncode_t(const aystl::net::host& host
                                                          , const filetransform_t& filetransform
                                                          , const bool&)>;
    using packagelist_function = std::function<std::optional<std::vector<std::string>>(const aystl::net::host& host
                                                                                     , const bool& logCom)>;

    // This command depends on the following external programs : bash, sshpass, sftp
    // filetransform: {"name-on-server", "name-on-client"}
    constexpr auto SFTPHandleDownloadOperation { [](const aystl::net::host& host
                                                  , const filetransform_t& filetransform
                                                  , const bool& logCom // log communications between client & server
    ) -> returncode_t {
        constexpr auto SFTPConstructDownloadRequest { [](const std::string& password
                                                       , const std::string& port
                                                       , const std::string& username
                                                       , const std::string& address
                                                       , const std::string& remoteFilename
                                                       , const std::string& localFilename
                                                       , const bool& logCom // log communications between client & server
        ) -> std::string {
            // Example output:
                // bash -c 'sshpass -p some-password sftp -P 22 -o "StrictHostKeyChecking accept-new" -q username@some.host.tld <<< "get /packages/packagename localname" &> /dev/null'

            #ifdef DEBUG
            //std::cout << "[DEBUG] : to -> " << localFilename << std::endl;
            #endif

            return (std::stringstream()
                << "bash -c 'sshpass -p " << password << " "
                << "sftp -P " << port << " "
                << "-o \"StrictHostKeyChecking accept-new\" "
                << "-q " << username << "@" << address << " <<< \""
                << "get /packages/" << remoteFilename << " " << localFilename<< "\""
                << (logCom ? "" : " &> /dev/null") << " '"
            ) .str();
        }   };

        const auto& [remotePackageName, localPackageName] { filetransform };
        const std::string SFTPPackageDownloadRequest { SFTPConstructDownloadRequest ( host.sftp->pass
                                                                                    , host.port
                                                                                    , host.sftp->user
                                                                                    , host.name
                                                                                    , remotePackageName
                                                                                    , localPackageName
                                                                                    , logCom
        )   };

        returncode_t returnCode = system(SFTPPackageDownloadRequest.c_str());
        
        return returnCode;
    }

    };

    // This command depends on the following external programs : bash, sshpass, sftp, ls, grep
    constexpr auto SFTPHandlePackageListOperation { [](const aystl::net::host& host
                                                     , const bool& /*logCom*/ // log communications between client & server
    ) -> std::optional<std::vector<std::string>> {
        using process_t = FILE*;
      
        constexpr auto SFTPConstructPackageListRequest { [](const std::string_view& password
                                                          , const std::string_view& port
                                                          , const std::string_view& username
                                                          , const std::string_view& address
                                                          , const std::string_view& packageExtension
        ) -> std::string {
            return (std::stringstream()
                << "bash -c 'sshpass -p " << password << " "
                << "sftp -P " << port << " "
                << "-o \"StrictHostKeyChecking accept-new\" "
                << "-q " << username << "@" << address
                << " <<< \"ls -1 /packages/*\" | grep packages --color=never | grep " << packageExtension << " --color=never | sed \"s/.*\\\\///g\" || true'"
            ) .str();

            // BASED ON THE FOLLOWING COMMAND :::
            //      bash -c "sshpass -p password sftp -P 22 -o \"StrictHostKeyChecking accept-new\" demo@test.rebex.net -q <<< \"ls\" | grep .txt --color=never || true"
        }   };

        const std::string& SFTPPackageListRequest { SFTPConstructPackageListRequest ( host.sftp->pass
                                                                                    , host.port
                                                                                    , host.sftp->user
                                                                                    , host.name
                                                                                    , PACKAGE_EXTENSION

        )   };

        process_t packageListProcess = (process_t) popen ( SFTPPackageListRequest .c_str()
                                                         , "r"
        );

        // ensure creation of process was successful.
        if(! packageListProcess)
            { return std::optional<std::vector<std::string>>(std::nullopt); }

        // Read all output info stringstream
        constexpr size_t processReadbufferSize { 0x4000 };
        char * processReadBuffer = new char[processReadbufferSize];
        std::stringstream processReadResultStream;
        while (fgets(processReadBuffer, processReadbufferSize, packageListProcess) != NULL) {
            processReadResultStream << processReadBuffer;
        }

        const auto processClosureErrorCode = pclose(packageListProcess);

        // Move result into usable data structure
        std::vector<std::string> packageNames{};
        std::string processReadResultString = processReadResultStream .str();
        while (processReadResultString.find("\n") != std::string::npos) {
            packageNames.emplace_back(processReadResultString.substr(0,processReadResultString.find("\n")));
            processReadResultString = processReadResultString.substr(processReadResultString.find("\n")+1);
        }

        // Pclose used to be here, absolute idiot

        return processClosureErrorCode ? std::optional<std::vector<std::string>>(std::nullopt)
                                       : std::optional<std::vector<std::string>>(packageNames) ;

    }   };

    constexpr auto GetHosts { [](const std::string& hostsFile
                               , const std::vector<std::string>& CLIHosts
                               , const bool logErrors
    ) -> std::vector<aystl::net::host> {
        // string parsable format: sftp(22)://demo&password@test.rebex.net
        // Convert newline delimited list to vector of host datastructure

        std::vector<aystl::net::host> hosts(std::count(hostsFile.cbegin()
                                                     , hostsFile.cend()
                                                     , '\n'
                                            ) +1
        ); // +1 to account for last line

        // Parse hosts in hosts file
        std::generate(hosts.begin(),hosts.end(), [l=0, &hostsFile]() mutable -> aystl::net::host {
            size_t startPosition = aystl::algorithm::find_nth_of(hostsFile.cbegin()
                                                               , hostsFile.cend()
                                                               , '\n', 0, l) + (l!=0?1:0) // this ternary is because elsewise the endline is included for parsing
                 , endPosition   = aystl::algorithm::find_nth_of(hostsFile.cbegin()
                                                               , hostsFile.cend()
                                                               , '\n', 0, ++l)
                 , length        = endPosition
                                     - startPosition
            ;
            return aystl::net::ParseHost(
                hostsFile.substr(startPosition, length)
            );
        }   );

        // Parse hosts passed from CLI invocation, Maybe this context isn't needed here?; Pass a vector of vector of
            // string and process into vector of host_t
        for(const auto& CLIHost : CLIHosts) {
            try {
                hosts.emplace_back(aystl::net::ParseHost(CLIHost));
            } catch (const std::exception& e) {
                if(logErrors)
                    { std::cerr << "Error when parsing CLI-specified host : " << e.what() << std::endl; }
            }
        }

        return hosts;
    }   };

    // Self-explanatory
    constexpr auto GetHostsFile { [](const std::string& filePath
                                   , const bool logErrors
    ) -> std::string {
        const std::optional<std::string>& optHostsFile = aystl::dbr::ReadDisk(filePath);
        if( (logErrors && (! optHostsFile.has_value())) ) {
            { std::cout << "Failed to read hosts file!" << std::endl; }
        }
        
        const std::string& hostsFile { optHostsFile.value_or(std::string())};

        return hostsFile;
    }   };

    constexpr auto GetHostsFilePath { [](const std::vector<std::string>& hostsConfigPaths
    ) -> std::optional<std::string> {
        const std::vector<std::optional<std::string>>& validatedPaths
        {   aystl::algorithm::apply(
                hostsConfigPaths
              , []( const std::string& hostsConfigPathString
                ) -> std::optional<std::string> {
                    std::filesystem::path hostsConfigPath ( hostsConfigPathString );

                    return std::filesystem::exists( hostsConfigPath )
                        ? (std::optional<std::string>) (hostsConfigPathString)
                        : (std::optional<std::string>) (std::nullopt         );
                }   )
        };

        const std::optional<std::string>& hostsPath =
            aystl::algorithm::get_first_satisfier_or(
                validatedPaths
              , (std::optional<std::string>) std::nullopt
              , [](
                    const std::optional<std::string>& item
                ) -> bool { return item.has_value();
            }   );


        return hostsPath;
    } };

    // +++++ #===#------- END OF INTERNAL FUNCTION DECLERATIONS !!! -------#===# +++++

    const bool toggleQuiet { toggleTags.at("--quiet") };
    const bool toggleEmbed { toggleTags.at("--embed") };

    if(collectionTags.at("--to").size() > 2) {
        if(! (toggleQuiet || toggleEmbed))
            { std::cerr << "Only one user-defined location may be specified under --to; fatal." << std::endl; }
        return false;
    }

    const auto& hostsPath {
        GetHostsFilePath(
            aystl::algorithm::concat_containers<
                std::vector<std::string>
              , std::vector<std::string>
              , std::vector<std::string>
            > ( std::vector<std::string> {
                    std::any_cast<std::string> (
                        resources.at(
                            "config:hosts"
    )   )   } , collectionTags.at("--hosts-config-paths")
    )   )   };
    
    if(! hostsPath.has_value()) {
        if(! (toggleQuiet || toggleEmbed)) {
            std::cerr << "Failed to find a valid hosts file: fatal." << std::endl;
        } return false;
    }
    
    const std::string& hostsFile = GetHostsFile(hostsPath.value(), ! (toggleQuiet||toggleEmbed));

    std::vector<std::string> cachedPackages;
    // Packages already stored in download cache, With this 2 guard due to a crash.
    if ( collectionTags.at("--to").size() > 2 ) {
        cachedPackages =
            aystl::fs::ListDirFiles(
                collectionTags.at("--to").back()
        ) .value_or( std::vector<std::string>{} );
    }

    // Packages already stored in cache, but in a more convenient format
    std::vector<std::pair<std::string,std::size_t>> sanitisedCachedPackageNameVersionBindings
    {   aystl::algorithm::apply
        (   cachedPackages
          , []( std::string const& cachedPackageName
            ) -> std::pair<std::string,std::size_t> {
                if(DEBUG) std::cout << "[DEBUG] : cachedPackageName : " << cachedPackageName << std::endl;

                std::string sanitisedPackageName = cachedPackageName.substr(0,
                    (   std::find(cachedPackageName.cbegin(), cachedPackageName.cend(), '-') != cachedPackageName.cend())
                      ? cachedPackageName.find_last_of('-') // Version segments are present
                      : cachedPackageName.find_last_of('.') // Version segments are not present
                );

                if(DEBUG) std::cout << "[DEBUG] : sanitisedPackageName : " << sanitisedPackageName << std::endl;

                // Very heavily reused code, abstract into function post-NEA, if I don't restart with a whole new project architecture (sounds like heaven) :p
                std::size_t const packageVersion =
                (   std::find(cachedPackageName.cbegin(),cachedPackageName.cend(),'-') != cachedPackageName.cend())
                      ? std::stoull( std::string( aystl::algorithm::apply(
                            cachedPackageName.substr(
                                cachedPackageName.find_last_of('-') +1
                              , cachedPackageName.find_last_of('.') - cachedPackageName.find_last_of('-')-1
                            ), [](const char& elem) -> char {
                                return elem == '.' ? '0' : elem;
                        }   ).data()   )   )
                      : 0
                ;

                if(DEBUG) std::cout << "[DEBUG] : packageVersion : " << packageVersion << std::endl;

                return
                {   sanitisedPackageName
                  , packageVersion
                };
            }
        )
    };

    // Convert host literals to aystl data structure
    const std::vector<aystl::net::host>& hosts {
        GetHosts(hostsFile, collectionTags.at("--from"), ! (toggleQuiet||toggleEmbed))
    };

    // Log what packages are attempting to be downloaded, and where they maybe from
    if(! (toggleQuiet || toggleEmbed)) {
        std::cout << "Attempting to locate the following packages:";
        for (const std::string& packageName : collectionTags.at("--packages")) {
            std::cout << "\n\r\t" << packageName;
        }
        
        std::cout << std::endl << "Within the following servers:";
        
        for (const std::string& serverName : aystl::algorithm::apply(hosts, [](const aystl::net::host& host) -> std::string { return host.name; })) {
            std::cout << "\n\r\t" << serverName;
        }
        
        std::cout << std::endl;
    }

    // Setup protocols & their respective methods (Could be a class with templates, OOP==EW), also I love how clean this is :D
    const std::unordered_map<std::string, std::tuple<download_function, packagelist_function>>
    protocolHandlers {
        {"sftp", {SFTPHandleDownloadOperation, SFTPHandlePackageListOperation}}
    };

    std::unique_ptr<std::vector<std::string>[]> packageGroupsThreaded( new std::vector<std::string>[hosts.size()] );
    std::vector<std::thread> packageGroupGenerators ( hosts.size() ); 
    std::mutex stdCoutLock;

    // Create a thread for each server, list packages within & store
    std::generate( packageGroupGenerators.begin(), packageGroupGenerators.end(), [idx = -1,&toggleQuiet,&toggleEmbed,&hosts,&protocolHandlers,&stdCoutLock,&packageGroupsThreaded](   
    ) mutable -> std::thread { idx += 1;
        return std::thread(
            [&toggleQuiet,&toggleEmbed,&hosts,&protocolHandlers,&stdCoutLock, &packageGroupsThreaded](const decltype(idx) idx){
                const auto& optPackageList = std::get<packagelist_function>(protocolHandlers.at(hosts.at(idx).prot))(hosts.at(idx), toggleQuiet);
                if(! optPackageList.has_value()) {
                    if(! (toggleQuiet || toggleEmbed)) {
                        const auto& _lock { std::lock_guard<std::mutex>(stdCoutLock) };
                        std::cout << "[" << hosts.at(idx).name << "] : Failed to retrieve package list!" << std::endl;
                    } packageGroupsThreaded[idx] = std::vector<std::string>();
                }
                const auto& packageList = optPackageList.value(); // Value WILL be present here
                packageGroupsThreaded[idx] = packageList;
            }
        , idx   );
    }   );

    // Rejoin package list threads
    std::for_each(
        packageGroupGenerators.begin()
      , packageGroupGenerators.end()
      , [](auto& packageGroupGenerator
        ) -> void {
            packageGroupGenerator.join();
    }   );

    // Move package lists from thread-friendly data structure to 2D-Vector ### May be nice to optomise this out later!
    std::vector<std::vector<std::string>> packageGroups (hosts.size());
    std::generate(packageGroups.begin(),packageGroups.end(),[idx=-1,&packageGroupsThreaded](
    ) mutable -> std::vector<std::string>& {
        return packageGroupsThreaded[++idx];
    }   );

        // Uncomment and eventually wrap into a debug logger (Make a logging library first :))
    /*if(! (toggleQuiet || toggleEmbed))
    {
        std::cout << "hits:" << std::endl;
        int i = 0;
        for (const auto& packageGroup : packageGroups) {
            for (const auto& packageName : packageGroup) {
                std::cout << "\t[" << hosts.at(i).name << "] : " << packageName << std::endl;
            } i += 1;
        }
    }*/

    using hostindex_t      = __uint16_t;
    using packageversion_t = __uint64_t;
    using host_version_binding_t = std::pair<hostindex_t, packageversion_t>;
    std::unordered_map<std::string, host_version_binding_t> installationTargets;
    std::unordered_map<std::string, std::string> installationTargetFullNames;
    hostindex_t hostIndex = 0;

    // Find the highest version of each desired package, storing result for processing
    for(const auto& packageGroup : packageGroups) {
        for (const auto& packageName : packageGroup) {

            // Calculate package name without version info & file extension
            std::string sanitisedPackageName = packageName.substr(0,
                (std::find(packageName.cbegin(), packageName.cend(), '-') != packageName.cend())
                ? packageName.find_last_of('-') // Version segments are present
                : packageName.find_last_of('.') // Version segments are not present
            );

            // If the package isn't one the user wants to download, attempt the next package
            if(std::find(collectionTags.at("--packages").cbegin()
                          , collectionTags.at("--packages").cend()
                          , sanitisedPackageName
                ) == collectionTags.at("--packages").cend()
            ) { continue; }
            
            // Calculate a numerical package version, corresponding to it's string version
            unsigned long long int const packageVersion =
            (   std::find(packageName.cbegin(),packageName.cend(),'-') != packageName.cend())
                  ? std::stoull( std::string( aystl::algorithm::apply(
                        packageName.substr(
                            packageName.find_last_of('-') +1
                          , packageName.find_last_of('.') - packageName.find_last_of('-')-1
                        ), [](const char& elem
                        ) -> char {
                            return elem == '.' ? '0' : elem;
                    }   ).data()   )   )
                    : 0 // If there is no version information in the package name, assume the version is the lowest possible.
            ;

            // Check if we already have a cached package of equal or higher version number, if so then the download is not-needed;
                // if it is, the user should clear their cache as a solution.
            bool downloadUnnecessary = false;
            for (auto const& sanitisedCachedPackageNameVersionBinding : sanitisedCachedPackageNameVersionBindings)
            {   auto const& [sanitisedCachedPackageName, cachedPackageVersion] { sanitisedCachedPackageNameVersionBinding };
                if( (sanitisedPackageName == sanitisedCachedPackageName) && (packageVersion <= cachedPackageVersion) )
                {   if(! (toggleQuiet || toggleEmbed))
                    {   std::cout << "A cached package with a version either equal-to or greater-than is already stored in cache; skipping : " << packageName << std::endl;
                    }
                    downloadUnnecessary = true;
            }   }
            if(downloadUnnecessary)
            {   continue;
            }

            // Move the index of the host providing this package, along with the package version into a convenient data structure
            host_version_binding_t hostVersionBinding = {
                hostIndex
              , packageVersion
            };
            if(installationTargets.find(sanitisedPackageName) == installationTargets.cend()
            ) { // If this package is yet to be included within installation targets, include it unconditionally
                installationTargets[sanitisedPackageName] = hostVersionBinding;
                installationTargetFullNames[sanitisedPackageName] = packageName;
            } else { // Else, only update the package to this one if the version is higher.
                if(installationTargets[sanitisedPackageName].second
                     < hostVersionBinding.second
                ) {
                    installationTargets[sanitisedPackageName] = hostVersionBinding;
                    installationTargetFullNames[sanitisedPackageName] = packageName;
                }
            }
        }

        hostIndex += 1;
    }
    
    // Print to the user where each package is coming from. Or if it was not found; Refactor to not rely on exceptions!!
    for (const auto& packageName : collectionTags.at("--packages")) {
        //std::cout << "best server for " << packageName << " is " << hosts.at(installationTargets.at("readme").first).name << " with version " << installationTargets.at("readme").second << std::endl;
        try {
            const std::string& packageHostName { hosts.at(installationTargets.at(packageName).first).name };
            std::cout << packageName << " : " << packageHostName << std::endl; }
        catch ( std::exception& e ) {
            if(std::string(e.what()) == "unordered_map::at") // Bad practices???? :(
                { std::cout << packageName << " : Package not found!" << std::endl; }
            else throw e; // ew
            }
    }

    for (const auto& installationTarget : installationTargets) {
        const auto& [packageName, hostVersionBinding] { installationTarget };
        const auto& [hostIndex  , packageVersion    ] { hostVersionBinding };
        const auto& packageFullName = installationTargetFullNames[packageName];

        if(DEBUG) std::cout << "[DEBUG] : packageName : " << packageName << std::endl;

        if(! (toggleQuiet || toggleEmbed))
        {
            std::cout
                << "Downloading " << packageFullName << "..." << std::endl;

                // Later have it so latter is only printed if verbose

                //<< "Downloading " << packageFullName << " "
                //<< "with version [" << packageVersion << "] "
                //<< "from host : " << hosts.at(hostIndex).name << std::endl;
        }

        // Download the package
        std::get<download_function>(
            protocolHandlers.at(
                hosts.at(hostIndex).prot
            ))(
                hosts.at(hostIndex)
              , {
                    packageFullName
                  , ( std::stringstream()
                         << collectionTags.at("--to").back()
                         << "/" << packageFullName 
                    ) .str()
                }, toggleTags.at("--print-download-coms") // Print direct communication data if this tag is used in invocation
        );
    }
    
    if(! (toggleQuiet || toggleEmbed))
        { std::cout << "Package downloading operation completed." << std::endl; }
    return true;
}

// CHANGES HERE AFTER I REMEMBERED THIS NEEDS TO BE DONE _AFTER_ FINISHING THE DOWNLOADING FUNCTION, BUT I CAN IMPROVE IT :)

/*
    * -> The method of sequentially interacting with servers to get package lists was slow, so I made it concurrent
    *    while doing this, I came across the issue of declaring a std::vector, initialising it with the size I needed
    *    and then using std::copy_n with a std::back_inserter to fill it with the needed data. This would allocate
    *    new elements at the back of the vector instead of using the space already allocated causing the program to
    *    crash as the packageGroups datastructure would have more elements than there are hosts. So when working out
    *    which host belonged to which group and element past the end of the datastructure would be accessed, prompting
    *    a crash. Another datastructure change I made was converting packageGroups from a
    *    std::vector<std::vector<std::string>> to a std::unique_ptr<std::vector<std::string>[]>
    *    this change meant I could write to the datastructure concurrently without a mutex. This makes the operation of
    *    the program faster. I also had to create a datastructure to store the variable amount of threads running so I
    *    could join them later on.
    *
    *
    *
    *
    *
    *
    *
    *
*/