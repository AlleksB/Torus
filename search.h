#pragma once

#include "board.h"
#include "eval.h"
#include <chrono>
#include <memory>

enum TTFlag { TT_EXACT, TT_LOWER, TT_UPPER };

struct TTEntry {
    U64 hash     = 0;
    int score    = 0;
    int bestMove = 0;
    int depth    = 0;
    int flag     = 0;
};

class Search {
public:
    static constexpr int INF       = 1 << 20;
    static constexpr int CONTEMPT  = 25;
    static constexpr int MAX_DEPTH = 128;
    static constexpr int TT_SIZE   = 1 << 22;

    Search();

    int  findBestMove(Board& board, int softLimitMs, int hardLimitMs);

    void clearTT();

private:
    std::unique_ptr<TTEntry[]> tt_;
    int  killers_[2][MAX_DEPTH];
    int historyTable_[13][64];
    int counterMoves_[13][64];
    int lmrTable_[MAX_DEPTH][256];

    bool stopSearch_;
    int  softLimit_;
    int  hardLimit_;
    int  nodesProcessed_;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime_;

    Evaluator evaluator_;

    void checkTime();
    int  getMoveScore(const Board& board, int move, int ttMove, int ply) const;

    int  quiescence(Board& board, int alpha, int beta);
    int  alphaBeta (Board& board, int depth, int alpha, int beta, int ply);
};
