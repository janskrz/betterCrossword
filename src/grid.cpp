#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

#include "grid.h"

// convenience macros for grid access at its only a 1D array internallly
#define GIDX(row, col) ((row)*m_internal_column_count + col)

// ATTENTION: These macros do not check for row/columns out-of-bounds!
#define INC_ROW(gidx) (gidx += m_internal_column_count)
#define INC_COLUMN(gidx) (++gidx)

#define UP_CELL(gidx) (gidx - m_internal_column_count)
#define DOWN_CELL(gidx) (gidx + m_internal_column_count)
#define LEFT_CELL(gidx) (gidx - 1)
#define RIGHT_CELL(gidx) (gidx + 1)

using namespace Crossword;

char const _Grid::EMPTY_CHAR = '.';

bool Location::operator<(Location const &other) const
{
    if (row == other.row && column == other.column)
    {
        return direction < other.direction;
    }
    else if (row == other.row)
    {
        return column < other.column;
    }
    return row < other.row;
}

_Grid::_Grid(gidx max_row_count, gidx max_column_count)
    : m_internal_row_count(2 * max_row_count),
      m_internal_column_count(2 * max_column_count), m_crossing_count(0),
      m_max_row_count(max_row_count), m_max_column_count(max_column_count),
      // First word will be placed in the center of the internal grid.
      // This is the passed row/column count, as row/column count is doubled
      // internally to allow flexible placements of words in all directions
      m_min_row_used(max_row_count), m_max_row_used(max_row_count),
      m_min_column_used(max_column_count), m_max_column_used(max_column_count)
{
    // Internally, we keep a grid of double given the column and row size.
    // That way, we can simply place the first word in the middle of the grid
    // and still have space in all directions. Thus, we do not restrict possible
    // solutions due to unfortunate placements of the first word.
    gidx gridsize = max_row_count * 2 * max_column_count * 2;
    m_grid = std::make_unique<char[]>(gridsize);
    std::fill(m_grid.get(), m_grid.get() + gridsize, EMPTY_CHAR);
}

bool _Grid::is_in_bounds(Word const &word, Location const &loc) const
{
    gidx start_row = loc.row;
    gidx start_col = loc.column;
    // Hack to avoid branching. Assumes that vertical = 0, horizontal = 1
    gidx end_row = loc.row + (word.length - 1) * (1 - loc.direction);
    gidx end_col = loc.column + (word.length - 1) * loc.direction;

    bool out_of_bounds = start_row < 0 || end_row >= m_internal_row_count;
    out_of_bounds |= start_col < 0 || end_col >= m_internal_column_count;
    out_of_bounds |= std::max(end_col, m_max_column_used) -
                         std::min(start_col, m_min_column_used) >=
                     m_max_column_count;
    out_of_bounds |=
        std::max(end_row, m_max_row_used) - std::min(start_row, m_min_row_used) >=
        m_max_row_count;

    return !out_of_bounds;
}

bool _Grid::is_valid_placement(Word const &word, Location const &loc) const
{
    if (!is_in_bounds(word, loc))
        return false;

    gidx start_row = loc.row;
    gidx start_col = loc.column;
    // Hack to avoid branching. Assumes that vertical = 0, horizontal = 1
    gidx end_row = loc.row + (word.length - 1) * (1 - loc.direction);
    gidx end_col = loc.column + (word.length - 1) * loc.direction;

    gidx cell = GIDX(start_row, start_col);
    bool conflict = false;
    switch (loc.direction)
    {
    case Direction::HORIZONTAL:
        // check cells before and after the word
        conflict |= start_col > 0 && m_grid[LEFT_CELL(cell)] != EMPTY_CHAR;
        conflict |= end_col + 1 < m_internal_column_count &&
                    m_grid[RIGHT_CELL(GIDX(end_row, end_col))] != EMPTY_CHAR;

        for (auto c = 0; c < word.length; c++)
        {
            if (m_grid[cell] == EMPTY_CHAR)
            {
                // if this cell is empty, the one above and below musst be empty as well
                if (start_row > 0)
                    conflict |= m_grid[UP_CELL(cell)] != EMPTY_CHAR;
                if (start_row + 1 < m_internal_row_count)
                    conflict |= m_grid[DOWN_CELL(cell)] != EMPTY_CHAR;
            }
            else
            {
                // As this cell is not empty, it must be the same value as the
                // letter of the word that we want to place here.
                conflict |= m_grid[cell] != word[c];

                // If we have a valid crossing here, the next character must be free!
                // If this is not the case, this means there is already another word
                // placed here with the same orientation. Needed to prevent placing a
                // word on a word with overlapping suffix/prefix. Like "testtest" on
                // "testt"
                conflict |=
                    m_grid[cell] == word[c] && m_grid[RIGHT_CELL(cell)] != EMPTY_CHAR;
            }

            INC_COLUMN(cell); // afterwards as we have to check letter 0 as well
        }
        break;
    case Direction::VERTICAL:
        // check cells above and below the word
        conflict |= start_row > 0 && m_grid[UP_CELL(cell)] != EMPTY_CHAR;
        conflict |= end_row + 1 < m_internal_row_count &&
                    m_grid[DOWN_CELL(GIDX(end_row, end_col))] != EMPTY_CHAR;

        for (auto c = 0; c < word.length; c++)
        {
            if (m_grid[cell] == EMPTY_CHAR)
            {
                // if this cell is empty, the one left and right musst be empty as well
                if (start_col > 0)
                    conflict |= m_grid[LEFT_CELL(cell)] != EMPTY_CHAR;
                if (start_col + 1 < m_internal_column_count)
                    conflict |= m_grid[RIGHT_CELL(cell)] != EMPTY_CHAR;
            }
            else
            {
                // As this cell is not empty, it must be the same value as the
                // letter of the word that we want to place here.
                conflict |= m_grid[cell] != word[c];

                // If we have a valid crossing here, the next character must be free!
                // If this is not the case, this means there is already another word
                // placed here with the same orientation. Needed to prevent placing a
                // word on a word with overlapping suffix/prefix. Like "testtest" on
                // "testt"
                conflict |=
                    m_grid[cell] == word[c] && m_grid[DOWN_CELL(cell)] != EMPTY_CHAR;
            }

            INC_ROW(cell);
        }
        break;
    }
    return !conflict;
}

bool _Grid::place_word_unchecked(Word const &word, Location const &loc)
{
    gidx cell = GIDX(loc.row, loc.column);
    switch (loc.direction)
    {
    case Direction::HORIZONTAL:
        for (auto i = 0; i < word.length; i++)
        {
            if (m_grid[cell] != EMPTY_CHAR)
                m_crossing_count++;

            m_grid[cell] = word[i];
            m_char_loc_lookup[word[i]].insert(cell);
            INC_COLUMN(cell);
        }
        m_max_row_used = std::max(m_max_row_used, loc.row);
        m_max_column_used =
            std::max(m_max_column_used, loc.column + word.length - 1);
        break;
    case Direction::VERTICAL:
        for (auto i = 0; i < word.length; i++)
        {
            if (m_grid[cell] != EMPTY_CHAR)
                m_crossing_count++;

            m_grid[cell] = word[i];
            m_char_loc_lookup[word[i]].insert(cell);
            INC_ROW(cell);
        }
        m_max_row_used = std::max(m_max_row_used, loc.row + word.length - 1);
        m_max_column_used = std::max(m_max_column_used, loc.column);
        break;
    }
    m_min_column_used = std::min(m_min_column_used, loc.column);
    m_min_row_used = std::min(m_min_row_used, loc.row);

    std::pair<Location, Word> placement(loc, word);
    m_words.insert(std::move(placement));

    return true;
}

bool _Grid::place_word(Word const &word, Location const &loc)
{
    if (!is_valid_placement(word, loc))
        return false;

    return place_word_unchecked(word, loc);
}

bool _Grid::place_first_word(Word const &word, Direction direction)
{
    if (!m_words.empty())
    {
        return false;
    }

    // calculate the start position of the word, so that its in the middle of the
    // grid
    Location loc = {m_internal_row_count / 2, m_internal_column_count / 2,
                    direction};
    switch (direction)
    {
    case Direction::HORIZONTAL:
        loc.column -= word.length / 2;
        break;
    case Direction::VERTICAL:
        loc.row -= word.length / 2;
        break;
    }

    return place_word(word, loc);
}

void _Grid::get_valid_placements(Word const &word,
                                 std::vector<Location> &buffer) const
{
    for (auto cidx = 0; cidx < word.length; cidx++)
    {
        auto const &letter = word[cidx];
        if (m_char_loc_lookup.count(letter) > 0)
        {
            for (auto const &cell : m_char_loc_lookup.at(letter))
            {
                gidx const row = cell / m_internal_column_count;
                gidx const col = cell % m_internal_column_count;
                Location loc = {row - cidx, col, Direction::VERTICAL};
                if (is_valid_placement(word, loc))
                {
                    buffer.push_back(loc);
                }
                loc = {row, col - cidx, Direction::HORIZONTAL};
                if (is_valid_placement(word, loc))
                {
                    buffer.push_back(loc);
                }
            }
        }
    }
}

std::int_fast32_t _Grid::get_height() const
{
    return m_max_row_used - m_min_row_used + 1;
}

std::int_fast32_t _Grid::get_width() const
{
    return m_max_column_used - m_min_column_used + 1;
}

std::int_fast32_t _Grid::get_placed_letter_count() const
{
    std::int_fast32_t count = 0;
    for (auto const &[key, char_locs] : m_char_loc_lookup)
    {
        count += char_locs.size();
    }
    return count;
}

std::int_fast32_t _Grid::get_placed_word_count() const
{
    return m_words.size();
}

std::int_fast32_t _Grid::get_word_crossing_count() const
{
    return m_crossing_count;
}

char _Grid::get_cell_content(gidx row, gidx column) const
{
    if (row < 0 || row >= get_height() || column < 0 || column >= get_width())
        return EMPTY_CHAR;

    return m_grid[GIDX(m_min_row_used + row, m_min_column_used + column)];
}

Word const *_Grid::get_word_starting_at(gidx row, gidx column,
                                        Direction dir) const
{
    row += m_min_row_used;
    column += m_min_column_used;
    Location const loc = {row, column, dir};

    if (m_words.count(loc) > 0)
    {
        return &m_words.at(loc);
    }
    return nullptr;
}

void _Grid::print_on_console(bool full_internal_grid) const
{
    std::ostringstream os;

    os << std::endl
       << "Printing crossword grid" << std::endl;
    os << "Grid dimensions (H,W): " << get_height() << " " << get_width()
       << std::endl;
    os << "Number of words placed: " << get_placed_word_count() << std::endl;
    os << "Number of charactes placed: " << get_placed_letter_count()
       << std::endl;
    os << "Number of word crossings: " << get_word_crossing_count() << std::endl;

    size_t start_row = full_internal_grid ? 0 : m_min_row_used;
    size_t start_col = full_internal_grid ? 0 : m_min_column_used;
    size_t end_row =
        full_internal_grid ? m_internal_row_count - 1 : m_max_row_used;
    size_t end_col =
        full_internal_grid ? m_internal_column_count - 1 : m_max_column_used;

    for (auto row = start_row; row <= end_row; row++)
    {
        os << " ";
        for (auto column = start_col; column <= end_col; column++)
        {
            os << m_grid[GIDX(row, column)];
        }
        os << std::endl;
    }
    std::cout << os.str() << std::endl;
}
