#pragma once

#include <iostream>
#include <string>
#include <stdint.h>

typedef uint64_t U64;

enum Piece {
    EMPTY,
    wP, wN, wB, wR, wQ, wK,
    bP, bN, bB, bR, bQ, bK
};

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQ
};

enum Color { WHITE, BLACK, BOTH };

enum CastlingRight { CASTLE_K, CASTLE_Q, CASTLE_k, CASTLE_q };

enum FENState { PIECE_POSITIONS, TO_MOVE, CASTLING, EN_PASSANT, HALFMOVES, FULLMOVES };

enum MoveFlag {
    MOVE_QUIET, MOVE_DOUBLE_PUSH, MOVE_K_CASTLE, MOVE_Q_CASTLE,
    MOVE_CAPTURE, MOVE_EP_CAPTURE,
    MOVE_PROMO_N = 8, MOVE_PROMO_B, MOVE_PROMO_R, MOVE_PROMO_Q,
    MOVE_PROMOTION = 8
};

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

extern const int PIECE_VALUES[13];

struct MoveList {
    int moves[256];
    int count = 0;
};

struct UndoInfo {
    int move;
    int castlingRights;
    int enPassantSq;
    int fiftyMoveCounter;
    int capturedPiece;
    U64 zobristHash;
};

static inline int getLSB(U64 x) {
    return x ? __builtin_ctzll(x) : -1;
}

static inline int countBits(U64 x) {
    return __builtin_popcountll(x);
}

static inline void popBit(U64& x, int sq) {
    x &= ~(1ULL << sq);
}

static inline int pullLsb(U64& bitboard) {
    int sq = getLSB(bitboard);
    bitboard &= (bitboard - 1);
    return sq;
}

static inline int encodeMove(int from, int to, int flags) {
    return from | (to << 6) | (flags << 12);
}

static inline int getMoveSource(int move) { return move & 63; }
static inline int getMoveTarget(int move) { return (move >> 6) & 63; }
static inline int getMoveFlags(int move)  { return (move >> 12) & 15; }

extern U64 fileMasks[8];
extern U64 rankMasks[8];

#define FILE_A fileMasks[0]
#define FILE_B fileMasks[1]
#define FILE_C fileMasks[2]
#define FILE_D fileMasks[3]
#define FILE_E fileMasks[4]
#define FILE_F fileMasks[5]
#define FILE_G fileMasks[6]
#define FILE_H fileMasks[7]

#define RANK_1 rankMasks[0]
#define RANK_2 rankMasks[1]
#define RANK_3 rankMasks[2]
#define RANK_4 rankMasks[3]
#define RANK_5 rankMasks[4]
#define RANK_6 rankMasks[5]
#define RANK_7 rankMasks[6]
#define RANK_8 rankMasks[7]

extern U64 knightAttacks[64];
extern U64 kingAttacks[64];

std::string moveToString(int move);
