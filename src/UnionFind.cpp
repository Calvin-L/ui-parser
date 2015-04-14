#include "UnionFind.hpp"

UnionFind::UnionFind(size_t count) : ranks(count, 0) {
    parents.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        parents.push_back(i); // every node links to itself
    }
}

size_t UnionFind::find(size_t x) {
    size_t parent = x;
    while (parents[parent] != parent) {
        parent = parents[parent];
    }
    while (parents[x] != x) {
        size_t next = parents[x];
        parents[x] = parent;
        x = next;
    }
    return x;
}

void UnionFind::join(size_t x, size_t y) {
    x = find(x);
    y = find(y);
    if (x != y) {
        if (ranks[x] < ranks[y]) {
            parents[x] = y;
        } else if (ranks[x] > ranks[y]) {
            parents[y] = x;
        } else {
            parents[y] = x;
            ++ranks[x];
        }
    }
}
