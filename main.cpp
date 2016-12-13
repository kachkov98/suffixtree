#include <exception>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "suffixtree.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
	{
		printf ("Launch parameters: ./substrings [input_file] [output_file]");
		return 0;
	}
	try
    {
        SuffixTree::GeneralizedSuffixTree suffix_tree;
        suffix_tree.BuildSuffixTreeFromFile (argv[1]);
        suffix_tree.DumpCommonStringsToFile (argv[2]);
#ifndef NDEBUG
        suffix_tree.DumpSuffixTreeToDotFile ("gtree.dot");
#endif
    }
    catch (std::exception exception)
    {
        printf ("%s\n", exception.what());
        exit (EXIT_FAILURE);
    }
    catch (std::string exception)
    {
        printf ("%s\n", exception.c_str());
        exit (EXIT_FAILURE);
    }
    return 0;
}
