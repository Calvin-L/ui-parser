#ifndef EXPLANATION_H
#define EXPLANATION_H 1

#include <vector>
#include <opencv2/core/core.hpp>

#include "voting.hpp"

enum LayoutObjectType {
    LAYOUT_BOX, MEASUREMENT
};

enum MeasurementRel {
    TOP_BORDER,
    LEFT_BORDER,
    RIGHT_BORDER,
    BOTTOM_BORDER
};

struct LayoutObject {
    LayoutObjectType type;

    union {

        int boxData[4]; // x, y, w, h

        // some part of box1 is some distance from box2
        struct {
            const LayoutObject* box1;
            MeasurementRel rel1;

            const LayoutObject* box2;
            MeasurementRel rel2;
        } measurementData;

    } data;

};

std::vector<LayoutObject*> explain(std::vector<VotedStroke> strokes);
cv::Mat displayObjects(const cv::Mat& bg, const std::vector<LayoutObject*>& objects);

#endif
