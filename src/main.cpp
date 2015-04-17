
#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <utility>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "UnionFind.hpp"
#include "geometry.hpp"

using namespace std;
using namespace cv;

template <class T>
ostream& operator<<(ostream& out, vector<T> v) {
    out << '[';
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) out << ", ";
        out << v[i];
    }
    return out << ']';
}

Mat cleanup(Mat m, unsigned char thresh) {
    for (int row = 0; row < m.rows; ++row) {
        for (int col = 0; col < m.cols; ++col) {
            unsigned char& val = m.at<unsigned char>(row, col);
            val = val < thresh ? 0 : 0xff;
        }
    }
    return m;
}

bool segmentsTooClose(const Vec4i& v1, const Vec4i& v2) {
    auto vd1 = dirOf(v1);
    auto vd2 = dirOf(v2);
    double d = abs(vd1.ddot(vd2) / (norm(vd1) * norm(vd2)));
    return d >= 0.8 && closestApproach(v1, v2) < 10.0;
}

double dominantAngle(const vector<Vec4i>& lines, int nbuckets = 8) {
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

Vec2d centroid(const vector<Vec4i>& lines) {
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

pair<Vec4i, double> mergeLines(const vector<Vec4i>& lines) {
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
    return make_pair(
        Vec4i(
            worstX + center[0],
            worstY + center[1],
            bestX  + center[0],
            bestY  + center[1]),
        angle);
}

int main(int argc, char** argv) {

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <file>" << endl;
        return 1;
    }

    const char* file = argv[1];

    Mat input = imread(file, CV_LOAD_IMAGE_GRAYSCALE);

    if (input.size().width <= 0 || input.size().height <= 0) {
        cerr << "failed to read image '" << file << '\'' << endl;
        return 1;
    }

    imshow("input", input);

    Mat dst, display;
    // Canny(input, dst, 50, 200, 3);
    dst = Scalar::all(255) - input;
    dst = cleanup(dst, 10);

    // OCR
    {
        tesseract::TessBaseAPI ocr;

        if (ocr.Init(NULL, "eng" /*, tesseract::OEM_TESSERACT_CUBE_COMBINED */)) {
            cerr << "Could not initialize tesseract." << endl;
            return 1;
        }

        Mat txt = Scalar::all(255) - dst;
        Mat draw;
        cvtColor(txt, draw, CV_GRAY2BGR);

        ocr.SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
        ocr.SetVariable("tessedit_char_whitelist", "0123456789px% .-");
        ocr.SetImage(txt.data, txt.size().width, txt.size().height, txt.step[1], txt.step[0]);
        Boxa* boxes = ocr.GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
        printf("Found %d textline image components.\n", boxes->n);
        for (int i = 0; i < boxes->n; i++) {
            BOX* box = boxaGetBox(boxes, i, L_CLONE);
            ocr.SetRectangle(box->x, box->y, box->w, box->h);
            const char* ocrResult = ocr.GetUTF8Text();
            int conf = ocr.MeanTextConf();
            fprintf(stdout, "Box[%d]: x=%d, y=%d, w=%d, h=%d, confidence: %d, text: %s",
                i, box->x, box->y, box->w, box->h, conf, ocrResult);

            Scalar color = Scalar(rand() % 255, rand() % 100 + 155, rand() % 255);
            rectangle(draw, Point(box->x, box->y), Point(box->x + box->w, box->y + box->h), color);

            putText(draw, ocrResult, Point(box->x + box->w + 1, box->y + box->h + 1), FONT_HERSHEY_SIMPLEX, 0.5, Scalar::all(0), 1);
            putText(draw, ocrResult, Point(box->x + box->w, box->y + box->h), FONT_HERSHEY_SIMPLEX, 0.5, color, 1);

            delete[] ocrResult;
        }
        boxaDestroy(&boxes);

        imshow("text recoginition", draw);
    }


    vector<Vec4i> lines;

    HoughLinesP(dst, lines, 1, TAU/360, 20, 10, 25);

    auto groups = group(lines, segmentsTooClose);

    cvtColor(dst, display, CV_GRAY2BGR);
    srand(0);
    for (const auto& g : groups) {
        Scalar color = Scalar(rand() % 255, rand() % 255, rand() % 100 + 155);
        for (const auto& l : g) {
            line(display,
                Point(l[0], l[1]),
                Point(l[2], l[3]),
                color, 3, CV_AA);
        }
    }
    imshow("grouping", display);

    srand(0);
    cout << groups.size() << endl;
    cvtColor(dst, display, CV_GRAY2BGR);
    for (const auto& g : groups) {
        auto info = mergeLines(g);
        Vec4i l = info.first;
        double angle = info.second;
        lines.push_back(l);
        cout << "  " << g.size() << "  \t" << l << "  \t(theta=" << dominantAngle(g) << ')' << endl;
        Scalar color = Scalar(rand() % 255, rand() % 255, rand() % 100 + 155);
        auto center = centroid(g);
        line(display,
            Point(l[0], l[1]),
            Point(l[2], l[3]),
            color, 3, CV_AA);
        line(display,
            Point(center[0], center[1]),
            Point(center[0] + cos(angle)*20, center[1] + sin(angle)*20),
            Scalar(0, 0xff, 0), 2, CV_AA);
    }
    imshow("lines", display);

    waitKey(0);

    return 0;

}
