#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "generator.h"
#include "latexgenerator.h"

#include "INIReader.h"

#define SEED_RNG (time(NULL)) // seed is current time in seconds

#define CONFIG_FILE "config.ini"

using namespace Crossword;

Generator::Generator(std::int_fast32_t number_of_crosswords_to_generate,
                     std::int_fast32_t crossword_max_width,
                     std::int_fast32_t crossword_max_height,
                     std::unique_ptr<WordProvider> provider,
                     std::unique_ptr<Scorer> grid_scorer)
    : m_rng(std::default_random_engine{}),
      m_gen_count(number_of_crosswords_to_generate),
      m_cw_max_width(crossword_max_width),
      m_cw_max_height(crossword_max_height),
      m_grid_scorer(std::move(grid_scorer))
{
    auto rng_seed = SEED_RNG;
    m_rng.seed(rng_seed);
    provider->retrieve_word_list(word_list);
    std::cout << "Initialized crossword generator. " << std::endl;
    std::cout << "Random generator seed is: " << rng_seed << std::endl;
}

Grid Generator::generate_single_grid()
{
    std::uniform_int_distribution<int> dist(
        0, 1); // for generating random vert/horizontal
    auto grid = std::make_shared<_Grid>(m_cw_max_height, m_cw_max_width);
    WordList unused_words(word_list);

    // place random first word
    std::shuffle(std::begin(unused_words), std::end(unused_words), m_rng);
    Word const first_word = unused_words.back();
    Direction const first_dir = static_cast<Direction>(dist(m_rng));

    grid->place_first_word(first_word, first_dir);
    unused_words.pop_back();

    // in every iteration, shuffle not yet placed words and try to add them at
    // random valid location in shuffle order. Repeat until no words are left
    // or no remaining word can be placed.
    while (unused_words.size() != 0)
    {
        bool word_placed = false;
        std::shuffle(std::begin(unused_words), std::end(unused_words), m_rng);

        WordList unplaced_words;
        for (auto const &word : unused_words)
        {
            std::vector<Location> valid_placements;
            grid->get_valid_placements(word, valid_placements);
            if (valid_placements.size() == 0)
            {
                unplaced_words.push_back(word);
            }
            else
            {
                std::uniform_int_distribution<int> rng(0, valid_placements.size() - 1);
                Location rand_loc = valid_placements[rng(m_rng)];
                grid->place_word_unchecked(word, rand_loc);
                word_placed = true;
            }
        }
        unused_words = std::move(unplaced_words);

        if (!word_placed)
            break;
    }

    return grid;
}

Grid Generator::generate()
{
    int const worker_thread_count = 5;
    int const grids_per_thread = m_gen_count / worker_thread_count;
    int const total_grids = grids_per_thread * worker_thread_count;

    auto gridBuffer = std::make_shared<SharedGridBuffer>(worker_thread_count);

    std::cout << "Generating " << total_grids << " grids on " << worker_thread_count << " threads and choosing the best"
              << std::endl;
    auto begin = std::chrono::high_resolution_clock::now();

    auto worker_fun = [this, &grids_per_thread, &gridBuffer](int threadId)
    {
        int gen_count = 0;
        while (gen_count < grids_per_thread)
        {
            gridBuffer->addNextGrid(threadId, generate_single_grid());
            gen_count++;
        }
    };

    Grid best_grid = nullptr;
    score highest_grid_score = 0;
    auto process_fun = [this, &total_grids, &best_grid, &highest_grid_score, &gridBuffer]()
    {
        int processed_grids = 0;
        while (processed_grids < total_grids)
        {
            Grid next_grid = gridBuffer->getNextGridToProcess();
            std::int_fast32_t unplaced_words =
                word_list.size() - next_grid->get_placed_word_count();
            score grid_score = m_grid_scorer->score_grid(next_grid, unplaced_words);
            if (!best_grid || grid_score > highest_grid_score)
            {
                highest_grid_score = grid_score;
                best_grid = next_grid;
            }
            processed_grids++;

            if (processed_grids % PRINT_PROGRESS_EVERY_GRIDS == 0)
            {
                std::cout << "Generated " << processed_grids << " out of " << m_gen_count << " grids. "
                          << std::endl;
                std::cout << "The current best grid has a score of " << highest_grid_score
                          << ". It is: " << std::endl;
                best_grid->print_on_console();
            }
        }
    };

    std::vector<std::thread> generator_threads;
    for (int i = 0; i < worker_thread_count; i++)
    {
        generator_threads.push_back(std::thread(worker_fun, i));
    }
    std::thread grid_processor(process_fun);

    for (int i = 0; i < worker_thread_count; i++)
    {
        generator_threads[i].join();
    }
    grid_processor.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = end - begin;
    auto dur_in_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << "Generated all " << m_gen_count << " grids!" << std::endl;
    std::cout << "This took me a total of " << dur_in_ms / 1000.0 << " seconds."
              << std::endl;
    std::cout << "The final grid has a score of " << highest_grid_score
              << ". It is: " << std::endl;
    best_grid->print_on_console();

    gridBuffer->clear();

    return best_grid;
}

SharedGridBuffer::SharedGridBuffer(int number_of_threads) : m_number_of_threads(number_of_threads)
{
    m_next_filled_grids_by_thread.resize(number_of_threads, 0);
    for (int i = 0; i <= number_of_threads; i++)
    {
        m_next_filled_grids_by_thread[i] = i;
    }
    for (int i = 0; i < GRID_BUFFER_SIZE; i++)
    {
        m_filed_grid_flags[i] = 0;
    }
}

void SharedGridBuffer::addNextGrid(int threadId, Grid const &grid)
{
    int nextId = m_next_filled_grids_by_thread[threadId];
    while (m_filed_grid_flags[nextId] != 0)
    {
        std::cout << "Warning:: Grid buffer is full. Processing grids is too slow!" << std::endl;
        // wait until it is processed...
    }
    m_grid_buffer[nextId] = grid;
    m_filed_grid_flags[nextId] = 1;
    m_next_filled_grids_by_thread[threadId] = (nextId + m_number_of_threads) % GRID_BUFFER_SIZE;
}

Grid SharedGridBuffer::getNextGridToProcess()
{
    while (m_filed_grid_flags[m_next_processed_grid] == 0)
    {
        // wait until grid is filled...
    }
    Grid toReturn = m_grid_buffer[m_next_processed_grid];
    m_filed_grid_flags[m_next_processed_grid] = 0;
    m_next_processed_grid = (m_next_processed_grid + 1) % GRID_BUFFER_SIZE;

    return toReturn;
}

void SharedGridBuffer::clear()
{
    for (int i = 0; i < GRID_BUFFER_SIZE; i++)
    {
        m_grid_buffer[i] = nullptr;
    }
}