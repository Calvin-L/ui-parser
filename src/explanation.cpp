#include "explanation.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <algorithm>
#include "geometry.hpp"

using namespace std;
using namespace cv;

Vec4i getLine(const LayoutObject* box, MeasurementRel relation) {
    int top = box->data.boxData[1];
    int left = box->data.boxData[0];
    int right = box->data.boxData[0] + box->data.boxData[2];
    int bot = box->data.boxData[1] + box->data.boxData[3];
    switch (relation) {
        case TOP_BORDER:    return Vec4i(left, top, right, top);
        case LEFT_BORDER:   return Vec4i(left, top, left, bot);
        case RIGHT_BORDER:  return Vec4i(right, top, right, bot);
        case BOTTOM_BORDER: return Vec4i(left, bot, right, bot);
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

LayoutObject* findBestBox(vector<VotedStroke>& strokes) {
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

    return new LayoutObject(obj);
}

void considerTarget(const LayoutObject* o, const Vec2i& pt, const Vec4i& line, MeasurementRel lineRel, const LayoutObject* &best, MeasurementRel& rel, double& bestScore) {
    double score = closestApproach(pt, line);
    if (best == nullptr || score < bestScore) {
        bestScore = score;
        best = o;
        rel = lineRel;
    }
}

const char* printRel(MeasurementRel rel) {
    switch (rel) {
        case TOP_BORDER:    return "TOP_BORDER";
        case LEFT_BORDER:   return "LEFT_BORDER";
        case RIGHT_BORDER:  return "RIGHT_BORDER";
        case BOTTOM_BORDER: return "BOTTOM_BORDER";
    }
}

bool findTopTarget(const VotedStroke& s, const vector<LayoutObject*>& objs, LayoutObject& line) {
    auto l = orientTB(s.stroke.line);
    auto pt = Vec2i(l[0], l[1]);

    const LayoutObject* best = nullptr;
    MeasurementRel rel;
    double bestScore;
    for (auto o : objs) {
        if (o->type == LAYOUT_BOX) {
            considerTarget(o, pt, getLine(o, TOP_BORDER),    TOP_BORDER,    best, rel, bestScore);
            considerTarget(o, pt, getLine(o, BOTTOM_BORDER), BOTTOM_BORDER, best, rel, bestScore);
        }
    }

    line.data.measurementData.box1 = best;
    line.data.measurementData.rel1 = rel;

    if (best != nullptr) {
        cerr << "top: " << l << " --> " << best->data.boxData[0] << ", " << best->data.boxData[1] << ", " << best->data.boxData[2] << ", " << best->data.boxData[3] << "(" << printRel(rel) << ")" << endl;
    }

    return best != nullptr;
}

bool findBottomTarget(const VotedStroke& s, const vector<LayoutObject*>& objs, LayoutObject& line) {
    auto l = orientTB(s.stroke.line);
    auto pt = Vec2i(l[2], l[3]);

    const LayoutObject* best = nullptr;
    MeasurementRel rel;
    double bestScore;
    for (auto o : objs) {
        if (o->type == LAYOUT_BOX) {
            considerTarget(o, pt, getLine(o, TOP_BORDER),    TOP_BORDER,    best, rel, bestScore);
            considerTarget(o, pt, getLine(o, BOTTOM_BORDER), BOTTOM_BORDER, best, rel, bestScore);
        }
    }

    if (best != nullptr) {
        cerr << "bot: " << l << " --> " << best->data.boxData[0] << ", " << best->data.boxData[1] << ", " << best->data.boxData[2] << ", " << best->data.boxData[3] << "(" << printRel(rel) << ")" << endl;
    }

    line.data.measurementData.box2 = best;
    line.data.measurementData.rel2 = rel;

    return best != nullptr;
}

bool findLeftTarget(const VotedStroke& s, const vector<LayoutObject*>& objs, LayoutObject& line) {
    auto l = orientLR(s.stroke.line);
    auto pt = Vec2i(l[0], l[1]);

    const LayoutObject* best = nullptr;
    MeasurementRel rel;
    double bestScore;
    for (auto o : objs) {
        if (o->type == LAYOUT_BOX) {
            considerTarget(o, pt, getLine(o, LEFT_BORDER),  LEFT_BORDER,  best, rel, bestScore);
            considerTarget(o, pt, getLine(o, RIGHT_BORDER), RIGHT_BORDER, best, rel, bestScore);
        }
    }

    line.data.measurementData.box1 = best;
    line.data.measurementData.rel1 = rel;

    return best != nullptr;
}

bool findRightTarget(const VotedStroke& s, const vector<LayoutObject*>& objs, LayoutObject& line) {
    auto l = orientLR(s.stroke.line);
    auto pt = Vec2i(l[2], l[3]);

    const LayoutObject* best = nullptr;
    MeasurementRel rel;
    double bestScore;
    for (auto o : objs) {
        if (o->type == LAYOUT_BOX) {
            considerTarget(o, pt, getLine(o, LEFT_BORDER),  LEFT_BORDER,  best, rel, bestScore);
            considerTarget(o, pt, getLine(o, RIGHT_BORDER), RIGHT_BORDER, best, rel, bestScore);
        }
    }

    line.data.measurementData.box2 = best;
    line.data.measurementData.rel2 = rel;

    return best != nullptr;
}

bool layoutLine(const VotedStroke& s, const vector<LayoutObject*>& objs, LayoutObject& line) {
    if (mostlyVertical(s.stroke.line)) {
        return findTopTarget(s, objs, line) && findBottomTarget(s, objs, line);
    } else if (mostlyHorizontal(s.stroke.line)) {
        return findLeftTarget(s, objs, line) && findRightTarget(s, objs, line);
    }
    return false;
}

const char* getLabel(const VotedStroke& s) {
    for (auto& v : s.votes) {
        if (v.type == TEXT) {
            return v.label.text;
        } else if (v.type == MEASUREMENT_LINE) {
            return v.label.text;
        }
    }
    return "";
}

vector<LayoutObject*> explain(vector<VotedStroke> strokes) {
    vector<LayoutObject*> result;

    int nboxes = estimateBoxCount(strokes);
    cerr << "guessing there are " << nboxes << " boxes..." << endl;
    for (int i = 0; i < nboxes; ++i) {
        result.push_back(findBestBox(strokes));
    }

    auto it = strokes.begin();
    int nlines = 0;
    LayoutObject o;
    o.type = MEASUREMENT;
    while (it != strokes.end()) {

        const VotedStroke& s = *it;

        if (hasVote(s, MEASUREMENT_LINE) && layoutLine(s, result, o)) {
            o.data.measurementData.text = getLabel(s);
            result.push_back(new LayoutObject(o));
            it = strokes.erase(it);
            ++nlines;
        } else {
            ++it;
        }

    }

    cerr << "#lines = " << nlines << endl;

    return result;
}

Mat displayObjects(const Mat& bg, const vector<LayoutObject*>& objects) {
    Mat display;
    cvtColor(bg, display, CV_GRAY2BGR);

    for (auto o : objects) {
        const LayoutObject& obj = *o;

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
                cerr << "start: " << " --> " << obj.data.measurementData.box1->data.boxData[0] << ", " << obj.data.measurementData.box1->data.boxData[1] << ", " << obj.data.measurementData.box1->data.boxData[2] << ", " << obj.data.measurementData.box1->data.boxData[3] << " (" << printRel(obj.data.measurementData.rel1) << ")" << endl;
                cerr << "  end: " << " --> " << obj.data.measurementData.box2->data.boxData[0] << ", " << obj.data.measurementData.box2->data.boxData[1] << ", " << obj.data.measurementData.box2->data.boxData[2] << ", " << obj.data.measurementData.box2->data.boxData[3] << " (" << printRel(obj.data.measurementData.rel2) << ")" << endl;
                line(display, p1, p2, Scalar(0, 255, 0));
                break;

        }

    }

    return display;
}
