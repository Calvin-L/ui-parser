#include "strokes.hpp"
#include "UnionFind.hpp"
#include "geometry.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

static double dominantAngle(const vector<Vec4i>& lines, int nbuckets = 8) {
    int topBucket = 0;
    vector<int> buckets(nbuckets, 0);

    for (auto& l : lines) {
        double angle = angleOf(l);
        if (angle < 0) angle += TAU;
        for (int k = 0; k < 2; ++k) {
            int bucket = 0;
            for (; bucket < nbuckets; ++bucket) {
                if (bucket * (TAU/nbuckets) + (TAU/nbuckets/2) > angle) break;
            }
            if (bucket == nbuckets) {
                bucket = 0;
            }
            ++buckets[bucket];
            if (buckets[bucket] > buckets[topBucket]) {
                topBucket = bucket;
            }
            angle -= TAU/2;
        }
    }
    return topBucket * (TAU/nbuckets);
}

static bool segmentsTooClose(const Vec4i& v1, const Vec4i& v2) {
    auto vd1 = dirOf(v1);
    auto vd2 = dirOf(v2);
    double d = abs(vd1.ddot(vd2) / (norm(vd1) * norm(vd2)));
    return d >= 0.8 && closestApproach(v1, v2) < 10.0;
}

static Vec2d centroid(const vector<Vec4i>& lines) {
    double x = 0;
    double y = 0;
    for (auto& l : lines) {
        x += l[0] + l[2];
        y += l[1] + l[3];
    }
    x /= lines.size() * 2;
    y /= lines.size() * 2;
    return Vec2d(x, y);
}

static Stroke mergeLines(const vector<Vec4i>& lines) {
    const Vec2d center = centroid(lines);
    const double angle = dominantAngle(lines);
    const double dx = cos(angle);
    const double dy = sin(angle);
    double minScore, maxScore;
    int worstX = 0, worstY = 0, bestX = 0, bestY = 0;
    bool first = true;
    for (auto& l : lines) {
        int x0 = l[0] - center[0];
        int y0 = l[1] - center[1];
        int x1 = l[2] - center[0];
        int y1 = l[3] - center[1];

        // TODO: tweak these
        double score0 = (x0 * dx + y0 * dy);
        double score1 = (x1 * dx + y1 * dy);

        if (score0 > score1) {
            swap(score0, score1);
            swap(x0, x1);
            swap(y0, y1);
        }

        if (first) {
            minScore = score0;
            maxScore = score1;
            worstX = x0;
            worstY = y0;
            bestX = x1;
            bestY = y1;
            first = false;
        } else {
            if (score0 < minScore) {
                minScore = score0;
                worstX = x0;
                worstY = y0;
            }
            if (score1 > maxScore) {
                maxScore = score1;
                bestX = x1;
                bestY = y1;
            }
        }
    }
    return Stroke {
        Vec4i(
            worstX + center[0],
            worstY + center[1],
            bestX  + center[0],
            bestY  + center[1]),
        angle };
}

vector<Stroke> findStrokes(const vector<Vec4i>& segments) {
    vector<Stroke> strokes;
    for (const auto& g : group(segments, segmentsTooClose)) {
        strokes.push_back(mergeLines(g));
    }
    return strokes;
}

Mat displayStrokes(const Mat& bg, const vector<Stroke>& strokes) {
    Mat display;
    cvtColor(bg, display, CV_GRAY2BGR);
    srand(0);
    for (auto& stroke : strokes) {
        Scalar color = Scalar(rand() % 255, rand() % 255, rand() % 100 + 155);
        auto& l = stroke.line;
        double angle = stroke.angle;
        auto center = Vec2i((l[0] + l[2]) / 2, (l[1] + l[3]) / 2);
        line(display,
            Point(l[0], l[1]),
            Point(l[2], l[3]),
            color, 3, CV_AA);
        line(display,
            Point(center[0], center[1]),
            Point(center[0] + cos(angle)*20, center[1] + sin(angle)*20),
            Scalar(0, 0xff, 0), 2, CV_AA);
    }
    return display;
}
