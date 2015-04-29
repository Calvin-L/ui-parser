#include "ocr.hpp"
#include <cstdlib>
#include <iostream>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

vector<TextBox> findText(const Mat& img) {
    tesseract::TessBaseAPI ocr;

    if (ocr.Init(NULL, "eng" /*, tesseract::OEM_TESSERACT_CUBE_COMBINED */)) {
        cerr << "Could not initialize tesseract." << endl;
        exit(1);
    }

    Mat txt = Scalar::all(255) - img;
    vector<TextBox> result;

    ocr.SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
    ocr.SetVariable("tessedit_char_whitelist", "0123456789px% .-");
    ocr.SetImage(txt.data, txt.size().width, txt.size().height, txt.step[1], txt.step[0]);
    Boxa* boxes = ocr.GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
    for (int i = 0; i < boxes->n; i++) {
        BOX* box = boxaGetBox(boxes, i, L_CLONE);
        ocr.SetRectangle(box->x, box->y, box->w, box->h);
        const char* ocrResult = ocr.GetUTF8Text();
        if (*ocrResult != 0) {
            result.push_back(TextBox {
                Rect(Point(box->x, box->y), Size(box->w, box->h)),
                shared_ptr<const char>(ocrResult) });
        } else {
            delete[] ocrResult;
        }
    }
    boxaDestroy(&boxes);

    return result;
}

Mat displayText(const Mat& bg, const vector<TextBox>& textBoxes) {
    Mat display;
    cvtColor(bg, display, CV_GRAY2BGR);
    for (auto& text : textBoxes) {
        Scalar color = Scalar(rand() % 255, rand() % 100 + 155, rand() % 255);
        rectangle(display, text.boundary.tl(), text.boundary.br(), color);
        putText(display, text.text.get(), text.boundary.br() + Point(1,1), FONT_HERSHEY_SIMPLEX, 0.5, Scalar::all(0), 1);
        putText(display, text.text.get(), text.boundary.br(), FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
    }
    return display;
}
