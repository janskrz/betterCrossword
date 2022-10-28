#include <utility>

#include "csvwordprovider.h"

using namespace Crossword;

std::map<std::string, std::function<std::unique_ptr<WordProvider>(std::string)>> WordProvider::m_factories =
    {
        {"csv", [](const std::string &location)
         {
             return std::make_unique<CSVWordProvider>(location);
         }}};

std::string WordProvider::trim(std::string const &str)
{
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

std::unique_ptr<WordProvider> WordProvider::create(const std::string &type, const std::string &location)
{
    if (m_factories.count(type) > 0)
    {
        return m_factories[type](location);
    }
    else
    {
        return nullptr;
    }
}