#include <header/managehosts.hxx>

 // This file has code duplication issues - use aystl::dbr !!!

AYSTL_CMD_METHOD(handleManageHosts,AYSTL_CMD_TOGGLETAGS_NAME,,AYSTL_CMD_RESOURCES_NAME) {

    const auto& toggleQuiet { AYSTL_CMD_TOGGLETAGS_NAME.at("--quiet") };
    const auto& toggleEmbed { AYSTL_CMD_TOGGLETAGS_NAME.at("--embed") };

    std::string uuid;
    { // Stack memory is only needed temporarily
        FILE * uuidGenerator = fopen("/proc/sys/kernel/random/uuid", "rb");
        if(! uuidGenerator) {
            if(! (toggleQuiet || toggleEmbed)) {
                std::cerr
                    << "Failed to open kernel "
                       "UUID file handle: fatal."
                    << std::endl;
        } return false; }

        char uuidGeneratorReadBuffer;

        while (fread(&uuidGeneratorReadBuffer,1,1,uuidGenerator)){
            if(uuidGeneratorReadBuffer != '\n')
            {   uuid.append(std::string_view(&uuidGeneratorReadBuffer,1));
            }
        }

        if (fclose(uuidGenerator)) {
            if(! (toggleQuiet || toggleEmbed)) {
                std::cerr
                    << "Failed to close kernel "
                    "UUID file handle: fatal."
                    << std::endl;
        } return false; }
    }

    std::string hosts;
    { // Stack memory is only needed temporarily
        using process_t = FILE *;
        process_t hostsReader = popen(
            (std::stringstream()
                << "cat "
                << std::any_cast<std::string> (AYSTL_CMD_RESOURCES_NAME.at("config:hosts"))
                << " | brotli -d"
            ) .str().c_str()
          , "r"
        );

        if(! hostsReader) {
            if(! (toggleQuiet || toggleEmbed)) {
                std::cerr
                    << "Failed to open "
                       "host reader process: fatal."
                    << std::endl;
        } return false; }

        char buffer[0x10000];

        int read;
        while ((read = fread(buffer, 1, sizeof(buffer), hostsReader))) {
            hosts.append( std::string_view(buffer, read) );
        }

        if(pclose(hostsReader)) {
            if(! (toggleEmbed || toggleQuiet)) {
                std::cerr
                    << "Failed to close "
                       "host reader process: fatal."
                    << std::endl;
        } return false; }
    }

    std::string temporaryHostsDirectory = (
        std::stringstream()
            << "/tmp/gin-"
            << uuid
            << "/"
    ) .str();

    if(system((
        std::stringstream()
            << "mkdir "
            << temporaryHostsDirectory
        ) .str() .c_str()
    )   ) {
        if(! (toggleQuiet || toggleEmbed)) {
            std::cerr
                << "Failed to create "
                   "temporary directory: fatal."
                << std::endl;
    } return false; }

    {
        FILE * temporaryHostsHandle = fopen((std::stringstream()<<temporaryHostsDirectory<<"hosts.dat.temp").str().c_str(),"wb");
        if(! temporaryHostsHandle) {
            if(! (toggleQuiet || toggleEmbed)) {
                std::cerr
                    << "Failed to open temporary "
                       "hosts file: fatal."
                    << std::endl;
        } return false; }
        
        if(hosts.length() !=
            fwrite(hosts.data()
                 , 1
                 , hosts.length()
                 , temporaryHostsHandle
        )   ){
            if(! (toggleQuiet || toggleEmbed)) {
                std::cerr
                    << "Failed to write hosts data "
                       "to temporary hosts file: fatal."
                    << std::endl;
        } return false; }

        if(fclose(temporaryHostsHandle)) {
            if(! (toggleQuiet || toggleEmbed)) {
                std::cout
                    << "Failed to close temporary "
                       "hosts file: fatal"
                    << std::endl;
        } return false; }
    }

    if(system((std::stringstream()
        << "nano -LE0RSZG "
        << temporaryHostsDirectory
        << "hosts.dat.temp"
    ) .str() .c_str())) {
        if(! (toggleQuiet || toggleEmbed)) {
            std::cerr
                << "Failed to edit temporary "
                   "hosts file: fatal."
            << std::endl;
    } return false; }

    if(system(
        ( std::stringstream()
            << "cat "
            << temporaryHostsDirectory
            << "hosts.dat.temp | brotli -Zf > "
            << std::any_cast<std::string> (AYSTL_CMD_RESOURCES_NAME.at("config:hosts"))
        ) .str() .c_str()
    )   ) {
        if(! (toggleQuiet || toggleEmbed)) {
            std::cerr
                << "Failed to rewrite "
                   "hosts file: fatal."
                << std::endl;
    } return false; }
    
    if(system((
        std::stringstream()
            << "rm "
            << temporaryHostsDirectory
            << "hosts.dat.temp"
    ) .str() .c_str())) {
        if(! (toggleQuiet || toggleEmbed)) {
            std::cerr
                << "Failed to remove temporary "
                   "hosts file: fatal."
                << std::endl;
    } return false; }

    if(! (toggleQuiet || toggleEmbed)) {
        std::cout
            << "Updated hosts file "
               "successfully!"
            << std::endl;
    }

    return true;

}