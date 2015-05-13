#include "explanation.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <algorithm>
#include "geometry.hpp"

using namespace std;
using namespace cv;

Vec4i getLine(LayoutObject* box, MeasurementRel relation) {
    switch (relation) {
        case TOP_BORDER:    return Vec4i(box->data.boxData[0], box->data.boxData[1], box->data.boxData[0] + box->data.boxData[2], box->data.boxData[1]);
        case LEFT_BORDER:   return Vec4i(box->data.boxData[0], box->data.boxData[1], box->data.boxData[0], box->data.boxData[1] + box->data.boxData[3]);
        case RIGHT_BORDER:  return Vec4i(box->data.boxData[0] + box->data.boxData[2], box->data.boxData[1], box->data.boxData[0] + box->data.boxData[2], box->data.boxData[1] + box->data.boxData[3]);
        case BOTTOM_BORDER: return Vec4i(box->data.boxData[0], box->data.boxData[1] + box->data.boxData[3], box->data.boxData[0] + box->data.boxData[2], box->data.boxData[1] + box->data.boxData[3]);
    }
}

bool hasBestVote(const VotedStroke& s, VoteType type) {
    VoteType best = bestVote(s)->type;
    int bestCount = 0;
    int tyCount = 0;
    for (auto& v : s.votes) {
        if (v.type == best) ++bestCount;
        if (v.type == type) ++tyCount;
    }
    return tyCount == bestCount;
}

bool hasVote(const VotedStroke& s, VoteType type) {
    for (auto& v : s.votes) {
        if (v.type == type) {
            return true;
        }
    }
    return false;
}

int estimateBoxCount(const vector<VotedStroke>& strokes) {
    int ntops = 0, nlefts = 0, nrights = 0, nbots = 0;

    for (auto& s : strokes) {
        if (hasVote(s, BOX_TOP)) ++ntops;
        if (hasVote(s, BOX_LEFT)) ++nlefts;
        if (hasVote(s, BOX_RIGHT)) ++nrights;
        if (hasVote(s, BOX_BOTTOM)) ++nbots;
    }

    return min(min(ntops, nlefts), min(nrights, nbots));
}

vector<VotedStroke> findByVote(const vector<VotedStroke>& strokes, VoteType vote) {
    vector<VotedStroke> result;
    for (auto& s : strokes) {
        if (hasVote(s, vote)) {
            result.push_back(s);
        }
    }
    return result;
}

double scoreBox(const VotedStroke& top, const VotedStroke& left, const VotedStroke& right, const VotedStroke& bot) {
    return
      -(closestApproach(top.stroke.line, left.stroke.line) +
        closestApproach(left.stroke.line, bot.stroke.line) +
        closestApproach(bot.stroke.line, right.stroke.line) +
        closestApproach(right.stroke.line, top.stroke.line));
}

LayoutObject findBestBox(vector<VotedStroke>& strokes) {
    LayoutObject obj;
    obj.type = LAYOUT_BOX;

    auto tops   = findByVote(strokes, BOX_TOP);
    auto lefts  = findByVote(strokes, BOX_LEFT);
    auto rights = findByVote(strokes, BOX_RIGHT);
    auto bots   = findByVote(strokes, BOX_BOTTOM);

    bool first = true;
    double bestScore;
    VotedStroke bestTop, bestLeft, bestRight, bestBot;

    for (auto& top : tops) {
        for (auto& left : lefts) {
            for (auto& right : rights) {
                for (auto& bot : bots) {
                    double score = scoreBox(top, left, right, bot);
                    if (first || score > bestScore) {
                        first = false;
                        bestScore = score;
                        bestTop = top;
                        bestLeft = left;
                        bestRight = right;
                        bestBot = bot;
                    }
                }
            }
        }
    }

    if (first) {
        cerr << "failed to find box" << endl;
    } else {
        int* rect = obj.data.boxData;
        rect[0] = midpoint(bestLeft.stroke.line)[0];
        rect[1] = midpoint(bestTop.stroke.line)[1];
        rect[2] = midpoint(bestRight.stroke.line)[0] - rect[0];
        rect[3] = midpoint(bestBot.stroke.line)[1] - rect[1];

        strokes.erase(find(strokes.begin(), strokes.end(), bestTop));
        strokes.erase(find(strokes.begin(), strokes.end(), bestLeft));
        strokes.erase(find(strokes.begin(), strokes.end(), bestRight));
        strokes.erase(find(strokes.begin(), strokes.end(), bestBot));
    }

    return obj;
}

vector<LayoutObject> explain(vector<VotedStroke> strokes) {
    vector<LayoutObject> result;

    int nboxes = estimateBoxCount(strokes);
    cerr << "guessing there are " << nboxes << " boxes..." << endl;
    for (int i = 0; i < nboxes; ++i) {
        result.push_back(findBestBox(strokes));
    }

    return result;
}

Mat displayObjects(const Mat& bg, const vector<LayoutObject>& objects) {
    Mat display;
    cvtColor(bg, display, CV_GRAY2BGR);

    for (auto& obj : objects) {

        switch (obj.type) {

            case LAYOUT_BOX:
                rectangle(display,
                    Point(obj.data.boxData[0], obj.data.boxData[1]),
                    Point(obj.data.boxData[0] + obj.data.boxData[2], obj.data.boxData[1] + obj.data.boxData[3]),
                    Scalar(255, 0, 0));
                break;

            case MEASUREMENT:
                Point p1 = midpoint(getLine(obj.data.measurementData.box1, obj.data.measurementData.rel1));
                Point p2 = midpoint(getLine(obj.data.measurementData.box2, obj.data.measurementData.rel2));
                line(display, p1, p2, Scalar(0, 255, 0));
                break;

        }

    }

    return display;
}
