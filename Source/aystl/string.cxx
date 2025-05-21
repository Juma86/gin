#include <header/aystl/string.hxx>

std::size_t aystl::string::damerau_levenshtein_distance( const std::string_view other) {
    std::size_t l_string_length1 = this->length();
    std::size_t l_string_length2 = other.length();

    std::unordered_map<std::size_t, std::unordered_map<std::size_t, std::size_t>>d;

    std::size_t i;
    std::size_t j;
    std::size_t l_cost;

    for (i = 0;i <= l_string_length1;i++)
    {
        d[i][0] = i;
    }
    for(j = 0; j<= l_string_length2; j++)
    {
        d[0][j] = j;
    }
    for (i = 1;i <= l_string_length1;i++)
    {
        for(j = 1; j<= l_string_length2; j++)
        {
            if( this->at(i-1) == other[j-1] )
            {
                l_cost = 0;
            }
            else
            {
                l_cost = 1;
            }
            d[i][j] = std::min(
            d[i-1][j] + 1,                  // delete
            std::min(d[i][j-1] + 1,         // insert
            d[i-1][j-1] + l_cost)           // substitution
            );
            if( (i > 1) && 
            (j > 1) && 
            (this->at(i-1) == other[j-2]) && 
            (this->at(i-2) == other[j-1])
            ) 
            {
            d[i][j] = std::min(
            d[i][j],
             d[i-2][j-2] + l_cost   // transposition
            );
            }
        }
    }

    return d[l_string_length1][l_string_length2];
}