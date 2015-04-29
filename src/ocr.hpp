#ifndef OCR_H
#define OCR_H 1

#include <memory>
#include <vector>
#include <opencv2/core/core.hpp>

struct TextBox {
    cv::Rect boundary;
    std::shared_ptr<const char> text;
};

std::vector<TextBox> findText(const cv::Mat& img);
cv::Mat displayText(const cv::Mat& bg, const std::vector<TextBox>& textBoxes);

#endif
