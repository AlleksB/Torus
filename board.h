#pragma once

#include "types.h"

extern U64 zobristPieces[13][64];
extern U64 zobristSide;
extern U64 zobristCastling[16];
extern U64 zobristEnPassant[9];

class Board {
public:
    U64 pieceBoards[13]  = {};
    U64 colorBoards[3]   = {};
    int pieceList[64]    = {};

    int sideToMove       = WHITE;
    int enPassantSq      = NO_SQ;
    int castlingRights   = 0;
    int fiftyMoveCounter = 0;
    int fullMoves        = 0;
    U64 zobristHash      = 0;

    UndoInfo history[1024];
    int historyPtr = 0;

    Board();

    void resetBoard();
    void parseFEN(const std::string& fen);

    bool makeMove(int move);
    void unmakeMove();

    void makeNullMove();
    void unmakeNullMove();

    bool isRepetition() const;

    inline int getPieceOnSquare(int sq) const { return pieceList[sq]; }

    U64 computeZobristHash() const;

    void printBoard() const;
    void printBitboard(U64 bb) const;

private:
    void addPiece(int piece, int sq);
    void removePiece(int sq, int piece);
    void placePiece(int sq, int piece);
    void updateCastlingRights(int from, int to);
};
