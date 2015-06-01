#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H 1

#include <vector>

#include "explanation.hpp"

enum Unit {
    UNIT_PX,
    UNIT_PERCENT
};

struct Length {
    Unit unit;
    double value;
};

struct Constraint {

};

std::vector<Constraint> formConstraints(const std::vector<LayoutObject*>& objects);

#endif
