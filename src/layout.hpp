#ifndef LAYOUT_H
#define LAYOUT_H 1

#include <vector>
#include <iostream>

#include "explanation.hpp"
#include "constraints.hpp"

struct Layout {

};

Layout toLayout(
    const std::vector<LayoutObject>& objects,
    const std::vector<Constraint>& constraints);

std::ostream& operator<<(std::ostream& stream, const Layout& layout);

#endif
