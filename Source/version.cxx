#include <header/version.hxx>
#include <string.h>

AYSTL_CMD_METHOD(handleVersion,,,AYSTL_CMD_RESOURCES_NAME)
{
    const char * versionString {
        std::any_cast<std::string_view> (resources.at("software:version")). data ()
    };

    // True if all data is written to stdout
    return (
        strlen(versionString)
     == fwrite(versionString
             , 1
             , strlen(versionString)
             , stdout
    )   );

}