#include "constraints.hpp"
#include "geometry.hpp"

using namespace std;
using namespace cv;

static const Length ZERO { UNIT_PX, 0.0 };

static bool contains(LayoutObject* o1, LayoutObject* o2) {
    auto& box1 = o1->data.boxData;
    auto& box2 = o2->data.boxData;
    return contains(
        Rect(Point(box1[0], box1[1]), Size(box1[2], box1[3])),
        Rect(Point(box2[0], box2[1]), Size(box2[2], box2[3])));
}

static void findContainmentConstraints(const vector<LayoutObject*>& objects, vector<Constraint>& dst) {

    for (auto o1 : objects) {
        for (auto o2 : objects) {
            if (o1->type == LAYOUT_BOX && o2->type == LAYOUT_BOX && contains(o1, o2)) {
                dst.push_back(Constraint { CONSTRAINT_CONTAINS, o1, o2, ZERO });
                dst.push_back(Constraint { CONSTRAINT_WIDTH, o2, nullptr, Length { UNIT_PERCENT, o2->data.boxData[2] * 100.0 / o1->data.boxData[2] }});
                dst.push_back(Constraint { CONSTRAINT_PAD_LEFT, o1, o2, Length { UNIT_PERCENT, (o2->data.boxData[0] - o1->data.boxData[0]) * 100.0 / o1->data.boxData[2] }});
                dst.push_back(Constraint { CONSTRAINT_PAD_TOP, o1, o2, Length { UNIT_PX, static_cast<double>(o2->data.boxData[1] - o1->data.boxData[1]) }});
            }
        }
    }

}

vector<Constraint> formConstraints(const vector<LayoutObject*>& objects) {
    vector<Constraint> result;

    findContainmentConstraints(objects, result);

    for (auto o : objects) {
        result.push_back(Constraint { CONSTRAINT_HEIGHT, o, nullptr, Length { UNIT_PX, static_cast<double>(o->data.boxData[3]) }});
    }

    return result;
}
