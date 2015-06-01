#ifndef LAYOUT_H
#define LAYOUT_H 1

#include <vector>
#include <ostream>

#include "explanation.hpp"
#include "constraints.hpp"

struct Element;

struct Layout {
    Element* root;
};

Layout toLayout(
    const std::vector<LayoutObject*>& objects,
    const std::vector<Constraint>& constraints);

std::ostream& operator<<(std::ostream& stream, const Layout& layout);

#endif
