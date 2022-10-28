#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "csvwordprovider.h"

using namespace Crossword;

CSVWordProvider::CSVWordProvider(const std::string &csv_location,
                                 bool ignore_header, char delim) : m_csv_location(csv_location), m_ignore_header(ignore_header), m_delim(delim)
{
    std::cout << "Initialized CSV word list provider. "
              << "CSV location: " << csv_location << std::endl;
}

void CSVWordProvider::retrieve_word_list(WordList &words) const
{
    // WordList words may have already some entries. Thus, get the highest
    // existing id in it first.
    wid next_id = 0;
    for (auto const &word : words)
    {
        next_id = std::max(next_id, word.id);
    }
    next_id++;

    std::ifstream csv_file(m_csv_location);
    if (!csv_file.is_open())
    {
        throw std::runtime_error(
            "Could not open the CSV file! Does it exist?\n"
            "(Filename: " +
            m_csv_location + ")");
    }

    std::string line;
    if (m_ignore_header)
    {
        std::getline(csv_file, line);
    }

    while (std::getline(csv_file, line))
    {
        std::stringstream tokens(line);

        std::string clue;
        std::string word;
        getline(tokens, clue, m_delim);
        getline(tokens, word, m_delim);
        // make all characters upper case
        clue = trim(clue);
        word = trim(word);
        std::for_each(word.begin(), word.end(), [](char &c)
                      { c = toupper(c); });

        Word new_word(next_id++, clue, word);
        if (new_word.clue.empty() || new_word.word.empty())
        {
            throw std::runtime_error(
                "Invalid CSV line format! \n"
                "Clue or solution word is empty \n"
                "The offending line: " +
                line);
        }

        words.push_back(new_word);
        if (!tokens.eof())
        {
            throw std::runtime_error(
                "Invalid CSV line format! \n"
                "Expected two columns separated by '" +
                std::string(1, m_delim) + "' but got more.\n"
                                          "The offending line: " +
                line);
        }
    }
}