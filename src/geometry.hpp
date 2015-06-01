#ifndef GEOMETRY_H
#define GEOMETRY_H 1

#include <cmath>
#include <opencv2/core/core.hpp>

// Pi is the False Constant. I believe in Tau.
static const double TAU = M_PI * 2;

template <class T>
cv::Vec<T,2> dirOf(const cv::Vec<T,4>& line) {
    return cv::Vec<T,2>(line[2] - line[0], line[3] - line[1]);
}

template<class T>
double closestApproach(cv::Vec<T,4> v1, cv::Vec<T,4> v2) {
    // source courtesy of http://geomalgorithms.com/a07-_distance.html

    using cv::Vec;
    const double SMALL_NUM = 0.0001;

    Vec<T,2> u = dirOf(v1);
    Vec<T,2> v = dirOf(v2);
    Vec<T,2> w(v1[0] - v2[0], v1[1] - v2[1]);
    // Vector   u = S1.P1 - S1.P0;
    // Vector   v = S2.P1 - S2.P0;
    // Vector   w = S1.P0 - S2.P0;
    double   a = u.ddot(u);        // always >= 0
    double   b = u.ddot(v);
    double   c = v.ddot(v);        // always >= 0
    double   d = u.ddot(w);
    double   e = v.ddot(w);
    double   D = a*c - b*b;        // always >= 0
    double   sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
    double   tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

    // compute the line parameters of the two closest points
    if (D < SMALL_NUM) { // the lines are almost parallel
        sN = 0.0;         // force using point P0 on segment S1
        sD = 1.0;         // to prevent possible division by 0.0 later
        tN = e;
        tD = c;
    }
    else {                 // get the closest points on the infinite lines
        sN = (b*e - c*d);
        tN = (a*e - b*d);
        if (sN < 0.0) {        // sc < 0 => the s=0 edge is visible
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else if (sN > sD) {  // sc > 1  => the s=1 edge is visible
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0) {            // tc < 0 => the t=0 edge is visible
        tN = 0.0;
        // recompute sc for this edge
        if (-d < 0.0)
            sN = 0.0;
        else if (-d > a)
            sN = sD;
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {      // tc > 1  => the t=1 edge is visible
        tN = tD;
        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sN = 0;
        else if ((-d + b) > a)
            sN = sD;
        else {
            sN = (-d +  b);
            sD = a;
        }
    }
    // finally do the division to get sc and tc
    sc = (std::abs(sN) < SMALL_NUM ? 0.0 : sN / sD);
    tc = (std::abs(tN) < SMALL_NUM ? 0.0 : tN / tD);

    // get the difference of the two closest points
    cv::Vec2d dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

    return norm(dP);   // return the closest distance
}

template<class T>
double closestApproach(cv::Vec<T,2> v1, cv::Vec<T,4> v2) {
    return closestApproach(cv::Vec<T,4>(v1[0], v1[1], v1[0], v1[1]), v2);
}

/** in range [-TAU/2, TAU/2] */
template <class T>
double angleOf(const cv::Vec<T,4>& v) {
    return atan2(v[3] - v[1], v[2] - v[0]);
}

/** in range [-TAU/2, TAU/2] */
float anglediff(float a1, float a2);

template <class T>
double segmentLength(const cv::Vec<T,4>& lineSegment) {
    double dx = lineSegment[0] - lineSegment[2];
    double dy = lineSegment[1] - lineSegment[3];
    return sqrt(dx*dx + dy*dy);
}

template <class T>
double distance(const cv::Vec<T,2> p1, const cv::Vec<T,2> p2) {
    return norm(p1 - p2);
}

template <class T>
cv::Vec<T,2> p1(const cv::Vec<T,4> line) {
    return cv::Vec<T,2>(line[0], line[1]);
}

template <class T>
cv::Vec<T,2> p2(const cv::Vec<T,4> line) {
    return cv::Vec<T,2>(line[2], line[3]);
}

template <class T>
cv::Vec<T,2> midpoint(const cv::Vec<T,4> line) {
    return cv::Vec<T,2>(
        (line[2] + line[0]) / 2,
        (line[3] + line[1]) / 2);
}

template <class T>
cv::Vec<T,4> clipLineTop(cv::Vec<T,4> line, T top) {
    if (line[1] < top && line[3] < top) { return cv::Vec<T,4>(0,0,0,0); }
    if (line[1] >= top && line[3] >= top) { return line; }
    if (line[1] > line[3]) {
        std::swap(line[0], line[2]);
        std::swap(line[1], line[3]);
    }
    const T dx = line[2] - line[0];
    return cv::Vec<T,4>(line[0] + dx * ((double)(line[1] - top) / (line[3] - line[1])), top, line[2], line[3]);
}

template <class T>
cv::Vec<T,4> clipLineBot(cv::Vec<T,4> line, T bot) {
    if (line[1] > bot && line[3] > bot) { return cv::Vec<T,4>(0,0,0,0); }
    if (line[1] <= bot && line[3] <= bot) { return line; }
    if (line[1] > line[3]) {
        std::swap(line[0], line[2]);
        std::swap(line[1], line[3]);
    }
    const T dx = line[2] - line[0];
    return cv::Vec<T,4>(line[0], line[1], line[0] + dx * ((double)(bot - line[1]) / (line[3] - line[1])), bot);
}

template <class T>
cv::Vec<T,4> clipLineLeft(cv::Vec<T,4> line, T left) {
    if (line[0] < left && line[2] < left) { return cv::Vec<T,4>(0,0,0,0); }
    if (line[0] >= left && line[2] >= left) { return line; }
    if (line[0] > line[2]) {
        std::swap(line[0], line[2]);
        std::swap(line[1], line[3]);
    }
    const T dy = line[3] - line[1];
    return cv::Vec<T,4>(left, line[1] + dy * ((double)(line[2] - left) / (line[2] - line[0])), line[2], line[3]);
}

template <class T>
cv::Vec<T,4> clipLineRight(cv::Vec<T,4> line, T right) {
    if (line[0] > right && line[2] > right) { return cv::Vec<T,4>(0,0,0,0); }
    if (line[0] <= right && line[2] <= right) { return line; }
    if (line[0] > line[2]) {
        std::swap(line[0], line[2]);
        std::swap(line[1], line[3]);
    }
    const T dy = line[3] - line[1];
    return cv::Vec<T,4>(line[0], line[1], right, line[1] + dy * ((double)(right - line[0]) / (line[2] - line[0])));
}

/** compute the subsegment that lies in the given rectangle */
cv::Vec4i segmentOverlapWithRect(cv::Vec4i segment, const cv::Rect& rect);

template <class T>
bool mostlyHorizontal(const cv::Vec<T,4>& v) {
    return std::abs(cos(angleOf(v))) > std::abs(cos(45));
}

template <class T>
bool mostlyVertical(const cv::Vec<T,4>& v) {
    return !mostlyHorizontal(v);
}

template <class T>
cv::Vec<T,4> orientLR(cv::Vec<T,4> l) {
    if (l[0] > l[2]) {
        std::swap(l[0], l[2]);
        std::swap(l[1], l[3]);
    }
    return l;
}

template <class T>
cv::Vec<T,4> orientTB(cv::Vec<T,4> l) {
    if (l[1] > l[3]) {
        std::swap(l[0], l[2]);
        std::swap(l[1], l[3]);
    }
    return l;
}

// r2 contained in r1?
bool contains(const cv::Rect& r1, const cv::Rect& r2);

#endif
