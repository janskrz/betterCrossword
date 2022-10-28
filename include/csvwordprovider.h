#pragma once

#include <string>

#include "word.h"
#include "wordprovider.h"

namespace Crossword
{
  class CSVWordProvider : public WordProvider
  {
  private:
    std::string m_csv_location;
    bool m_ignore_header;
    char m_delim;

  public:
    /**
       Constructs a new crossword word provider that reads words from a CSV file.
       @param csv_location The CSV file name.
       @param ignore_header Specifies if the first row should be ignored. (Default true)
       @param delim The delimiter separating clue and word. (Default ',')
     */
    CSVWordProvider(const std::string &csv_location,
                    bool ignore_header = true, char delim = ',');

    /**
       Retrieves a list of crossword words from the CSV file specified in
       the constructor call and appends it to a WordList.
       @param wordlist The word list to which the retrieved words are appended to.
     */
    void retrieve_word_list(WordList &wordlist) const override;
  };
}