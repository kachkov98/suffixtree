#include <exception>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "suffixtree.h"

int main()
{
    try
    {
        SuffixTree::GeneralizedSuffixTree suffix_tree;
        suffix_tree.BuildSuffixTreeFromFile ("input.txt");
        suffix_tree.DumpCommonStringsToFile ("output.txt");
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
