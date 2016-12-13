/*
 * Class GeneralizedSuffixTree - finding longest common substrings in array of strings
 * Consists of 2 parts:
 * 1. Building generalized suffix tree using Ukkonen's algorithm
 *    Identifiers that obtained by MakeLetter() are used as separating symbols
 *    All nodes and leafs are stored in continuous memory (memory is allocated by AllocNode and AllocLeaf)
 * 2. Finding deepest (in terms of string length) node in the tree that have d different leaves in its subtree,
 *    where d - number of strings that contain this substring using Lucas Chi Kwong Hui's algorithm
 *    References: Dan Gusfield "Algorithms on Strings, Trees and Sequences", 7.6;
 *    Finding of lowest common ancestors between pairs of leaves with same string_number id that enters in
 *    the current node's subtree is implemented by Tarjan's algorithm
 */
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "suffixtree.h"

namespace SuffixTree
{
static inline uint MakeLetter (char symbol, uint string)
{
    return (symbol) ? static_cast<uint> (symbol) : string << 8;
}

GeneralizedSuffixTree::GeneralizedSuffixTree() :
    nodes (nullptr),
    leaves (nullptr),
    nodes_num (0),
    leaves_num (0),
    root (nullptr)
{
    srand (time (nullptr));
}

void GeneralizedSuffixTree::BuildSuffixTreeFromFile (std::string filename)
{
    // load strings from file
    printf ("Loading strings...\n");
    FILE *file = fopen (filename.c_str(), "r");
    if (!file)
        throw std::string ("Can not open file: ") + filename;
    char strbuf[1024];
    uint symbols_num = 0;
    while (fgets (strbuf, 1024, file))
    {
        uint len = strlen (strbuf);
        if (len > 0)
        {
            strbuf[len - 1] = '\0';
            strings.push_back (strbuf);
            symbols_num += len;
        }
    }
    fclose (file);
    printf ("\r\rStrings were loaded successfully\n");
    // allocate space for nodes and leaves
	if (symbols_num)
	{
		nodes = static_cast<char *> (malloc (symbols_num * sizeof (Node)));
		leaves = static_cast<char *> (malloc (symbols_num * sizeof (Leaf)));
	}
    else
	{
		nodes = static_cast<char *> (malloc (sizeof (Node)));
		leaves = static_cast<char *> (malloc (sizeof (Leaf)));
	}
    // build tree
    root = AllocNode (nullptr, nullptr);
    root->suffix_link = root;
    active_node = root;
    active_edge = 0;
    active_length = 0;
    remainder = 0;
    for (uint i = 0; i < strings.size(); ++i)
    {
        printf ("\rBuilding tree: %.2f %%", i * 100.0f / strings.size());
        for (uint j = 0; j <= strings[i].size(); ++j)
        {
            char symbol = strings[i][j];
            uint letter = MakeLetter (symbol, i);
            Node *last_inserted_node = nullptr;
            ++remainder;
            while (remainder)
            {
                // try add symbol
                Node *old_active_node = active_node;
                if (AddSymbol (symbol))
                {
                    if (last_inserted_node)
                        last_inserted_node->suffix_link = old_active_node;
                    break;
                }
                // insert node or leaf
                if (active_length)
                {
                    // insert node
                    Node *node = InsertNode (letter, i, j);
                    if (last_inserted_node)
                        last_inserted_node->suffix_link = node;
                    last_inserted_node = node;
                }
                else
                {
                    // insert leaf
                    InsertLeaf (letter, i, j);
                    if (last_inserted_node)
                        last_inserted_node->suffix_link = active_node;
                    last_inserted_node = active_node;
                }
                // move to next active point
                if (active_node != root)
                {
                    active_node = active_node->suffix_link;
                    if (active_length)
                        Move (i, j - active_length);
                }
                else if (active_length)
                {
                    --active_length;
                    Move (i, j - active_length);
                }
                --remainder;
            }
        }
    }
    printf ("\rTree was built successfully\n");
}

void GeneralizedSuffixTree::DumpSuffixTreeToDotFile (std::string filename)
{
    printf ("Dumping tree...\n");
    FILE *file = fopen (filename.c_str(), "w");
    if (!file)
        throw std::string ("Can not open file: ") + filename;
    fprintf (file, "digraph suffixtree\n{\n");
    DumpNode (root, file);
    fprintf (file, "}\n");
    fclose (file);
    printf ("Tree was dumped successfully\n");
}

void GeneralizedSuffixTree::DumpCommonStringsToFile (std::string filename)
{
    printf ("Finding common substrings...\n");
    // process lca_queries
    previous_suffixes.resize (strings.size(), nullptr);
    ProcessLCAQueries (root);
    // calculate number of leaves and least common ancestors
    CalculateNumLeavesNumLCA (root);
    // find answers for all numbers of substrings
    answers.resize (strings.size(), std::pair<Node *, uint> (nullptr, 0));
    FindAnswers (root, 0);
    printf ("Common substrings were found\n");
    // write to file
    FILE *file = fopen (filename.c_str(), "w");
    if (!file)
        throw std::string ("Can not open file: ") + filename;
    std::string ans;
    uint cur_max = 0;
    for (uint i = strings.size() - 1; i > 0; --i)
    {
        printf ("\rWriting substrings to file: %.2f %%", (strings.size() - i) * 100.0f / strings.size());
        fprintf (file, "%u common substrings: ", i + 1);
        if (answers[i].second > cur_max)
        {
            cur_max = answers[i].second;
            ans.clear();
            Node *cur_node = answers[i].first;
            while (cur_node->parent)
            {
                for (const auto &edge_pair : cur_node->parent->edges)
                    if (edge_pair.second.node == cur_node)
                    {
                        ans = strings[edge_pair.second.string_number].substr (edge_pair.second.start,
                                edge_pair.second.length) + ans;
                        break;
                    }
                cur_node  = cur_node->parent;
            }
        }
        if (ans.size())
            fprintf (file, "%s\n", ans.c_str());
        else
            fprintf (file, "<Empty string>\n");
    }
    fclose (file);
    printf ("\rSubstrings were written successfully\n");
}

GeneralizedSuffixTree::~GeneralizedSuffixTree()
{
    printf ("Clearing tree...\n");
    if (nodes && leaves)
    {
        for (uint i = 0; i < nodes_num; ++i)
            reinterpret_cast<Node *> (nodes + i * sizeof (Node))->~Node();
        for (uint i = 0; i < leaves_num; ++i)
            reinterpret_cast<Leaf *> (leaves + i * sizeof (Leaf))->~Leaf();
        free (nodes);
        free (leaves);
    }
    printf ("Tree was cleared successfully\n");
}

Node *GeneralizedSuffixTree::AllocNode (Node *parent, Node *suffix_link)
{
    return new (nodes + (nodes_num++) * sizeof (Node)) Node (parent, suffix_link);
}

Leaf *GeneralizedSuffixTree::AllocLeaf (Node *parent, uint string_number)
{
    assert (string_number < strings.size());
	return new (leaves + (leaves_num++) * sizeof (Leaf)) Leaf (parent, string_number);
}

void GeneralizedSuffixTree::IncActiveLength()
{
	++active_length;
	Edge &edge = active_node->edges[active_edge];
	if (edge.length && active_length == edge.length)
	{
		active_node = edge.node;
		active_length = 0;
	}
}

bool GeneralizedSuffixTree::AddSymbol (char symbol)
{
    if (!symbol)
        return false;
    if (active_length)
    {
        // active point is on edge
        Edge &edge = active_node->edges[active_edge];
        if (strings[edge.string_number][edge.start + active_length] == symbol)
        {
            IncActiveLength();
            return true;
        }
        else
            return false;
    }
    else
    {
        // active point is on node
        if (active_node->edges.count (static_cast<uint> (symbol)))
        {
            active_edge = static_cast<uint> (symbol);
            IncActiveLength();
            return true;
        }
        else
            return false;
    }
}

void GeneralizedSuffixTree::Move (uint string, uint pos)
{
    assert (string < strings.size() && pos <= strings[string].size());
	for (;;)
    {
        if (!active_length)
            break;
        active_edge = static_cast<uint> (strings[string][pos]);
        Edge &edge = active_node->edges[active_edge];
        if (!edge.length)
            break;
        if (active_length < edge.length)
            break;
        pos += edge.length;
        active_node = edge.node;
        active_length -= edge.length;
    }
}

Leaf *GeneralizedSuffixTree::InsertLeaf (uint letter, uint string, uint pos)
{
    // can not insert leaf on the middle of edge
    assert (!active_length);
    assert (!active_node->edges.count (letter));
    Leaf *leaf = AllocLeaf (active_node, string);
    active_node->edges.emplace (letter, Edge (leaf, string, pos));
    return leaf;
}

Node *GeneralizedSuffixTree::InsertNode (uint letter, uint string, uint pos)
{
    // can not insert node if active point is not on edge
    assert (active_length);
    // create node
    Node *node = AllocNode (active_node, root);
    Edge &edge = active_node->edges[active_edge];
    // old branch
    uint old_pos = edge.start + active_length;
    if (edge.length)
    {
        node->edges.emplace (MakeLetter (strings[edge.string_number][old_pos], edge.string_number),
                             Edge (edge.node, edge.string_number, old_pos, edge.length - active_length));
        edge.node->parent = node;
    }
    else
    {
        node->edges.emplace (MakeLetter (strings[edge.string_number][old_pos], edge.string_number),
                             Edge (edge.leaf, edge.string_number, old_pos));
        edge.leaf->parent = node;
    }
    // new leaf
    node->edges.emplace (letter, Edge (AllocLeaf (node, string), string, pos));
    // setup parent
    edge.node = node;
    edge.length = active_length;
    return node;
}

void GeneralizedSuffixTree::DumpNode (Node *node, FILE *file)
{
    // dump style of current node
    fprintf (file, "p%p [shape=circle label=\"%u; %u\"]\n",
             static_cast<void *> (node), node->num_leaves, node->num_lca);
    // process all child of current node
    for (const auto &edge_pair : node->edges)
    {
        const Edge &edge = edge_pair.second;
        if (edge.length)
        {
            // child is node
            std::string label = strings[edge.string_number].substr (edge.start, edge.length);
            fprintf (file, "p%p -> p%p [label=\"%s\"]\n",
                     static_cast<void *> (node), static_cast<void *> (edge.node), label.c_str());
            DumpNode (edge.node, file);
        }
        else
        {
            // child is leaf
            std::string label = strings[edge.string_number].substr (edge.start) + "$";
            fprintf (file, "p%p -> p%p [label=\"%s\"]\n",
                     static_cast<void *> (node), static_cast<void *> (edge.leaf), label.c_str());
            fprintf (file, "p%p [style=filled color=grey shape=circle label=\"%u\"]\n",
                     static_cast<void *> (edge.leaf), edge.leaf->string_number);
        }
    }
}

Node *GeneralizedSuffixTree::DSUFind (Node *node)
{
    if (node->dsu_parent == node)
        return node;
    return node->dsu_parent = DSUFind (node->dsu_parent);
}

Node *GeneralizedSuffixTree::DSUUnite (Node *fst, Node *snd)
{
    fst = DSUFind (fst);
    snd = DSUFind (snd);
    if (rand() % 2)
    {
        fst->dsu_parent = snd;
        return snd;
    }
    else
    {
        snd->dsu_parent = fst;
        return fst;
    }
}

void GeneralizedSuffixTree::ProcessLCAQueries (Node *node)
{
    for (const auto &edge_pair : node->edges)
        if (edge_pair.second.length)
        {
            ProcessLCAQueries (edge_pair.second.node);
            Node *leader = DSUUnite (node, edge_pair.second.node);
            leader->lca_class = node;
        }
    for (const auto &edge_pair : node->edges)
        if (!edge_pair.second.length)
        {
            uint string_num = edge_pair.second.leaf->string_number;
            if (previous_suffixes[string_num])
                ++ (DSUFind (previous_suffixes[string_num])->lca_class->num_lca);
            previous_suffixes[string_num] = node;
        }
}

void GeneralizedSuffixTree::CalculateNumLeavesNumLCA (Node *node)
{
    for (const auto &edge_pair : node->edges)
        if (edge_pair.second.length)
        {
            CalculateNumLeavesNumLCA (edge_pair.second.node);
            node->num_leaves += edge_pair.second.node->num_leaves;
            node->num_lca += edge_pair.second.node->num_lca;
        }
        else
            ++node->num_leaves;
}

void GeneralizedSuffixTree::FindAnswers (Node *node, uint cur_substring_length)
{
    for (const auto &edge_pair : node->edges)
        if (edge_pair.second.length)
            FindAnswers (edge_pair.second.node, cur_substring_length + edge_pair.second.length);
    uint num_substrings = node->num_leaves - node->num_lca;
    if (num_substrings > 1 && cur_substring_length > answers[num_substrings - 1].second)
        answers[num_substrings - 1] = std::pair<Node *, uint> (node, cur_substring_length);
}
}
