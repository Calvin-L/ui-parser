#include "layout.hpp"
#include <cstdlib>

using namespace std;

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

Layout toLayout(
    const vector<LayoutObject*>& objects,
    const vector<Constraint>& constraints) {

    auto root = new Element;
    root->type = ELEMENT_ROOT;
    root->nextSibling = nullptr;
    root->data.rootData.children = nullptr;

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
    stream << l.value << ' ' << unitToStr(l.unit);
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
    stream << "<head><title>generated page</title></head>";
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
    stream << "<div style=\"";
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
