#ifndef STROKES_H
#define STROKES_H 1

#include <vector>
#include <opencv2/core/core.hpp>

struct Stroke {
    cv::Vec4i line;
    double angle;
};

std::vector<Stroke> findStrokes(const std::vector<cv::Vec4i>& segments);
cv::Mat displayStrokes(const cv::Mat& bg, const std::vector<Stroke>& strokes);

#endif
