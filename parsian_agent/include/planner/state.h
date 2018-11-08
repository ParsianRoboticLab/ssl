#ifndef STATE_H
#define STATE_H

#include "parsian_util/geom/geom.h"
#include <QList>
#include <vector>

using namespace std;

struct state {
    int depth;
    double dist;
    Vector2D pos;
    state *parent , *next;

    state *left , *right;

    state(Vector2D _pos  , state *_parent , vector<Vector2D> &res) {
        pos = _pos;
        parent = _parent;
        if (parent) {
            depth = parent->depth + 1;
            if (depth < res.size()) {
                dist = parent->dist + pos.dist(res[depth]);
            } else if (res.size()) {
                dist = parent->dist + pos.dist(res[res.size() - 1]);
            } else {
                dist = parent->dist + pos.dist(parent->pos);
            }
        } else {
            depth = 0;
            if (depth < res.size()) {
                dist = pos.dist(res[depth]);
            } else if (res.size()) {
                dist = pos.dist(res[res.size() - 1]);
            } else {
                dist = 0;
            }
        }

        left = right = next = NULL;
    }
    state() {
        pos.invalidate();
        parent = left = right = next = NULL;
    }
};

#endif // STATE_H
