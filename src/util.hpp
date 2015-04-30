#ifndef UTIL_H
#define UTIL_H 1

#include <map>

template <class It>
auto mode(It start, const It& end) -> typename It::value_type {
    typedef typename It::value_type T;
    std::map<T, int> counts;
    int bestCount = 0;
    T bestVal;
    for (; start != end; ++start) {
        const T& v = *start;
        int count = ++counts[v];
        if (count > bestCount) {
            bestVal = v;
        }
    }
    return bestVal;
}

#endif
