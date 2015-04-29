#ifndef EXPLANATION_H
#define EXPLANATION_H 1

#include <vector>

#include "voting.hpp"

enum LayoutObjectType {
    LAYOUT_BOX, MEASUREMENT
};

struct LayoutObject {
    LayoutObjectType type;
};

std::vector<LayoutObject> explain(const std::vector<VotedStroke>& strokes);

#endif
