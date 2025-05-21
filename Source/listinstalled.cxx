#include <header/listinstalled.hxx>

AYSTL_CMD_METHOD(handleListInstalled,,,AYSTL_CMD_RESOURCES_NAME) {

    auto const& dirListing
    {   ::aystl::fs::ListDirItems
        (   ::aystl::string
            {   std::any_cast<std::string>
                (   AYSTL_CMD_RESOURCES_NAME.at("registry:packages")
            )   }
          , ::aystl::fs::pathtype::directory
    )   };

    auto const& dirListingVec { dirListing.value_or( std::vector<::aystl::string>() ) };
    std::vector<aystl::string> packageNames;
    std::copy_if(
        dirListingVec.cbegin()
      , dirListingVec.cend()
      , std::back_inserter(packageNames)
      , []( auto const& packageName
        ) -> bool {
            return
            (   (packageName != "..")
             && (packageName != "." )
    );  }   );

    for (auto const& packageName : packageNames )
    {   std::cout << packageName << std::endl;
    }

    return dirListing.has_value();
}