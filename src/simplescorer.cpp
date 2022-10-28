#include <iostream>
#include <sstream>

#include "simplescorer.h"

using namespace Crossword;

SimpleScorer::SimpleScorer(INIReader const &config)
    : m_base_score(config.GetInteger("scoring", "base_score", 0)),
      m_placed_word_bonus(config.GetInteger("scoring", "placed_word_bonus", 0)),
      m_placed_letter_bonus(
          config.GetInteger("scoring", "placed_letter_bonus", 0)),
      m_word_crossing_bonus(
          config.GetInteger("scoring", "word_crossing_bonus", 0)),
      m_missing_word_penalty(
          config.GetInteger("scoring", "missing_word_penalty", 0)),
      m_used_row_penalty(config.GetInteger("scoring", "used_row_penalty", 0)),
      m_used_column_penalty(
          config.GetInteger("scoring", "used_column_penalty", 0))
{
    std::ostringstream os;

    os << "Initialized simple scorer with the following parameters" << std::endl;
    os << "base_score = " << m_base_score << std::endl;
    os << "placed_word_bonus = " << m_placed_word_bonus << std::endl;
    os << "placed_letter_bonus = " << m_placed_letter_bonus << std::endl;
    os << "word_crossing_bonus = " << m_word_crossing_bonus << std::endl;
    os << "missing_word_penalty = " << m_missing_word_penalty << std::endl;
    os << "used_row_penalty = " << m_used_row_penalty << std::endl;
    os << "used_column_penalty = " << m_used_column_penalty << std::endl;

    std::cout << os.str();
}

score SimpleScorer::score_grid(Grid const &grid,
                               std::int_fast32_t unplaced_word_count) const
{
    score result = m_base_score - m_missing_word_penalty * unplaced_word_count;
    result += grid->get_word_crossing_count() * m_word_crossing_bonus;
    result += grid->get_placed_word_count() * m_placed_word_bonus;
    // a crossing has the same letter of two words. This is not included in
    // get_placed_letter_count
    result += (grid->get_word_crossing_count() + grid->get_placed_letter_count()) *
              m_placed_letter_bonus;
    result -= grid->get_width() * m_used_column_penalty;
    result -= grid->get_height() * m_used_row_penalty;

    return result;
}
