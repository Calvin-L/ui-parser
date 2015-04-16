#ifndef UNIONFIND_H
#define UNIONFIND_H 1

#include <vector>

class UnionFind {
public:
    UnionFind(size_t count);
    size_t find(size_t x);
    void join(size_t x, size_t y);
private:
    std::vector<size_t> parents;
    std::vector<int> ranks;
};

template <class T, class F>
std::vector<std::vector<T>> group(std::vector<T> v, F similar) {
    UnionFind uf(v.size());

    for (size_t i = 0; i < v.size(); ++i) {
        for (size_t j = i + 1; j < v.size(); ++j) {
            if (similar(v[i], v[j])) {
                uf.join(i, j);
            }
        }
    }

    std::vector<int> idxmap(v.size(), -1);
    std::vector<std::vector<T>> vs;
    for (size_t i = 0; i < v.size(); ++i) {
        size_t set = uf.find(i);
        int group = idxmap[set];
        if (group < 0) {
            idxmap[set] = vs.size();
            vs.emplace_back(1, v[i]);
        } else {
            vs[group].push_back(v[i]);
        }
    }
    return vs;
}

#endif
