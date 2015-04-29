#ifndef VOTING_H
#define VOTING_H 1

#include <vector>

#include "strokes.hpp"
#include "ocr.hpp"

enum VoteType {
    BOX_TOP, BOX_LEFT, BOX_RIGHT, BOX_BOTTOM, MEASUREMENT_LINE, TEXT
};

struct Vote {
    VoteType type;
    TextBox label;
};

struct VotedStroke : public Stroke {
    std::vector<Vote> votes;
};

std::vector<VotedStroke> placeVotes(
    const std::vector<Stroke>& strokes,
    const std::vector<TextBox>& ocr);

#endif
