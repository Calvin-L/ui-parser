#include "geometry.hpp"

using namespace cv;

float anglediff(float a1, float a2) {
    float diff = a1 - a2;
    if (diff > M_PI) {
        diff -= 2*M_PI;
    } else if (diff < -M_PI) {
        diff += 2*M_PI;
    }
    return diff;
}

Vec4i segmentOverlapWithRect(Vec4i segment, const Rect& rect) {
    return
        clipLineRight(
            clipLineLeft(
                clipLineBot(
                    clipLineTop(segment, rect.y),
                    rect.y + rect.height),
                rect.x),
            rect.x + rect.width);
}
