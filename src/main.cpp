
#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <cstring>
#include <algorithm>

// #include <tesseract/baseapi.h>
// #include <leptonica/allheaders.h>
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

Vec4i mergeLines(const vector<Vec4i>& lines) {
    double angle = dominantAngle(lines);
    double dx = cos(angle);
    double dy = sin(angle);
    double minScore, maxScore;
    int worstX = 0, worstY = 0, bestX = 0, bestY = 0;
    bool first = true;
    for (auto& l : lines) {
        int x0 = l[0];
        int y0 = l[1];
        int x1 = l[2];
        int y1 = l[3];

        double norm0 = sqrt(x0*x0 + y0*y0);
        double norm1 = sqrt(x1*x1 + y1*y1);

        // TODO: these need lots of tweaking...
        // Right now they measure how closely the vector (x,y) matches (dx,dy),
        // which is not right! Somehow we need to normalize for position of the
        // line.
        double score0 = (x0 * dx + y0 * dy) / norm0;
        double score1 = (x1 * dx + y1 * dy) / norm1;

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
                bestX = x0;
                bestY = y0;
            }
        }
    }
    return Vec4i(worstX, worstY, bestX, bestY);
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

    vector<Vec4i> lines;

    HoughLinesP(dst, lines, 1, TAU/360, 20, 10, 25);

    auto groups = group(lines, segmentsTooClose);

    lines.clear();
    cout << groups.size() << endl;
    for (auto& g : groups) {
        Vec4i l = mergeLines(g);
        lines.push_back(l);
        cout << "  " << g.size() << "  \t" << l << "  \t(theta=" << dominantAngle(g) << ')' << endl;
    }

    cvtColor(dst, display, CV_GRAY2BGR);
    srand(0);
    for (const auto& g : groups) {
        Scalar color = cv::Scalar(rand() % 255, rand() % 255, rand() % 100 + 155);
        for (const auto& l : g) {
            line(display,
                Point(l[0], l[1]),
                Point(l[2], l[3]),
                color, 3, CV_AA);
        }
    }
    imshow("grouping", display);

    srand(0);
    cvtColor(dst, display, CV_GRAY2BGR);
    for (const auto& l : lines) {
        Scalar color = cv::Scalar(rand() % 255, rand() % 255, rand() % 100 + 155);
        line(display,
            Point(l[0], l[1]),
            Point(l[2], l[3]),
            color, 3, CV_AA);
    }
    imshow("lines", display);

    waitKey(0);

#if 0
    tesseract::TessBaseAPI ocr;

    if (ocr.Init(NULL, "eng")) {
        cerr << "Could not initialize tesseract." << endl;
        return 1;
    }

    Pix* image = pixRead(file);

    cout << "Image stats:" << endl;
    cout << "  size        = " << image->w << 'x' << image->h << endl;
    cout << "  depth/pixel = " << image->d << endl;
    cout << "  channels    = " << image->spp << endl;

    ocr.SetImage(image);

#if 0
    // ocr.SetRectangle(136, 156, 130, 92);
    const char* text = ocr.GetUTF8Text();
    cout << "Read text: '" << text << '\'' << endl;
    const char* p = text;
    while (*p) {
        printf("%02x ", (int)(*p));
        ++p;
    }
    cout << endl;
    delete[] text;
#endif

#if 1
    ocr.Recognize(0);
    auto ri = ocr.GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
    if (ri != 0) {
        do {
            const char* word = ri->GetUTF8Text(level);
            float conf = ri->Confidence(level);
            int x1, y1, x2, y2;
            ri->BoundingBox(level, &x1, &y1, &x2, &y2);
            printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
                word, conf, x1, y1, x2-x1, y2-y1);
            delete[] word;
        } while (ri->Next(level));
    }
#endif

#if 0
    Boxa* boxes = ocr.GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
    printf("Found %d textline image components.\n", boxes->n);
    for (int i = 0; i < boxes->n; i++) {
        BOX* box = boxaGetBox(boxes, i, L_CLONE);
        ocr.SetRectangle(box->x, box->y, box->w, box->h);
        const char* ocrResult = ocr.GetUTF8Text();
        int conf = ocr.MeanTextConf();
        fprintf(stdout, "Box[%d]: x=%d, y=%d, w=%d, h=%d, confidence: %d, text: %s",
            i, box->x, box->y, box->w, box->h, conf, ocrResult);
        delete[] ocrResult;
    }
#endif

    ocr.End();
    pixDestroy(&image);
#endif

    return 0;

}
