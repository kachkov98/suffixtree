#pragma once
#include <vector>
#include <string>
#include <unordered_map>

namespace SuffixTree
{
typedef unsigned int uint;

struct Node;
struct Leaf;

struct Edge
{
    union
    {
        Node *node;
        Leaf *leaf;
    };
    uint string_number;
    uint start;
    uint length;
    Edge ()
    {}
    Edge (Leaf *leaf_, uint string_number_, uint start_) :
        leaf (leaf_),
        string_number (string_number_),
        start (start_),
        length (0)
    {}
    Edge (Node *node_, uint string_number_, uint start_, uint length_) :
        node (node_),
        string_number (string_number_),
        start (start_),
        length (length_)
    {}
};

struct Node
{
    Node *parent;
    Node *suffix_link;
    Node *lca_class, *dsu_parent;
    uint num_leaves, num_lca;
    std::unordered_map<uint, Edge> edges;
    Node (Node *parent_, Node *suffix_link_) :
        parent (parent_),
        suffix_link (suffix_link_),
        lca_class (this),
        dsu_parent (this),
        num_leaves (0),
        num_lca (0)
    {}
};

struct Leaf
{
    Node *parent;
    uint string_number;
    Leaf (Node *parent_, uint string_number_) :
        parent (parent_),
        string_number (string_number_)
    {}
};

class GeneralizedSuffixTree
{
public:
    GeneralizedSuffixTree ();
    GeneralizedSuffixTree (const GeneralizedSuffixTree &) = delete;
    GeneralizedSuffixTree operator= (const GeneralizedSuffixTree &) = delete;
    void BuildSuffixTreeFromFile (std::string filename);
    void DumpSuffixTreeToDotFile (std::string filename);
    void DumpCommonStringsToFile (std::string filename);
    ~GeneralizedSuffixTree ();
private:
    Node *AllocNode (Node *parent, Node *suffix_link);
    Leaf *AllocLeaf (Node *parent, uint string_number);
	void IncActiveLength ();
    bool AddSymbol (char symbol);
    void Move (uint string, uint pos);
    Leaf *InsertLeaf (uint letter, uint string, uint pos);
    Node *InsertNode (uint letter, uint string, uint pos);
    void DumpNode (Node *node, FILE *file);
    Node *DSUFind (Node *node);
    Node *DSUUnite (Node *fst, Node *snd);
    void ProcessLCAQueries (Node *node);
    void CalculateNumLeavesNumLCA (Node *node);
    void FindAnswers (Node *node, uint cur_substring_length);

    std::vector<std::string> strings;
    std::vector<Node *> previous_suffixes;
    std::vector<std::pair<Node *, uint>> answers;
    char *nodes, *leaves;
    uint nodes_num, leaves_num;
    Node *root;
    // active point
    Node *active_node;
    uint active_edge, active_length, remainder;
};
}
