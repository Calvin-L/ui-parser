#include "voting.hpp"
#include "geometry.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

static const TextBox* findEnclosingTextBox(const Stroke& stroke, const vector<TextBox>& ocr) {
    double len = segmentLength(stroke.line);
    for (auto& box : ocr) {
        if (segmentLength(segmentOverlapWithRect(stroke.line, box.boundary)) > 0.85 * len) {
            return &box;
        }
    }
    return NULL;
}

vector<VotedStroke> placeVotes(
    const vector<Stroke>& strokes,
    const vector<TextBox>& ocr) {

    vector<VotedStroke> v;
    for (auto& stroke : strokes) {
        v.push_back(VotedStroke { stroke, vector<Vote>() });
    }

    for (auto& stroke : v) {
        const TextBox* text = findEnclosingTextBox(stroke.stroke, ocr);
        if (text != nullptr) {
            stroke.votes.push_back({ TEXT, *text });
        }
    }

    return v;
}

Mat displayVotes(const Mat& bg, const vector<VotedStroke>& votes) {
    Mat display;
    cvtColor(bg, display, CV_GRAY2BGR);
    for (auto& stroke : votes) {
        for (auto& vote : stroke.votes) {
            Scalar color(100, 100, 100);
            switch (vote.type) {
                case BOX_TOP:          color = Scalar(255, 0,   0);   break;
                case BOX_LEFT:         color = Scalar(0,   255, 0);   break;
                case BOX_RIGHT:        color = Scalar(0,   0,   255); break;
                case BOX_BOTTOM:       color = Scalar(255, 0,   255); break;
                case MEASUREMENT_LINE: color = Scalar(0,   255, 255); break;
                case TEXT:             color = Scalar(255, 255, 0);   break;
            }
            line(display,
                Point(stroke.stroke.line[0], stroke.stroke.line[1]),
                Point(stroke.stroke.line[2], stroke.stroke.line[3]),
                color, 3, CV_AA);
            break;
        }
    }
    return display;
}
