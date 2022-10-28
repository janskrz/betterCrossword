#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace Crossword
{
    typedef std::uint_fast16_t wid;

    typedef struct Word
    {
        wid id;

        std::string clue;
        std::string word;

        std::int_fast16_t length;

        Word(wid id_, std::string const &clue_, std::string const &word_) : id(id_), clue(clue_), word(word_), length(word_.length())
        {
        }

        char const &operator[](int index) const
        {
            return word[index];
        }
    } Word;

    typedef std::vector<Word> WordList;
}