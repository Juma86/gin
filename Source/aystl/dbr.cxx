#include <header/aystl/dbr.hxx>

// This functio relies on the following external programs : cat, brotli
std::optional<std::string> aystl::dbr::ReadDisk(const std::string& filePath) {

    using process_t = FILE *;

    constexpr auto brotliHandleDecompressionCommand { [](const std::string& filePath
    ) -> std::optional<std::string> {

        constexpr auto brotliGenerateDecompressionCommand { [](const std::string& filePath
        ) -> std::string {
            return ( std::stringstream()
                << "cat \""
                << filePath
                << "\" | brotli -d"
            ) .str();
        }   };

        const std::string brotliDecompressionCommand {
            brotliGenerateDecompressionCommand(filePath)
        };

        process_t brotliDecompressor = (process_t) popen (brotliDecompressionCommand.c_str(), "r");
        if(! brotliDecompressor) { return std::nullopt; }
    
        std::stringstream brotliDecompressorOutputStream;

        constexpr size_t brotliDecompressorOutputBufferSize = 8;
        char brotliDecompressorOutputBuffer[brotliDecompressorOutputBufferSize];
        while (fgets(brotliDecompressorOutputBuffer, brotliDecompressorOutputBufferSize, brotliDecompressor) != NULL) {
            brotliDecompressorOutputStream << brotliDecompressorOutputBuffer;
        }

        const std::string& brotliDecompressionCommandOutput {
            brotliDecompressorOutputStream.str()
        };

        return ! pclose(brotliDecompressor)
                  ? std::optional<std::string>(brotliDecompressionCommandOutput)  // Success
                  : std::optional<std::string>(std::nullopt                    ); // Failure
    }   };

    return brotliHandleDecompressionCommand(filePath);
}

bool aystl::dbr::WriteDisk(const std::string& filePath, const std::string& dbrFile ) {

    using process_t = FILE *;

    constexpr auto brotliHandleCompressionCommand { [](const std::string& filePath, const std::string& dbrFile){

        constexpr auto brotliGenerateCompressionCommand { [](const std::string& filePath, const std::string& dbrFile
        ) -> std::string {
            return (std::stringstream()
                << "bash -c 'echo -ne \""
                << dbrFile
                << "\" | brotli -Zf > "
                << filePath
                << "'"
            ) .str() ;
        }   };

        const std::string brotliCompressionCommand {
            brotliGenerateCompressionCommand(filePath, dbrFile)
        };

        process_t brotliCompressor = (process_t) popen (brotliCompressionCommand.c_str(), "r");
        if(! brotliCompressor) { return false; }

        return ! pclose(brotliCompressor)
                  ? true    // Success
                  : false ; // Failure
    }   };

    return brotliHandleCompressionCommand(filePath, dbrFile);
}
