#ifndef VOTING_H
#define VOTING_H 1

#include <vector>
#include <opencv2/core/core.hpp>

#include "strokes.hpp"
#include "ocr.hpp"

enum VoteType {
    BOX_TOP, BOX_LEFT, BOX_RIGHT, BOX_BOTTOM, MEASUREMENT_LINE, TEXT
};

struct Vote {
    VoteType type;
    TextBox label; // when type == MEASUREMENT_LINE or TEXT
};

struct VotedStroke {
    Stroke stroke;
    std::vector<Vote> votes;
};

bool operator==(const VotedStroke& v1, const VotedStroke& v2);

std::vector<VotedStroke> placeVotes(
    const std::vector<Stroke>& strokes,
    const std::vector<TextBox>& ocr);

const Vote* bestVote(const VotedStroke& stroke);

cv::Mat displayVotes(const cv::Mat& bg, const std::vector<VotedStroke>& votes);

#endif
