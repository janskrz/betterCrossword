#pragma once

#include "scorer.h"

#include "INIReader.h"

namespace Crossword
{
    class SimpleScorer : public Scorer
    {
    private:
        score m_base_score;
        score m_placed_word_bonus;
        score m_placed_letter_bonus;
        score m_word_crossing_bonus;
        score m_missing_word_penalty;
        score m_used_row_penalty;
        score m_used_column_penalty;

    public:
        SimpleScorer(INIReader const &config);

        score score_grid(Grid const &grid,
                         std::int_fast32_t unplaced_word_count) const override;
    };

} // namespace crossword