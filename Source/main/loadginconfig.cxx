#include <header/main/loadginconfig.hxx>

volatile constexpr const long GIN_CONFIG_MAX_SIZE { 0xFFFF };

std::pair<std::optional<toml::table>, loadginconfig_parseerror> LoadGinConfig ( std::string_view const& a_svGinConfigPath, bool const a_bLogError ) {

    try {
        std::filesystem::path const& f_fspGinConfigPath { a_svGinConfigPath };

        if(! std::filesystem::exists( f_fspGinConfigPath )) {
            return {
                std::nullopt
            , loadginconfig_parseerror::EC_FILE_NOT_FOUND
            };
        }

        FILE * f_fpGinConfig { fopen(a_svGinConfigPath.data(), "rb") };
        if( f_fpGinConfig == 0 ) {
            return {
                std::nullopt
            , loadginconfig_parseerror::EC_FILE_OPEN_FAIL
            };
        }

        if(0 != fseek(f_fpGinConfig, 0, SEEK_END)) {
            if (0 != fclose(f_fpGinConfig))
                return {std::nullopt, loadginconfig_parseerror::EC_FILE_EXIT_FAIL};
            return {
                std::nullopt
            , loadginconfig_parseerror::EC_FILE_SEEK_FAIL
            };
        }

        long const& f_lFileSize { ftell(f_fpGinConfig) };
        if(0 > f_lFileSize) {
            if (0 != fclose(f_fpGinConfig))
                return {std::nullopt, loadginconfig_parseerror::EC_FILE_EXIT_FAIL};
            return {
                std::nullopt
            , loadginconfig_parseerror::EC_FILE_TELL_FAIL
            };
        }

        if(f_lFileSize > GIN_CONFIG_MAX_SIZE) {
            if (0 != fclose(f_fpGinConfig))
                return {std::nullopt, loadginconfig_parseerror::EC_FILE_EXIT_FAIL};
            return {
                std::nullopt
            , loadginconfig_parseerror::EC_FILE_TOO_LARGE
            };
        }

        if(0 != fseek(f_fpGinConfig, 0, SEEK_SET)) {
            if (0 != fclose(f_fpGinConfig))
                return {std::nullopt, loadginconfig_parseerror::EC_FILE_EXIT_FAIL};
            return {
                std::nullopt
            , loadginconfig_parseerror::EC_FILE_SEEK_FAIL
            };
        }

        using byte = __uint8_t;
        std::unique_ptr<byte> f_upbFileReadBuffer ( new(std::nothrow) byte [f_lFileSize] );

        if(0 == f_upbFileReadBuffer) {
            if (0 != fclose(f_fpGinConfig))
                return {std::nullopt, loadginconfig_parseerror::EC_FILE_EXIT_FAIL};
            return {
                std::nullopt
            , loadginconfig_parseerror::EC_MEM_ALLOC_FAIL
            };
        }

        if(f_lFileSize != (long) fread(f_upbFileReadBuffer.get(), 1, f_lFileSize, f_fpGinConfig)) {
            if (0 != fclose(f_fpGinConfig))
                return {std::nullopt, loadginconfig_parseerror::EC_FILE_EXIT_FAIL};
            return {
                std::nullopt
            , loadginconfig_parseerror::EC_FILE_READ_FAIL
            };
        }

        if(0 != fclose(f_fpGinConfig))
            return {std::nullopt, loadginconfig_parseerror::EC_FILE_EXIT_FAIL};

        toml::table f_ttGinConfigTable;
        try { f_ttGinConfigTable = toml::parse( std::string_view { (const char *) f_upbFileReadBuffer.get(), (std::size_t) f_lFileSize } ); }
        catch( toml::parse_error const& e ) {
            if(a_bLogError)
                { std::cerr << e.what() << std::endl; }
            return {
                std::nullopt
              , loadginconfig_parseerror::EC_TOML_PARSE_FAIL
        };  }

        return { f_ttGinConfigTable, loadginconfig_parseerror::EC_OK };
    }

    catch( std::exception& e) {
        if(a_bLogError)
            { std::cerr << e.what() << std::endl; }
        return {
            std::nullopt
          , loadginconfig_parseerror::EC_UNKNOWN_EXCEPTION
        };
    }

    return {
        std::nullopt
      , loadginconfig_parseerror::EC_UNREACHABLE_AREA
    };

}