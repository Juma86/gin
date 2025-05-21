#include <header/aystl/fs.hxx>

#include <iostream>

std::optional<std::vector<std::string>> aystl::fs::ListDirFiles(const std::string_view dirPath) {
    auto const& res
    {   ::aystl::fs::ListDirItems(
            aystl::string{dirPath.data()}
          , ::aystl::fs::pathtype::file
    )   };
    if(! res.has_value())
    {   return std::nullopt;
    }
    return aystl::algorithm::apply
    (   res.value()
      , [](::aystl::string const& aystlString
        ) -> std::string {
            return std::string(aystlString.data());
    }   );
}

std::optional<std::vector<::aystl::string>> aystl::fs::ListDirItems(aystl::string const& dirPath, ::aystl::fs::pathtype const& filter )
{   auto const& ResolvePathType
    {   [](aystl::fs::pathtype const& pathtype) -> decltype(DT_UNKNOWN) {
            switch (pathtype) {
                case ::aystl::fs::pathtype::directory: return DT_DIR;
                case ::aystl::fs::pathtype::file     : return DT_REG;
                default: return DT_UNKNOWN;
            }
    }   };
    std::vector<aystl::string> dirFileList;
    { // Stack memory only needed in this scope
        auto const& resolvedEntryType
        {   (filter != ::aystl::fs::pathtype::any)
              ? ResolvePathType(filter)
              : DT_UNKNOWN
        }; // DT_UNKNOWN used as placeholder
        DIR * dir;
        struct dirent * ent;
        if((dir = opendir(dirPath.data())) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if(ent->d_type == resolvedEntryType || filter == ::aystl::fs::pathtype::any)
                {   dirFileList
                        .emplace_back(
                            aystl::string{
                                ent->d_name
            }   );  }   }
            if(closedir(dir)) {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    }
    return dirFileList;
}

std::optional<bool> aystl::fs::PathIsDir(const std::string_view path) {

    using return_t = std::optional<bool>;

    struct stat pathStat;
    return 0 != stat(path.data(),&pathStat)
      ? (return_t) (std::nullopt)
      : (return_t) (pathStat.st_mode & S_IFDIR)
    ;

}