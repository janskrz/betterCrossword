#pragma once

#include <cstdint>
#include <random>
#include <utility>

#include "wordprovider.h"
#include "scorer.h"
#include "grid.h"

namespace Crossword
{
    class SharedGridBuffer
    {
    private:
        const static int GRID_BUFFER_SIZE = 5000;

        int m_number_of_threads = 0;
        std::vector<int> m_next_filled_grids_by_thread;
        int m_next_processed_grid = 0;

        Grid m_grid_buffer[GRID_BUFFER_SIZE];
        int m_filed_grid_flags[GRID_BUFFER_SIZE];
        int m_processed_grid_flags[GRID_BUFFER_SIZE];

    public:
        SharedGridBuffer(int number_of_threads);

        void addNextGrid(int threadId, Grid const &grid);
        Grid getNextGridToProcess();

        void clear();
    };

    class Generator
    {
    private:
        const std::int_fast32_t PRINT_PROGRESS_EVERY_GRIDS = 2500;

        std::default_random_engine m_rng;

        std::int_fast32_t m_gen_count;
        std::int_fast32_t m_cw_max_width;
        std::int_fast32_t m_cw_max_height;

        WordList word_list;
        std::unique_ptr<Scorer> m_grid_scorer;

        Grid generate_single_grid();

    public:
        Generator(std::int_fast32_t number_of_crosswords_to_generated,
                  std::int_fast32_t crossword_max_width, std::int_fast32_t crossword_max_height,
                  std::unique_ptr<WordProvider> provider, std::unique_ptr<Scorer> grid_scorer);

        Grid generate();
    };
}