#include <algorithm>
#include <iostream>

#include "latexgenerator.h"

using namespace Crossword;

void LatexGenerator::add_preamble(std::ofstream &of) const
{
    of << R"""(
\documentclass{article}

\usepackage{amssymb}
\usepackage{multicol}
\usepackage{enumitem}
\usepackage[a4paper, margin=1cm]{geometry}
\usepackage[small]{cwpuzzle}

\setlist[enumerate]{itemsep=0mm}

\begin{document}
\pagestyle{empty}
)""";
}

void LatexGenerator::add_puzzle_macros(std::ofstream &of) const
{
    of << R"""(
\renewcommand{\PuzzleUnitlength}{13pt}
\newcommand{\cluer}[1]{\textbf{#1}^\blacktriangleright}
\newcommand{\clued}[1]{\textbf{#1}\blacktriangledown}
)""";
}

void LatexGenerator::add_puzzle(Grid const &grid, std::ofstream &of) const
{
    // add 1 to height and width as we place the word-starting markers in the cell above
    // or left of the first cell of the word (depending on the orientation)
    of << std::endl
       << "\\begin{Puzzle}{"
       << grid->get_width() + 1 << "}{" << grid->get_height() + 1 << "}" << std::endl;

    // lambda to construct a single cell string
    auto fill_cell = [&, grid](std::ofstream &of, char cell_content,
                               std::int_fast32_t vert_marker, std::int_fast32_t hori_marker)
    {
        of << "|";

        // add marker to mark the beginning of a word
        if (vert_marker > 0 || hori_marker > 0)
        {
            of << "[$";
            if (vert_marker > 0)
                of << "_{\\clued{" << vert_marker << "}}";
            if (hori_marker > 0)
                of << "^{\\cluer{" << hori_marker << "}}";
            of << "$]";
        }

        if (cell_content == _Grid::EMPTY_CHAR)
        {
            of << "{}";
        }
        else
        {
            of << " " << cell_content;
        }
    };

    auto vert_count = 0, hori_count = 0;
    for (auto i = 0; i <= grid->get_height(); i++)
    {
        for (auto j = 0; j <= grid->get_width(); j++)
        {
            Word const *vword = grid->get_word_starting_at(i, j - 1, Direction::VERTICAL);
            Word const *hword = grid->get_word_starting_at(i - 1, j, Direction::HORIZONTAL);
            auto vmarker = 0, hmarker = 0;
            if (vword != nullptr)
                vmarker = ++vert_count;
            if (hword != nullptr)
                hmarker = ++hori_count;
            fill_cell(of, grid->get_cell_content(i - 1, j - 1), vmarker, hmarker);
        }
        of << "|." << std::endl;
    }
    of << "\\end{Puzzle}" << std::endl
       << std::endl;
}

void LatexGenerator::add_hints(Grid const &grid, std::ofstream &of) const
{
    of << "\\begin{multicols*}{2}" << std::endl;
    of << "VERTICAL CLUES" << std::endl;
    of << "\\begin{enumerate}" << std::endl;
    for (auto i = 0; i <= grid->get_height(); i++)
    {
        for (auto j = 0; j <= grid->get_width(); j++)
        {
            Word const *vword = grid->get_word_starting_at(i, j, Direction::VERTICAL);
            if (vword != nullptr)
            {
                of << "\\item " << vword->clue << std::endl;
            }
        }
    }
    of << "\\end{enumerate}" << std::endl;
    of << "\\vfill\\null" << std::endl;
    of << "\\columnbreak" << std::endl;

    of << "HORIZONTAL CLUES" << std::endl;
    of << "\\begin{enumerate}" << std::endl;
    for (auto i = 0; i <= grid->get_height(); i++)
    {
        for (auto j = 0; j <= grid->get_width(); j++)
        {
            Word const *vword = grid->get_word_starting_at(i, j, Direction::HORIZONTAL);
            if (vword != nullptr)
            {
                of << "\\item " << vword->clue << std::endl;
            }
        }
    }
    of << "\\end{enumerate}" << std::endl;
    of << "\\end{multicols*}" << std::endl;
}

void LatexGenerator::add_pagebreak(std::ofstream &of) const
{
    of << std::endl
       << "\\pagebreak" << std::endl;
}

void LatexGenerator::add_solutionmode(std::ofstream &of) const
{
    of << std::endl
       << "\\PuzzleSolution" << std::endl;
}

void LatexGenerator::add_closing(std::ofstream &of) const
{
    of << std::endl
       << "\\end{document}" << std::endl;
}

void LatexGenerator::generate(Grid const &grid, std::string const &fileloc) const
{
    std::ofstream of;
    of.open(fileloc);

    add_preamble(of);
    add_puzzle_macros(of);

    add_puzzle(grid, of);
    add_pagebreak(of);
    add_hints(grid, of);

    add_pagebreak(of);
    add_solutionmode(of);
    add_puzzle(grid, of);

    add_closing(of);
}