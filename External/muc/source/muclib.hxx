#pragma once

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
#include <libssh/sftp.h>

namespace fs = std::filesystem;

namespace muc {

using byte = unsigned char;

typedef unsigned char *binarydata;

typedef struct {
    unsigned long long int
        fileNameStartLocation,
        fileNameEndLocation,
        fileDataStartLocation,
        fileDataEndLocation;

} metafile;

typedef struct file {
    std::string
        filePath;

    binarydata
        binaryFile;

    unsigned long long int
        binaryFileSize;

    ~file() {}

} file;

enum VERSIONINFO_CONSTANTS_ENUM {
    DIRTYPE_FLAT = 0   ,
    DIRTYPE_SIMPLE     ,
    DIRTYPE_RECURSE    ,

    HASHTYPE_NONE      ,
    HASHTYPE_SHA256
};

// mucStandardID must always be first to allow for version checking on legacy
typedef struct {
    const unsigned char
        mucStandardID ,
        dirType       ,
        hashType      ;

    const unsigned short
        mucFileCount  ;

} versioninfo;

std::optional<muc::binarydata> AllocateBinaryData( const size_t& size );

void DeallocateBinaryData( std::optional<muc::binarydata>& data );


//reset a muc::file
void clean( muc::file& file );

//clean optional muc::file
void clean ( std::optional<muc::file>& optFile );

//reset a vector of muc::files
void clean( std::vector<muc::file>& files );

//clean optional vector of muc::files
void clean( std::optional<std::vector<muc::file>>& optFiles );

//clean vector of optional muc::files
void clean( std::vector<std::optional<muc::file>>& filesOpt );


//clean optional vector of optional muc::files
void clean( std::optional<std::vector<std::optional<muc::file>>>& optFilesOpt );

std::optional<muc::file> ReadDisk(const std::string& filePath);

// soon to be asynchronous
std::optional<std::vector<muc::file>> ReadDiskBatch(const std::vector<std::string>& filePaths);

bool fileExists(const std::string& filePath);

bool WriteDisk(const muc::file& fileToWrite, const std::string& filePath);

bool WriteDiskBatch(std::optional<std::vector<std::optional<muc::file>>>& optFilesToWrite, const std::string& filesPath);

std::optional<muc::file> BuildMucFile(const std::string& fileName, const std::optional<std::vector<muc::file>>& optionalFiles, const versioninfo& flags);

bool BuildMucFileToDisk(const std::string& fileName, std::optional<std::vector<muc::file>>& optFiles, const muc::versioninfo& versionInfo);

bool BuildMucFileToDiskFromDisk(const std::string& fileName, const std::vector<std::string>& filePaths, const muc::versioninfo& versionInfo);

std::vector<std::optional<muc::file>> BuildMucFileBatch(const std::vector<std::string>& fileName, std::vector<std::optional<std::vector<muc::file>>>& optMucSubFiles, const versioninfo& versionInfo);

const std::optional<std::pair<const muc::versioninfo, std::vector<muc::file>>> DissolveMucFile(const std::optional<muc::file>& optionalFile);

bool DissolveMucFileToDisk(const std::string& unsanitisedWritePath, std::optional<muc::file>& optMucFile);

const std::optional<std::pair<muc::versioninfo, std::vector<muc::file>>> DissolveMucFileFromDisk(const std::string& fileDir, const std::string& fileName);

bool DissolveMucFileToDiskFromDisk(const std::string& inFilePath, const std::string outDirPath, const unsigned long long int blockSize = 0x1000000);

std::optional<std::vector<std::string>> GetMucSubFileNames(const std::optional<muc::file>& optMucFile );

std::optional<std::vector<std::string>> GetMucSubFileNamesFromDisk( const std::string& filePath );

bool DissolveMUCSubFileFromFileHandleToFileHandle(const std::pair<FILE*,FILE*>& fileHandlePair, const std::string_view subfileName);

std::optional<std::unique_ptr<muc::byte[]>> DissolveMUCSubFileFromFileHandleToMemory(FILE* MUCFileHandle, std::string const& MUCInternalSubFilePath);

}
