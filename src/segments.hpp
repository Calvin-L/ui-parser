#ifndef SEGMENTS_H
#define SEGMENTS_H 1

#include <vector>
#include <opencv2/core/core.hpp>

std::vector<cv::Vec4i> findSegments(const cv::Mat& img);
cv::Mat displaySegments(const cv::Mat& bg, const std::vector<cv::Vec4i>& segments);

#endif
