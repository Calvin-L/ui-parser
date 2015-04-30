#include "voting.hpp"
#include "geometry.hpp"
#include "util.hpp"
#include <algorithm>
#include <opencv2/imgproc/imgproc.hpp>

const static TextBox NO_TEXT { };

using namespace std;
using namespace cv;

static const TextBox* findEnclosingTextBox(const Stroke& stroke, const vector<TextBox>& ocr) {
    double len = segmentLength(stroke.line);
    for (auto& box : ocr) {
        if (segmentLength(segmentOverlapWithRect(stroke.line, box.boundary)) > 0.85 * len) {
            return &box;
        }
    }
    return nullptr;
}

static bool mostlyHorizontal(const Vec4i& v) {
    return abs(cos(angleOf(v))) > abs(cos(45));
}

static bool mostlyVertical(const Vec4i& v) {
    return !mostlyHorizontal(v);
}

static bool couldBeMeasurementText(const Vec4i& line, const TextBox& text) {
    // no overlap
    if (segmentLength(segmentOverlapWithRect(line, text.boundary)) > 1) {
        return false;
    }

    // pretty close
    const int thresh = 100;
    if (mostlyVertical(line)) {
        return min(abs(line[0] - text.boundary.x), abs(line[0] - (text.boundary.x + text.boundary.width))) < thresh;
    } else if (mostlyHorizontal(line)) {
        return min(abs(line[1] - text.boundary.y), abs(line[1] - (text.boundary.y + text.boundary.height))) < thresh;
    }
    return false;
}

static vector<const TextBox*> findMeasurementTextBoxes(const Stroke& stroke, const vector<TextBox>& ocr) {
    vector<const TextBox*> result;
    for (auto& box : ocr) {
        if (couldBeMeasurementText(stroke.line, box)) {
            // TODO: find top object and bottom object
            result.push_back(&box);
        }
    }
    return result;
}

static const double CORNER_THRESH = 0.2;

static void orientLR(VotedStroke& stroke) {
    Vec4i& l = stroke.stroke.line;
    if (l[0] > l[2]) {
        std::swap(l[0], l[2]);
        std::swap(l[1], l[3]);
    }
}

static VotedStroke* findLeftStroke(VotedStroke& stroke, vector<VotedStroke>& strokes) {
    orientLR(stroke);
    double len = segmentLength(stroke.stroke.line);
    for (auto& s : strokes) {
        if (mostlyVertical(s.stroke.line) &&
                min(distance(p1(stroke.stroke.line), p1(s.stroke.line)),
                    distance(p1(stroke.stroke.line), p2(s.stroke.line))) < CORNER_THRESH * len) {
            return &s;
        }
    }
    return nullptr;
}

static VotedStroke* findRightStroke(VotedStroke& stroke, vector<VotedStroke>& strokes) {
    orientLR(stroke);
    double len = segmentLength(stroke.stroke.line);
    for (auto& s : strokes) {
        if (mostlyVertical(s.stroke.line) &&
                min(distance(p2(stroke.stroke.line), p1(s.stroke.line)),
                    distance(p2(stroke.stroke.line), p2(s.stroke.line))) < CORNER_THRESH * len) {
            return &s;
        }
    }
    return nullptr;
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

        if (mostlyHorizontal(stroke.stroke.line)) {
            VotedStroke* leftSide = findLeftStroke(stroke, v);
            VotedStroke* rightSide = findRightStroke(stroke, v);
            bool isTop = false;
            if (leftSide != nullptr) {
                leftSide->votes.push_back({ BOX_LEFT });
                int idx = abs(leftSide->stroke.line[1] - stroke.stroke.line[1]) > abs(leftSide->stroke.line[3] - stroke.stroke.line[1]) ? 1 : 3;
                isTop = leftSide->stroke.line[idx] > stroke.stroke.line[1];
            }
            if (rightSide != nullptr) {
                rightSide->votes.push_back({ BOX_RIGHT });
                int idx = abs(rightSide->stroke.line[1] - stroke.stroke.line[1]) > abs(rightSide->stroke.line[3] - stroke.stroke.line[1]) ? 1 : 3;
                isTop = rightSide->stroke.line[idx] > stroke.stroke.line[1];
            }
            if (leftSide != nullptr || rightSide != nullptr) {
                stroke.votes.push_back({ isTop ? BOX_TOP : BOX_BOTTOM });
            }
        } else if (mostlyVertical(stroke.stroke.line)) {
            // stroke.votes.push_back({ BOX_LEFT });
        }

        for (auto* ptr : findMeasurementTextBoxes(stroke.stroke, ocr)) {
            if (ptr != nullptr) {
                stroke.votes.push_back({ MEASUREMENT_LINE, *ptr });
            }
        }
    }

    return v;
}

static VoteType toType(const Vote& v) {
    return v.type;
}

const Vote* bestVote(const VotedStroke& stroke) {
    if (stroke.votes.size() == 0) {
        return nullptr;
    }

    vector<VoteType> types(stroke.votes.size());
    std::transform(stroke.votes.begin(), stroke.votes.end(), types.begin(), toType);
    VoteType bestType = mode(types.begin(), types.end());

    for (auto& v : stroke.votes) {
        if (v.type == bestType) {
            return &v;
        }
    }
    return nullptr;
}

Mat displayVotes(const Mat& bg, const vector<VotedStroke>& votes) {
    Mat display;
    cvtColor(bg, display, CV_GRAY2BGR);
    for (auto& stroke : votes) {
        const Vote* v = bestVote(stroke);
        if (v != nullptr) {
            Scalar color(100, 100, 100);
            switch (v->type) {
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
        }
    }
    return display;
}
