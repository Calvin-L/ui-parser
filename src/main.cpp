
#include <iostream>
#include <vector>
#include <set>
#include <cmath>

// #include <tesseract/baseapi.h>
// #include <leptonica/allheaders.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

class UnionFind {
public:
    UnionFind(size_t count) {
        parents.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            parents.push_back(i); // every node links to itself
        }
    }
    size_t find(size_t x) {
        while (parents[x] != x) {
            x = parents[x];
        }
        return x;
    }
    void join(size_t x, size_t y) {
        x = find(x);
        y = find(y);
        parents[x] = y;
    }
private:
    vector<size_t> parents;
};

template <class T, class F>
vector<T> merge(vector<T> v, F similar) {
    UnionFind uf(v.size());

    for (size_t i = 0; i < v.size(); ++i) {
        for (size_t j = i + 1; j < v.size(); ++j) {
            if (similar(v[i], v[j])) {
                uf.join(i, j);
            }
        }
    }

    set<size_t> seen;
    size_t newEnd = 1;
    size_t end = 1;
    seen.insert(uf.find(0));
    for (; end < v.size(); ++end) {
        size_t set = uf.find(end);
        if (seen.find(set) == seen.end()) {
            seen.insert(set);
            v[newEnd] = v[end];
            ++newEnd;
        }
    }

    v.erase(v.begin() + newEnd, v.end());

    return v;
}

float anglediff(float a1, float a2) {
    float diff = a1 - a2;
    if (diff > M_PI) {
        diff -= 2*M_PI;
    } else if (diff < -M_PI) {
        diff += 2*M_PI;
    }
    return diff;
}

void fix(Vec2f& v) {
    if (v[0] < 1) {
        v[0] = abs(v[0]);
        v[1] += M_PI;
        if (v[1] > M_PI) {
            v[1] -= M_PI * 2;
        }
    }
}

bool tooClose(Vec2f l1, Vec2f l2) {
    fix(l1);
    fix(l2);
    return
        abs(l2[0] - l1[0]) < 50 &&
        abs(anglediff(l2[1], l1[1])) < M_PI / 10;
}

template <class T>
ostream& operator<<(ostream& out, vector<T> v) {
    out << '[';
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) out << ", ";
        out << v[i];
    }
    return out << ']';
}

int main(int argc, char** argv) {

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <file>" << endl;
        return 1;
    }

    const char* file = argv[1];

    Mat input = imread(file, CV_LOAD_IMAGE_GRAYSCALE);
    imshow("input", input);

    Mat dst, display;
    // Canny(input, dst, 50, 200, 3);
    dst = Scalar::all(255) - input;
    cvtColor(dst, display, CV_GRAY2BGR);

    vector<Vec2f> lines;
    HoughLines(dst, lines, 1, CV_PI/180, 100, 0, 0);

    cout << tooClose(Vec2f(150, 0.0349066), Vec2f(162, 0.0523599)) << endl;
    cout << lines << endl;
    lines = merge(lines, tooClose);
    cout << lines << endl;
    cout.flush();
    for (const auto& l : lines) {
        float rho = l[0], theta = l[1];
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        Point pt1, pt2;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line(display, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }

    imshow("output", display);

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
