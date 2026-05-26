#include "movegen.h"

static const U64 BISHOP_MAGIC_NUMBERS[64] = {
    0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
    0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
    0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
    0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
    0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
    0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
    0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
    0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
    0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
    0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
    0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
    0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
    0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
    0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
    0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
    0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
};

static const U64 ROOK_MAGIC_NUMBERS[64] = {
    0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
    0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
    0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
    0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
    0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
    0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
    0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
    0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
    0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
    0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
    0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
    0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
    0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
    0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL
};

static const int BISHOP_SHIFTS[64] = {
    58, 59, 59, 59, 59, 59, 59, 58,
    59, 59, 59, 59, 59, 59, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 59, 59, 59, 59, 59, 59,
    58, 59, 59, 59, 59, 59, 59, 58
};

static const int ROOK_SHIFTS[64] = {
    52, 53, 53, 53, 53, 53, 53, 52,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 53, 53, 53, 53, 53
};

struct Magic {
    U64  mask;
    U64  magic;
    int  shift;
    U64* ptr;
};

static U64   bishopTable[5248];
static U64   rookTable[102400];
static Magic bishopMagics[64];
static Magic rookMagics[64];

static U64 pawnAttacks[2][64];

U64 knightAttacks[64];
U64 kingAttacks[64];

U64 rookAttacksSlow(int sq, U64 blocks) {
    U64 attack = 0;
    int rank = sq / 8, file = sq % 8;

    for (int r = rank - 1; r >= 0; r--) {
        int s = r * 8 + file; attack |= (1ULL << s);
        if (blocks & (1ULL << s)) break;
    }
    for (int r = rank + 1; r < 8; r++) {
        int s = r * 8 + file; attack |= (1ULL << s);
        if (blocks & (1ULL << s)) break;
    }
    for (int f = file - 1; f >= 0; f--) {
        int s = rank * 8 + f; attack |= (1ULL << s);
        if (blocks & (1ULL << s)) break;
    }
    for (int f = file + 1; f < 8; f++) {
        int s = rank * 8 + f; attack |= (1ULL << s);
        if (blocks & (1ULL << s)) break;
    }
    return attack;
}

U64 bishopAttacksSlow(int sq, U64 blocks) {
    U64 attack = 0;
    int rank = sq / 8, file = sq % 8;

    for (int r = rank-1, f = file-1; r >= 0 && f >= 0; r--, f--) {
        int s = r*8+f; attack |= (1ULL << s); if (blocks & (1ULL << s)) break;
    }
    for (int r = rank-1, f = file+1; r >= 0 && f < 8; r--, f++) {
        int s = r*8+f; attack |= (1ULL << s); if (blocks & (1ULL << s)) break;
    }
    for (int r = rank+1, f = file-1; r < 8 && f >= 0; r++, f--) {
        int s = r*8+f; attack |= (1ULL << s); if (blocks & (1ULL << s)) break;
    }
    for (int r = rank+1, f = file+1; r < 8 && f < 8; r++, f++) {
        int s = r*8+f; attack |= (1ULL << s); if (blocks & (1ULL << s)) break;
    }
    return attack;
}

static U64 maskBishopAttacks(int sq) {
    U64 attacks = 0;
    int tr = sq / 8, tf = sq % 8;
    for (int r=tr+1,f=tf+1; r<=6&&f<=6; r++,f++) attacks |= (1ULL<<(r*8+f));
    for (int r=tr+1,f=tf-1; r<=6&&f>=1; r++,f--) attacks |= (1ULL<<(r*8+f));
    for (int r=tr-1,f=tf+1; r>=1&&f<=6; r--,f++) attacks |= (1ULL<<(r*8+f));
    for (int r=tr-1,f=tf-1; r>=1&&f>=1; r--,f--) attacks |= (1ULL<<(r*8+f));
    return attacks;
}

static U64 maskRookAttacks(int sq) {
    U64 attacks = 0;
    int tr = sq / 8, tf = sq % 8;
    for (int r=tr+1; r<=6; r++) attacks |= (1ULL<<(r*8+tf));
    for (int r=tr-1; r>=1; r--) attacks |= (1ULL<<(r*8+tf));
    for (int f=tf+1; f<=6; f++) attacks |= (1ULL<<(tr*8+f));
    for (int f=tf-1; f>=1; f--) attacks |= (1ULL<<(tr*8+f));
    return attacks;
}

static U64 setOccupancy(int index, int bitsInMask, U64 mask) {
    U64 occupancy = 0;
    for (int i = 0; i < bitsInMask; i++) {
        int sq = getLSB(mask);
        popBit(mask, sq);
        if (index & (1 << i)) occupancy |= (1ULL << sq);
    }
    return occupancy;
}

static void initBishopMagics() {
    int offset = 0;
    for (int sq = 0; sq < 64; sq++) {
        bishopMagics[sq] = { maskBishopAttacks(sq), BISHOP_MAGIC_NUMBERS[sq], BISHOP_SHIFTS[sq], &bishopTable[offset] };

        int bits = countBits(bishopMagics[sq].mask);
        int variations = 1 << bits;

        for (int i = 0; i < variations; i++) {
            U64 occ = setOccupancy(i, bits, bishopMagics[sq].mask);
            int idx = (int)((occ * bishopMagics[sq].magic) >> bishopMagics[sq].shift);
            bishopMagics[sq].ptr[idx] = bishopAttacksSlow(sq, occ);
        }
        offset += variations;
    }
}

static void initRookMagics() {
    int offset = 0;
    for (int sq = 0; sq < 64; sq++) {
        rookMagics[sq] = { maskRookAttacks(sq), ROOK_MAGIC_NUMBERS[sq], ROOK_SHIFTS[sq], &rookTable[offset] };

        int bits = countBits(rookMagics[sq].mask);
        int variations = 1 << bits;

        for (int i = 0; i < variations; i++) {
            U64 occ = setOccupancy(i, bits, rookMagics[sq].mask);
            int idx = (int)((occ * rookMagics[sq].magic) >> rookMagics[sq].shift);
            rookMagics[sq].ptr[idx] = rookAttacksSlow(sq, occ);
        }
        offset += variations;
    }
}

static void initPawnAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        U64 bit = 1ULL << sq;
        pawnAttacks[WHITE][sq] = 0;
        pawnAttacks[BLACK][sq] = 0;
        if (bit & ~FILE_A) pawnAttacks[WHITE][sq] |= (bit << 7);
        if (bit & ~FILE_H) pawnAttacks[WHITE][sq] |= (bit << 9);
        if (bit & ~FILE_A) pawnAttacks[BLACK][sq] |= (bit >> 9);
        if (bit & ~FILE_H) pawnAttacks[BLACK][sq] |= (bit >> 7);
    }
}

static void initLeaperAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        knightAttacks[sq] = 0;
        kingAttacks[sq]   = 0;
        U64 bit = 1ULL << sq;

        // King
        if (bit & ~RANK_8)             kingAttacks[sq] |= (bit << 8);
        if (bit & ~RANK_1)             kingAttacks[sq] |= (bit >> 8);
        if (bit & ~FILE_H)             kingAttacks[sq] |= (bit << 1);
        if (bit & ~FILE_A)             kingAttacks[sq] |= (bit >> 1);
        if (bit & ~RANK_8 & ~FILE_H)   kingAttacks[sq] |= (bit << 9);
        if (bit & ~RANK_8 & ~FILE_A)   kingAttacks[sq] |= (bit << 7);
        if (bit & ~RANK_1 & ~FILE_H)   kingAttacks[sq] |= (bit >> 7);
        if (bit & ~RANK_1 & ~FILE_A)   kingAttacks[sq] |= (bit >> 9);

        // Knight
        if (bit & ~RANK_8 & ~RANK_7 & ~FILE_H) knightAttacks[sq] |= (bit << 17);
        if (bit & ~RANK_8 & ~RANK_7 & ~FILE_A) knightAttacks[sq] |= (bit << 15);
        if (bit & ~RANK_1 & ~RANK_2 & ~FILE_H) knightAttacks[sq] |= (bit >> 15);
        if (bit & ~RANK_1 & ~RANK_2 & ~FILE_A) knightAttacks[sq] |= (bit >> 17);
        if (bit & ~FILE_G & ~FILE_H & ~RANK_8) knightAttacks[sq] |= (bit << 10);
        if (bit & ~FILE_A & ~FILE_B & ~RANK_8) knightAttacks[sq] |= (bit << 6);
        if (bit & ~FILE_G & ~FILE_H & ~RANK_1) knightAttacks[sq] |= (bit >> 6);
        if (bit & ~FILE_A & ~FILE_B & ~RANK_1) knightAttacks[sq] |= (bit >> 10);
    }
}

void initAttackTables() {
    initBishopMagics();
    initRookMagics();
    initPawnAttacks();
    initLeaperAttacks();
}

U64 getBishopAttacks(int sq, U64 occupancy) {
    occupancy &= bishopMagics[sq].mask;
    occupancy *= bishopMagics[sq].magic;
    occupancy >>= bishopMagics[sq].shift;
    return bishopMagics[sq].ptr[occupancy];
}

U64 getRookAttacks(int sq, U64 occupancy) {
    occupancy &= rookMagics[sq].mask;
    occupancy *= rookMagics[sq].magic;
    occupancy >>= rookMagics[sq].shift;
    return rookMagics[sq].ptr[occupancy];
}

U64 getQueenAttacks(int sq, U64 occupancy) {
    return getBishopAttacks(sq, occupancy) | getRookAttacks(sq, occupancy);
}

bool isSquareAttacked(const Board& board, int sq, int attackerColor) {
    U64 pawns = board.pieceBoards[attackerColor == WHITE ? wP : bP];
    if (pawnAttacks[attackerColor ^ 1][sq] & pawns) return true;

    U64 knights = board.pieceBoards[attackerColor == WHITE ? wN : bN];
    if (knightAttacks[sq] & knights) return true;

    U64 occupancy = board.colorBoards[BOTH];
    U64 bishops = board.pieceBoards[attackerColor == WHITE ? wB : bB];
    U64 queens = board.pieceBoards[attackerColor == WHITE ? wQ : bQ];
    if (getBishopAttacks(sq, occupancy) & (bishops | queens)) return true;

    U64 rooks = board.pieceBoards[attackerColor == WHITE ? wR : bR];
    if (getRookAttacks(sq, occupancy) & (rooks | queens)) return true;

    U64 kings = board.pieceBoards[attackerColor == WHITE ? wK : bK];
    if (kingAttacks[sq] & kings) return true;

    return false;
}

static void generatePawnMoves(const Board& board, MoveList* list) {
    int  side      = board.sideToMove;
    U64  pawns     = board.pieceBoards[side == WHITE ? wP : bP];
    U64  enemies   = board.colorBoards[side ^ 1];
    U64  occ       = board.colorBoards[BOTH];
    U64  promoRank = (side == WHITE) ? RANK_8 : RANK_1;

    while (pawns) {
        int from = pullLsb(pawns);
        U64 atk  = pawnAttacks[side][from] & enemies;

        while (atk) {
            int to = pullLsb(atk);
            if ((1ULL << to) & promoRank) {
                list->moves[list->count++] = encodeMove(from, to, MOVE_PROMO_N | MOVE_CAPTURE);
                list->moves[list->count++] = encodeMove(from, to, MOVE_PROMO_B | MOVE_CAPTURE);
                list->moves[list->count++] = encodeMove(from, to, MOVE_PROMO_R | MOVE_CAPTURE);
                list->moves[list->count++] = encodeMove(from, to, MOVE_PROMO_Q | MOVE_CAPTURE);
            }
            else {
                list->moves[list->count++] = encodeMove(from, to, MOVE_CAPTURE);
            }
        }

        if (board.enPassantSq != NO_SQ &&
            ((1ULL << board.enPassantSq) & pawnAttacks[side][from])) {
            list->moves[list->count++] = encodeMove(from, board.enPassantSq, MOVE_EP_CAPTURE);
        }

        U64 bit = 1ULL << from;
        if (side == WHITE) {
            if ((bit & ~(RANK_7 | RANK_8)) && !((bit << 8) & occ))
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_QUIET);
            if ((bit & RANK_2) && !((bit << 8) & occ) && !((bit << 16) & occ))
                list->moves[list->count++] = encodeMove(from, from + 16, MOVE_DOUBLE_PUSH);
            if ((bit & RANK_7) && !((bit << 8) & occ)) {
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_PROMO_N);
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_PROMO_B);
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_PROMO_R);
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_PROMO_Q);
            }
        }
        else {
            if ((bit & ~(RANK_1 | RANK_2)) && !((bit >> 8) & occ))
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_QUIET);
            if ((bit & RANK_7) && !((bit >> 8) & occ) && !((bit >> 16) & occ))
                list->moves[list->count++] = encodeMove(from, from - 16, MOVE_DOUBLE_PUSH);
            if ((bit & RANK_2) && !((bit >> 8) & occ)) {
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_PROMO_N);
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_PROMO_B);
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_PROMO_R);
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_PROMO_Q);
            }
        }
    }
}

static void generatePieceMoves(const Board& board, MoveList* list, int piece, U64 (*getAttacks)(int, U64)) {
    int side    = board.sideToMove;
    U64 pieces  = board.pieceBoards[piece];
    U64 enemies = board.colorBoards[side ^ 1];
    U64 occ     = board.colorBoards[BOTH];

    while (pieces) {
        int from  = pullLsb(pieces);
        U64 moves = getAttacks(from, occ) & ~board.colorBoards[side];

        while (moves) {
            int to = pullLsb(moves);
            int flag = ((1ULL << to) & enemies) ? MOVE_CAPTURE : MOVE_QUIET;
            list->moves[list->count++] = encodeMove(from, to, flag);
        }
    }
}

static void generateLeaperMoves(const Board& board, MoveList* list, int piece, U64 attacks[64]) {
    int side    = board.sideToMove;
    U64 pieces  = board.pieceBoards[piece];
    U64 enemies = board.colorBoards[side ^ 1];

    while (pieces) {
        int from  = pullLsb(pieces);
        U64 moves = attacks[from] & ~board.colorBoards[side];

        while (moves) {
            int to   = pullLsb(moves);
            int flag = ((1ULL << to) & enemies) ? MOVE_CAPTURE : MOVE_QUIET;
            list->moves[list->count++] = encodeMove(from, to, flag);
        }
    }
}

static void generateKingMoves(const Board& board, MoveList* list) {
    int side      = board.sideToMove;
    int enemySide = side ^ 1;

    generateLeaperMoves(board, list, side == WHITE ? wK : bK, kingAttacks);

    U64 occ = board.colorBoards[BOTH];

    if (side == WHITE) {
        if ((board.castlingRights & (1 << CASTLE_K)) &&
            !((1ULL << F1) & occ) && !((1ULL << G1) & occ) &&
            !isSquareAttacked(board, E1, enemySide) &&
            !isSquareAttacked(board, F1, enemySide))
            list->moves[list->count++] = encodeMove(E1, G1, MOVE_K_CASTLE);

        if ((board.castlingRights & (1 << CASTLE_Q)) &&
            !((1ULL << D1) & occ) && !((1ULL << C1) & occ) && !((1ULL << B1) & occ) &&
            !isSquareAttacked(board, E1, enemySide) &&
            !isSquareAttacked(board, D1, enemySide))
            list->moves[list->count++] = encodeMove(E1, C1, MOVE_Q_CASTLE);
    }
    else {
        if ((board.castlingRights & (1 << CASTLE_k)) &&
            !((1ULL << F8) & occ) && !((1ULL << G8) & occ) &&
            !isSquareAttacked(board, E8, enemySide) &&
            !isSquareAttacked(board, F8, enemySide))
            list->moves[list->count++] = encodeMove(E8, G8, MOVE_K_CASTLE);

        if ((board.castlingRights & (1 << CASTLE_q)) &&
            !((1ULL << D8) & occ) && !((1ULL << C8) & occ) && !((1ULL << B8) & occ) &&
            !isSquareAttacked(board, E8, enemySide) &&
            !isSquareAttacked(board, D8, enemySide))
            list->moves[list->count++] = encodeMove(E8, C8, MOVE_Q_CASTLE);
    }
}

static void generatePawnCaptures(const Board& board, MoveList* list) {
    int  side      = board.sideToMove;
    U64  pawns     = board.pieceBoards[side == WHITE ? wP : bP];
    U64  enemies   = board.colorBoards[side ^ 1];
    U64  occ       = board.colorBoards[BOTH];
    U64  promoRank = (side == WHITE) ? RANK_8 : RANK_1;

    while (pawns) {
        int from = pullLsb(pawns);
        U64 atk  = pawnAttacks[side][from] & enemies;

        while (atk) {
            int to = pullLsb(atk);
            if ((1ULL << to) & promoRank) {
                list->moves[list->count++] = encodeMove(from, to, MOVE_PROMO_Q | MOVE_CAPTURE);
                list->moves[list->count++] = encodeMove(from, to, MOVE_PROMO_R | MOVE_CAPTURE);
                list->moves[list->count++] = encodeMove(from, to, MOVE_PROMO_B | MOVE_CAPTURE);
                list->moves[list->count++] = encodeMove(from, to, MOVE_PROMO_N | MOVE_CAPTURE);
            }
            else {
                list->moves[list->count++] = encodeMove(from, to, MOVE_CAPTURE);
            }
        }

        if (board.enPassantSq != NO_SQ && ((1ULL << board.enPassantSq) & pawnAttacks[side][from])) {
            list->moves[list->count++] = encodeMove(from, board.enPassantSq, MOVE_EP_CAPTURE);
        }

        U64 bit = 1ULL << from;
        if (side == WHITE) {
            if ((bit & RANK_7) && !((bit << 8) & occ)) {
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_PROMO_Q);
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_PROMO_R);
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_PROMO_B);
                list->moves[list->count++] = encodeMove(from, from + 8, MOVE_PROMO_N);
            }
        }
        else {
            if ((bit & RANK_2) && !((bit >> 8) & occ)) {
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_PROMO_Q);
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_PROMO_R);
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_PROMO_B);
                list->moves[list->count++] = encodeMove(from, from - 8, MOVE_PROMO_N);
            }
        }
    }
}

static void generatePieceCaptures(const Board& board, MoveList* list, int piece, U64 (*getAttacks)(int, U64)) {
    int side    = board.sideToMove;
    U64 pieces  = board.pieceBoards[piece];
    U64 enemies = board.colorBoards[side ^ 1];
    U64 occ     = board.colorBoards[BOTH];

    while (pieces) {
        int from  = pullLsb(pieces);
        U64 moves = getAttacks(from, occ) & enemies;

        while (moves) {
            int to = pullLsb(moves);
            list->moves[list->count++] = encodeMove(from, to, MOVE_CAPTURE);
        }
    }
}

static void generateLeaperCaptures(const Board& board, MoveList* list, int piece, U64 attacks[64]) {
    int side    = board.sideToMove;
    U64 pieces  = board.pieceBoards[piece];
    U64 enemies = board.colorBoards[side ^ 1];

    while (pieces) {
        int from  = pullLsb(pieces);
        U64 moves = attacks[from] & enemies;

        while (moves) {
            int to   = pullLsb(moves);
            list->moves[list->count++] = encodeMove(from, to, MOVE_CAPTURE);
        }
    }
}

void generateCaptures(const Board& board, MoveList* list) {
    list->count = 0;
    int side = board.sideToMove;

    generatePawnCaptures(board, list);
    generateLeaperCaptures(board, list, side == WHITE ? wN : bN, knightAttacks);
    generatePieceCaptures(board, list, side == WHITE ? wB : bB, getBishopAttacks);
    generatePieceCaptures(board, list, side == WHITE ? wR : bR, getRookAttacks);
    generatePieceCaptures(board, list, side == WHITE ? wQ : bQ, getQueenAttacks);
    generateLeaperCaptures(board, list, side == WHITE ? wK : bK, kingAttacks);
}

void generateAllMoves(const Board& board, MoveList* list) {
    list->count = 0;
    int side = board.sideToMove;

    generatePawnMoves(board, list);
    generateLeaperMoves(board, list, side == WHITE ? wN : bN, knightAttacks);
    generatePieceMoves(board, list, side == WHITE ? wB : bB, getBishopAttacks);
    generatePieceMoves(board, list, side == WHITE ? wR : bR, getRookAttacks);
    generatePieceMoves(board, list, side == WHITE ? wQ : bQ, getQueenAttacks);
    generateKingMoves(board, list);
}

int seeCapture(const Board& board, int move) {
    int from  = getMoveSource(move);
    int to    = getMoveTarget(move);
    int flags = getMoveFlags(move);

    int gain[32];
    int d = 0;

    int movingPiece   = board.getPieceOnSquare(from);
    int capturedPiece = (flags == MOVE_EP_CAPTURE) ? (board.sideToMove == WHITE ? bP : wP) : board.getPieceOnSquare(to);

    if (flags & MOVE_PROMOTION) {
        int promoPiece = (board.sideToMove == WHITE ? wN : bN) + (flags & 3);
        gain[d] = PIECE_VALUES[capturedPiece] + PIECE_VALUES[promoPiece] - PIECE_VALUES[movingPiece];
        movingPiece = promoPiece; 
    }
    else {
        gain[d] = PIECE_VALUES[capturedPiece];
    }

    U64 occ = board.colorBoards[BOTH];
    occ ^= (1ULL << from); 
    occ |= (1ULL << to);   
    if (flags == MOVE_EP_CAPTURE) {
        int epSq = (board.sideToMove == WHITE) ? (to - 8) : (to + 8);
        occ ^= (1ULL << epSq);
    }

    int side = board.sideToMove ^ 1;
    U64 attackers = 0;

    auto getAttackers = [&]() {
        U64 att = 0;
        att |= pawnAttacks[BLACK][to] & board.pieceBoards[wP];
        att |= pawnAttacks[WHITE][to] & board.pieceBoards[bP];
        att |= knightAttacks[to] & (board.pieceBoards[wN] | board.pieceBoards[bN]);
        att |= kingAttacks[to] & (board.pieceBoards[wK] | board.pieceBoards[bK]);
        att |= getBishopAttacks(to, occ) & (board.pieceBoards[wB] | board.pieceBoards[bB] | board.pieceBoards[wQ] | board.pieceBoards[bQ]);
        att |= getRookAttacks(to, occ) & (board.pieceBoards[wR] | board.pieceBoards[bR] | board.pieceBoards[wQ] | board.pieceBoards[bQ]);
        return att & occ;
    };

    attackers = getAttackers();

    while (true) {
        d++;
        gain[d] = PIECE_VALUES[movingPiece] - gain[d - 1];

        U64 ourAttackers = attackers & board.colorBoards[side];
        if (!ourAttackers) break;

        int attackerSq = -1;
        if      (ourAttackers & board.pieceBoards[side == WHITE ? wP : bP]) { attackerSq = getLSB(ourAttackers & board.pieceBoards[side == WHITE ? wP : bP]); movingPiece = side == WHITE ? wP : bP; }
        else if (ourAttackers & board.pieceBoards[side == WHITE ? wN : bN]) { attackerSq = getLSB(ourAttackers & board.pieceBoards[side == WHITE ? wN : bN]); movingPiece = side == WHITE ? wN : bN; }
        else if (ourAttackers & board.pieceBoards[side == WHITE ? wB : bB]) { attackerSq = getLSB(ourAttackers & board.pieceBoards[side == WHITE ? wB : bB]); movingPiece = side == WHITE ? wB : bB; }
        else if (ourAttackers & board.pieceBoards[side == WHITE ? wR : bR]) { attackerSq = getLSB(ourAttackers & board.pieceBoards[side == WHITE ? wR : bR]); movingPiece = side == WHITE ? wR : bR; }
        else if (ourAttackers & board.pieceBoards[side == WHITE ? wQ : bQ]) { attackerSq = getLSB(ourAttackers & board.pieceBoards[side == WHITE ? wQ : bQ]); movingPiece = side == WHITE ? wQ : bQ; }
        else if (ourAttackers & board.pieceBoards[side == WHITE ? wK : bK]) { attackerSq = getLSB(ourAttackers & board.pieceBoards[side == WHITE ? wK : bK]); movingPiece = side == WHITE ? wK : bK; }

        occ ^= (1ULL << attackerSq);
        
        attackers = getAttackers();
        
        side ^= 1;
    }

    while (--d) {
        gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
    }

    return gain[0];
}
