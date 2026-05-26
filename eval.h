#pragma once

#include "board.h"

struct Score {
    int mg = 0;
    int eg = 0;

    Score& operator+=(const Score& o) { mg += o.mg; eg += o.eg; return *this; }
    Score& operator-=(const Score& o) { mg -= o.mg; eg -= o.eg; return *this; }
    Score  operator+ (const Score& o) const { return { mg + o.mg, eg + o.eg }; }
    Score  operator- (const Score& o) const { return { mg - o.mg, eg - o.eg }; }
    Score  operator-()                const { return { -mg, -eg }; }
};

class Evaluator {
public:
    int evaluate(const Board& board) const;

private:
    static const Score PIECE_SCORE[13];

    static const int PAWN_MG[64],   PAWN_EG[64];
    static const int KNIGHT_MG[64], KNIGHT_EG[64];
    static const int BISHOP_MG[64], BISHOP_EG[64];
    static const int ROOK_MG[64],   ROOK_EG[64];
    static const int QUEEN_MG[64],  QUEEN_EG[64];
    static const int KING_MG[64],   KING_EG[64];

    // Pawn structure
    static const int DOUBLED_PAWN_PENALTY_MG,  DOUBLED_PAWN_PENALTY_EG;
    static const int ISOLATED_PAWN_PENALTY_MG, ISOLATED_PAWN_PENALTY_EG;
    static const int PASSED_PAWN_BONUS_MG[8],  PASSED_PAWN_BONUS_EG[8];

    // Pieces
    static const int BISHOP_PAIR_MG,    BISHOP_PAIR_EG;
    static const int ROOK_OPEN_FILE_MG, ROOK_OPEN_FILE_EG;
    static const int ROOK_SEMI_OPEN_MG, ROOK_SEMI_OPEN_EG;
    static const int ROOK_ON_7TH_MG,    ROOK_ON_7TH_EG;

    // Mobility
    static const int KNIGHT_MOB_MG, KNIGHT_MOB_EG;
    static const int BISHOP_MOB_MG, BISHOP_MOB_EG;
    static const int ROOK_MOB_MG,   ROOK_MOB_EG;
    static const int QUEEN_MOB_MG,  QUEEN_MOB_EG;

    // King safety
    static const int ATTACKER_WEIGHT[13];
    static const int SAFETY_TABLE[100];

    Score evalSide(const Board& board, int side, int& kingAttackScore) const;

    static bool isDoubled (U64 ours,   int sq, int side);
    static bool isIsolated(U64 ours,   int sq);
    static bool isPassed  (U64 ours, U64 theirs, int sq, int side);

    static int  taperedPST(const int* mgTable, const int* egTable,
                           int sq, int color, int phase);

    static int  taper(int mg, int eg, int phase);

    static int  computePhase(const Board& board);
};
