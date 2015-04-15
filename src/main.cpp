
#include <iostream>
#include <vector>
#include <set>
#include <cmath>
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

Vec4i mergeLines(const vector<Vec4i>& lines) {
    double avgangle = 0;
    for (const auto& l : lines) { avgangle += nonDirectionalAngleOf(l); }
    avgangle /= lines.size();
    return lines[0];
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
    cvtColor(dst, display, CV_GRAY2BGR);

    vector<Vec4i> lines;

    HoughLinesP(dst, lines, 1, TAU/360, 20, 10, 25);

    auto groups = group(lines, segmentsTooClose);

    cout << groups.size() << endl;
    for (auto& g : groups) {
        cout << "  " << g.size() << endl;
    }

    lines.clear();
    transform(groups.begin(), groups.end(), lines.begin(), mergeLines);

    for (const auto& g : groups) {
        Scalar color = cv::Scalar(rand() % 255, rand() % 255, rand() % 100 + 155);
        for (const auto& l : g) {
            line(display,
                Point(l[0], l[1]),
                Point(l[2], l[3]),
                color, 3, CV_AA);
        }
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
