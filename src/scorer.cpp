#include "scorer.h"
#include "simplescorer.h"

using namespace Crossword;

std::unique_ptr<Scorer> Scorer::create(std::string const &type, INIReader const &config)
{
    if (type == "simple")
    {
        return std::make_unique<SimpleScorer>(config);
    }
    return nullptr;
}
