#ifndef PRINTING_H
#define PRINTING_H 1

// Extra print routines.

#include <ostream>
#include <vector>

template <class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& v) {
    stream << '[';
    for (int i = 0; i < v.size(); ++i) {
        if (i > 0) {
            stream << ", ";
        }
        stream << v[i];
    }
    return stream << ']';
}

#endif
