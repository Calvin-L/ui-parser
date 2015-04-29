#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include "segments.hpp"
#include "strokes.hpp"
#include "ocr.hpp"
#include "voting.hpp"
#include "constraints.hpp"
#include "layout.hpp"

using namespace std;
using namespace cv;

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

    auto segments    = findSegments(input);
    auto strokes     = findStrokes(segments);
    auto ocr         = findText(input);
    auto votes       = placeVotes(strokes, ocr);
    auto objects     = explain(votes);
    auto constraints = formConstraints(objects);
    auto layout      = toLayout(objects, constraints);

    imshow("input",    input);
    imshow("segments", displaySegments(input, segments));
    imshow("strokes",  displayStrokes(input, strokes));
    imshow("text",     displayText(input, ocr));

    cout << layout << endl;

    waitKey(0);
    return 0;

}
