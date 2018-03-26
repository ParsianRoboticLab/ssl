#include <planner/c2dtree.h>


C2DTree::C2DTree() {
    head = first = nullptr;
    cnt = 0;
    mod = 5;
}

C2DTree::~C2DTree() {
    removeAll();
}

int C2DTree::size() {
    return cnt;
}

void C2DTree::removeAll() {
    cnt = 0;
    mod = 5;
    if (head == nullptr) {
        return;
    }
    removeBranch(head);
    head = first = nullptr;
}

void C2DTree::removeBranch(state *node) {
    if (node) {
        removeBranch(node->left);
        removeBranch(node->right);
        delete node;
    }
}

void C2DTree::add(state * newState) {
    allNodes[cnt++] = newState;

    //  if( head == nullptr )
    //    head = newState;
    //  return;

    if (cnt % 33 == 0) {
        mod++;
    }

    if (cnt % mod) {
        addNode(newState , head , 0);
    } else {
        int hi = cnt - 1;
        int lo = 0;
        head = makeBalanced(allNodes , 0 , lo , hi);
    }
}

void C2DTree::quicksort(state *a[] , int lo , int hi , int k) {
    int i = lo , j = hi;
    double x = a[(lo + hi) / 2]->pos[k];
    do {
        while (a[i]->pos[k] < x) {
            i++;
        }
        while (a[j]->pos[k] > x) {
            j--;
        }
        if (i <= j) {
            swap(a[i] , a[j]);
            i++;
            j--;
        }
    } while (i < j);


    if (lo < j) {
        quicksort(a , lo , j , k);
    }
    if (i < hi) {
        quicksort(a , i , hi , k);
    }
}
void C2DTree::addNode(state * const &newState , state *&node , int depth) {
    if (node == nullptr) {
        node = newState;
    } else {
        k = depth % 2;
        if (newState->pos[k] < node->pos[k]) {
            addNode(newState , node->left , depth + 1);
        } else {
            addNode(newState , node->right , depth + 1);
        }
    }
}

state* C2DTree::makeBalanced(state *all[] , int depth , int lo , int hi) {
    if (lo > hi) {
        return nullptr;
    }
    k = depth % 2;
    if (lo < hi) {
        quicksort(all , lo , hi , k);
    }
    int mid = (lo + hi) / 2;

    all[mid]->left = makeBalanced(all , depth + 1 , lo , mid - 1);
    all[mid]->right = makeBalanced(all , depth + 1 , mid + 1 , hi);
    return all[mid];
}


state *C2DTree::findNearest(Vector2D point) {
    //  double mn = 10000;
    //  state *node;
    //  for( int i=0 ; i<cnt ; i++)
    //    if( allNodes[i]->pos.dist(point) < mn ){
    //      mn = allNodes[i]->pos.dist(point);
    //      node = allNodes[i];
    //    }
    //  return node;

    if (head) {
        return findNearestNode(point , head , 0);
    } else {
        return nullptr;
    }
}


state *C2DTree::findNearestNode(Vector2D &point , state * const &node , int depth) {
    k = depth % 2;
    state *best , *otherSideBest;
    if (point[k] < node->pos[k]) {
        if (node->left) {
            best = findNearestNode(point , node->left , depth + 1);
            if (node->pos.dist(point) < best->pos.dist(point)) {
                best = node;
            }
            if (node->right && fabs(node->pos[k] - point[k]) < point.dist(best->pos)) {
                otherSideBest = findNearestNode(point , node->right , depth + 1);
                if (point.dist(otherSideBest->pos) < point.dist(best->pos)) {
                    best = otherSideBest;
                }
            }
        } else {
            best = node;
            if (node->right && fabs(node->pos[k] - point[k]) < point.dist(best->pos)) {
                otherSideBest = findNearestNode(point , node->right , depth + 1);
                if (point.dist(otherSideBest->pos) < point.dist(best->pos)) {
                    best = otherSideBest;
                }
            }
        }
    } else {
        if (node->right) {
            best = findNearestNode(point , node->right , depth + 1);
            if (node->pos.dist(point) < best->pos.dist(point)) {
                best = node;
            }
            if (node->left && fabs(node->pos[k] - point[k]) < point.dist(best->pos)) {
                otherSideBest = findNearestNode(point , node->left , depth + 1);
                if (point.dist(otherSideBest->pos) < point.dist(best->pos)) {
                    best = otherSideBest;
                }
            }
        } else {
            best = node;
            if (node->left && fabs(node->pos[k] - point[k]) < point.dist(best->pos)) {
                otherSideBest = findNearestNode(point , node->left , depth + 1);
                if (point.dist(otherSideBest->pos) < point.dist(best->pos)) {
                    best = otherSideBest;
                }
            }
        }
    }
    return best;
}

void C2DTree::drawBranch(state *first , state* second , QColor color) {
    if (second == nullptr) {
        return;
    }
    drawer->draw(Segment2D(first->pos , second->pos) , color);
    drawBranch(second , second->parent , color);
}

