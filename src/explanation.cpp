#include "explanation.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

vector<LayoutObject> explain(const vector<VotedStroke>& strokes) {
    vector<LayoutObject> result;

    return result;
}

Mat displayObjects(const Mat& bg, const vector<LayoutObject>& objects) {
    Mat display;
    cvtColor(bg, display, CV_GRAY2BGR);

    return display;
}
