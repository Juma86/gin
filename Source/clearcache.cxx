#include <header/clearcache.hxx>

AYSTL_CMD_METHOD(handleClearCache,AYSTL_CMD_TOGGLETAGS_NAME,AYSTL_CMD_COLLECTIONTAGS_NAME,) {

    const auto toggleQuiet { toggleTags.at("--quiet") };
    const auto toggleEmbed { toggleTags.at("--embed") };

    if(collectionTags.at("--to").size() > 1) {
        if(! (toggleQuiet || toggleEmbed))
            { std::cerr << "No user-defined location may be specified under --to; fatal." << std::endl; }
        return EXIT_FAILURE;
    }

    const std::string clearCacheCommand =
        ( std::stringstream()
            << "rm "
            << collectionTags.at("--to").back()
            << "/*"
        ) .str()
    ;

    if(! (toggleQuiet || toggleEmbed)) {
        std::cout << "Clearing cache..." << std::endl;
    }

    int returnCode =
        system(clearCacheCommand.c_str());

    if(! (toggleQuiet || toggleEmbed)) {
        std::cout << "Cached clear" <<
            ( returnCode
                ? "ing failed."
                : "ed successfully"
            ) << std::endl;
    }

    return returnCode == EXIT_SUCCESS;

}