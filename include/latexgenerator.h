#pragma once

#include <fstream>

#include "grid.h"

namespace Crossword
{
    class LatexGenerator
    {
    private:
        void add_preamble(std::ofstream &) const;
        void add_puzzle_macros(std::ofstream &) const;
        void add_puzzle(Grid const &grid, std::ofstream &) const;
        void add_hints(Grid const &, std::ofstream &) const;
        void add_pagebreak(std::ofstream &) const;
        void add_solutionmode(std::ofstream &) const;
        void add_closing(std::ofstream &) const;

    public:
        void generate(Grid const &grid, std::string const &fileloc) const;
    };
} // namespace Crossword