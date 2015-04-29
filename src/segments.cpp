#include "segments.hpp"
#include "geometry.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

static Mat cleanup(Mat m, unsigned char thresh) {
    for (int row = 0; row < m.rows; ++row) {
        for (int col = 0; col < m.cols; ++col) {
            unsigned char& val = m.at<unsigned char>(row, col);
            val = val < thresh ? 0 : 0xff;
        }
    }
    return m;
}

vector<Vec4i> findSegments(const Mat& img) {
    Mat dst = cleanup(Scalar::all(255) - img, 10);
    vector<Vec4i> lines;
    HoughLinesP(dst, lines, 1, TAU/360, 20, 10, 25);
    return lines;
}

cv::Mat displaySegments(const cv::Mat& bg, const std::vector<cv::Vec4i>& segments) {
    Mat display;
    cvtColor(bg, display, CV_GRAY2BGR);
    srand(0);
    for (const auto& seg : segments) {
        Scalar color = Scalar(rand() % 255, rand() % 255, rand() % 100 + 155);
        line(display,
            Point(seg[0], seg[1]),
            Point(seg[2], seg[3]),
            color, 3, CV_AA);
    }
    return display;
}
