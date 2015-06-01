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

enum ConstraintType {
    CONSTRAINT_CONTAINS,   // obj1 contains obj2 (len ignored)
    CONSTRAINT_VERTSPACE,  // obj1 above & vertical space between obj1 and obj2
    CONSTRAINT_HORIZSPACE, // obj1 left & horizontal space between obj1 and obj2
    CONSTRAINT_PAD_TOP,    // obj1 contains obj2, top padding exists
    CONSTRAINT_PAD_RIGHT,  // obj1 contains obj2, right padding exists
    CONSTRAINT_PAD_BOTTOM, // obj1 contains obj2, bottom padding exists
    CONSTRAINT_PAD_LEFT,   // obj1 contains obj2, left padding exists
    CONSTRAINT_WIDTH,      // obj1 width (obj2 ignored)
    CONSTRAINT_HEIGHT,     // obj1 height (obj2 ignored)
};

struct Constraint {
    ConstraintType type;
    LayoutObject* obj1;
    LayoutObject* obj2;
    Length len;
};

std::vector<Constraint> formConstraints(const std::vector<LayoutObject*>& objects);

#endif
