#include <header/install.hxx>

#ifndef DEBUG
#define DEBUG false
#endif // DEBUG

// #define VERBOSE


AYSTL_CMD_METHOD(handleInstall,AYSTL_CMD_TOGGLETAGS_NAME,AYSTL_CMD_COLLECTIONTAGS_NAME,AYSTL_CMD_RESOURCES_NAME)
{   const auto translatePackagenameToAbsPath {
        [&collectionTags, &resources] (const std::string_view packagename) -> std::string {
            char packagenameAbsPathBuffer[PATH_MAX];
            if(
                (std::string_view(packagename.data(),2) == "./")
                && (std::string_view(packagename.data()+packagename.length()-4,4)) == ".api"
            ) {
                #ifdef DEBUG
                std::cout << "Local package detected; returning abspath." << std::endl;
                #endif
                char * returnPtr = realpath(packagename.data(),packagenameAbsPathBuffer);
                // What is this following line? Who knows!
                // Hello past me, it's comparing the returned pointer to the original pointer, which should be equal according to the documentation of the function.
                    // That is, if everything went smoothly.
                if((std::size_t)returnPtr^(std::size_t)packagename.data()) { std::cout << "[DEBUG INSTALL.CXX:tra...ath] : returnPtr, packagename mismatch {" << (std::size_t)returnPtr << ", " << (std::size_t)packagename.data() << "}" << std::endl; }
                return std::string(packagenameAbsPathBuffer);
            } else {
                // Cached package assumed; returning corresponding cached package with highest version; if it exists.
                    // ( On the great post-NEA rewrite, allow for manual specification of version )
                auto const& defaultPackageCachePath { std::any_cast<std::string> (resources.at("cache:downloads")) };
                const std::vector<std::string> cachedPackageNamesUnfiltered {
                    aystl::fs::ListDirFiles(defaultPackageCachePath).value_or(
                        std::vector<std::string> {}
                )   };
                if( cachedPackageNamesUnfiltered.size() == 0 )
                    return std::string();
                std::vector<std::string> cachedPackageNames;
                std::copy_if(cachedPackageNamesUnfiltered.cbegin()
                            , cachedPackageNamesUnfiltered.cend()
                            , std::back_inserter(cachedPackageNames)
                            , [&packagename](const std::string& cachedPackageName) -> bool {
                                return cachedPackageName.substr( 0
                                    , std::count(cachedPackageName.cbegin(),cachedPackageName.cend(),'-')
                                    ? cachedPackageName.find_last_of('-')
                                    : cachedPackageName.find_last_of('.')
                                ) == packagename;
                            }
                );
                if(! cachedPackageNames.size())
                    return std::string();
                using versionnum_t = __uint64_t;
                std::vector<std::pair<std::string_view, versionnum_t>> parsedPackageFiles(cachedPackageNames.size());
                std::generate(parsedPackageFiles.begin(), parsedPackageFiles.end(), [index=-1, &cachedPackageNames](
                ) mutable -> std::pair<std::string_view, versionnum_t> {
                    index += 1;
                    const auto& cachedPackageName
                        { cachedPackageNames[index] };
                    const std::size_t nameVersionSplitIdx
                        { cachedPackageName.find_last_of("-") }; 
                    const std::size_t versionUnParsedLength {
                        cachedPackageName.find_last_of('.')
                            - nameVersionSplitIdx - 1
                    };
                    const std::string_view parsedPackageName
                        ( cachedPackageName.data(), nameVersionSplitIdx-1 );
                    const versionnum_t cachedPackageVersion {
                        (std::find(cachedPackageName.cbegin(),cachedPackageName.cend(),'-') != cachedPackageName.cend())
                            ? std::stoull( std::string( aystl::algorithm::apply(
                                cachedPackageName.substr(
                                    nameVersionSplitIdx +1
                                    , versionUnParsedLength
                                ), [](const char& elem) -> char {
                                    return elem == '.' ? '0' : elem;
                        }   ).data()   )   )
                            : 0 // If there is no version information in the package name, assume the version is the lowest possible.
                    };
                    return std::pair<std::string_view, versionnum_t>
                        ( cachedPackageName, cachedPackageVersion );
                }   );

                x86simdsort::object_qsort(parsedPackageFiles.data(),parsedPackageFiles.size(),
                    [](std::pair<std::string_view, uint_least64_t> const& a)->uint_least64_t{
                        return a.second;
                    }
                );

                if(parsedPackageFiles.size() > 1) {
                    std::cout 
                        << "Multiple cached instances of "
                            "package, selecting highest available "
                            "version: "
                        << parsedPackageFiles.front().first
                        << std::endl;
                }
                return (std::stringstream()
                    << defaultPackageCachePath << "/"
                    << parsedPackageFiles.front().first
                ) .str();
            }
    }   };
    // .api package is a renamed .muc archive (format I wrote)

    // Reflective package is a tar or cpio archive that is overlaid over the root file system.
        // Reflective package installation WILL overwrite any files that collide!
    const auto handleInstallReflectivePackageCommand { [&toggleTags, &collectionTags, &resources](const std::string& packagePath, const std::string& internalPackageName, const std::string& internalUninstallerDataName
    ) -> bool {
        constexpr const auto generateInstallReflectivePackageCommand {
            [] (const std::string& packagePath
              , const std::string& internalPackageName
        ) -> std::string {
            std::unordered_map<std::string,std::string> archiveCommand
            {   { "cpio", "cpio -idvum --quiet" }
              , { "tar" , "tar -xf - -C /"}
            };
            std::unordered_map<std::string,std::string> compressionCommand
            {   { "lzma", "lzma -d" }
            };
            const std::string compressionExtension =
                internalPackageName.substr(
                    internalPackageName.find_last_of('.')
                      + 1
                );
            const std::size_t
                archiveExtensionStart {
                    aystl::algorithm::find_nth_of(
                        internalPackageName.cbegin()
                    , internalPackageName.cend()
                    , '.', (std::size_t) 0, std::count(
                            internalPackageName.cbegin()
                        , internalPackageName.cend()
                        , '.'
            ) -1 ) +1 }
              , archiveExtensionLength {
                    internalPackageName.find_last_of('.')
                    - archiveExtensionStart
          };
          const std::string_view archiveExtension(
              internalPackageName.data()
                  + archiveExtensionStart
            , archiveExtensionLength
          );
          return (std::stringstream()
              << "bash -c 'set -e ; muc --embed --quiet -cC -i " << internalPackageName << " -o " << packagePath << " | "
              << compressionCommand.at(compressionExtension) << " | "
              << archiveCommand.at(std::string(archiveExtension)) << " | "
                << "grep -qve \"Permission denied\"' 2>&1" // REPLACE WITH A LOG FILE LATER, OR WAY TO SEE!!!
                << " ; exit 0"
            ) .str();
        }   };
        // std::cout << "DBG -> " << generateInstallReflectivePackageCommand(packagePath, internalPackageName) << std::endl;
        using process_t = FILE *;
        process_t installProcess = popen(
            generateInstallReflectivePackageCommand(packagePath, internalPackageName)
                .data(), "r"
        );
        constexpr const auto installProcessReadBufferSize { 0x10000 };
        char installProcessReadBuffer[installProcessReadBufferSize];
        std::size_t bytesRead;
        while((bytesRead = fread(installProcessReadBuffer, 1, installProcessReadBufferSize, installProcess)))
            { /*std::cout.write(installProcessReadBuffer,bytesRead);*/ }
        int installProcessExitCode =
            pclose(installProcess);
        if(installProcessExitCode && (! (toggleTags.at("--quiet") || toggleTags.at("--embed")))) {
            std::cerr << "Non-zero install process exit code : " << installProcessExitCode << std::endl;
        }
        const std::string packageFileName {
            std::filesystem::path(packagePath).filename().string()
        }, packageName {
            packageFileName.substr(0,
                packageFileName.find_last_of("-") != packageFileName.npos
                    ? packageFileName.find_last_of ("-")
                    : packageFileName.find_first_of(".")
                )
        };
        if(! (toggleTags.at("--quiet") || toggleTags.at("--embed"))) {
            std::cout << "Creating package entry: /var/lib/gin/packages/" << packageName << std::endl;
        }
        std::filesystem::path packageEntryPath { (std::stringstream() << "/var/lib/gin/packages/" << packageName).str() };
        if(! std::filesystem::create_directories(packageEntryPath))
        {   if(! (toggleTags.at("--quiet") || toggleTags.at("--embed")) )
            {   std::cerr
                    << "Failed to create package "
                       "registry entry: Fatal."
                    << std::endl;
            }
            return false;
        }
        std::string packageInfo { ( std::stringstream()
            << "{"
                << "\"gin\":{"
                    << "\"version\":\"" << std::any_cast<std::string_view>(resources.at("software:version"     )) << "\","
                    << "\"arch\":\""    << std::any_cast<std::string_view>(resources.at("software:architecture")) << "\","
                    << "\"os\":\""      << std::any_cast<std::string_view>(resources.at("software:os"          )) << "\","
                    << "\"kernel\":\""  << std::any_cast<std::string_view>(resources.at("software:kernel"      )) << "\""
                << "},"
                <<"\"package\":{"
                    << "\"filename\":\"" << packageFileName << "\","
                    << "\"uninstall\":\"" << "uninstall.muc" << "\""
                << "}"
            << "}"
        ).str() };
        std::string packageEntryInfoPath { ( std::stringstream()
            << packageEntryPath.string()
            << "/info.json"
        ).str() };
        FILE * packageEntryInfoHandle { fopen( packageEntryInfoPath.data(), "wb" ) };
        if(nullptr == packageEntryInfoHandle)
        {   if(! (toggleTags.at("--quiet") || toggleTags.at("--embed")))
            {   std::cerr
                    << "Failed to open package "
                       "entry meta-info file handle: Fatal."
                    << std::endl;
            }
            return false;
        }
        if(packageInfo.length() != fwrite(packageInfo.data(),1,packageInfo.length(),packageEntryInfoHandle))
        {   if(! (toggleTags.at("--quiet") || toggleTags.at("--embed")))
            {   std::cerr
                    << "Failed to write required data "
                       "to package entry meta-info file handle: Fatal."
                    << std::endl;
            }
            if(fclose(packageEntryInfoHandle))
            {   if(! (toggleTags.at("--quiet") || toggleTags.at("--embed")))
                {   std::cerr
                        << "Failed to close package entry "
                           "meta-info file handle while writing: Fatal."
                    << std::endl;
                }
                return false;
            }
            return false;
        }
        if(fclose(packageEntryInfoHandle))
        {   if(! (toggleTags.at("--quiet") || toggleTags.at("--embed")))
            {   std::cerr
                    << "Failed to close handle for package "
                       "entry meta-info file handle: Fatal."
                    << std::endl;
            }
            return false;
        }
        std::string packageEntryUninstallPath { (std::stringstream()
            << packageEntryPath.string()
            << "/uninstall.muc"
        ) .str() };
        const std::pair<FILE *, FILE *> fileTransform {
            fopen(packagePath     .c_str(), "rb")
          , fopen(packageEntryUninstallPath.c_str(), "wb")
        };
        if((fileTransform.first == nullptr || fileTransform.second == nullptr)) {
            if(! (toggleTags.at("--quiet") || toggleTags.at("--embed"))) {
                std::cerr << "Failed to create package entry file transform: fatal." << fileTransform.first << " " << fileTransform.second << std::endl;
            }
            if(fileTransform.first ) fclose(fileTransform.first );
            if(fileTransform.second) fclose(fileTransform.second);
            return false;
        }
        bool result = muc::DissolveMUCSubFileFromFileHandleToFileHandle(fileTransform, internalUninstallerDataName);
        if(fileTransform.first ) fclose(fileTransform.first );
        if(fileTransform.second) fclose(fileTransform.second);
        if(result == false) {
            if(! (toggleTags.at("--quiet") || toggleTags.at("--embed"))) {
                std::cerr << "Failed to produce package entry: fatal." << std::endl;
            } return false;
        }
        return (installProcessExitCode ? false : true) && (result);
    }   };
    // END OF HELPER FUNCTION DEFINITIONS!!! ///////////////////////////////////////////////////////
    // END OF HELPER FUNCTION DEFINITIONS!!! ///////////////////////////////////////////////////////
    // END OF HELPER FUNCTION DEFINITIONS!!! ///////////////////////////////////////////////////////
    // END OF HELPER FUNCTION DEFINITIONS!!! ///////////////////////////////////////////////////////
    // END OF HELPER FUNCTION DEFINITIONS!!! ///////////////////////////////////////////////////////
    const bool
        toggleQuiet
        {   toggleTags.at("--quiet")
        }
      , toggleEmbed
        {   toggleTags.at("--embed")
        };
    const auto& collectionPackages
    {   collectionTags.at("--packages")
    };
    const std::unordered_map<std::string
                           , std::function<bool(const std::string&
                                              , const std::string&
                                              , const std::string&)>>
    classHandler {
        {"reflective", handleInstallReflectivePackageCommand}
    };
    std::unique_ptr<bool[]> installSucesses( new bool[collectionPackages.size()] );

    std::generate(&installSucesses.get()[0],&(installSucesses.get())[collectionPackages.size()]
      , [&collectionPackages, &toggleQuiet, &toggleEmbed, &classHandler, &resources, &translatePackagenameToAbsPath, packageIndex= -1]() mutable -> bool {
            packageIndex += 1;
            const auto packageName
            {   collectionPackages.at(packageIndex)
            };
            const std::string packagePath
            {   translatePackagenameToAbsPath(packageName)
            };
            std::vector<std::string> const& installedPackageNames
            {   aystl::algorithm::apply( aystl::fs::ListDirItems( aystl::string { std::any_cast<std::string>( resources.at("registry:packages") ) }
                                                                , aystl::fs::pathtype::directory
                ).value_or( std::vector<aystl::string> () ), [](auto const& dirItem
                ) -> std::string {
                    if(DEBUG) std::cout << "[DEBUG] : dirItem : " << dirItem << std::endl;
                    return std::string
                    {   dirItem.c_str()
                    };
            }   )   };
            for (auto const& installedPackageName : installedPackageNames)
            {   if(DEBUG) std::cout << "[DEBUG] installedPackageName : " << installedPackageName << std::endl;
            }
            if( std::find( installedPackageNames.cbegin(), installedPackageNames.cend(), packageName ) != installedPackageNames.cend() )
            {   if(! (toggleQuiet || toggleEmbed))
                {   std::cout << packageName << " is already installed; skipping..."
                    #ifdef VERBOSE // temp
                    " !!! IN THE FUTURE ENSURE THAT VERSION IS ALSO CHECKED HERE AS UPDATING IS NICE !!! [UNLESS A BETTER SOLUTION IS FOUND, E.G. EXPLICITLY UPDATING INSTEAD OF INSTALLING.]"
                    #endif
                    << std::endl;
                    return false;
                }
            }
            if(packagePath == "") {
                std::cout
                    << "There is no cached package with a name matching ["
                    << packageName
                    << "]\n\r\t have you either not downloaded the package, "
                       "or made a typo?: Fatal."
                    << std::endl;
                
                    return false;
            }
            std::cout
                << "Installing "
                << packageName
                << " from "
                << packagePath
                << std::endl
            ;
            const std::optional<std::string>& packageMetadataOpt {
                ReadSubFile(packagePath, "/info.json", !(toggleQuiet || toggleEmbed))
            };
            if(! packageMetadataOpt.has_value())
            {   if(! (toggleQuiet || toggleEmbed))
                {   std::cerr
                        << "Failed to read metadata "
                           "from package file: fatal."
                        << std::endl;
            } return false; }
            const std::string_view packageMetadata {
                packageMetadataOpt.value()
            };
            const simdjson::padded_string packageMetadataPadded = packageMetadata;
            simdjson::ondemand::parser JSONParser;
            simdjson::ondemand::document JSONDocument =
                JSONParser.iterate(packageMetadataPadded);
            const std::string packageClass { std::string(
                JSONDocument["class"].get_string().take_value()
            )   };
            const std::string packageDataSubfilePath { std::string (
                JSONDocument["data"].get_string().take_value()
            )   };
            const std::string packageUninstallerSubfilePath { std::string (
                JSONDocument["uninstall"].get_string().take_value()
            )   };
            return classHandler.at(packageClass) ( packagePath
                                                 , packageDataSubfilePath
                                                 , packageUninstallerSubfilePath
            );
        }   );
    std::cout
        << "Installed "
        << std::count(
            &installSucesses.get()[0]
          , &installSucesses.get()[
                collectionPackages.size()
        ] , true )
        << " of "
        << collectionPackages.size()
        << " successfully."
        << std::endl;

    return true;
}

// muc --quiet --embed -cC -i /package.cpio.lzma -o simple-package-0.0.0.1.api | lzma -d | cpio -i .

/*
    * -> To create the installation method, I decided to read into the inner-workings of the muc file-format
    *    and implement my own code for parsing the package & installing it. Due to the beta-nature of the muc
    *    CLI program it is currently not in a production-ready state, since it loads a file _FULLY_ into memory
    *    before dissolving it to disk. My implementation will circumvent that issue by making use of chunking
    *    and OS pipes.
    *
    *
    *
    *
    *
    *
    *
    *
    *
    *
    *
*/