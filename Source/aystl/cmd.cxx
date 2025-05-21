#include <header/aystl/cmd.hxx>

#include <header/aystl/algorithm.hxx>
#include <memory>
#include <cstring>
#include <ranges>
#include <algorithm>

aystl::CommandLineProcessor::CommandLineProcessor(){}
    void aystl::CommandLineProcessor::
        SetToggleTags(const decltype(m_toggleTags)& toggleTags
        ) { this->m_toggleTags = toggleTags; }
    
    void aystl::CommandLineProcessor::
        SetCollectionTags(const decltype(m_collectionTags)& collectionTags
        ) { this->m_collectionTags = collectionTags; }

    void aystl::CommandLineProcessor::
        SetFunctionTags(const decltype(m_functionTags)& functionTags
        ) { this->m_functionTags = functionTags; }

    void aystl::CommandLineProcessor::
        SetAlternativeTags(const decltype(m_alternativeTags)& alternativeTags
        ) { this->m_alternativeTags = alternativeTags; }

    void aystl::CommandLineProcessor::
        SetResources(const decltype(m_resources)& resources
        ) { this->m_resources = resources; }

    void aystl::CommandLineProcessor::
        SetMacros(const decltype(m_macros)& macros
        ) { this->m_macros = macros; }

    bool aystl::CommandLineProcessor::
        ParseCLIArgs(int argc, char ** argv
        ) {
            if( m_macros.find(argv[1]) != m_macros.cend() ) {

                using cstring = char *;

                using
                    ::std::vector
                  , ::std::string
                  , ::std::size_t
                  , ::std::ranges::views::iota
                  , ::aystl::algorithm::concat_containers
                  , ::aystl::algorithm::apply
                ;

                vector<string> neoArgv = concat_containers
                (   { argv[0] }
                  , m_macros.at(argv[1])
                );

                for ( auto const& index : iota(2, argc) )
                    { neoArgv.emplace_back( argv[index] ); }


                vector<cstring> cstrings
                {   apply <vector<string>> ( neoArgv
                                           , [](auto const& argument) -> cstring {
                                                 return const_cast<cstring> (argument.c_str());
                }   )   };

                return ParseCLIArgs ( cstrings.size(), &cstrings[0] );
                
            }

            int argcIdx = 0 ; // Index 0 will be skipped; this is intentional.

            std::vector<
                std::function< bool (
                    decltype(m_toggleTags    ) const&
                  , decltype(m_collectionTags) const&
                  , decltype(m_resources     ) const&
            )>> actionFunctions;

            while (++ argcIdx < argc) {
                static auto handleToggleTag { [&](const auto& tagText) {
                    m_toggleTags.at(tagText) = true;
                } };
                static auto handleFunctionTag { [&](const auto& tagText) {
                    actionFunctions.emplace_back(this->m_functionTags.at(tagText));
                } };
                static auto handleCollectionTag { [&](const auto& tagText) {
                    while ((++argcIdx < argc) && (argv[argcIdx][0] != '-'))
                        { this->m_collectionTags.at(tagText).emplace_back(argv[argcIdx]); }
                    argcIdx -= 1; // next tag will be skipped if we don't go back
                } };
                static auto handleLongTag { [&](const auto& tagText) {
                    bool tagHit = false;

                    // These are intentionally not else if, as 1 tag may have multiple purposes :)
                    if(this->m_toggleTags.find(tagText) != this->m_toggleTags.end()) // toggle tag
                        { tagHit = true; handleToggleTag(tagText); }
                    if(this->m_functionTags.find(tagText) != this->m_functionTags.end()) // function tag
                        { tagHit = true; handleFunctionTag(tagText); }
                    if(this->m_collectionTags.find(tagText) != this->m_collectionTags.end()) // collection tag
                        { tagHit = true; handleCollectionTag(tagText); }
                    if(! tagHit) {
                        throw std::runtime_error(( std::stringstream()
                                << "Argument at position {"
                                <<argcIdx
                                <<"} -> ["
                                <<argv[argcIdx]
                                <<"] causes malformed invocation."
                            ).str().c_str());
                    }
                } };
                static auto handleItem { [&](const auto& tagText) {
                    // std::cout << "[DEBUG] : tagText -> " << tagText << std::endl;
                    // Value is a tag
                    if(tagText[0] == '-') {
                        // Double dash, long Tag, use all chars as ident
                        if(tagText[1] == '-') {
                            handleLongTag(tagText);
                        } else {
                            const decltype(tagText)& tagAlts { tagText.substr(1, std::string::npos) };
                            for ( const auto& tagAlt : tagAlts ) // These 2 for-loops could be replaced by an intersection alg
                            for ( const auto& [longIdent, altSet] : m_alternativeTags ) {
                                    if(altSet.find( std::string(&tagAlt, 1) ) != altSet.end())
                                        handleLongTag(longIdent);
                    }   }   }
                    
                    // Standalone value that is not part of a collection or tag
                    else throw std::runtime_error(( std::stringstream()
                        << "Argument at position {"
                        <<argcIdx
                        <<"} -> ["
                        <<argv[argcIdx]
                        <<"] causes malformed invocation."
                    ).str().c_str() );
                } };

                std::string tagText(argv[argcIdx]);

                handleItem (tagText);
            }

            //if(actionFunction.has_value())
            //    { return actionFunction.value()(this->m_toggleTags, this->m_collectionTags); }
            
            std::vector<bool> actionFunctionResults(actionFunctions.size());
            std::generate( actionFunctionResults.begin(), actionFunctionResults.end(), [index=-1, &actionFunctions, this](
            ) mutable -> bool { index += 1;
                return actionFunctions.at(index)(
                    this->m_toggleTags
                  , this->m_collectionTags
                  , this->m_resources
                );
            }   );

            return ((std::size_t) std::count(actionFunctionResults.cbegin(),actionFunctionResults.cend(),true)==actionFunctionResults.size())
                ? EXIT_SUCCESS
                : EXIT_FAILURE
            ;
        }