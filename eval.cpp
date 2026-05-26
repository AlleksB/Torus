#include "eval.h"
#include "movegen.h"

const Score Evaluator::PIECE_SCORE[13] = {
    {   0,   0},  // EMPTY
    {  82,  94},  // wP
    { 337, 281},  // wN
    { 365, 297},  // wB
    { 477, 512},  // wR
    {1025, 936},  // wQ
    {   0,   0},  // wK
    {  82,  94},  // bP
    { 337, 281},  // bN
    { 365, 297},  // bB
    { 477, 512},  // bR
    {1025, 936},  // bQ
    {   0,   0},  // bK
};

const int PIECE_VALUES[13] = {
    0,  82, 337, 365, 477, 1025, 0,
        82, 337, 365, 477, 1025, 0
};

const int Evaluator::PAWN_MG[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    98,134, 61, 95, 68,126, 34,-11,
    -6,  7, 26, 31, 65, 56, 25,-20,
   -14, 13,  6, 21, 23, 12, 17,-23,
   -27, -2, -5, 12, 17,  6, 10,-25,
   -26, -4, -4,-10,  3,  3, 33,-12,
   -35, -1,-20,-23,-15, 24, 38,-22,
     0,  0,  0,  0,  0,  0,  0,  0,
};
const int Evaluator::PAWN_EG[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
   178,173,158,134,147,132,165,187,
    94,100, 85, 67, 56, 53, 82, 84,
    32, 24, 13,  5, -2,  4, 17, 17,
    13,  9, -3, -7, -7, -8,  3, -1,
     4,  7, -6,  1,  0, -5, -1, -8,
    13,  8,  8, 10, 13,  0,  2, -7,
     0,  0,  0,  0,  0,  0,  0,  0,
};

const int Evaluator::KNIGHT_MG[64] = {
   -167,-89,-34,-49, 61,-97,-15,-107,
    -73,-41, 72, 36, 23, 62,  7, -17,
    -47, 60, 37, 65, 84,129, 73,  44,
     -9, 17, 19, 53, 37, 69, 18,  22,
    -13,  4, 16, 13, 28, 19, 21,  -8,
    -23, -9, 12, 10, 19, 17, 25, -16,
    -29,-53,-12, -3, -1, 18,-14, -19,
   -105,-21,-58,-33,-17,-28,-19, -23,
};
const int Evaluator::KNIGHT_EG[64] = {
    -58,-38,-13,-28,-31,-27,-63,-99,
    -25, -8,-25, -2, -9,-25,-24,-52,
    -24,-20, 10,  9, -1, -9,-19,-41,
    -17,  3, 22, 22, 22, 11,  8,-18,
    -18, -6, 16, 25, 16, 17,  4,-18,
    -23, -3, -1, 15, 10, -3,-20,-22,
    -42,-20,-10, -5, -2,-20,-23,-44,
    -29,-51,-23,-15,-22,-18,-50,-64,
};

const int Evaluator::BISHOP_MG[64] = {
    -29,  4,-82,-37,-25,-42,  7, -8,
    -26, 16,-18,-13, 30, 59, 18,-47,
    -16, 37, 43, 40, 35, 50, 37, -2,
     -4,  5, 19, 50, 37, 37,  7, -2,
     -6, 13, 13, 26, 34, 12, 10,  4,
      0, 15, 15, 15, 14, 27, 18, 10,
      4, 15, 16,  0,  7, 21, 33,  1,
    -33, -3,-14,-21,-13,-12,-39,-21,
};
const int Evaluator::BISHOP_EG[64] = {
    -14,-21,-11, -8, -7, -9,-17,-24,
     -8, -4,  7,-12, -3,-13, -4,-14,
      2, -8,  0, -1, -2,  6,  0,  4,
     -3,  9, 12,  9, 14, 10,  3,  2,
     -6,  3, 13, 19,  7, 10, -3, -9,
    -12, -3,  8, 10, 13,  3, -7,-15,
    -14,-18, -7, -1,  4, -9,-15,-27,
    -23, -9,-23, -5, -9,-16, -5,-17,
};

const int Evaluator::ROOK_MG[64] = {
     32, 42, 32, 51, 63,  9, 31, 43,
     27, 32, 58, 62, 80, 67, 26, 44,
     -5, 19, 26, 36, 17, 45, 61, 16,
    -24,-11,  7, 26, 24, 35, -8,-20,
    -36,-26,-12, -1,  9, -7,  6,-23,
    -45,-25,-16,-17,  3,  0, -5,-33,
    -44,-16,-20, -9, -1, 11, -6,-71,
    -19,-13,  1, 17, 16,  7,-37,-26,
};
const int Evaluator::ROOK_EG[64] = {
     13, 10, 18, 15, 12, 12,  8,  5,
     11, 13, 13, 11, -3,  3,  8,  3,
      7,  7,  7,  5,  4, -3, -5, -3,
      4,  3, 13,  1,  2,  1, -1,  2,
      3,  5,  8,  4, -5, -6, -8,-11,
     -4,  0, -5, -1, -7,-12, -8,-16,
     -6, -6,  0,  2, -9, -9,-11, -3,
     -9,  2,  3, -1, -5,-13,  4,-20,
};

const int Evaluator::QUEEN_MG[64] = {
    -28,  0, 29, 12, 59, 44, 43, 45,
    -24,-39, -5,  1,-16, 57, 28, 54,
    -13,-17,  7,  8, 29, 56, 47, 57,
    -27,-27,-16,-16, -1, 17, -2,  1,
     -9,-26, -9,-10, -2, -4,  3, -3,
    -14,  2,-11, -2, -5,  2, 14,  5,
    -35, -8, 11,  2,  8, 15, -3,  1,
     -1,-18, -9, 10,-15,-25,-31,-50,
};
const int Evaluator::QUEEN_EG[64] = {
     -9, 22, 22, 27, 27, 19, 10, 20,
    -17, 20, 32, 41, 58, 25, 30,  0,
    -20,  6,  9, 49, 47, 35, 19,  9,
      3, 22, 24, 45, 57, 40, 57, 36,
    -18, 28, 19, 47, 31, 34, 39, 23,
    -16,-27, 15,  6,  9, 17, 10,  5,
    -22,-23,-30,-16,-16,-23,-36,-32,
    -33,-28,-22,-43, -5,-32,-20,-41,
};

const int Evaluator::KING_MG[64] = {
    -65, 23, 16,-15,-56,-34,  2, 13,
     29, -1,-20, -7, -8, -4,-38,-29,
     -9, 24,  2,-16,-20,  6, 22,-22,
    -17,-20,-12,-27,-30,-25,-14,-36,
    -49, -1,-27,-39,-46,-44,-33,-51,
    -14,-14,-22,-46,-44,-30,-15,-27,
      1,  7, -8,-64,-43,-16,  9,  8,
    -15, 36, 12,-54,  8,-28, 24, 14,
};
const int Evaluator::KING_EG[64] = {
    -74,-35,-18,-18,-11, 15,  4,-17,
    -12, 17, 14, 17, 17, 38, 23, 11,
     10, 17, 23, 15, 20, 45, 44, 13,
     -8, 22, 24, 27, 26, 33, 26,  3,
    -18, -4, 21, 24, 27, 23,  9,-11,
    -19, -3, 11, 21, 23, 16,  7, -9,
    -27,-11,  4, 13, 14,  4, -5,-17,
    -53,-34,-21,-11,-28,-14,-24,-43,
};

const int Evaluator::DOUBLED_PAWN_PENALTY_MG  = -11;
const int Evaluator::DOUBLED_PAWN_PENALTY_EG  = -51;
const int Evaluator::ISOLATED_PAWN_PENALTY_MG = -15;
const int Evaluator::ISOLATED_PAWN_PENALTY_EG = -19;

const int Evaluator::PASSED_PAWN_BONUS_MG[8] = {  0, 10, 17, 15, 62, 168, 276, 0 };
const int Evaluator::PASSED_PAWN_BONUS_EG[8] = {  0, 28, 33, 41, 72, 177, 260, 0 };

const int Evaluator::BISHOP_PAIR_MG    = 22;
const int Evaluator::BISHOP_PAIR_EG    = 30;

const int Evaluator::ROOK_OPEN_FILE_MG = 23;
const int Evaluator::ROOK_OPEN_FILE_EG = 10;
const int Evaluator::ROOK_SEMI_OPEN_MG = 11;
const int Evaluator::ROOK_SEMI_OPEN_EG =  5;
const int Evaluator::ROOK_ON_7TH_MG    = 20;
const int Evaluator::ROOK_ON_7TH_EG    = 40;

const int Evaluator::KNIGHT_MOB_MG =  4;
const int Evaluator::KNIGHT_MOB_EG =  4;
const int Evaluator::BISHOP_MOB_MG =  5;
const int Evaluator::BISHOP_MOB_EG =  5;
const int Evaluator::ROOK_MOB_MG   =  3;
const int Evaluator::ROOK_MOB_EG   =  5;
const int Evaluator::QUEEN_MOB_MG  =  1;
const int Evaluator::QUEEN_MOB_EG  =  2;

const int Evaluator::ATTACKER_WEIGHT[13] = {
    0,  0,  2,  2,  3,  5,  0,
    0,  2,  2,  3,  5,  0
};

const int Evaluator::SAFETY_TABLE[100] = {
      0,   0,   1,   2,   3,   5,   7,   9,  12,  15,
     18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
     68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
    140, 150, 169, 180, 191, 202, 213, 225, 237, 250,
    263, 276, 290, 304, 319, 334, 350, 366, 383, 401,
    420, 440, 461, 483, 506, 530, 555, 581, 608, 637,
    666, 697, 729, 762, 796, 831, 867, 904, 943, 983,
    983, 983, 983, 983, 983, 983, 983, 983, 983, 983,
    983, 983, 983, 983, 983, 983, 983, 983, 983, 983,
    983, 983, 983, 983, 983, 983, 983, 983, 983, 983,
};

int Evaluator::taperedPST(const int* mgT, const int* egT, int sq, int color, int phase) {
    int idx = (color == WHITE) ? ((7 - sq / 8) * 8 + sq % 8) : sq;
    return (mgT[idx] * phase + egT[idx] * (24 - phase)) / 24;
}

int Evaluator::taper(int mg, int eg, int phase) {
    return (mg * phase + eg * (24 - phase)) / 24;
}

int Evaluator::computePhase(const Board& board) {
    int phase = 0;
    phase += countBits(board.pieceBoards[wN] | board.pieceBoards[bN]) * 1;
    phase += countBits(board.pieceBoards[wB] | board.pieceBoards[bB]) * 1;
    phase += countBits(board.pieceBoards[wR] | board.pieceBoards[bR]) * 2;
    phase += countBits(board.pieceBoards[wQ] | board.pieceBoards[bQ]) * 4;
    return std::min(phase, 24);
}

bool Evaluator::isDoubled(U64 ours, int sq, int /*side*/) {
    int file = sq % 8;
    U64 filePawns = ours & fileMasks[file];
    return countBits(filePawns) > 1;
}

bool Evaluator::isIsolated(U64 ours, int sq) {
    int file = sq % 8;
    U64 adjacentFiles = 0;
    if (file > 0) adjacentFiles |= fileMasks[file - 1];
    if (file < 7) adjacentFiles |= fileMasks[file + 1];
    return !(ours & adjacentFiles);
}

bool Evaluator::isPassed(U64 /*ours*/, U64 theirs, int sq, int side) {
    int rank = sq / 8;
    int file = sq % 8;

    U64 frontSpan = 0;
    if (side == WHITE) {
        for (int r = rank + 1; r < 8; r++) {
            if (file > 0) frontSpan |= (1ULL << (r * 8 + file - 1));
            frontSpan |= (1ULL << (r * 8 + file));
            if (file < 7) frontSpan |= (1ULL << (r * 8 + file + 1));
        }
    } else {
        for (int r = rank - 1; r >= 0; r--) {
            if (file > 0) frontSpan |= (1ULL << (r * 8 + file - 1));
            frontSpan |= (1ULL << (r * 8 + file));
            if (file < 7) frontSpan |= (1ULL << (r * 8 + file + 1));
        }
    }
    return !(theirs & frontSpan);
}

Score Evaluator::evalSide(const Board& board, int side, int& kingAttackScore) const {
    Score score;
    const int phase   = computePhase(board);
    const int them    = side ^ 1;
    const U64 occ     = board.colorBoards[BOTH];

    const U64 ourPawns   = board.pieceBoards[side == WHITE ? wP : bP];
    const U64 theirPawns = board.pieceBoards[side == WHITE ? bP : wP];

    int theirKingSq = getLSB(board.pieceBoards[them == WHITE ? wK : bK]);
    U64 kingZone    = kingAttacks[theirKingSq] | (1ULL << theirKingSq);

    // PAWNS
    {
        U64 pawns = ourPawns;
        while (pawns) {
            int sq = pullLsb(pawns);
            int rank = (side == WHITE) ? sq / 8 : 7 - sq / 8;


            score.mg += PIECE_SCORE[wP].mg + taperedPST(PAWN_MG, PAWN_EG, sq, side, phase);
            score.eg += PIECE_SCORE[wP].eg + taperedPST(PAWN_MG, PAWN_EG, sq, side, phase);

            if (isDoubled(ourPawns, sq, side)) {
                score.mg += DOUBLED_PAWN_PENALTY_MG;
                score.eg += DOUBLED_PAWN_PENALTY_EG;
            }
            if (isIsolated(ourPawns, sq)) {
                score.mg += ISOLATED_PAWN_PENALTY_MG;
                score.eg += ISOLATED_PAWN_PENALTY_EG;
            }
            if (isPassed(ourPawns, theirPawns, sq, side)) {
                score.mg += PASSED_PAWN_BONUS_MG[rank];
                score.eg += PASSED_PAWN_BONUS_EG[rank];
            }
        }
    }

    // KNIGHTS
    {
        U64 pieces = board.pieceBoards[side == WHITE ? wN : bN];
        while (pieces) {
            int sq  = pullLsb(pieces);
            U64 atk = knightAttacks[sq] & ~board.colorBoards[side];

            score.mg += PIECE_SCORE[wN].mg + taperedPST(KNIGHT_MG, KNIGHT_EG, sq, side, phase);
            score.eg += PIECE_SCORE[wN].eg + taperedPST(KNIGHT_MG, KNIGHT_EG, sq, side, phase);

            int mob = countBits(atk);
            score.mg += mob * KNIGHT_MOB_MG;
            score.eg += mob * KNIGHT_MOB_EG;

            if (atk & kingZone)
                kingAttackScore += ATTACKER_WEIGHT[wN];
        }
    }

    // BISHOPS
    {
        U64 pieces = board.pieceBoards[side == WHITE ? wB : bB];
        int bishopCount = 0;
        while (pieces) {
            int sq  = pullLsb(pieces);
            U64 atk = getBishopAttacks(sq, occ) & ~board.colorBoards[side];
            bishopCount++;

            score.mg += PIECE_SCORE[wB].mg + taperedPST(BISHOP_MG, BISHOP_EG, sq, side, phase);
            score.eg += PIECE_SCORE[wB].eg + taperedPST(BISHOP_MG, BISHOP_EG, sq, side, phase);

            int mob = countBits(atk);
            score.mg += mob * BISHOP_MOB_MG;
            score.eg += mob * BISHOP_MOB_EG;

            if (atk & kingZone)
                kingAttackScore += ATTACKER_WEIGHT[wB];
        }
        if (bishopCount >= 2) {
            score.mg += BISHOP_PAIR_MG;
            score.eg += BISHOP_PAIR_EG;
        }
    }

    // ROOKS
    {
        U64 pieces = board.pieceBoards[side == WHITE ? wR : bR];
        while (pieces) {
            int sq   = pullLsb(pieces);
            int file = sq % 8;
            int rank = sq / 8;
            U64 atk  = getRookAttacks(sq, occ) & ~board.colorBoards[side];

            score.mg += PIECE_SCORE[wR].mg + taperedPST(ROOK_MG, ROOK_EG, sq, side, phase);
            score.eg += PIECE_SCORE[wR].eg + taperedPST(ROOK_MG, ROOK_EG, sq, side, phase);

            int mob = countBits(atk);
            score.mg += mob * ROOK_MOB_MG;
            score.eg += mob * ROOK_MOB_EG;

            bool noOurPawn    = !(ourPawns   & fileMasks[file]);
            bool noTheirPawn  = !(theirPawns & fileMasks[file]);
            if (noOurPawn && noTheirPawn) {
                score.mg += ROOK_OPEN_FILE_MG;
                score.eg += ROOK_OPEN_FILE_EG;
            } else if (noOurPawn) {
                score.mg += ROOK_SEMI_OPEN_MG;
                score.eg += ROOK_SEMI_OPEN_EG;
            }

            int seventhRank = (side == WHITE) ? 6 : 1;
            if (rank == seventhRank) {
                score.mg += ROOK_ON_7TH_MG;
                score.eg += ROOK_ON_7TH_EG;
            }

            if (atk & kingZone)
                kingAttackScore += ATTACKER_WEIGHT[wR];
        }
    }

    // QUEENS
    {
        U64 pieces = board.pieceBoards[side == WHITE ? wQ : bQ];
        while (pieces) {
            int sq  = pullLsb(pieces);
            U64 atk = getQueenAttacks(sq, occ) & ~board.colorBoards[side];

            score.mg += PIECE_SCORE[wQ].mg + taperedPST(QUEEN_MG, QUEEN_EG, sq, side, phase);
            score.eg += PIECE_SCORE[wQ].eg + taperedPST(QUEEN_MG, QUEEN_EG, sq, side, phase);

            int mob = countBits(atk);
            score.mg += mob * QUEEN_MOB_MG;
            score.eg += mob * QUEEN_MOB_EG;

            if (atk & kingZone)
                kingAttackScore += ATTACKER_WEIGHT[wQ];
        }
    }

    // KING
    {
        int sq = getLSB(board.pieceBoards[side == WHITE ? wK : bK]);
        score.mg += taperedPST(KING_MG, KING_EG, sq, side, phase);
        score.eg += taperedPST(KING_MG, KING_EG, sq, side, phase);
    }

    return score;
}

int Evaluator::evaluate(const Board& board) const {
    int phase = computePhase(board);

    int whiteAttackScore = 0, blackAttackScore = 0;

    Score white = evalSide(board, WHITE, blackAttackScore);
    Score black = evalSide(board, BLACK, whiteAttackScore);

    Score total = white - black;

    int whiteSafety = SAFETY_TABLE[std::min(whiteAttackScore, 99)];
    int blackSafety = SAFETY_TABLE[std::min(blackAttackScore, 99)];

    total.mg -= whiteSafety;
    total.mg += blackSafety;

    int score = taper(total.mg, total.eg, phase);

    return (board.sideToMove == WHITE) ? score : -score;
}
