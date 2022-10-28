#include <iostream>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <string>

#include "generator.h"
#include "latexgenerator.h"

#include "INIReader.h"

#define CONFIG_FILE "config.ini"

using namespace Crossword;

int main(__attribute__((unused)) int argc, char *argv[])
{
	using namespace std::filesystem;

	std::cout << "Starting crossword generator." << std::endl;

	path exec_path = argv[0];
	path config_path = exec_path.parent_path().append(CONFIG_FILE);
	std::cout << "Reading config file " << config_path << std::endl;

	INIReader reader(config_path.string());
	if (reader.ParseError() != 0)
	{
		std::cerr << "Could not read config file! Does file '" << config_path
				  << "' exist and is formatted correctly?" << std::endl;
		return -1;
	}

	path wordlistloc = exec_path.parent_path()
						   .append(reader.Get("wordlist", "location", "INVALID"));

	auto cw_gen_count = reader.GetInteger("constraints", "crossword_generation_count", -1);
	auto cw_max_height = reader.GetInteger("constraints", "max_height", -1);
	auto cw_max_width = reader.GetInteger("constraints", "max_width", -1);

	if (cw_gen_count < 0 || cw_max_height < 0 || cw_max_width < 0)
	{
		std::cerr << "Error reading crossword constraints from config!" << std::endl;
		return -1;
	}

	std::string const wordprovider_type = reader.Get("wordlist", "type", "INVALID");
	std::string const scorer_type = reader.Get("scoring", "type", "INVALID");

	auto wordprovider = WordProvider::create(wordprovider_type, wordlistloc.string());
	auto scorer = Scorer::create(scorer_type, reader);

	if (wordprovider == nullptr)
	{
		std::cerr << "Error: Could not create word provider of type '"
				  << wordprovider_type << "'" << std::endl;
		return -1;
	}

	if (scorer == nullptr)
	{
		std::cerr << "Error: Could not create scorer of type '"
				  << scorer_type << "'" << std::endl;
		return -1;
	}

	Generator generator(cw_gen_count, cw_max_width, cw_max_height,
						std::move(wordprovider), std::move(scorer));

	Grid grid = generator.generate();

	LatexGenerator to_latex;
	to_latex.generate(grid, "crossword.tex");

	return 0;
}