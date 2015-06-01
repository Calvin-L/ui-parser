#include <iostream>
#include <cstring>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "segments.hpp"
#include "strokes.hpp"
#include "ocr.hpp"
#include "voting.hpp"
#include "constraints.hpp"
#include "layout.hpp"

using namespace std;
using namespace cv;

static void show(const char* title, const Mat& m) {
    Mat scaled;
    resize(m, scaled, Size(m.size().width/2, m.size().height/2));
    imshow(title, scaled);
}

static int usage(char** argv) {
    cerr << "Usage: " << argv[0] << " [--no-debug] <file>" << endl;
    return 1;
}

int main(int argc, char** argv) {

    if (argc < 2 || argc > 3) {
        return usage(argv);
    }

    bool interactive = true;
    if (argc == 3) {
        if (strcmp(argv[1], "--no-debug") == 0) {
            interactive = false;
        } else {
            return usage(argv);
        }
    }

    const char* file = argv[argc-1];

    Mat input = imread(file, CV_LOAD_IMAGE_GRAYSCALE);

    if (input.size().width <= 0 || input.size().height <= 0) {
        cerr << "failed to read image '" << file << '\'' << endl;
        return 1;
    }

    auto segments    = findSegments(input);
    auto strokes     = findStrokes(segments);
    auto ocr         = findText(input);
    auto votes       = placeVotes(strokes, ocr);
    auto objects     = explain(votes);
    auto constraints = formConstraints(objects);
    auto layout      = toLayout(objects, constraints);

    cout << layout << endl;

    if (interactive) {
        show("input",    input);
        show("segments", displaySegments(input, segments));
        show("strokes",  displayStrokes(input, strokes));
        show("text",     displayText(input, ocr));
        show("votes",    displayVotes(input, votes));
        show("objects",  displayObjects(input, objects));
        while (waitKey(0) != 'q') { }
    }

    return 0;
}
