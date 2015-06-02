#include "layout.hpp"

#include <cstdlib>
#include <iostream>
#include <algorithm>
// #include <z3++.h>

#include "UnionFind.hpp"
#include "printing.hpp"

using namespace std;
// using namespace z3;

enum ElementType {
    ELEMENT_ROOT,
    ELEMENT_TEXT,
    ELEMENT_BOX,
    ELEMENT_IMAGE,
};

enum TextFormat {
    TEXT_TITLE,
    TEXT_PARAGRAPH,
};

struct Element {
    ElementType type;
    Element* nextSibling;
    union {
        struct {
            Element* children;
        } rootData;

        struct {
            const char* text;
            TextFormat format;
        } textData;

        struct {
            Length width;
            Length height;
        } imageData;

        struct {
            Length width;
            Length height;
            Length margin[4]; // top, right, bottom, left
            Element* children;
        } boxData;
    } data;
};

template <class T>
static int indexOf(const vector<T>& vec, const T& val) {
    auto it = find(vec.begin(), vec.end(), val);
    return (it == vec.end()) ? -1 : (it - vec.begin());
}

// o1 contains o2?
static bool forcedContainment(const LayoutObject* o1, const LayoutObject* o2,
    const vector<Constraint>& constraints) {

    for (auto& c : constraints) {
        if (c.type == CONSTRAINT_CONTAINS && o1 == c.obj1 && o2 == c.obj2) {
            return true;
        }
    }
    return false;
}

vector<vector<int>> findTrees(vector<int> g,
    const vector<LayoutObject*>& objects,
    const vector<Constraint>& constraints) {

    struct Related {
        const vector<LayoutObject*>& objects;
        const vector<Constraint>& constraints;

        bool operator()(int i1, int i2) {
            return forcedContainment(objects[i1], objects[i2], constraints) ||
                forcedContainment(objects[i2], objects[i1], constraints);
        }
    };

    return group(g, Related { objects, constraints });
}

// moves root to position 0
static void findRoot(vector<int>& g,
    const vector<LayoutObject*>& objects,
    const vector<Constraint>& constraints) {

    int rootIdx = 0;
    for (int i = 1; i < g.size(); ++i) {
        auto o1 = objects[g[rootIdx]];
        auto o2 = objects[g[i]];

        if (forcedContainment(o2, o1, constraints)) {
            cerr << "woo@" << i << endl;
            rootIdx = i;
        }
    }

    if (rootIdx != 0) {
        swap(g[0], g[rootIdx]);
    }
}

void applySizeConstraints(Element* e, const LayoutObject* o, const vector<Constraint>& constraints) {
    for (auto& c : constraints) {
        switch (c.type) {
            case CONSTRAINT_WIDTH:
                if (c.obj1 == o) { e->data.boxData.width = c.len; }
                break;
            case CONSTRAINT_HEIGHT:
                if (c.obj1 == o) { e->data.boxData.height = c.len; }
                break;
            default:
                break;
        }
    }
}

void applyMarginConstraints(const LayoutObject* parent, Element* e, const LayoutObject* o, const vector<Constraint>& constraints) {
    for (auto& c : constraints) {
        switch (c.type) {
            case CONSTRAINT_PAD_TOP:
                if (c.obj1 == parent && c.obj2 == o) { e->data.boxData.margin[0] = c.len; }
                break;
            case CONSTRAINT_PAD_RIGHT:
                if (c.obj1 == parent && c.obj2 == o) { e->data.boxData.margin[1] = c.len; }
                break;
            case CONSTRAINT_PAD_BOTTOM:
                if (c.obj1 == parent && c.obj2 == o) { e->data.boxData.margin[2] = c.len; }
                break;
            case CONSTRAINT_PAD_LEFT:
                if (c.obj1 == parent && c.obj2 == o) { e->data.boxData.margin[3] = c.len; }
                break;
            default:
                break;
        }
    }
}

Element* buildLayout(
    const vector<int>& g,
    const vector<LayoutObject*>& objects,
    const vector<Constraint>& constraints,
    const LayoutObject* parent = nullptr) {

    if (g.size() == 0) {
        return nullptr;
    }

    auto trees = findTrees(g, objects, constraints);
    cerr << trees << endl;

    Element* e = nullptr;
    for (auto& t : trees) {
        cerr << t << endl;
        findRoot(t, objects, constraints);
        cerr << t << endl;
        int rootIdx = t[0];
        const LayoutObject* rootObj = objects[rootIdx];

        cerr << "root is " << rootObj->data.boxData[0] << ", " << rootObj->data.boxData[1] << ", " << rootObj->data.boxData[2] << ", " << rootObj->data.boxData[3] << endl;

        Element* root = new Element;
        root->type = ELEMENT_BOX;
        auto& data = root->data.boxData;

        data.width = data.height = Length { UNIT_PX, 100 };
        applySizeConstraints(root, rootObj, constraints);

        data.margin[0] = data.margin[1] = data.margin[2] = data.margin[3] = Length { UNIT_PX, 0.0 };
        if (parent != nullptr) {
            applyMarginConstraints(parent, root, rootObj, constraints);
        }
        t.erase(t.begin());
        root->data.boxData.children = buildLayout(t, objects, constraints, rootObj);

        root->nextSibling = e;
        e = root;
    }
    return e;
}

int deleteElement(Element*& e) {
    if (e == nullptr) {
        return 0;
    }
    int n = deleteElement(e->nextSibling);
    switch (e->type) {
        case ELEMENT_ROOT:
            n += deleteElement(e->data.rootData.children);
            break;
        case ELEMENT_BOX:
            n += deleteElement(e->data.boxData.children);
            break;
        default:
            break;
    }
    delete e;
    e = nullptr;
    return n + 1;
}

Layout toLayout(
    const vector<LayoutObject*>& objects,
    const vector<Constraint>& constraints) {

    // context ctx;
    // solver s(ctx);

    // auto parent_of = ctx.function("parent_of", ctx.int_sort(), ctx.int_sort());

    // for (int i = 0, len = objects.size(); i < len; ++i) {
    //     s.add(parent_of(i) >= 0 && parent_of(i) < len);
    // }

    // for (auto& c : constraints) {
    //     int idx1, idx2;
    //     switch (c.type) {
    //         case CONSTRAINT_CONTAINS:
    //             idx1 = indexOf(objects, c.obj1);
    //             idx2 = indexOf(objects, c.obj2);
    //             s.add(parent_of(idx2) == idx1);
    //             break;
    //         case CONSTRAINT_VERTSPACE:
    //             break;
    //         case CONSTRAINT_HORIZSPACE:
    //             break;
    //         case CONSTRAINT_PAD_TOP:
    //             break;
    //         case CONSTRAINT_PAD_RIGHT:
    //             break;
    //         case CONSTRAINT_PAD_BOTTOM:
    //             break;
    //         case CONSTRAINT_PAD_LEFT:
    //             break;
    //         case CONSTRAINT_WIDTH:
    //             break;
    //         case CONSTRAINT_HEIGHT:
    //             break;
    //     }
    // }

    // switch (s.check()) {
    //     case unknown:
    //         cerr << "constraints unsolvable [UNKNOWN]" << endl;
    //         return Layout { nullptr };
    //     case unsat:
    //         cerr << "constraints unsolvable [UNSAT]" << endl;
    //         return Layout { nullptr };
    //     case sat:
    //         break;
    // }

    // auto model = s.get_model();

    // auto root = new Element;
    // root->type = ELEMENT_ROOT;
    // root->nextSibling = nullptr;
    // root->data.rootData.children = nullptr;

    // Element** mirror = new Element*[objects.size()];
    // fill(mirror, mirror + objects.size(), nullptr);

    vector<int> g;
    for (int i = 0; i < objects.size(); ++i) {
        if (objects[i]->type == LAYOUT_BOX) {
            g.push_back(i);
        }
    }
    auto root = buildLayout(g, objects, constraints);

    if (root == nullptr) {
        cerr << "no trees found!" << endl;
        return Layout { nullptr };
    } else if (root->nextSibling != nullptr) {
        int ndeleted = deleteElement(root->nextSibling);
        cerr << "multiple trees found (discarded " << ndeleted << ")!" << endl;
    }

    root->type = ELEMENT_ROOT;
    root->data.rootData.children = root->data.boxData.children;

    // int rootIdx = *(roots.begin());
    // Element* root = new Element

    // delete[] mirror;

    return Layout { root };
}

static inline const char* unitToStr(Unit u) {
    switch (u) {
        case UNIT_PX: return "px";
        case UNIT_PERCENT: return "%";
    }
    return "";
}

static inline void printLength(ostream& stream, const Length& l) {
    stream << l.value << unitToStr(l.unit);
}

static void printByte(ostream& stream, const int byte) {
    for (int i = 0; i < 1; ++i) {
        int x = (byte >> (i * 4)) & 0xF;
        if (x >= 0 && x < 10) {
            stream << (char)(x + '0');
        } else {
            stream << (char)(x - 10 + 'A');
        }
    }
}

static void printRandomColor(ostream& stream) {
    static_assert(RAND_MAX >= 100, "RAND_MAX is not big enough!");
    stream << '#';
    int r = rand() % 100 + 155;
    int g = rand() % 100 + 155;
    int b = rand() % 100 + 155;
    printByte(stream, r);
    printByte(stream, g);
    printByte(stream, b);
}

static void printElement(ostream& stream, const Element* e);

static void printRoot(ostream& stream, const Element* root) {
    stream << "<!DOCTYPE html>\n";
    stream << "<html>";
    stream << "<head>";
    stream << "<title>generated page</title>";
    stream << "<style> * { margin: 0; padding: 0; } </style>";
    stream << "</head>";
    stream << "<body>";
    const Element* child = root->data.rootData.children;
    while (child != nullptr) {
        printElement(stream, child);
        child = child->nextSibling;
    }
    stream << "</body>";
    stream << "</html>";
}

static void printBox(ostream& stream, const Element* box) {
    auto& data = box->data.boxData;
    stream << "<div style=\"position:absolute;";
    stream << "width:";            printLength(stream, data.width);     stream << ';';
    stream << "height:";           printLength(stream, data.height);    stream << ';';
    stream << "margin-top:";       printLength(stream, data.margin[0]); stream << ';';
    stream << "margin-right:";     printLength(stream, data.margin[1]); stream << ';';
    stream << "margin-bottom:";    printLength(stream, data.margin[2]); stream << ';';
    stream << "margin-left:";      printLength(stream, data.margin[3]); stream << ';';
    stream << "background-color:"; printRandomColor(stream);            stream << ';';
    stream << "\">";
    const Element* child = data.children;
    while (child != nullptr) {
        printElement(stream, child);
        child = child->nextSibling;
    }
    stream << "</div>";
}

static void printImage(ostream& stream, const Element* image) {
    auto& data = image->data.imageData;
    stream << "<img style=\"width:";
    printLength(stream, data.width);
    printLength(stream, data.height);
    stream << "\" src=\"...\">";
}

static void printText(ostream& stream, const Element* text) {
    auto& data = text->data.textData;
    switch (data.format) {
        case TEXT_TITLE:     stream << "<h1>" << data.text << "</h1>"; break;
        case TEXT_PARAGRAPH: stream << "<p>"  << data.text << "</p>";  break;
    }
}

static void printElement(ostream& stream, const Element* e) {
    if (e == nullptr) {
        return;
    }

    switch (e->type) {
        case ELEMENT_ROOT:  printRoot (stream, e); break;
        case ELEMENT_TEXT:  printText (stream, e); break;
        case ELEMENT_BOX:   printBox  (stream, e); break;
        case ELEMENT_IMAGE: printImage(stream, e); break;
    }
}

ostream& operator<<(ostream& stream, const Layout& layout) {
    printElement(stream, layout.root);
    return stream;
}
