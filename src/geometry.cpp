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

bool contains(const cv::Rect& r1, const cv::Rect& r2) {
    return r1.x <= r2.x &&
        r1.y <= r2.y &&
        r1.width + r1.x >= r2.width + r2.x &&
        r1.height + r1.y >= r2.height + r2.y;
}
