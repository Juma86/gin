#include <header/search.hxx>
#include <yaml-cpp/yaml.h>

#include <ranges>

 // Currently only supports SFTP
AYSTL_CMD_METHOD(handleSearch,AYSTL_CMD_TOGGLETAGS_NAME,AYSTL_CMD_COLLECTIONTAGS_NAME,AYSTL_CMD_RESOURCES_NAME) {

    const auto& toggleQuiet { toggleTags.at("--quiet") };
    const auto& toggleEmbed { toggleTags.at("--embed") };

    if ( AYSTL_CMD_COLLECTIONTAGS_NAME. at("--count"). size() > 2 )
    {   if(! (toggleQuiet || toggleEmbed))
        {   std::cerr
                << "Count may only contain one user-defined value: fatal."
                << std::endl;
    } return false; }

    const std::optional<std::string>& hostsStringOpt =
        aystl::dbr::ReadDisk( std::any_cast <std::string> ( AYSTL_CMD_RESOURCES_NAME.at("config:hosts") ) );
    
    if(! hostsStringOpt.has_value())
    {   if(! (toggleQuiet || toggleEmbed))
        {   std::cerr
                << "Failed to load hosts "
                   "file from disk: fatal."
                << std::endl;
    } return false; }

    const std::string& hostsString =
        hostsStringOpt.value();
    
    // +1 as first/last entry shall not have leading/following newline
    std::vector<std::pair<std::string, aystl::net::host>> hosts( std::count(hostsString.cbegin(), hostsString.cend(), '\n') +1 );
    
    std::generate(hosts.begin(), hosts.end(), [hostIndex =-1, &hostsString](
    ) mutable -> std::pair<std::string, aystl::net::host> { hostIndex +=1;
        
        size_t startPosition = aystl::algorithm::find_nth_of(hostsString.cbegin()
                                                           , hostsString.cend()
                                                           , '\n', 0, hostIndex) + (hostIndex!=0?1:0) // this ternary is because elsewise the endline is included for parsing
             , endPosition   = aystl::algorithm::find_nth_of(hostsString.cbegin()
                                                           , hostsString.cend()
                                                           , '\n', 0, hostIndex +1)
             , length        = endPosition
                                 - startPosition
        ;
        std::string hostString = hostsString.substr(startPosition, length);

        return { hostString, aystl::net::ParseHost(hostString) };
    }   );

    typedef struct searchpoint_t { std::string name; std::vector<std::string> tags; } searchpoint_t;
    std::unique_ptr<std::pair<std::string, std::optional<std::vector<searchpoint_t>>>[]> hostsSearchPointsThreadFriendly { new std::pair<std::string, std::optional<std::vector<searchpoint_t>>>[hosts.size()] };
    for(const auto& host : hosts) {
        static std::size_t index = -1; index += 1;

        const auto& [hostString, hostStruct] {host};

        auto const& file = aystl::net::sftp::ReadRemoteFile(hostStruct, "meta/search-data.yaml");
        if(! file.has_value()) { hostsSearchPointsThreadFriendly[index] = {hostString, std::nullopt}; continue; }

        // File exists from this point forwards
        std::vector<searchpoint_t> hostSearchPoints;
        YAML::Node yamlFile = YAML::Load((const char *)(file.value().get()));

        for (auto const& packageEntryNode : yamlFile["packages"]) {
            auto const& name {
                packageEntryNode.first.as<std::string>()
            };

            auto const& packageAttributes {
                packageEntryNode.second
                    .as<std::unordered_map<
                        std::string
                      , std::vector<
                            std::string
            >>>() };

            std::optional<std::vector<std::string>> tags;

            try {
                tags = std::vector<std::string>();

                std::copy(
                    packageAttributes.at("tags").cbegin()
                  , packageAttributes.at("tags").cend()
                  , std::back_inserter(tags.value())
                );
            } // *Vomits*
            catch (const std::exception& e) {
                if(std::string(e.what()) != "unordered_map::at") { throw e; } // Gastly
                tags = std::nullopt;
            }

            const searchpoint_t searchPoint {
                .name { name }
              , .tags { tags.value_or(decltype(searchpoint_t::tags){}) }
            };

            hostSearchPoints.emplace_back( searchPoint );
        }

        hostsSearchPointsThreadFriendly[index] = {hostString, hostSearchPoints};

    }

    std::vector<std::pair<std::string, std::optional<std::vector<searchpoint_t>>>> hostsSearchPoints ( hosts.size() );
    for (std::size_t index = 0; index < hosts.size(); ++index) {
        hostsSearchPoints.at(index) =
            hostsSearchPointsThreadFriendly.get()[index]
        ;
    }

    const std::vector<std::string>& clientDesiredTags  { collectionTags.at("--tags"   ) };
    const std::vector<std::string>& clientQueries      { collectionTags.at("--query"  ) };
    const std::vector<std::string>& clientRegexFilters { collectionTags.at("--filters") };

    std::unordered_map<std::string, float> packageMatchLink;

    for (auto const& clientQuery : clientQueries) { // This is currently wrong, oh well.

        for (auto const& hostSearchPoints : hostsSearchPoints) {

            auto const& [hostString, packageSearchPoints] { hostSearchPoints };

            for (const auto& packageSearchPoint : packageSearchPoints.value()) {
                //std::cout << "[DEBUG] packagename : " << packageSearchPoint.name << std::endl;
                std::string_view packageName = {
                    packageSearchPoint.name.data()
                  , packageSearchPoint.name.find_last_of('-') != packageSearchPoint.name.npos // may be wrong
                      ? packageSearchPoint.name.find_last_of('-')
                      : packageSearchPoint.name.find_last_of('.')
                
                }; // std::cout << "[DEBUG] : processed package name : " << packageName << std::endl;

                // Do regex filter here
                bool skipPackageSearchPoint = false;
                for (const auto& regex_filter : clientRegexFilters) {
                    if(! std::regex_match(packageName.cbegin(), packageName.cend(), std::regex(regex_filter)))
                        { /*std::cout << "[DEBUG] : No regex match" << std::endl;*/ skipPackageSearchPoint = true; }
                } if (skipPackageSearchPoint) continue;

                // Get packagename match percentage
                // Where 1 represents full match & 0 represents no similarity
                float packageNameMatchValue { 1 };
                if(0 != clientQueries.size()) {
                    aystl::string aystlStringClientQuery { clientQuery }
                                , aystlStringPackageName { {packageName.data(), packageName.length()} }
                    ;
                    
                    packageNameMatchValue = 1
                    - ((float)aystlStringClientQuery. damerau_levenshtein_distance (aystlStringPackageName)
                        / (float)(std::max(aystlStringClientQuery.length(), aystlStringPackageName.length()))
                    );
                }

                // Get tag match percentage
                float packageTagMatchValue { 1 };
                if ( 0 != clientDesiredTags.size() ) [[ unlikely ]] {
                    const long maxMatchCount { (long) clientDesiredTags.size() };
                    long actualMatchCount { 0 };

                    for (auto const& clientDesiredTag : clientDesiredTags) {
                        for (auto const& packageTagName : packageSearchPoint.tags) {
                            actualMatchCount += (clientDesiredTag == packageTagName) ?1 :0;
                        }
                    }

                    packageTagMatchValue = (float)(actualMatchCount) / (float)(maxMatchCount);
                }

                const std::vector<std::size_t> packageWeightPriority = {7,11};
                const std::size_t packageWeightPrioritiesTotal = std::accumulate(packageWeightPriority.cbegin(), packageWeightPriority.cend(),0);
                const std::vector<float> packageWeightModifiers {
                    aystl::algorithm::apply(
                        packageWeightPriority
                      , [&packageWeightPrioritiesTotal]( const auto& prio
                        ) -> float {
                            return (float) prio / packageWeightPrioritiesTotal;
                }   )   };

                const std::vector<float>& customWeightedPackageScores {
                    aystl::algorithm::apply(
                        std::vector<float>{
                            packageNameMatchValue
                          , packageTagMatchValue
                        }
                      , [idx=-1,&packageWeightModifiers] ( const float& packageWeight
                        ) mutable->float {
                            return packageWeight
                              * packageWeightModifiers.at(++idx)
                            ;
                        }
                )   };

                const float finalPackageScore {
                    std::accumulate(
                        customWeightedPackageScores.cbegin()
                      , customWeightedPackageScores.cend(), 0.f
                    )
                };

                try {
                    if(packageMatchLink.at(std::string(packageName)) < finalPackageScore)
                        packageMatchLink.at(std::string(packageName)) = finalPackageScore;
                } catch ( const std::exception& e ) {
                    if(std::string(e.what()) != "unordered_map::at")
                        { throw e; }
                    
                    packageMatchLink.insert({std::string(packageName), finalPackageScore});

    }   }   }   }
    
    {
        using
            std::pair
          , std::vector
          , std::string
          , std::size_t
          , std::stoull
          , std::min
          , std::cout
          , std::views::counted
          , std::flush
          , std::endl
          , x86simdsort::object_qsort
        ;

        vector<pair<string,float>> packageScoreLinks
        {   packageMatchLink.cbegin(), packageMatchLink.cend()
        };

        object_qsort( packageScoreLinks.data(), packageScoreLinks.size()
          , [](pair<string,float>&_)->float {return _.second;}
        );
        
        size_t const resultsCount { min(
            packageScoreLinks.size()
          , (size_t) ( stoull ( AYSTL_CMD_COLLECTIONTAGS_NAME.at("--count").back() ) )
        )   };
        
        auto packageSearchResults { counted (packageScoreLinks.crbegin(), resultsCount) };
        
        cout << "Displaying top " << resultsCount << " results:" << flush;
        for (size_t i = -1; auto const& packageScoreLink : packageSearchResults) {
            cout << "\n\r\t " << ( ++ i + 1 ) <<  ") " << packageScoreLink.first;
        } cout << endl;
    }


    return true;

}