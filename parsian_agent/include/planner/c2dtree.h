#ifndef PARSIAN_AGENT_C2DTREE_H
#define PARSIAN_AGENT_C2DTREE_H

#include <planner/state.h>
#include <parsian_util/tools/drawer.h>

using namespace std;

class C2DTree {
public:
    C2DTree();
    ~C2DTree();
    void add(state*);
    void removeAll();
    state *findNearest(Vector2D);
    void drawBranch(state* , state* , QColor);
    int size();
    state* makeBalanced(state** , int , int , int);

    state *head , *first;
    int k;
    state *allNodes[1000];
private:
    void quicksort(state *a[] , int lo , int hi , int k);
    void addNode(state* const & , state*& , int);
    state *findNearestNode(Vector2D & , state * const & , int);
    void removeBranch(state *);

    int cnt , mod;
};

#endif //PARSIAN_AGENT_C2DTREE_H
