#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <memory>

#include "word.h"

namespace Crossword
{
    enum Direction // DO NOT CHANGE ORDER in_bounds check relies on it
    {
        VERTICAL = 0,
        HORIZONTAL = 1
    };

    typedef std::int_fast32_t gidx;
    typedef struct Location
    {
        gidx row;
        gidx column;
        Direction direction;

        bool operator<(Location const &other) const;
    } Location;

    class _Grid
    {
    private:
        // size of the internal grid
        gidx m_internal_row_count;
        gidx m_internal_column_count;
        std::unique_ptr<char[]> m_grid;

        // words placed on the grid
        std::map<Location, Word> m_words;

        std::map<char, std::set<gidx>> m_char_loc_lookup;
        std::int_fast32_t m_crossing_count;

        // maximum number of rows/columns that can be used by valid crossword.
        // Note: m_internal_[row/column]_count may be larger to allow for flexibility
        // in adding new words
        gidx m_max_row_count;
        gidx m_max_column_count;

        // bounds used by the words places on the current grid
        gidx m_min_row_used;
        gidx m_max_row_used;
        gidx m_min_column_used;
        gidx m_max_column_used;

    public:
        static char const EMPTY_CHAR;

        _Grid(gidx max_row_count, gidx max_column_count);

        /**
            Checks if the word 'word' can be placed at location 'loc' without running
            out-of-bounds and violating the size constraints of the grid.
         */
        bool is_in_bounds(Word const &word, Location const &loc) const;

        /**
            Checks if this word placement is valid. I.e, it is not out-of-bounds
            and does not create a conflict with any adjecent or crossing words
            already placed on the grid.
         */
        bool is_valid_placement(Word const &word, Location const &loc) const;

        /**
            Place a word on grid.  Note that no validity our out-of-bounds checks
            are performed! This means that letters of crossing words will be
            overwritten by this placement. Thus, it is important to check the validity
            of the placement with valid_placement(word, loc). Alternatively, use
            place_word(word, loc), which includes the validity check.

            @return true if the word was placed successfully, otherwise false.
         */
        bool place_word_unchecked(Word const &word, Location const &loc);

        /**
            Places a word on the grid. Before placement, the validity of the placement
            is checked with valid_placement(word, loc).

            @return true if the word was placed successfully, otherwise false.
         */
        bool place_word(Word const &word, Location const &loc);

        /**
            Place this first word in an empty grid.
            @return true if word was successfully placed. False if word could not be
            placed, e.g. because the grid is not empty.
         */
        bool place_first_word(Word const &word, Direction direction);

        void get_valid_placements(Word const &word, std::vector<Location> &buffer) const;

        // Various getter functions
        std::int_fast32_t get_height() const;
        std::int_fast32_t get_width() const;
        std::int_fast32_t get_placed_letter_count() const;
        std::int_fast32_t get_placed_word_count() const;
        std::int_fast32_t get_word_crossing_count() const;
        char get_cell_content(gidx row, gidx column) const;
        Word const *get_word_starting_at(gidx row, gidx column, Direction dir) const;

        /**
            Prints the current grid on console.
            If paramter full_internal_grid is false, only the actually needed subsection
            of the internal grid is printed. Otherwise the full grid is printed, including
            empty rows and columns.
         */
        void print_on_console(bool full_internal_grid = false) const;
    };
    using Grid = std::shared_ptr<_Grid>;

} // namespace crossword
