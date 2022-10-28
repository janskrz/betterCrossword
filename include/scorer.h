#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include "grid.h"

#include "INIReader.h"

namespace Crossword
{
    typedef std::int_fast32_t score;

    class Scorer
    {
    public:
        static std::unique_ptr<Scorer> create(std::string const &type, INIReader const &config);

        virtual score score_grid(Grid const &grid,
                                 std::int_fast32_t unplaced_word_count) const = 0;
    };
}