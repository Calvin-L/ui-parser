#include "ocr.hpp"
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <baseapi.h>     // Tesseract
#include <allheaders.h>  // Leptonica
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

void dilate(Mat& img, int size) {
    // see http://docs.opencv.org/doc/tutorials/imgproc/erosion_dilatation/erosion_dilatation.html
    Mat element = getStructuringElement(
        MORPH_RECT,
        Size(2*size + 1, 2*size+1),
        Point(size, size));
    erode(img, img, element);
}

vector<TextBox> findText(const Mat& img) {
    const double UPSCALE = 3.0;

    Mat pp; // preprocessed
    resize(img, pp, Size(0, 0), UPSCALE, UPSCALE, INTER_CUBIC);
    dilate(pp, 1);
    threshold(pp, pp, 230, 255, CV_THRESH_BINARY);

    tesseract::TessBaseAPI ocr;

    if (ocr.Init(NULL, "eng" /*, tesseract::OEM_TESSERACT_CUBE_COMBINED */)) {
        cerr << "Could not initialize tesseract." << endl;
        exit(1);
    }

    // Mat scldown;
    // resize(pp, scldown, Size(0, 0), .25, .25, INTER_CUBIC);
    // imshow("text preprocessing", scldown);

    vector<TextBox> result;

    ocr.SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
    // ocr.SetVariable("tessedit_char_whitelist", "0123456789px% .-");
    ocr.SetImage(pp.data, pp.size().width, pp.size().height, pp.step[1], pp.step[0]);
    Boxa* boxes = ocr.GetComponentImages(tesseract::RIL_WORD, true, NULL, NULL);
    if (boxes == nullptr) {
        return result;
    }

    for (int i = 0; i < boxes->n; i++) {
        BOX* box = boxaGetBox(boxes, i, L_CLONE);
        ocr.SetRectangle(box->x, box->y, box->w, box->h);
        const char* ocrResult = ocr.GetUTF8Text();
        if (ocrResult == nullptr) {
            continue;
        }
        int conf = ocr.MeanTextConf();
        if (*ocrResult != 0 && conf > 0) {
            int x = round(box->x/UPSCALE);
            int y = round(box->y/UPSCALE);
            int w = round(box->w/UPSCALE);
            int h = round(box->h/UPSCALE);
            cerr << "text box @" << x << ',' << y << ',' << w << ',' << h << "; conf=" << conf << "; text='" << ocrResult << '\'' << endl;
            result.push_back(TextBox {
                Rect(Point(x, y), Size(w, h)),
                ocrResult });
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
        putText(display, text.text, text.boundary.br() + Point(1,1), FONT_HERSHEY_SIMPLEX, 0.5, Scalar::all(0), 1);
        putText(display, text.text, text.boundary.br(), FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
    }
    return display;
}
