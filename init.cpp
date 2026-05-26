#include "init.h"
#include "movegen.h"

U64 fileMasks[8];
U64 rankMasks[8];

void initMasks() {
    for (int i = 0; i < 8; i++) {
        fileMasks[i] = 0;
        rankMasks[i] = 0;
    }
    for (int file = 0; file < 8; file++)
        for (int rank = 0; rank < 8; rank++) {
            int sq = rank * 8 + file;
            fileMasks[file] |= (1ULL << sq);
            rankMasks[rank] |= (1ULL << sq);
        }
}

U64 zobristPieces[13][64];
U64 zobristSide;
U64 zobristCastling[16];
U64 zobristEnPassant[9];

static U64 seed = 1'070'372ULL;

static U64 randomU64() {
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return seed;
}

void initZobrist() {
    for (int piece = 0; piece < 13; piece++)
        for (int sq = 0; sq < 64; sq++)
            zobristPieces[piece][sq] = randomU64();

    zobristSide = randomU64();

    for (int i = 0; i < 16; i++)
        zobristCastling[i] = randomU64();

    for (int i = 0; i < 9; i++)
        zobristEnPassant[i] = randomU64();
}

void init() {
    initMasks();
    initAttackTables();
    initZobrist();
}
