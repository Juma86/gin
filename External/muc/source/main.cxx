#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <optional>
#include <ctime>
#include <filesystem>
#include <functional>

#include <unistd.h>

#include "./muclib.hxx"

namespace fs = std::filesystem;

#define COUT_TRACE "\033[38;2;255;192;255m[TRACE] "
#define COUT_INFO  "\033[38;2;255;232;108m[INFO] "
#define COUT_ERROR "\033[38;2;255;32;32m[ERROR] "
#define COUT_WARN  "\033[38;2;255;255;12m[WARN] "

#ifndef HELP_MENU_B64
#define HELP_MENU_B64 ""
#endif

void displayHelpMenu(void){
    std::cout << COUT_INFO << std::flush;
    if( system((std::stringstream() << "echo \"" << HELP_MENU_B64 << "\" | base64 -d --wrap=0").str().c_str()))
        std::cout << COUT_ERROR << "System call failed!" << std::endl;
    else {} return;
}

void newMuc (const std::pair<std::vector<std::string>, std::vector<std::string>>& inFilesPair, const std::vector<std::string>& outFiles, const muc::versioninfo& versionInfo, const bool& quietRun ) {
    if (!quietRun) std::cout << COUT_TRACE << "Making new muc..." << std::endl;

    const auto& [inFilesAlias, inFiles] { inFilesPair };

    const auto& optInFilesData { muc::ReadDiskBatch(inFiles) };
    if (!optInFilesData.has_value()) {
        if (!quietRun) std::cout << COUT_ERROR << "Failed to read file(s) from disk!\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    } const auto& inFilesData { optInFilesData.value() };

    auto index { (unsigned long long int) ( -1 ) };
    auto aliasedInFiles { std::vector<muc::file>() };
    for (const auto& inFileAlias : inFilesAlias) {
        aliasedInFiles.push_back(
            muc::file{
                .filePath       = inFileAlias                         ,
                .binaryFile     = inFilesData[++index].binaryFile     ,
                .binaryFileSize = inFilesData[  index].binaryFileSize ,
            }
        );
    }

    const auto& optMucFile { muc::BuildMucFile( std::string("<?>"), aliasedInFiles, versionInfo) };

    if (!optMucFile.has_value()) {
        if(!quietRun) std::cout << COUT_ERROR << "Unable to build muc file!\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    } auto mucFile { optMucFile.value() };

    for (const auto& outFile : outFiles) {
        mucFile.filePath = outFile;
        const auto res {
            muc::WriteDisk(
                mucFile                                             ,
                outFile
                //fs::path(outFile).absolute().replace_filename( std::string() ).string()
        )   };
        if (!res) {
            std::cout << COUT_ERROR << "Failed to write file(s) to disk!\n\r\tExiting!" << std::endl;
        }
    } 
}

void dissolveMuc (const std::pair<std::vector<std::string>, std::vector<std::string>>& inFilesPair, std::vector<std::string> outFiles, const muc::versioninfo& /*versionInfo*/, const bool& quietRun, const bool& replace = false) {
    if (!quietRun) { std::cout << COUT_TRACE << "Dissolving muc..." << std::endl; }

    const auto& [inFilesAlias, inFiles] { inFilesPair };

    if (!quietRun && !outFiles.size()) {
        std::cout << COUT_WARN << "No outpath specified, dissolving to local directory!" << std::endl;
        outFiles.push_back( fs::path("./").root_path().string()  );
    }

    const auto& optMucFiles { muc::ReadDiskBatch(inFiles) };
    if (!optMucFiles.has_value()) {
        if (!quietRun) std::cout << COUT_ERROR << "Could not read file(s) from disk!\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    } const auto& mucFiles { optMucFiles.value() }; 

    auto pairToWriteFilesFlags { std::vector<std::pair<muc::versioninfo, std::vector<muc::file>>>() };
    for (const auto& mucFile : mucFiles) {

        const auto& optDissolvedData { muc::DissolveMucFile(mucFile) };
        if (!optDissolvedData.has_value()) {
            if (!quietRun) std::cout << COUT_ERROR << "unable to dissolve file(s)!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        } const auto& [dissolvedFlags, dissolvedFiles] { optDissolvedData.value() };

        pairToWriteFilesFlags.push_back( std::pair<muc::versioninfo, std::vector<muc::file>> { dissolvedFlags, dissolvedFiles} );
    }

    for (const auto& outPath : outFiles) {
        auto writeFailed { false };
        for (const auto& pairFlagsFiles : pairToWriteFilesFlags) {

            const auto& [flags, files] { pairFlagsFiles };

            for (const auto& file : files) {

                const auto& fileExistsTest { std::ifstream ( (fs::absolute(outPath).replace_filename(fs::path()).string() + file.filePath) ) };
                const auto& fileExists     { fileExistsTest.good() };

                const auto& filePath       { outPath + fs::path(file.filePath).parent_path().string() };

                switch ( flags.dirType ) {

                    case muc::DIRTYPE_SIMPLE:
                        fs::create_directories( filePath );
                        [[fallthrough]];

                    case muc::DIRTYPE_FLAT:
                        if ( (fileExists && replace) || !fileExists)
                            writeFailed |=! muc::WriteDisk( file, filePath );

                        break;
                    
                    default:
                        std::cout << COUT_ERROR << "DirType dissolve function not implemented for dirtype!\n\r\tSkipping!" << std::endl;
                }
            } if (writeFailed) {
                if (!quietRun) std::cout << COUT_ERROR << "Failed to write file(s) to disk!\n\r\tExitting!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

void appendMuc (const std::pair<std::vector<std::string>, std::vector<std::string>>& inFilesPair, const std::vector<std::string>& outFiles, const muc::versioninfo& versionInfo, const bool& quietRun, const std::string& priority = "start") {
    if (!quietRun) std::cout << COUT_TRACE << "Appending mucs..." << std::endl;

    const auto& [inFilesAlias, inFiles] { inFilesPair };

    const auto optToAppendFiles { muc::ReadDiskBatch(inFiles) };
    if (!optToAppendFiles.has_value()) {
        if (!quietRun) std::cout << COUT_ERROR << "Could not read file(s) from disk!\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    } auto toAppendFiles { optToAppendFiles.value() };

    const auto optToAppendMucs {muc::ReadDiskBatch(outFiles) };
    if (!optToAppendMucs.has_value()) {
        if (!quietRun) std::cout << COUT_ERROR << "Could not read file(s) from disk!\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    } auto toAppendMucs { optToAppendMucs.value() };

    auto dissolvedMucs { std::vector<std::pair<std::string, std::vector<muc::file>>>() };
    for (const auto& toAppendMuc : toAppendMucs) {

        const auto optMucSubFiles { muc::DissolveMucFile(toAppendMuc) };
        if (!optMucSubFiles.has_value()) {
            if (!quietRun) std::cout << COUT_ERROR << "Unable to dissolve muc file(s)!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        } const auto [mucSubFlags, mucSubFiles] { optMucSubFiles.value() };

        dissolvedMucs.push_back( {
            toAppendMuc.filePath  ,
            mucSubFiles
        }   ); 
    }

    auto toRebuildMucFiles { std::vector<std::pair<std::string, std::vector<muc::file>>>() };

    for (const auto& mucNameFilePair : dissolvedMucs) {
        //const auto [mucName, mucFiles] {mucNameFilePair};

        const auto
            mucName  { mucNameFilePair.first  };
        const auto
            mucFiles { mucNameFilePair.second };

        auto toRebuildMucFile { std::pair<std::string, std::vector<muc::file>>({mucName, {}}) };
        
        const std::vector<muc::file> priorityFiles {
            (priority == "start") ? mucFiles      :
            (priority == "end"  ) ? toAppendFiles :
            std::vector<muc::file>()
        }, toAttemptFiles {
            (priority == "end"  ) ? mucFiles      :
            (priority == "start") ? toAppendFiles :
            std::vector<muc::file>()
        };

        std::copy(
            priorityFiles.cbegin(), priorityFiles.cend() ,
            std::back_inserter(toRebuildMucFile.second)
        );

        std::copy_if(
            toAttemptFiles.cbegin(), toAttemptFiles.cend() ,
            std::back_inserter(toRebuildMucFile.second)  ,
            [&toRebuildMucFile](const muc::file& file){
                auto fileIncluded { false };
                for (const muc::file& preCopiedFile : toRebuildMucFile.second) {
                    fileIncluded |= (preCopiedFile.filePath == file.filePath);
                } return !fileIncluded;
            }
        );

        toRebuildMucFiles.push_back(toRebuildMucFile);
    }

    auto mucFiles { std::vector<muc::file>() };
    for (const auto& mucNameFilePair : toRebuildMucFiles) {
        const auto optMucFile { muc::BuildMucFile(mucNameFilePair.first, mucNameFilePair.second, versionInfo) }; // process version info

        if (!optMucFile.has_value()) {
            if (!quietRun) std::cout << COUT_ERROR << "Unable to build muc file(s)!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        } const auto mucFile { optMucFile.value() };

        mucFiles.push_back( mucFile );
    }

    auto index { 0 };
    auto writeFailed { false };
    for (const auto& mucFile : mucFiles) {
        writeFailed |=! muc::WriteDisk(mucFile, outFiles[index++]);
    }

    if (writeFailed && !quietRun) {
        std::cout << COUT_ERROR << "Could not write file(s) to disk!\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void removeMuc(const std::pair<std::vector<std::string>, std::vector<std::string>>& inFilesPair, const std::vector<std::string>& outFiles, const muc::versioninfo& versionInfo, const bool& quietRun) {
    if (!quietRun) std::cout << COUT_TRACE << "Removing files from muc..." << std::endl;
    
    const auto& [inFilesAlias, inFiles] { inFilesPair };

    const auto optTargetMucs { muc::ReadDiskBatch(outFiles) };
    if (!optTargetMucs.has_value()) {
        if (!quietRun) std::cout << COUT_ERROR << "Unable to read muc file(s) from disk\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    } const auto targetMucs { optTargetMucs.value() };

    auto fileNamesToFilter { inFiles };
    auto filteredMucs { std::vector<muc::file>() };
    for (const auto& targetMuc : targetMucs) {
        auto optMucSubFiles { muc::DissolveMucFile(targetMuc) };
        if (!optMucSubFiles.has_value()) {
            if (!quietRun) std::cout << COUT_ERROR << "Unable to dissolve muc file(s)!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        const auto [mucSubFlags, mucSubFiles] { optMucSubFiles.value() };

        auto filteredMucSubFiles { std::vector<muc::file>() };
        std::copy_if(
            mucSubFiles.cbegin(), mucSubFiles.cend() ,
            std::back_inserter(filteredMucSubFiles)  ,
            [&fileNamesToFilter](const muc::file& mucSubFile) -> bool {
                for (
                    const std::string& fileNameToFilter :
                    fileNamesToFilter ) {
                        if (fileNameToFilter == mucSubFile.filePath)
                            return false;
                } return true; } );

        const auto optFilteredMuc { muc::BuildMucFile(targetMuc.filePath, filteredMucSubFiles, versionInfo) }; // still sort out version info to make sure it is correct
        if (!optFilteredMuc.has_value()) {
            if (!quietRun) std::cout << COUT_ERROR << "Unable to build muc file(s)!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        }

        const auto filteredMuc { optFilteredMuc.value() };
        filteredMucs.push_back(filteredMuc);
    }

    auto iter { -1 };
    for ( const auto& filteredMuc : filteredMucs ) {
        auto res { muc::WriteDisk(filteredMuc, outFiles[++iter]) };
        if ( res ) continue;

        std::cout << COUT_ERROR << "Unable to write muc file(s)!\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void containsMuc(const std::pair<std::vector<std::string>, std::vector<std::string>>& inFilesPair, const std::vector<std::string>& outFiles, const muc::versioninfo& /*versionInfo*/, const bool& quietRun) {
    if (!quietRun) std::cout << COUT_TRACE << "Checking muc for files..." << std::endl;

    const auto& [inFilesAlias, inFiles] { inFilesPair };

    auto hit { std::vector<std::pair<std::string, std::vector<std::string>>>() };
    for (const auto& outFile : outFiles) {
        const auto optMucFile { muc::ReadDisk(outFile) };
        if (!optMucFile.has_value()) {
            if (!quietRun) std::cout << COUT_ERROR << "Unable to get muc file(s) from disk!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        } const auto mucFile { optMucFile.value() };

        const auto optMucSubFiles { muc::DissolveMucFile(mucFile) };
        if (!optMucSubFiles.has_value()) {
            if (!quietRun) std::cout << COUT_ERROR << "Unable to dissolve muc file(s)!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        } const auto [mucSubFlags, mucSubFiles] { optMucSubFiles.value() };

        auto mucSubFileNames { std::vector<std::string>() };
        for (const auto& mucSubFile : mucSubFiles)
            mucSubFileNames.push_back( mucSubFile.filePath );

        const auto targetFiles { inFiles };
        auto hitFiles { std::vector<std::string>() };
        std::copy_if(
            mucSubFileNames.cbegin(), mucSubFileNames.cend(),
            std::back_inserter(hitFiles),
            [&targetFiles](const std::string& mucSubFileName) -> bool {
                for (const auto& targetFileName : targetFiles)
                    if (targetFileName == mucSubFileName)
                        return true;
                return false;
            }
        );

        hit.push_back({
            outFile, hitFiles
        });
    }

    for (const auto& mucFileHitInfo : hit) {
        const auto [fileName, hitFiles] {mucFileHitInfo};

        std::cout << COUT_INFO << fileName << std::endl;
        for (const auto& hitFile : hitFiles) {
            std::cout << "|---" << hitFile << std::endl;
        }
    }
}

void catMuc(const std::pair<std::vector<std::string>, std::vector<std::string>>& inFilesPair, const std::vector<std::string>& outFiles, const muc::versioninfo& /*versionInfo*/, const bool& quietRun, const bool& embedRun) {
    if(! (quietRun || embedRun)) std::cout << "Catting mucs..." << std::endl;

    const auto& [inFiles, inFilesAlias] { inFilesPair };

    const auto& subFilesToCat { inFiles };

    const auto& mucFilesToSearch { outFiles };

    for ( const auto& mucFileToSearch : mucFilesToSearch ) {
    
        /*const auto& files { muc::DissolveMucFileFromDisk(mucFileToSearch, mucFileToSearch) };

        if(! files.has_value() &&! embedRun) {
            if(! quietRun) { std::cout << "Failed to dissolve muc file from disk!" << std::endl; }
            exit(EXIT_FAILURE);
        }

        for (const auto& file : files.value().second) {

            if(std::find(subFilesToCat.cbegin(), subFilesToCat.cend(), file.filePath) == subFilesToCat.cend())
                { continue; }
            
            if(! embedRun)std::cout << "file: " << mucFileToSearch << "@" << file.filePath << ":\n\r";
            std::cout.write((char *) (file.binaryFile), file.binaryFileSize);
            if(! embedRun)std::cout << std::endl;
        }*/

        for(const auto& subFileToCat : subFilesToCat) {
            FILE * mucFilePtr = fopen(mucFileToSearch.c_str(), "rb");
            if(! mucFilePtr &&! (quietRun || embedRun))
                { std::cout << COUT_ERROR << "Failed to open muc file(s) on disk!\n\r\tExitting!" << std::endl; return; }
            //FILE * E = fopen("./test","wb");
            const bool catWasSuccess = muc::DissolveMUCSubFileFromFileHandleToFileHandle(
                {mucFilePtr, stdout}, subFileToCat
            );

            if(! catWasSuccess) {
                if(! (quietRun || embedRun)) {
                    std::cout << COUT_ERROR << "Failed to cat sub file(s) from disk!\n\r\tExitting!" << std::endl; return;
                }
            }

            fclose(mucFilePtr); //fclose(E);
        }
        
    }
}

void mergeMuc (const std::pair<std::vector<std::string>, std::vector<std::string>>& inFilesPair, const std::vector<std::string>& outFiles, const muc::versioninfo& /*versionInfo*/, const bool& quietRun) {
    if (!quietRun) std::cout << COUT_TRACE << "Merging mucs..." << std::endl;

    const auto& [inFilesAlias, inFiles] { inFilesPair };

    const auto optMucFiles { muc::ReadDiskBatch( inFiles ) };
    if (!optMucFiles.has_value()) {
        if (!quietRun) std::cout << COUT_ERROR << "Unable to read muc file(s) from disk!\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    }

    const auto mucFiles { optMucFiles.value() };

    auto dissolvedMucSubFiles { std::vector<muc::file>() };

    for (const auto& mucFile : mucFiles) {
        const auto optDissolvedMuc { muc::DissolveMucFile( mucFile ) };

        if (!optDissolvedMuc.has_value()) {
            if (!quietRun) std::cout << COUT_ERROR << "Unable to dissolve muc file(s)!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        const auto [dissolvedFlags, dissolvedMuc] { optDissolvedMuc.value() };

        std::copy_if(
            dissolvedMuc.cbegin(), dissolvedMuc.cend() ,
            std::back_inserter(dissolvedMucSubFiles)   ,
            [&dissolvedMucSubFiles](const muc::file& mucSubFile){
                for ( const auto& dissolvedMucSubFile : dissolvedMucSubFiles ) {
                    std::cout << COUT_TRACE << "[ " << (dissolvedMucSubFile.filePath) << " | " << mucSubFile.filePath << " ]" << std::endl;
                    if ( dissolvedMucSubFile.filePath == mucSubFile.filePath)
                        return false;
                } return true;
            }
        );
    }

    const auto optMergedMucFile { muc::BuildMucFile("<null>", dissolvedMucSubFiles, muc::versioninfo{}) };
    if (!optMergedMucFile.has_value()) {
        if (!quietRun) std::cout << COUT_ERROR << "Unable to build muc file\n\r\tExitting!" << std::endl;
        exit(EXIT_FAILURE);
    } auto mergedMucFile { optMergedMucFile.value() };

    for (const auto& outFilePath : outFiles) {
        const auto writeSuccess { muc::WriteDisk(
            (   mergedMucFile.filePath = outFilePath,
                mergedMucFile                        ),
            outFilePath
        ) };
        
        if (!writeSuccess) {
            std::cout << COUT_ERROR << "Unable to write file(s) to disk!\n\r\tExitting!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void listMuc(const std::pair<std::vector<std::string>, std::vector<std::string>>& inFilesPair, const std::vector<std::string>& outFiles, const muc::versioninfo& /*versionInfo*/, const bool& quietRun) {

    const auto& [inFilesAlias, inFiles] { inFilesPair };

    std::vector<std::pair<std::string, std::optional<std::vector<std::string>>>> mucFileNameOptSubFileNames;

    for(const auto& inFile : inFiles) {
        mucFileNameOptSubFileNames.push_back({ inFile
          , muc::GetMucSubFileNamesFromDisk( inFile )
        });
    }

    std::stringstream textOutput = std::stringstream();

    for (const auto& nameFilesPair : mucFileNameOptSubFileNames) {
        textOutput << nameFilesPair.first << std::endl;
        for (const auto& subFileName
          : nameFilesPair.second.value_or(std::vector<std::string>{"\t-- invalid muc"})) {

        textOutput << "\t-- " << subFileName << std::endl;
    }   }

    if(! quietRun) std::cout << textOutput.str();

    for(const std::string& filePath : outFiles) {
        FILE * outFilePtr = fopen(filePath.c_str(), "wb");
        if(! outFilePtr) {
            if(! quietRun) std::cout << COUT_WARN
             << "Could not open outfile " << filePath << "\n\r\tIgnoring!" << std::endl;
            continue;
        }

        fwrite(textOutput.str().c_str(), 1, textOutput.str().length(), outFilePtr);
        fclose(outFilePtr);
    }
}

int main(const int argc, const char **argv) {
    const auto tStart { clock() };
    if(argc < 2) { std::cout << COUT_ERROR << "invalid program invocation, run ./[exec] --help for help." << std::endl; exit(EXIT_FAILURE); }

    auto programArgs { std::vector<std::string>() };
    {
        auto argIdx { argc };
        while (argIdx--) {
            programArgs.push_back(argv[argc-argIdx-1]);
        }
    }

    if ( std::find(programArgs.cbegin(), programArgs.cend(), "--help") != programArgs.cend() ) {
        displayHelpMenu();
        return EXIT_SUCCESS;
    }

    auto quietRun  { std::find(programArgs.cbegin(), programArgs.cend(), "--quiet" ) != programArgs.cend() };
    auto embedRun  { std::find(programArgs.cbegin(), programArgs.cend(), "--embed" ) != programArgs.cend() };
    // auto silentRun { std::find(programArgs.cbegin(), programArgs.cend(), "--silent") != programArgs.cend() };

    enum TagType { INFILES = 0, OUTFILES, ACTION, UNKNOWN };

    auto actionTags { std::set<std::string>({
        "-n" , "--new-muc"                    ,
        "-d" , "--dissolve-muc"               ,
        "-dI", "--dissolve-intact"            ,
        "-dR", "--dissolve-replace"           ,
        "-a" , "--append-files"               ,
        "-aI", "--append-intact"              ,
        "-aR", "--append-replace"             ,
        "-r" , "--remove-files"               , // for dir support use rR & rS for remove shallow && remove recursive
        "-c" , "--contains-files"             ,
        "-cC", "--cat-subfiles"               ,
        "-m" , "--merge-mucs"                 ,
        "-l" , "--list-files"                 ,
    }) };

    auto programAction { std::string("-?") };
    auto programArgTags { std::vector<std::pair<int, std::vector<std::string>>>() }; {
    auto index { -1 };
    while ((long unsigned int)++index < programArgs.size()) {
        const auto programArg { programArgs[index] };
        if (programArg.front() != '-') continue;

        auto nextTag {
            std::find_if(
                programArgs.cbegin() + index + 1,
                programArgs.cend(),
                [](const auto& t){
                    return t.front() == '-';
        }   )   };

        auto tagType {
            (programArg == "-i" || programArg == "--infiles" ) ? INFILES  :
            (programArg == "-o" || programArg == "--outfiles") ? OUTFILES :
            (actionTags.count(programArg)                    ) ? ACTION   :
            UNKNOWN
        };

        if (tagType == ACTION) programAction = programArg;

        programArgTags.push_back( {
            tagType, std::vector<std::string>{
                programArgs.cbegin() + index + 1,
                programArgs.cbegin() + std::distance(programArgs.cbegin(), nextTag)
    }}   );
    }}
    
    bool dirFlag { false };

    auto inFilesPair { std::pair< std::vector<std::string>, std::vector<std::string> >() };

    auto& [inFilesAlias, inFiles] { inFilesPair };

    auto
        inFilesRaw   { std::vector<std::string>{} },
        outFiles     { std::vector<std::string>{} }; {

    auto actionCount { 0 };
    for (const auto& tag : programArgTags) {
        if (ACTION   == tag.first) actionCount += 1;
        if (INFILES  == tag.first) std::copy(tag.second.cbegin(), tag.second.cend(), std::back_inserter(inFilesRaw ));
        if (OUTFILES == tag.first) std::copy(tag.second.cbegin(), tag.second.cend(), std::back_inserter(outFiles));
    } if (actionCount != 1) {
        if (!quietRun) std::cout << COUT_ERROR
            << "Invalid amount of actions passed to program, 1 should be passed, not ("
            << actionCount << ")." << std::endl;
        exit(EXIT_FAILURE);
    } for (const auto& inFile : inFilesRaw) {
        if ( (dirFlag |= std::filesystem::is_directory(inFile)) ) {
            const std::function<void(const std::string&)> readDirFiles {
                [&](const std::string& path) -> void {
                    for ( const auto& dirItem : std::filesystem::directory_iterator(path) ) {
                            if (std::filesystem::is_directory( dirItem.path().string() )) readDirFiles( dirItem.path().string() );
                            else { 
                                inFiles.push_back( dirItem.path().string() );
                                inFilesAlias.push_back(
                                    dirItem.path().string().substr(
                                        inFile.length(),
                                        dirItem.path().string().length() - inFile.length()
                )   );      }   }   }
            }; readDirFiles( inFile );
        } else { inFiles.push_back( inFile );  inFilesAlias.push_back( inFile ); }
    }}

    auto versionInfo { muc::versioninfo {
        .mucStandardID = 0                                                                   ,
        .dirType       = (unsigned char) (dirFlag ? muc::DIRTYPE_SIMPLE : muc::DIRTYPE_FLAT) ,
        .hashType      = muc::HASHTYPE_NONE                                                  ,
        .mucFileCount  = (short unsigned int)(inFiles.size() & 0xFFFF)                       ,
    } };

    if (programAction == "-n"  or programAction == "--new-muc"                ) { newMuc      (inFilesPair, outFiles, versionInfo, quietRun           ); } else
    if (programAction == "-d"  or programAction == "--dissolve-muc"           ) { dissolveMuc (inFilesPair, outFiles, versionInfo, quietRun           ); } else
    if (programAction == "-dI" or programAction == "--dissolve-intact"        ) { dissolveMuc (inFilesPair, outFiles, versionInfo, quietRun, false    ); } else // don't replace existing
    if (programAction == "-dR" or programAction == "--dissolve-replace"       ) { dissolveMuc (inFilesPair, outFiles, versionInfo, quietRun, true     ); } else // replace existing
    if (programAction == "-a"  or programAction == "--append-files"           ) { appendMuc   (inFilesPair, outFiles, versionInfo, quietRun           ); } else
    if (programAction == "-aI" or programAction == "--append-intact"          ) { appendMuc   (inFilesPair, outFiles, versionInfo, quietRun, "start"  ); } else
    if (programAction == "-aR" or programAction == "--append-replace"         ) { appendMuc   (inFilesPair, outFiles, versionInfo, quietRun, "end"    ); } else
    if (programAction == "-r"  or programAction == "--remove-files"           ) { removeMuc   (inFilesPair, outFiles, versionInfo, quietRun           ); } else
    if (programAction == "-c"  or programAction == "--contains-files"         ) { containsMuc (inFilesPair, outFiles, versionInfo, quietRun           ); } else
    if (programAction == "-cC" or programAction == "--cat-subfiles"           ) { catMuc      (inFilesPair, outFiles, versionInfo, quietRun, embedRun ); } else
    if (programAction == "-m"  or programAction == "--merge-mucs"             ) { mergeMuc    (inFilesPair, outFiles, versionInfo, quietRun           ); } else
    if (programAction == "-l"  or programAction == "--list-files"             ) { listMuc     (inFilesPair, outFiles, versionInfo, quietRun           ); }

    const auto tTaken { (double)(clock() - tStart) / CLOCKS_PER_SEC };
    if (!quietRun) std::cout << COUT_TRACE << "tTaken for proc => " << tTaken * 1000 << "milliseconds." << std::endl;

    return EXIT_SUCCESS;
}
