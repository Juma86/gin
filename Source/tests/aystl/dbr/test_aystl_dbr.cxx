#include <header/tests/aystl/dbr/test_aystl_dbr.hxx>

#include <memory>
#include <string>
#include <string_view>

#include <simdutf.h>

bool test::aystl::dbr::TestDump( test::aystl::dbr::TestDumpData const& data )
{   bool passed = true;

    if
    (!  ::aystl::dbr::WriteDisk(
              std::string           (data.input.filePath)
          , std::string (data.input.data    )
    )   ) {
        std::cerr << "::aystl::dbr::WriteDisk : Failed to run function!" << std::endl;
        passed = false;

        return passed;
    }

    std::fstream testFile (data.input.filePath.data(), std::ios::in | std::ios::binary);
    if(! testFile.is_open())
    {   std::cerr << "::aystl::dbr::WriteDisk : Failed to open out'file to ensure correct output!" << std::endl;
        passed = false;

        return passed;
    }

    std::streampos begin, end;
    begin = testFile.tellg();
    testFile.seekg(0, std::ios::end);
    end  = testFile.tellg();
    testFile.seekg(0, std::ios::beg);

    std::size_t size ( end - begin );

    std::unique_ptr<char[]> fileDataBuffer    { new char[ size ] };
    std::unique_ptr<char[]> fileDataBufferB64 { new char[ simdutf::base64_length_from_binary(size) ]};

    testFile.read(fileDataBuffer.get(), size);

    testFile.close();
    
    simdutf::binary_to_base64(fileDataBuffer.get(), size, fileDataBufferB64.get());

    std::string fileDataB64(fileDataBufferB64.get(), simdutf::base64_length_from_binary(size));

    if ( data.output.data != fileDataB64 )
    {   std::cerr << "::aystl::dbr::WriteDisk : Data mismatch with expected value!" << std::endl;
        passed = false;
    }

    return passed;

}

bool test::aystl::dbr::TestLoad( test::aystl::dbr::TestLoadData const& data, std::function<void(std::stringstream const&)> const& errorLogger )
{   bool passed
    {   true
    };
    auto const& dataReadFromDisk
    {   ::aystl::dbr::ReadDisk( std::string (data.input.filePath) )
    };
    if(! dataReadFromDisk.has_value())
    {   errorLogger(std::stringstream() << "::aystl::dbr::ReadDisk : Function returned nothing!");
        passed = false;
        return passed;
    }
    if( dataReadFromDisk.value() != data.output.data )
    {   errorLogger(std::stringstream() << "aystl::dbr::ReadDisk : Data mismatch with expected value! -> " << dataReadFromDisk.value() << " vs " << data.output.data);
        passed = false;
        return passed;
    }
    return passed;
}