#ifndef EXPLANATION_H
#define EXPLANATION_H 1

#include <vector>
#include <opencv2/core/core.hpp>

#include "voting.hpp"

enum LayoutObjectType {
    LAYOUT_BOX, MEASUREMENT
};

struct LayoutObject {
    LayoutObjectType type;
};

std::vector<LayoutObject> explain(const std::vector<VotedStroke>& strokes);
cv::Mat displayObjects(const cv::Mat& bg, const std::vector<LayoutObject>& objects);

#endif
