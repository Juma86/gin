#include <header/install/muc/read_subfile.hxx>

std::optional<std::string> ReadSubFile(const std::string& packagePath, const std::string& subfileName, const bool logErrors) {
    //typedef unsigned char byte;
    
    FILE * packageHandle {
        fopen(packagePath.c_str(), "rb")
    }; if(! packageHandle)
    {   if(logErrors) {
        std::cerr
            << "Failed to open package "
               "file handle: fatal."
            << std::endl;
    } return std::nullopt; }
    unsigned char MUCVersionConstant {};
    if(sizeof(MUCVersionConstant)
        != fread((void *) &MUCVersionConstant
               , 1, 1, packageHandle
    )){ if(logErrors) {
            std::cerr
                << "Failed to read package MUC"
                   " version constant: fatal."
                << std::endl;
    } fclose(packageHandle); return std::nullopt; }
    if(MUCVersionConstant != 0)
    {   if(logErrors) {
            std::cerr
                << "Unknown MUC version"
                   ": fatal. -> " << (int)MUCVersionConstant
                << std::endl;
    } fclose(packageHandle); return std::nullopt; }
    unsigned short metafileCount {};
    if(fseek(packageHandle, 4, SEEK_SET))
    {   if(logErrors) {
            std::cerr
                << "Failed to seek to "
                   "metafile-count: fatal."
                << std::endl;
    } fclose(packageHandle); return std::nullopt; }
    if(sizeof(metafileCount)
        != fread((void *) &metafileCount
               , 1, sizeof(metafileCount)
               , packageHandle))
    {   if(logErrors) {
        std::cerr
            << "Unable to read metafile "
               "count: fatal."
            << std::endl;
    } fclose(packageHandle); return std::nullopt; }

    if(fseek(packageHandle,6,SEEK_SET))
    {   if(logErrors)
        {   std::cerr
                << "Failed to seek to "
                   "metafile-section: fatal"
                << std::endl;
    } fclose(packageHandle); return std::nullopt; }
    for (unsigned short metafileIndex = 0
       ; metafileIndex < metafileCount
       ; metafileIndex += 1
    ) {
        std::pair<unsigned long long int
                , unsigned long long int> subfilenameBounds{};
        
        if(sizeof(subfilenameBounds)
            != fread(&subfilenameBounds
                   , 1, sizeof(subfilenameBounds)
                   , packageHandle))
        {   if(logErrors)
            {   std::cerr
                    << "Failed to read metafile["
                    << metafileIndex
                    << "]: fatal."
                    << std::endl;
        } fclose(packageHandle); return std::nullopt; }

        std::unique_ptr<char[]> filenameBuffer ( new char[subfilenameBounds.second
                                                            - subfilenameBounds.first] );
        
        fseek(packageHandle,subfilenameBounds.first,SEEK_SET);
        if(subfilenameBounds.second-subfilenameBounds.first !=
            fread(filenameBuffer.get(), 1, (subfilenameBounds.second-subfilenameBounds.first),packageHandle)) {
                if(logErrors) {
                    std::cerr << "Failed to read subfile name: fatal." << std::endl;
            } fclose(packageHandle); return std::nullopt; }

        const std::string subfilename {
            std::string(
                filenameBuffer.get()
              , subfilenameBounds.second
                  - subfilenameBounds.first
            )
        };

        if(subfilename == subfileName) {
            if(fseek(packageHandle,22+metafileIndex*32,SEEK_SET))
            {   if(logErrors)
                {   std::cerr
                        << "Failed to seek to desired "
                           "metafile data segment: fatal."
                        << std::endl;
            } fclose(packageHandle); return std::nullopt; }

            std::pair<unsigned long long int
                    , unsigned long long int> subfiledataBounds {};
            
            if(sizeof(subfiledataBounds)
                != fread(&subfiledataBounds, 1, sizeof(subfiledataBounds), packageHandle))
            {   if(logErrors)
                {   std::cerr
                        << "Failed to read "
                           "subfiledataBounds: fatal."
                        << std::endl;
            } fclose(packageHandle); return std::nullopt; }

            std::unique_ptr<char[]> subfileData( new char[subfiledataBounds.second
                                                            - subfiledataBounds.first]);
            
            if(fseek(packageHandle,subfiledataBounds.first,SEEK_SET))
            {   if(logErrors)
                {   std::cerr
                        << "Failed to seek to location "
                           "of "
                        << subfileName
                        << ": fatal."
                        << std::endl;
            } fclose(packageHandle); return std::nullopt; }

            if((subfiledataBounds.second-subfiledataBounds.first)
                 != fread(subfileData.get(), 1, (subfiledataBounds.second-subfiledataBounds.first),packageHandle))
            {   if(logErrors)
                {   std::cerr
                        << "Failed to read data for "
                        << subfileName
                        << ": fatal."
                        << std::endl;
            } fclose(packageHandle); return std::nullopt; }

            fclose(packageHandle);
            return std::string((const char *)subfileData.get(), subfiledataBounds.second-subfiledataBounds.first);
        }

        if(fseek(packageHandle,6+(metafileIndex+1)*32,SEEK_SET))
        {   if(logErrors)
            {   std::cerr
                    << "Failed to seek to "
                       "next metafile: fatal."
                    << std::endl;
        } fclose(packageHandle); return std::nullopt; }
    }

    fclose(packageHandle);

    return std::nullopt;
}