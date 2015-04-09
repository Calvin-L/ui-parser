
#include <iostream>

#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>
#include <leptonica/allheaders.h>

using namespace std;

int main(int argc, char** argv) {

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <file>" << endl;
        return 1;
    }

    const char* file = argv[1];

    tesseract::TessBaseAPI ocr;

    if (ocr.Init(NULL, "eng")) {
        cerr << "Could not initialize tesseract." << endl;
        return 1;
    }

    Pix* image = pixRead(file);

    ocr.SetImage(image);

#if 1
    ocr.SetRectangle(136, 156, 130, 92);
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

#if 0
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
                word, conf, x1, y1, x2, y2);
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
    }
#endif

    ocr.End();
    pixDestroy(&image);

    return 0;

}
