#include "board.h"

Board::Board() {
    resetBoard();
}

void Board::resetBoard() {
    for (int i = 0; i < 13; i++) pieceBoards[i] = 0;
    for (int i = 0; i < 3;  i++) colorBoards[i] = 0;
    for (int i = 0; i < 64; i++) pieceList[i]   = EMPTY;

    sideToMove       = WHITE;
    enPassantSq      = NO_SQ;
    castlingRights   = 0;
    fiftyMoveCounter = 0;
    fullMoves        = 0;
    zobristHash      = 0;
    historyPtr       = 0;
}

void Board::parseFEN(const std::string& fen) {
    resetBoard();

    int state  = PIECE_POSITIONS;
    int currSq = A8;

    for (char c : fen) {
        if (c == ' ') {
            state++;
            continue;
        }

        switch (state) {
        case PIECE_POSITIONS:
            switch (c) {
                case '/': currSq -= 16; break;
                case 'P': addPiece(wP, currSq++); break;
                case 'N': addPiece(wN, currSq++); break;
                case 'B': addPiece(wB, currSq++); break;
                case 'R': addPiece(wR, currSq++); break;
                case 'Q': addPiece(wQ, currSq++); break;
                case 'K': addPiece(wK, currSq++); break;
                case 'p': addPiece(bP, currSq++); break;
                case 'n': addPiece(bN, currSq++); break;
                case 'b': addPiece(bB, currSq++); break;
                case 'r': addPiece(bR, currSq++); break;
                case 'q': addPiece(bQ, currSq++); break;
                case 'k': addPiece(bK, currSq++); break;
                default:
                    if (c >= '1' && c <= '8') currSq += c - '0';
                    break;
            }
            break;

        case TO_MOVE:
            if      (c == 'w') sideToMove = WHITE;
            else if (c == 'b') sideToMove = BLACK;
            break;

        case CASTLING:
            switch (c) {
                case 'K': castlingRights |= (1 << CASTLE_K); break;
                case 'Q': castlingRights |= (1 << CASTLE_Q); break;
                case 'k': castlingRights |= (1 << CASTLE_k); break;
                case 'q': castlingRights |= (1 << CASTLE_q); break;
                default:  castlingRights  = 0;               break;
            }
            break;

        case EN_PASSANT:
            if      (c >= 'a' && c <= 'h') enPassantSq  = c - 'a';
            else if (c >= '1' && c <= '8') enPassantSq += (c - '1') * 8;
            else                           enPassantSq  = NO_SQ;
            break;

        case HALFMOVES:
            fiftyMoveCounter = 10 * fiftyMoveCounter + (c - '0');
            break;

        case FULLMOVES:
            fullMoves = 10 * fullMoves + (c - '0');
            break;
        }
    }

    zobristHash = computeZobristHash();
}

void Board::addPiece(int piece, int sq) {
    pieceBoards[piece]     |= (1ULL << sq);
    int color               = (piece <= wK) ? WHITE : BLACK;
    colorBoards[color]     |= (1ULL << sq);
    colorBoards[BOTH]      |= (1ULL << sq);
    pieceList[sq]           = piece;
}

void Board::removePiece(int sq, int piece) {
    U64 bit = 1ULL << sq;
    pieceBoards[piece]    &= ~bit;
    int color              = (piece <= wK) ? WHITE : BLACK;
    colorBoards[color]    &= ~bit;
    colorBoards[BOTH]     &= ~bit;
    pieceList[sq]          = EMPTY;
    zobristHash           ^= zobristPieces[piece][sq];
}

void Board::placePiece(int sq, int piece) {
    U64 bit = 1ULL << sq;
    pieceBoards[piece]    |= bit;
    int color              = (piece <= wK) ? WHITE : BLACK;
    colorBoards[color]    |= bit;
    colorBoards[BOTH]     |= bit;
    pieceList[sq]          = piece;
    zobristHash           ^= zobristPieces[piece][sq];
}

void Board::updateCastlingRights(int from, int to) {
    if      (from == E1) castlingRights &= ~(1 << CASTLE_K) & ~(1 << CASTLE_Q);
    else if (from == E8) castlingRights &= ~(1 << CASTLE_k) & ~(1 << CASTLE_q);

    if (from == H1 || to == H1) castlingRights &= ~(1 << CASTLE_K);
    if (from == A1 || to == A1) castlingRights &= ~(1 << CASTLE_Q);
    if (from == H8 || to == H8) castlingRights &= ~(1 << CASTLE_k);
    if (from == A8 || to == A8) castlingRights &= ~(1 << CASTLE_q);
}

bool Board::makeMove(int move) {
    const int from  = getMoveSource(move);
    const int to    = getMoveTarget(move);
    const int flags = getMoveFlags(move);
    const int side  = sideToMove;

    history[historyPtr] = { move, castlingRights, enPassantSq, fiftyMoveCounter, EMPTY, zobristHash };
    historyPtr++;

    int oldEpFile = (enPassantSq == NO_SQ) ? 8 : (enPassantSq % 8);
    zobristHash ^= zobristEnPassant[oldEpFile];
    zobristHash ^= zobristCastling[castlingRights];

    int capturedPiece = EMPTY;
    if (flags == MOVE_EP_CAPTURE) {
        capturedPiece  = (side == WHITE) ? bP : wP;
        int epPawnSq   = (side == WHITE) ? (to - 8) : (to + 8);
        removePiece(epPawnSq, capturedPiece);
    }
    else if (flags & MOVE_CAPTURE) {
        capturedPiece  = getPieceOnSquare(to);
        removePiece(to, capturedPiece);
    }
    history[historyPtr - 1].capturedPiece = capturedPiece;

    int movingPiece = getPieceOnSquare(from);
    removePiece(from, movingPiece);
    placePiece(to, movingPiece);

    enPassantSq = NO_SQ;

    if (flags == MOVE_DOUBLE_PUSH) {
        enPassantSq = (side == WHITE) ? (to - 8) : (to + 8);

    }
    else if (flags == MOVE_K_CASTLE) {
        if (side == WHITE) { removePiece(H1, wR); placePiece(F1, wR); }
        else               { removePiece(H8, bR); placePiece(F8, bR); }

    }
    else if (flags == MOVE_Q_CASTLE) {
        if (side == WHITE) { removePiece(A1, wR); placePiece(D1, wR); }
        else               { removePiece(A8, bR); placePiece(D8, bR); }

    }
    else if (flags & MOVE_PROMOTION) {
        removePiece(to, movingPiece);
        int promoted = (side == WHITE ? wN : bN) + (flags & 3);
        placePiece(to, promoted);
    }

    updateCastlingRights(from, to);

    int newEpFile = (enPassantSq == NO_SQ) ? 8 : (enPassantSq % 8);
    zobristHash ^= zobristEnPassant[newEpFile];
    zobristHash ^= zobristCastling[castlingRights];
    zobristHash ^= zobristSide;

    if (movingPiece == wP || movingPiece == bP || (flags & MOVE_CAPTURE))
        fiftyMoveCounter = 0;
    else
        fiftyMoveCounter++;

    sideToMove ^= 1;

    int kingSq = getLSB(pieceBoards[side == WHITE ? wK : bK]);

    extern bool isSquareAttacked(const Board&, int, int);
    if (isSquareAttacked(*this, kingSq, sideToMove)) {
        unmakeMove();
        return false;
    }

    return true;
}

void Board::unmakeMove() {
    historyPtr--;
    const UndoInfo* last = &history[historyPtr];

    const int move  = last->move;
    const int from  = getMoveSource(move);
    const int to    = getMoveTarget(move);
    const int flags = getMoveFlags(move);
    const int side  = sideToMove ^ 1;

    int movingPiece = getPieceOnSquare(to);
    removePiece(to, movingPiece);
    placePiece(from, movingPiece);

    if (flags & MOVE_PROMOTION) {
        removePiece(from, movingPiece);
        placePiece(from, (side == WHITE) ? wP : bP);
    }

    if (last->capturedPiece != EMPTY) {
        if (flags == MOVE_EP_CAPTURE) {
            int epPawnSq = (side == WHITE) ? (to - 8) : (to + 8);
            placePiece(epPawnSq, last->capturedPiece);
        }
        else {
            placePiece(to, last->capturedPiece);
        }
    }

    if (flags == MOVE_K_CASTLE) {
        if (side == WHITE) { removePiece(F1, wR); placePiece(H1, wR); }
        else               { removePiece(F8, bR); placePiece(H8, bR); }
    }
    else if (flags == MOVE_Q_CASTLE) {
        if (side == WHITE) { removePiece(D1, wR); placePiece(A1, wR); }
        else               { removePiece(D8, bR); placePiece(A8, bR); }
    }

    castlingRights   = last->castlingRights;
    enPassantSq      = last->enPassantSq;
    fiftyMoveCounter = last->fiftyMoveCounter;
    zobristHash      = last->zobristHash;
    sideToMove       = side;
}

void Board::makeNullMove() {
    history[historyPtr].enPassantSq    = enPassantSq;
    history[historyPtr].zobristHash    = zobristHash;
    history[historyPtr].fiftyMoveCounter = fiftyMoveCounter;
    historyPtr++;

    int epFile = (enPassantSq == NO_SQ) ? 8 : (enPassantSq % 8);
    zobristHash ^= zobristEnPassant[epFile];

    enPassantSq = NO_SQ;

    zobristHash ^= zobristEnPassant[8];
    zobristHash ^= zobristSide;

    sideToMove ^= 1;
    fiftyMoveCounter++;
}

void Board::unmakeNullMove() {
    historyPtr--;
    const UndoInfo* last = &history[historyPtr];

    enPassantSq      = last->enPassantSq;
    zobristHash      = last->zobristHash;
    fiftyMoveCounter = last->fiftyMoveCounter;
    sideToMove      ^= 1;
}

bool Board::isRepetition() const {
    for (int i = historyPtr - 2; i >= 0; i -= 2) {
        if (history[i + 1].fiftyMoveCounter == 0) break;
        if (history[i].zobristHash == zobristHash) return true;
    }
    return false;
}

U64 Board::computeZobristHash() const {
    U64 hash = 0;

    for (int sq = 0; sq < 64; sq++) {
        int piece = pieceList[sq];
        if (piece != EMPTY)
            hash ^= zobristPieces[piece][sq];
    }

    if (sideToMove == BLACK)
        hash ^= zobristSide;

    hash ^= zobristCastling[castlingRights];

    int epFile = (enPassantSq == NO_SQ) ? 8 : (enPassantSq % 8);
    hash ^= zobristEnPassant[epFile];

    return hash;
}

void Board::printBoard() const {
    static const char* pieceChars = ".PNBRQKpnbrqk";

    std::cout << "\n  A B C D E F G H\n";
    for (int rank = 8; rank >= 1; rank--) {
        std::cout << rank << ' ';
        for (int file = 0; file < 8; file++) {
            int sq    = (rank - 1) * 8 + file;
            int piece = EMPTY;
            for (int i = wP; i <= bK; i++) {
                if ((1ULL << sq) & pieceBoards[i]) { piece = i; break; }
            }
            std::cout << pieceChars[piece] << ' ';
        }
        std::cout << rank << '\n';
    }
    std::cout << "  A B C D E F G H\n\n";
}

void Board::printBitboard(U64 bb) const {
    std::cout << "\n  A B C D E F G H\n";
    for (int rank = 8; rank >= 1; rank--) {
        std::cout << rank << ' ';
        for (int file = 0; file < 8; file++) {
            int sq = (rank - 1) * 8 + file;
            std::cout << (((1ULL << sq) & bb) ? '*' : '.') << ' ';
        }
        std::cout << rank << '\n';
    }
    std::cout << "  A B C D E F G H\n\n";
}
