#include <header/tests/aystl/algorithm/test_aystl_algorithm.hxx>

#include <mutex>

bool test::aystl::algorithm::TestFind_nth_of( test::aystl::algorithm::TestFind_nth_ofData const& data
                                            , std::function<void(std::stringstream const&)> const& errorLogger
) {

    std::vector<bool> successes;
    int index = -1;
    for (auto const& testCase : data.input.params)
    {   index += 1;
        auto const& testDataStructure
        {   std::get<0>(testCase)
        };
        auto const& foundIndex
        {   ::aystl::algorithm::find_nth_of( testDataStructure.cbegin()
                                           , testDataStructure.cend()
                                           , std::get<1>(testCase)
                                           , std::get<2>(testCase)
                                           , std::get<3>(testCase)
        )   };
        successes.emplace_back
        (   foundIndex == data.output.indices.at(index)
        );
        if(! (foundIndex == data.output.indices.at(index)))
        {   errorLogger( std::stringstream()
                << "Mismatch between found index and expected index : "
                << foundIndex
                << " vs "
                << data.output.indices.at(index)
    );  }   }
    bool const success
    {   (std::size_t) std::accumulate( successes.cbegin()
                                     , successes.cend()
                                     , 0
        ) == (std::size_t) data.input.params.size()
    };
    return success;

}