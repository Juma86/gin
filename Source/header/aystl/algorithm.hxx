#pragma once
#include <algorithm>
#include <numeric>
#include <vector>

namespace aystl::algorithm {
    template < typename _Thss
            , typename _Thse
            , typename _Tndl
            , typename _Tsid
            , typename _Tnth
    > _Tsid find_nth_of( const _Thss& haystackBegin // 1257863
                                        , const _Thse& haystackEnd
                                        , const _Tndl& needle
                                        , const _Tsid  start
                                        , const _Tnth  nth
    ) {
        if (! nth) return 0;
        const auto found { std::find_if(haystackBegin
                                        + start
                                    , haystackEnd
                                    , [&needle](const auto& it)
                                            { return needle == it; }
        )   };
        if ( nth-1 == 0 || found == haystackEnd )
            { return std::distance(haystackBegin, found); }
        
        return find_nth_of( haystackBegin
                        , haystackEnd
                        , needle
                        , std::distance(haystackBegin
                                        , found
                                        ) +1
                        , nth -1
        );
    }

    template <class Container, class Function>
    auto apply (const Container &cont, Function fun) {
        std::vector< typename
        std::result_of<Function(const typename Container::value_type&)>::type> ret;
        if(0 == cont.size())
        {   return ret;
        }
        ret.reserve(cont.size());
        for (const auto &v : cont) {
            ret.emplace_back(fun(v));
        }
        return ret;
    }

    template <class Container, class Function>
    auto apply_mutable (Container &cont, Function fun) {
        std::vector< typename
                std::result_of<Function(typename Container::value_type&)>::type> ret;
        ret.reserve(cont.size());
        for (auto &v : cont) {
            ret.emplace_back(fun(v));
        }
        return ret;
    }

    template <class Container1, class Container2, class ContainerReturn>
    ContainerReturn concat_containers ( Container1 const& C1, Container2 const& C2 ) {
        ContainerReturn ret;
        
        std::copy(C1.cbegin(),C1.cend(),std::back_inserter(ret));
        std::copy(C2.cbegin(),C2.cend(),std::back_inserter(ret));

        return ret;
    }

    template<typename T>
    T concat_containers ( T const& a, T const& b  ) {
        return concat_containers <T,T,T> ( a, b );
    }

    // Get value of first item in haystack that satisfies predicated, or default if none apply.
    template < class    _Tcont
             , typename _Tdflt
             , class    _Tpred
    > _Tdflt get_first_satisfier_or ( const _Tcont& hstk
                                    , const _Tdflt& dflt
                                    , const _Tpred& pred
    ) { for ( const auto& item : hstk )
        {   if (pred(item)) return item;
        } return dflt;
    }

    template < typename _Tret
             , class    _Thstk
             , typename _Tsize
    > _Tret get_container_average ( _Thstk const& hstk
                                  , _Tsize const& size
    ) { return (_Tret)
            std::accumulate(hstk.cbegin(), hstk.cend(), 0)
          / size
        ;
    }
/*
    template < typename _Trslt
             , typename _Tcnt1
             , typename _Tct1l
             , typename _Tcnt2
             , typename _Tct2l
    > std::optional<std::vector<_Trslt>> multiply_containers ( _Tcnt1 const& containerA
                                                             , _Tct1l const& containerASize
                                                             , _Tcnt2 const& containerB
                                                             , _Tct2l const& containerBSize
    ) {
        if( containerASize != containerBSize ) return std::nullopt;
        std::vector<_Trslt> rslts ( containerASize );
        std::generate( containerA.cbegin(), containerA.cend(), [idx=-1](auto const& containerItem)mutable->_Trslt{return containerAItem * containerB[++idx];} )
    }*/
}
