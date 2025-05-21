#include <optional>
#include <vector>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <numeric>
#include <filesystem>
#include <algorithm>
#include <string>
#include <functional>

#include <fcntl.h>
#include <libssh/sftp.h>

namespace fs = std::filesystem;

#include "./muclib.hxx"

namespace muc {

// returns [size] bytes muc::binarydata object if there is enough free memory
std::optional<muc::binarydata> AllocateBinaryData( const size_t& size ) {
    auto binaryData { malloc(size) };
    if (!(binaryData)) return std::nullopt;
    return (muc::binarydata)binaryData;
}

// frees muc::binarydata back to OS if allocated
void DeallocateBinaryData( std::optional<muc::binarydata>& data ) {
    if(!data.has_value())return;
    if(data.value() == nullptr) return;
    free(data.value());
}


//reset a muc::file
void clean( muc::file& file ) {
    if (file.binaryFile) free(file.binaryFile);
    file = muc::file();
}

//clean optional muc::file
void clean ( std::optional<muc::file>& optFile ) {
    if (!optFile.has_value()) return;
    auto& file { optFile.value() };
    muc::clean(file);
    optFile.emplace( muc::file() );
}

//reset a vector of muc::files
void clean( std::vector<muc::file>& files ) {
    for (auto& file : files)
        muc::clean(file);
    files = std::vector<muc::file>();
}

//clean optional vector of muc::files
void clean( std::optional<std::vector<muc::file>>& optFiles ) {
    if (!optFiles.has_value()) return;
    auto& files { optFiles.value() };
    muc::clean(files);
    optFiles.emplace( std::vector<muc::file>() );
}

//clean vector of optional muc::files
void clean( std::vector<std::optional<muc::file>>& filesOpt ) {
    for ( auto& optFile : filesOpt ) {
        if (!optFile.has_value()) continue;
        auto& file { optFile.value() };
        muc::clean(file);
    }
}


//clean optional vector of optional muc::files
void clean( std::optional<std::vector<std::optional<muc::file>>>& optFilesOpt ) {
    if (!optFilesOpt.has_value()) return;
    auto& filesOpt { optFilesOpt.value() };
    muc::clean( filesOpt );
    optFilesOpt.emplace( std::vector<std::optional<muc::file>>() );
}

// reads file at [filePath] if exists and returns muc::file else returns std::nullopt
std::optional<muc::file> ReadDisk(const std::string& filePath) {

    const auto file { fopen(filePath.c_str(), "rb") };
    if (!file) return std::nullopt;

    fseek(file, 0, SEEK_END);
    const auto fileSize { (unsigned long long int) ( ftell(file) ) };
    fseek(file, 0, SEEK_SET);

    auto fileData { (muc::binarydata)malloc(fileSize) };
    if (!fileData) { fclose(file); return std::nullopt; }

    const auto ures { fread(fileData, 1, fileSize, file) };
    if (ures != fileSize) { fclose (file); return std::nullopt; }

    fclose(file);

    return muc::file {
        //.fileName = std::filesystem::path(filePath).filename().string(),
        .filePath       = filePath,
        .binaryFile     = fileData,
        .binaryFileSize = fileSize
    };
}

// soon to be asynchronous
// Reads a vector of paths into a vector of muc::file's.
// If a read fails all files are ignored and returns std::nullopt
std::optional<std::vector<muc::file>> ReadDiskBatch(const std::vector<std::string>& filePaths) {
    auto outFiles { std::vector<muc::file>() };

    for (const auto& filePath : filePaths) {
        auto optFile { muc::ReadDisk(filePath) };
        if (!optFile.has_value()) {
            for (const auto& file : outFiles)
                { free(file.binaryFile); }
            return std::nullopt;
        } outFiles.push_back( optFile.value() );
    } return outFiles;
}

// #== NOT IMPLEMENTED ==#
// Returns a boolean for wether the specified filePath exists on disk
bool fileExists(const std::string&) {
    std::cout << "not implemented" << std::endl;
    return false;
}

// Writes [fileToWrite] to disk at [filePath]
// Returns true on success else false
bool WriteDisk(const muc::file& fileToWrite, const std::string& filePath) {

    const auto& absoluteFilePath { (
            fs::path(filePath).parent_path().string() + (fs::path(filePath).has_parent_path()?"/":"") +
            fs::path(fileToWrite.filePath).filename().string()
        )
    };
    printf("%s\n\r", absoluteFilePath.c_str());
    auto file { fopen( absoluteFilePath.c_str() , "wb" ) };
    if (!file) return false;
    fwrite(fileToWrite.binaryFile, 1, fileToWrite.binaryFileSize, file);
    fclose(file);

    return true;
}

// Writes a vector of [optFilesToWrite] to a single [filesPath], returns true on success for all files
bool WriteDiskBatch(std::optional<std::vector<std::optional<muc::file>>>& optFilesToWrite, const std::string& filesPath) {
    if (!optFilesToWrite.has_value()) { return false; }
    const auto& filesToWrite { optFilesToWrite.value() };
    auto writeFailed { false };
    for (const auto& optFileToWrite : filesToWrite) {
        if (writeFailed |= !optFilesToWrite.has_value()) continue;
        const auto& fileToWrite { optFileToWrite.value() };
        writeFailed |=! muc::WriteDisk( fileToWrite, filesPath );
    } return writeFailed;
}

// Builds a muc from files stored within memory, returns the muc stored within memory
std::optional<muc::file> BuildMucFile(const std::string& fileName, const std::optional<std::vector<muc::file>>& optionalFiles, const versioninfo& flags) {
    if (!optionalFiles.has_value()) { return std::nullopt; }
    const auto& files { optionalFiles.value() };

    unsigned long long int
        metaFileSectionSize { sizeof(muc::metafile) * flags.mucFileCount },
        nameFileSectionSize { 0 },
        dataFileSectionSize { 0 },
        infoFileSectionSize { sizeof( flags )} ;

    for (const auto& file : files)
        nameFileSectionSize += file.filePath.length() ,
        dataFileSectionSize += file.binaryFileSize    ;

    const auto& fileSize {
        metaFileSectionSize +
        nameFileSectionSize +
        dataFileSectionSize +
        infoFileSectionSize
    };

    const auto& filePtr { (muc::binarydata)malloc(fileSize) };
    if ( filePtr == nullptr ) return std::nullopt;

    memcpy(filePtr, &flags, sizeof(flags));

    const auto& metaFileSection { filePtr + infoFileSectionSize };
    const auto& nameFileSection { metaFileSection + metaFileSectionSize };
    const auto& dataFileSection { nameFileSection + nameFileSectionSize };

    unsigned long long int
        metaFileSectionOffset { 0 },
        nameFileSectionOffset { 0 },
        dataFileSectionOffset { 0 };

    for (const auto& file : files) {
        const auto metaFile { muc::metafile{
            .fileNameStartLocation = (unsigned long long int)(nameFileSection + nameFileSectionOffset                             - filePtr),
            .fileNameEndLocation   = (unsigned long long int)(nameFileSection + (nameFileSectionOffset += file.filePath.length()) - filePtr),
            .fileDataStartLocation = (unsigned long long int)(dataFileSection + dataFileSectionOffset                             - filePtr),
            .fileDataEndLocation   = (unsigned long long int)(dataFileSection + (dataFileSectionOffset += file.binaryFileSize)    - filePtr),
        }   };
        memcpy(metaFileSection + metaFileSectionOffset, &metaFile, sizeof(muc::metafile)); metaFileSectionOffset += sizeof(muc::metafile);
        memcpy(nameFileSection + nameFileSectionOffset - file.filePath.length(), file.filePath.c_str(), file.filePath.length());
        memcpy(dataFileSection + dataFileSectionOffset - file.binaryFileSize   , file.binaryFile      , file.binaryFileSize);
    }

    return muc::file {
        .filePath       = fileName ,
        .binaryFile     = filePtr  ,
        .binaryFileSize = fileSize ,
    };
}

// Builds a muc from files stored in memory, writes to disk.
// Returns true on success
bool BuildMucFileToDisk(const std::string& fileName, std::optional<std::vector<muc::file>>& optFiles, const muc::versioninfo& versionInfo) {

    if ( !optFiles.has_value() ) return false;
    const auto& files { optFiles.value() };

    const unsigned long long int
        metaFileSectionSize {
            sizeof(muc::metafile) * versionInfo.mucFileCount
        },

        nameFileSectionSize { (unsigned long long int)
            std::accumulate(
                files.cbegin(), files.cend(), 0,
                [](const unsigned long long int& acc, const muc::file& file){
                    return acc + file.filePath.length();
                }
            )
        },

        infoFileSectionSize {
            sizeof( versionInfo )
        }
    ;

    unsigned long long int
        infoFileSectionPos { 0                                        },
        metaFileSectionPos { infoFileSectionPos + infoFileSectionSize },
        nameFileSectionPos { metaFileSectionPos + metaFileSectionSize },
        dataFileSectionPos { nameFileSectionPos + nameFileSectionSize }
    ;

    const auto filePtr { fopen(fileName.c_str(), "wb") };
    if (!filePtr) return false; // file-creation failed

    fseek (filePtr, infoFileSectionPos, SEEK_SET);
    fwrite(&versionInfo, sizeof(versionInfo), 1, filePtr);

    for (const auto& file : files) {

        //write muc-local file path
        fseek (filePtr, nameFileSectionPos, SEEK_SET);
        fwrite(file.filePath.c_str(), file.filePath.length(), 1, filePtr);

        //write file binary data
        fseek (filePtr, dataFileSectionPos, SEEK_SET);
        fwrite(file.binaryFile, file.binaryFileSize, 1, filePtr);

        //write metafile
        fseek (filePtr, metaFileSectionPos, SEEK_SET);
        const auto& metaFile { muc::metafile {
                .fileNameStartLocation = nameFileSectionPos                             ,
                .fileNameEndLocation   = (nameFileSectionPos += file.filePath.length()) ,
                .fileDataStartLocation = dataFileSectionPos                             ,
                .fileDataEndLocation   = (dataFileSectionPos += file.binaryFileSize)
        }   };
        fwrite(&metaFile, sizeof(metaFile), 1, filePtr);
        metaFileSectionPos += sizeof(metaFile);

    }

    fclose(filePtr);

    return true;

}

// Builds a muc from [filePaths] and stored on disk at [fileName]
bool BuildMucFileToDiskFromDisk(const std::string& fileName, const std::vector<std::string>& filePaths, const muc::versioninfo& versionInfo) {

    const unsigned long long int
        infoFileSectionSize {
            sizeof(versionInfo)
        },

        metaFileSectionSize {
            sizeof(muc::metafile) * versionInfo.mucFileCount
        },

        nameFileSectionSize { (unsigned long long int)
            std::accumulate(
                filePaths.cbegin(), filePaths.cend(), 0,
                [](const unsigned long long int acc, const std::string& filePath){
                    return (unsigned long long int) (acc + filePath.length());
                }
            )
        }
    ;

    const auto outFilePtr { fopen(fileName.c_str(), "wb") };
    if (!outFilePtr) { return false; }

    fwrite(&versionInfo, 1, sizeof(versionInfo), outFilePtr);

    auto inFilePtrs { std::vector<FILE         *>( versionInfo.mucFileCount ) };
    auto metaFiles  { std::vector<muc::metafile >( versionInfo.mucFileCount ) };

    unsigned long long int
        infoFileSectionPos { 0                                        },
        metaFileSectionPos { infoFileSectionPos + infoFileSectionSize },
        nameFileSectionPos { metaFileSectionPos + metaFileSectionSize },
        dataFileSectionPos { nameFileSectionPos + nameFileSectionSize }
    ;

    std::generate(
        inFilePtrs.begin(), inFilePtrs.end(),
        [&filePaths](){
            static auto i { (unsigned long long int) -1 };
            return fopen( filePaths[++i].c_str(), "rb" );
        }
    );

    std::generate(
        metaFiles.begin(), metaFiles.end(),
        [&inFilePtrs, &filePaths, &nameFileSectionPos, &dataFileSectionPos](){
            static auto i { (unsigned long long int) -1 }; ++i;
            static auto namePos { (unsigned long long int) nameFileSectionPos };
            static auto dataPos { (unsigned long long int) dataFileSectionPos };

            // will reserve the current read position within inFilePtr
            const auto currentPos { (unsigned long long int) ftell(inFilePtrs[i]) };
            fseek(inFilePtrs[i], 0, SEEK_END);
            const auto inFileSize { (unsigned long long int) ftell(inFilePtrs[i]) };
            fseek(inFilePtrs[i], currentPos, SEEK_SET);

            return muc::metafile {
                .fileNameStartLocation = namePos,
                .fileNameEndLocation   = namePos += filePaths[i].length(),
                .fileDataStartLocation = dataPos,
                .fileDataEndLocation   = dataPos += inFileSize
            };
        }
    );

    for (const auto& metaFile : metaFiles)
        fwrite(&metaFile, 1, sizeof(metaFile), outFilePtr);

    for (const auto& filePath : filePaths)
        fwrite(filePath.c_str(), 1, filePath.length(), outFilePtr);

    const auto chunkSize { 1024 };
    unsigned char chunk[ chunkSize ] { 0 };
    auto readSize { (unsigned long long int) 0 };
    for (const auto& inFilePtr : inFilePtrs)

            while ((readSize = fread(chunk, 1, chunkSize, inFilePtr))) {
                fwrite(chunk, 1, readSize, outFilePtr);
            }

    for (const auto& inFilePtr : inFilePtrs)
	fclose(inFilePtr);

    fclose(outFilePtr);

    return true;

}

// Build a batch of mucs from files stored in memory, returns vector of optional mucs
std::vector<std::optional<muc::file>> BuildMucFileBatch(const std::vector<std::string>& fileNames, std::vector<std::optional<std::vector<muc::file>>>& optMucSubFiles, const versioninfo& versionInfo) {
    auto outMucFiles { std::vector<std::optional<muc::file>>() };
    auto iter { (unsigned long long int) -1 };
    for (auto& optSubFiles : optMucSubFiles) {
        if (!optSubFiles.has_value()) { outMucFiles.push_back(std::nullopt); continue; }
        auto subFiles   { optSubFiles.value() };
        auto optMucFile { muc::BuildMucFile(fileNames[++iter], subFiles, versionInfo) };
        if (!optMucFile .has_value()) { outMucFiles.push_back(std::nullopt); continue; }
        const auto& mucFile { optMucFile.value() };
        outMucFiles.push_back( mucFile );
    }

    return outMucFiles;
}

// Dissolve muc from memory, return files to memory on success else std::nullopt
const std::optional<std::pair<const muc::versioninfo, std::vector<muc::file>>> DissolveMucFile(const std::optional<muc::file>& optionalFile) {
    if (!optionalFile.has_value()) return std::nullopt;
    const auto& file { optionalFile.value() };

    const auto& filePtr { file.binaryFile };

    auto outFiles { std::vector<muc::file>{} };

    const muc::versioninfo& flags { *(muc::versioninfo*)filePtr };

    if (flags.mucStandardID == 0) {
        const auto fileCount { flags.mucFileCount };
        auto metaFiles { std::vector<muc::metafile>() };
        for ( unsigned short i { 0 }; i < fileCount; ++i )
            metaFiles.push_back( *(muc::metafile*)(filePtr + sizeof(flags) + sizeof(muc::metafile) * i) );

        for (const auto& m : metaFiles) {

            const auto& fileName { std::string((const char*)filePtr + m.fileNameStartLocation).substr(0, m.fileNameEndLocation - m.fileNameStartLocation)  };
            const auto& fileSize { m.fileDataEndLocation - m.fileDataStartLocation };
            const auto& fileData { (muc::binarydata)malloc(fileSize) };

            memcpy(fileData, filePtr + m.fileDataStartLocation, fileSize);

            outFiles.push_back(
                muc::file {
                    .filePath       = fileName,
                    .binaryFile     = fileData,
                    .binaryFileSize = fileSize
                }
            );
        }
    } else
    return std::nullopt;

    return std::pair<const muc::versioninfo, std::vector<muc::file>> ( {flags, outFiles} );
}

// Dissolve muc file from memory to disk at [unsanitisedWritePath]
bool DissolveMucFileToDisk(const std::string& unsanitisedWritePath, std::optional<muc::file>& optMucFile) {
    if (!optMucFile.has_value()) { return false; }
    const auto& mucFile { optMucFile.value() };
    const auto& writePath { fs::absolute(unsanitisedWritePath).replace_filename(fs::path()).string() }; // get directory path

    const auto& filePtr { mucFile.binaryFile };

    const muc::versioninfo& versionInfo { *(muc::versioninfo*)filePtr };

    if (versionInfo.mucStandardID == 0) {
        const auto& fileCount = versionInfo.mucFileCount;
        auto metaFiles { std::vector<muc::metafile>() };
        for ( unsigned short i { 0 }; i < fileCount; ++i ) {
            metaFiles.push_back( *(muc::metafile*)(filePtr + sizeof(versionInfo) + sizeof(muc::metafile) * i) );
        }

        for (const auto& metaFile : metaFiles) {
            const auto filePathLength { (size_t) metaFile.fileNameEndLocation - metaFile.fileNameStartLocation };
            const auto& filePath { std::string ((const char *) (filePtr + metaFile.fileNameStartLocation), filePathLength) };

            FILE * mucSubFile { fopen( (writePath + filePath).c_str() , "wb") };
            if (!mucSubFile) { return false; }

            const auto mucSubFileDataSize { (size_t) (metaFile.fileDataEndLocation - metaFile.fileDataStartLocation) };
            const bool writeStatus = mucSubFileDataSize == fwrite((mucFile.binaryFile + metaFile.fileDataStartLocation), 1, mucSubFileDataSize, mucSubFile);

            fclose(mucSubFile);

            if (!writeStatus) {
                remove( filePath.c_str() );
                return false;
            }
        }

    }

    return true;
}

// Dissolve muc file from disk to optional pair, including both versioninfo and files
const std::optional<std::pair<muc::versioninfo, std::vector<muc::file>>> DissolveMucFileFromDisk(const std::string& fileDir, const std::string& fileName) {
    const auto& filePath { (fs::absolute(fileDir).replace_filename(fs::path()).string() + fs::absolute(fileName).filename().string()) };
    FILE * fpMucFile { fopen(filePath.c_str(), "rb") };
    if (!fpMucFile) { return std::nullopt; }

    muc::versioninfo versionInfoBuffer[1] {};
    if ( sizeof(muc::versioninfo) != fread(versionInfoBuffer, 1, sizeof(muc::versioninfo), fpMucFile) ) { return std::nullopt; }
    const auto& versionInfo { muc::versioninfo ( *versionInfoBuffer ) };

    auto product { ( std::optional<std::pair<muc::versioninfo, std::vector<muc::file>>> ) ( std::nullopt ) };

    switch ( versionInfo.mucStandardID ) {
        case 0:

            [&product](FILE * fpMucFile, const muc::versioninfo& versionInfo) -> void {

                auto productFiles { std::vector<muc::file>() }; // possibly preallocate memory &&& memory leak if the function fails and quits without cleaning vector -- implement clean-file-batch function in muc nmsp!

                auto metaFileBuffer { muc::metafile () };

                auto metaFiles { std::vector<muc::metafile>( versionInfo.mucFileCount ) };
                for ( unsigned short i { 0 }; i < versionInfo.mucFileCount; ++i ) {
                    if (sizeof( muc::metafile ) != fread(&metaFileBuffer, 1, sizeof(muc::metafile), fpMucFile)) { muc::clean(productFiles); return; } // product default initialised to std::nullopt hence not set.
                    metaFiles[i] = ( muc::metafile ( metaFileBuffer ) );
                }

                for (const auto& metaFile : metaFiles) {

                    auto subFilePathBufferSize { (unsigned long long int) ( 0 ) } ; // junk value overwritten
                    auto subFilePathBuffer { malloc( subFilePathBufferSize = metaFile.fileNameEndLocation - metaFile.fileNameStartLocation ) };
                    if (!subFilePathBuffer) { muc::clean(productFiles); return; }

                    fseek(fpMucFile, metaFile.fileNameStartLocation, SEEK_SET);
                    if (subFilePathBufferSize != fread(subFilePathBuffer, 1, subFilePathBufferSize, fpMucFile)) { free(subFilePathBuffer); muc::clean(productFiles); return; }

                    const auto& subFilePath { std::string ( (char *) subFilePathBuffer ) };
                    free(subFilePathBuffer);

                    auto subFileDataBufferSize { (unsigned long long int) ( 0 ) };
                    auto subFileDataBuffer { malloc( subFileDataBufferSize = metaFile.fileDataEndLocation - metaFile.fileDataStartLocation ) };
                    if (!subFileDataBuffer) { muc::clean(productFiles); return; }

                    fseek(fpMucFile, metaFile.fileDataStartLocation, SEEK_SET);
                    if (subFileDataBufferSize != fread(subFileDataBuffer, 1, subFileDataBufferSize, fpMucFile)) { free(subFileDataBuffer); muc::clean(productFiles); return; }

                    productFiles.push_back( muc::file {
                        .filePath       = subFilePath,
                        .binaryFile     = (muc::binarydata)subFileDataBuffer,
                        .binaryFileSize = subFileDataBufferSize
                    } );

                }

            product.emplace( std::pair<muc::versioninfo, std::vector<muc::file>>( {versionInfo, productFiles} ) );

            }(fpMucFile, versionInfo); break;

        default: return std::nullopt;
    }

    return product;

}

// Dissolves muc stored on disk at [inFilePath] and writes subfiles to [outDirPath]
bool DissolveMucFileToDiskFromDisk(const std::string& inFilePath, const std::string outDirPath, const unsigned long long int blockSize) {

    auto buffer { (muc::binarydata) malloc ( blockSize ) }; // alloc to heap as buffer size may exceed stack
    if (!buffer) { return false; }

    FILE * inFile { fopen(inFilePath.c_str(), "rb") };
    if (!inFile) { free(buffer); return false;}

    muc::versioninfo versionInfoBuffer[1] {};
    if (sizeof(muc::versioninfo) != fread(versionInfoBuffer, 1, sizeof(muc::versioninfo), inFile)) { fclose(inFile); free(buffer); return false; }
    const auto& versionInfo { muc::versioninfo ( *versionInfoBuffer ) };

    switch (versionInfo.mucStandardID) {
        case 0:
            auto metaFiles { std::vector<muc::metafile>() };
            for (unsigned short i { 0 }; i < versionInfo.mucFileCount; ++i) {
                auto metaFile { muc::metafile () };
                if (sizeof(muc::metafile) != fread(&metaFile, 1, sizeof(muc::metafile), inFile)) { fclose(inFile); free(buffer); return false; }
                metaFiles.push_back( metaFile );
            }

            for (const auto& metaFile : metaFiles) {
                const auto fileNameLength { (unsigned long long int) ( metaFile.fileNameEndLocation - metaFile.fileNameStartLocation ) };
                char * fileNameBuffer { (char *) malloc ( fileNameLength +1)};
                if (!fileNameBuffer) { fclose(inFile); free(buffer); return false; }

                fseek(inFile, metaFile.fileNameStartLocation, SEEK_SET);
                if (fileNameLength != fread(fileNameBuffer, 1, fileNameLength, inFile)) { fclose(inFile); free(fileNameBuffer); free(buffer); return false; }
                const auto& fileName { std::string (fileNameBuffer, fileNameLength) };
                free(fileNameBuffer);

                const auto& outFilePath { (fs::absolute(outDirPath).replace_filename(fs::path()).string() + fs::path(fileName).string()) };
                fs::create_directories( fs::path(outFilePath).replace_filename(fs::path()).string() );

                FILE * outFile { fopen (outFilePath.c_str(), "wb") };
                if (!outFile) { fclose(inFile); free(buffer); return false; }

                fseek(inFile, metaFile.fileDataStartLocation, SEEK_SET);
                unsigned long long int fileDataSize { metaFile.fileDataEndLocation - metaFile.fileDataStartLocation };

                for (unsigned long long int fPos { 0 }; fPos < fileDataSize; fPos += blockSize) {
                    unsigned long long int readSize = (fPos + blockSize) < fileDataSize ? ( blockSize ) : (fileDataSize - fPos);

                    if (readSize != fread (buffer, 1, readSize, inFile )) { fclose(outFile); fclose(inFile); free(buffer); return false; } // secure?
                    if (readSize != fwrite(buffer, 1, readSize, outFile)) { fclose(outFile); fclose(inFile); free(buffer); return false; }
                }

                fclose(outFile);

            }
        break;

    }
    fclose(inFile);
    free(buffer);

    return true;
}

// Returns all file names of items stored within muc, muc being stored in memory
std::optional<std::vector<std::string>> GetMucSubFileNames(const std::optional<muc::file>& optMucFile ) {
    if(! optMucFile.has_value()) { return std::nullopt; }
    const auto& mucFile = optMucFile.value();

    auto fileCount = ((muc::versioninfo*)mucFile.binaryFile)->mucFileCount;

    std::vector<std::string> subFilePaths;
    for(long i = 0; i < fileCount; i += 1) {

        const auto metaFile = * (muc::metafile*)
        (
            mucFile.binaryFile       +
            sizeof(muc::versioninfo) +
            sizeof(muc::metafile)    * i
        );
        
        subFilePaths.emplace_back(
            std::string((char*) (mucFile.binaryFile + metaFile.fileNameStartLocation)
              , metaFile.fileNameEndLocation - metaFile.fileNameStartLocation
            )
        );
    }

    return subFilePaths;
}

// Returns all file names of items stored within muc, muc being stored on disk at [filePasth]
std::optional<std::vector<std::string>> GetMucSubFileNamesFromDisk( const std::string& filePath ) {
    if(! fs::exists(filePath)) { return std::nullopt; }

    FILE * mucFilePtr = fopen( filePath.c_str(), "rb" );
    if(! mucFilePtr) { return std::nullopt; }

    muc::versioninfo versionInfo {};

    if(fread( &versionInfo, 1, sizeof(muc::versioninfo), mucFilePtr )
        != sizeof(muc::versioninfo)) { fclose(mucFilePtr); return std::nullopt; }

    std::vector<std::string> fileNames;
    for(long i = 0; i < versionInfo.mucFileCount; i += 1) {                                                                                                                                            // MEMORY MANAGE
        muc::metafile metaFile;

        // get meta file
        fseek(mucFilePtr,
            sizeof(muc::versioninfo) +
            sizeof(muc::metafile   ) * i ,
        SEEK_SET);

        if(fread( &metaFile, 1, sizeof(metafile), mucFilePtr )
            != sizeof(muc::metafile)) { fclose(mucFilePtr); return std::nullopt; }

        // get sub-file name
        fseek( mucFilePtr, metaFile.fileNameStartLocation, SEEK_SET );

        size_t subFileNameLength = metaFile.fileNameEndLocation
          - metaFile.fileNameStartLocation;
        
        char* subFileName = new char[subFileNameLength];
        if(fread( subFileName, 1, subFileNameLength, mucFilePtr )
            != subFileNameLength) { fclose(mucFilePtr); delete[] subFileName; return std::nullopt; }

        fileNames.emplace_back( std::string(
            subFileName, subFileNameLength
        )   );

        delete[] subFileName;
    }

    fclose(mucFilePtr);
    return fileNames;

}

bool DissolveMUCSubFileFromFileHandleToFileHandle(const std::pair<FILE*,FILE*>& fileHandlePair, const std::string_view subfileName)
{   
    constexpr const auto HandleMUCVersion0 { [](const std::pair<FILE*,FILE*>& fileHandlePair
                                        , const muc::versioninfo& MUCVersionInfo
                                        , const std::string_view subfileName
    ) -> bool {

        const auto& [infileHandle, outfileHandle] { fileHandlePair };

        for(std::size_t index = 0; index < MUCVersionInfo.mucFileCount; ++index) {
            muc::metafile metaFile;

            if(fseek(infileHandle,sizeof(muc::versioninfo)+sizeof(muc::metafile)*index,SEEK_SET))
                { return false; }
            if(sizeof(metaFile) !=
                fread(&metaFile, 1, sizeof(metaFile), infileHandle)
            ) { return false; }
            
            if(fseek(infileHandle, metaFile.fileNameStartLocation, SEEK_SET))
                { return false; }

            const std::size_t filenameLength {
                metaFile.fileNameEndLocation
                    - metaFile.fileNameStartLocation
            };
            
            { // filenameBuffer's memory is only needed here
                std::unique_ptr<char[]> filenameBuffer( new char[filenameLength] );
                if(filenameLength !=
                    fread(filenameBuffer.get(), 1, filenameLength, infileHandle)
                ) { return false; }

                if(std::strcmp(filenameBuffer.get(), subfileName.data()))
                    { continue; }
            }
            // Correct file found from this point forth.
            if(fseek(infileHandle, metaFile.fileDataStartLocation, SEEK_SET))
                { return false; }
            
            decltype(muc::metafile::fileDataStartLocation) fileDataIterator
                { metaFile.fileDataStartLocation };

            while ( fileDataIterator < metaFile.fileDataEndLocation )
            {   constexpr const static std::size_t fileDataBufferSize { 2 };
                static unsigned char fileDataBuffer[ fileDataBufferSize ];

                const auto fileDataReadWriteSize {
                    std::min( (unsigned long long int) fileDataBufferSize
                           , metaFile.fileDataEndLocation
                               - fileDataIterator
                )   };

                if(fileDataReadWriteSize !=
                    fread(fileDataBuffer, 1, fileDataReadWriteSize, infileHandle)
                ) { return false; }
                if(fileDataReadWriteSize !=
                    fwrite(fileDataBuffer, 1, fileDataReadWriteSize, outfileHandle)
                ) { return false; }

                fileDataIterator += fileDataReadWriteSize;
            } break;

        }

        return true;
    }   };
    
    const auto& [infileHandle, outfileHandle] {fileHandlePair};

    muc::versioninfo MUCVersionInfo {};
    if(sizeof(MUCVersionInfo) !=
        fread(&MUCVersionInfo, 1, sizeof(MUCVersionInfo), infileHandle)
    ) { return false; }

    switch( MUCVersionInfo.mucStandardID ) {
        case 0: return HandleMUCVersion0(fileHandlePair, MUCVersionInfo, subfileName);
        default: return false;
    }

}

std::optional<std::unique_ptr<muc::byte[]>> DissolveMUCSubFileFromFileHandleToMemory(FILE* MUCFileHandle, std::string const& MUCInternalSubFilePath)
{   constexpr const auto HandleMUCVersion0 { [](FILE * MUCFileHandle
                                        , const muc::versioninfo& MUCVersionInfo
                                        , const std::string_view subfileName
    ) -> std::optional<std::unique_ptr<muc::byte[]>> {

        for(std::size_t index = 0; index < MUCVersionInfo.mucFileCount; ++index)
        {   muc::metafile metaFile;

            if(fseek(MUCFileHandle,sizeof(muc::versioninfo)+sizeof(muc::metafile)*index,SEEK_SET)
            ) { return std::nullopt; }
            if(sizeof(metaFile) !=
                fread(&metaFile, 1, sizeof(metaFile), MUCFileHandle)
            ) { return std::nullopt; }
            if(fseek(MUCFileHandle, metaFile.fileNameStartLocation, SEEK_SET)
            ) { return std::nullopt; }

            const std::size_t filenameLength
            {   metaFile.fileNameEndLocation
                    - metaFile.fileNameStartLocation
            };
            
            { // filenameBuffer's memory is only needed here
                std::unique_ptr<char[]> filenameBuffer( new char[filenameLength] );
                if(filenameLength !=
                    fread(filenameBuffer.get(), 1, filenameLength, MUCFileHandle)
                ) { return std::nullopt; }
                if(std::strcmp(filenameBuffer.get(), subfileName.data())
                ) { continue; }
            }
            // Correct file found from this point forth.
            if(fseek(MUCFileHandle, metaFile.fileDataStartLocation, SEEK_SET))
                { return std::nullopt; }

            std::size_t const& internalSubFileSize
            {   metaFile.fileDataEndLocation
                  - metaFile.fileDataStartLocation
            };
            std::unique_ptr<muc::byte[]> internalSubFileBinaryData
            (   new (std::nothrow) muc::byte [internalSubFileSize + 1] // +1 for size of null terminal
            );
            if(nullptr == internalSubFileBinaryData.get()
            ) { return std::nullopt; }
            internalSubFileBinaryData.get()[internalSubFileSize] = '\0'; // null terminate!!! [][][] Sadly had to be a bugfix silly billy tut tut tut [][]][]
            if(internalSubFileSize !=
                fread(internalSubFileBinaryData.get(), 1, internalSubFileSize, MUCFileHandle)
            ) { return std::nullopt; }

            return internalSubFileBinaryData;
        }

        // Will be reached if there is no subfile matching the name requested within the muc archive
        return std::nullopt;
    }   };

    if(0 != fseek(MUCFileHandle, 0, SEEK_SET))
        { return std::nullopt; }
    muc::versioninfo MUCVersionInfo {};
    if(sizeof(MUCVersionInfo) !=
        fread(&MUCVersionInfo,1,sizeof(MUCVersionInfo),MUCFileHandle)
    ) { return std::nullopt; }

    switch ( MUCVersionInfo.mucStandardID )
    {   case 0: return HandleMUCVersion0(MUCFileHandle, MUCVersionInfo, MUCInternalSubFilePath);
        default: return std::nullopt;
    }

    // Should be unreachable.
    return std::nullopt;
}


}