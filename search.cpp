#include "search.h"
#include "movegen.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <cmath>

Search::Search()
    : tt_(std::make_unique<TTEntry[]>(TT_SIZE))
    , stopSearch_(false)
    , softLimit_(0)
    , hardLimit_(0)
    , nodesProcessed_(0)
{
    clearTT();
    std::memset(killers_, 0, sizeof(killers_));
    std::memset(counterMoves_, 0, sizeof(counterMoves_));

    // Precompute Logarithmic LMR table
    for (int d = 0; d < MAX_DEPTH; d++) {
        for (int m = 0; m < 256; m++) {
            if (d >= 2 && m >= 2) {
                lmrTable_[d][m] = 1 + static_cast<int>(std::log(d) * std::log(m) / 2.25);
            }
            else {
                lmrTable_[d][m] = 0;
            }
        }
    }
}

void Search::clearTT() {
    std::fill(tt_.get(), tt_.get() + TT_SIZE, TTEntry{});
}

void Search::checkTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count();
    if (elapsed > hardLimit_) stopSearch_ = true;
}

int Search::getMoveScore(const Board& board, int move, int ttMove, int ply) const {
    if (move == ttMove) return 3'000'000;

    const int flags = getMoveFlags(move);
    const int from  = getMoveSource(move);
    const int to    = getMoveTarget(move);

    if ((flags & MOVE_CAPTURE) || (flags & MOVE_PROMOTION)) {
        int seeVal = seeCapture(board, move);
        if (seeVal < 0) {
            return 1'000 + seeVal; 
        }
        else {
            int score = 20'000;
            if (flags & MOVE_CAPTURE) {
                int attacker = board.pieceList[from];
                int victim   = (flags == MOVE_EP_CAPTURE) ? (board.sideToMove == WHITE ? bP : wP) : board.pieceList[to];
                score += 10 * PIECE_VALUES[victim] - PIECE_VALUES[attacker];
            }
            if (flags & MOVE_PROMOTION) {
                int promoType = flags & 3;
                if (promoType == 3)      score += 9'000; // queen
                else if (promoType == 0) score += 3'000; // knight
                else                     score += 1'000; // rook/bishop
            }
            return score;
        }
    }

    // Killer moves
    if (move == killers_[0][ply]) return 16'000;
    if (move == killers_[1][ply]) return 15'000;

    // Countermove heuristic
    if (board.historyPtr > 0) {
        int prevMove = board.history[board.historyPtr - 1].move;
        if (prevMove != 0) {
            int prevTo = getMoveTarget(prevMove);
            int prevPiece = board.getPieceOnSquare(prevTo);
            if (prevPiece != EMPTY && counterMoves_[prevPiece][prevTo] == move) {
                return 14'000;
            }
        }
    }

    return historyTable_[board.pieceList[from]][to];
}

int Search::quiescence(Board& board, int alpha, int beta) {
    if (stopSearch_) return 0;

    nodesProcessed_++;

    if (board.isRepetition()) return -CONTEMPT;

    int standPat = evaluator_.evaluate(board);
    if (standPat >= beta)  return beta;
    if (standPat > alpha)  alpha = standPat;

    MoveList list;
    generateCaptures(board, &list);

    int scores[256];
    for (int i = 0; i < list.count; i++)
        scores[i] = getMoveScore(board, list.moves[i], 0, 0);

    for (int i = 0; i < list.count; i++) {
        for (int j = i + 1; j < list.count; j++) {
            if (scores[j] > scores[i]) {
                std::swap(scores[i],       scores[j]);
                std::swap(list.moves[i],   list.moves[j]);
            }
        }

        int move = list.moves[i];

        // Prune losing captures from the quiescence search
        if (seeCapture(board, move) < 0) {
            continue;
        }

        if (!board.makeMove(move)) continue;
        
        int score = -quiescence(board, -beta, -alpha);
        board.unmakeMove();

        if (score >= beta)  return beta;
        if (score > alpha)  alpha = score;
    }
    return alpha;
}

int Search::alphaBeta(Board& board, int depth, int alpha, int beta, int ply) {
    if (stopSearch_) return 0;

    if (!(++nodesProcessed_ & 4095)) checkTime();

    if (ply >= MAX_DEPTH) return evaluator_.evaluate(board);

    if (ply > 0 && board.isRepetition()) return -CONTEMPT;

    if (depth <= 0) return quiescence(board, alpha, beta);

    const int originalAlpha = alpha;

    // Transposition table lookup
    TTEntry& entry = tt_[board.zobristHash & (TT_SIZE - 1)];

    if (entry.hash == board.zobristHash && entry.depth >= depth) {
        int score = entry.score;
        if (score >  INF - 200) score -= ply;
        if (score < -INF + 200) score += ply;

        if      (entry.flag == TT_EXACT) return score;
        else if (entry.flag == TT_LOWER) alpha = std::max(alpha, score);
        else if (entry.flag == TT_UPPER) beta  = std::min(beta,  score);

        if (alpha >= beta) return score;
    }

    int ttMove = (entry.hash == board.zobristHash) ? entry.bestMove : 0;

    int staticEval = evaluator_.evaluate(board);

    bool inCheck = isSquareAttacked(board,
        getLSB(board.pieceBoards[board.sideToMove == WHITE ? wK : bK]),
        board.sideToMove ^ 1);

    // Reverse Futility Pruning
    if (ply > 0 && depth <= 3 && !inCheck) {
        int rfpMargin = 120 * depth;
        if (staticEval - rfpMargin >= beta) {
            return staticEval;
        }
    }

    int nonPawnPieces = countBits(board.colorBoards[board.sideToMove]) - countBits(board.pieceBoards[board.sideToMove == WHITE ? wP : bP]) - 1;

    if (ply > 0 && depth >= 3 && !inCheck && nonPawnPieces > 0) {
        board.makeNullMove();
        // Search at reduced depth with a null window around beta
        int nullScore = -alphaBeta(board, depth - 3, -beta, -beta + 1, ply + 1);
        board.unmakeNullMove();

        if (nullScore >= beta) return beta;
    }

    // Futility Pruning Setup
    bool fPruning = false;
    if (ply > 0 && depth <= 2 && !inCheck) {
        int fpMargin = 150 * depth;
        if (staticEval + fpMargin <= alpha) {
            fPruning = true;
        }
    }

    MoveList list;
    generateAllMoves(board, &list);

    int scores[256];
    for (int i = 0; i < list.count; i++)
        scores[i] = getMoveScore(board, list.moves[i], ttMove, ply);

    int legalMoves    = 0;
    int bestMoveFound = 0;

    int quietMovesSearched[256];
    int numQuiets = 0;

    for (int i = 0; i < list.count; i++) {
        for (int j = i + 1; j < list.count; j++) {
            if (scores[j] > scores[i]) {
                std::swap(scores[i],     scores[j]);
                std::swap(list.moves[i], list.moves[j]);
            }
        }

        int move = list.moves[i];
        int flags = getMoveFlags(move);
        bool isCapture = (flags & MOVE_CAPTURE);
        bool isPromotion = (flags & MOVE_PROMOTION);
        bool isKiller = (move == killers_[0][ply] || move == killers_[1][ply]);

        if (!board.makeMove(move)) continue;

        legalMoves++;

        int enemyKingSq = getLSB(board.pieceBoards[board.sideToMove == WHITE ? wK : bK]);
        bool givesCheck = isSquareAttacked(board, enemyKingSq, board.sideToMove ^ 1);
        
        if (fPruning && legalMoves > 1 && !isCapture && !isPromotion && !givesCheck && !isKiller) {
            board.unmakeMove();
            continue;
        }

        if (!isCapture && !isPromotion) {
            quietMovesSearched[numQuiets++] = move;
        }

        int score;
        int extension = givesCheck ? 1 : 0;
        int newDepth = depth - 1 + extension;

        if (legalMoves == 1) {
            score = -alphaBeta(board, newDepth, -beta, -alpha, ply + 1);
        }
        else {
            // Late Move Reductions
            int reduction = 0;
            
            if (depth >= 3 && legalMoves >= 2 && !isCapture && !isPromotion && !inCheck && !givesCheck) {
                reduction = lmrTable_[depth][std::min(legalMoves, 255)];
                
                if (isKiller) {
                    reduction -= 1;
                }
                else {
                    int piece = board.pieceList[getMoveSource(move)];
                    int to = getMoveTarget(move);
                    int history = historyTable_[piece][to];
                    
                    reduction -= std::max(-2, std::min(2, history / 4096));
                }

                reduction = std::max(0, std::min(reduction, depth - 1));
            }

            score = -alphaBeta(board, newDepth - reduction, -alpha - 1, -alpha, ply + 1);

            if (reduction > 0 && score > alpha) {
                score = -alphaBeta(board, newDepth, -alpha - 1, -alpha, ply + 1);
            }

            if (score > alpha && score < beta) {
                score = -alphaBeta(board, newDepth, -beta, -alpha, ply + 1);
            }
        }

        board.unmakeMove();

        if (stopSearch_) return 0;

        if (score >= beta) {
            // History Gravity
            int bonus = depth * depth;
            if (bonus > 400) bonus = 400;

            for (int j = 0; j < numQuiets; j++) {
                int qMove = quietMovesSearched[j];
                int qPiece = board.pieceList[getMoveSource(qMove)];
                int qTo = getMoveTarget(qMove);
                
                if (qMove == move) {
                    historyTable_[qPiece][qTo] += bonus - historyTable_[qPiece][qTo] * bonus / 8192;
                }
                else {
                    historyTable_[qPiece][qTo] -= bonus + historyTable_[qPiece][qTo] * bonus / 8192;
                }
            }

            // Killers and Countermoves
            if (!isCapture && !isPromotion) {
                killers_[1][ply] = killers_[0][ply];
                killers_[0][ply] = move;

                if (board.historyPtr > 0) {
                    int prevMove = board.history[board.historyPtr - 1].move;
                    if (prevMove != 0) {
                        int prevTo = getMoveTarget(prevMove);
                        int prevPiece = board.getPieceOnSquare(prevTo);
                        if (prevPiece != EMPTY) {
                            counterMoves_[prevPiece][prevTo] = move;
                        }
                    }
                }
            }

            tt_[board.zobristHash & (TT_SIZE - 1)] = { board.zobristHash, beta, move, depth, TT_LOWER };
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMoveFound = list.moves[i];
        }
    }

    if (legalMoves == 0) {
        int kingSq = getLSB(board.pieceBoards[board.sideToMove == WHITE ? wK : bK]);
        return isSquareAttacked(board, kingSq, board.sideToMove ^ 1)
               ? -INF + ply   // checkmate
               : 0;           // stalemate
    }

    int scoreToStore = alpha;
    if (scoreToStore >  INF - 200) scoreToStore += ply;
    if (scoreToStore < -INF + 200) scoreToStore -= ply;

    int flag = (alpha <= originalAlpha) ? TT_UPPER : TT_EXACT;
    tt_[board.zobristHash & (TT_SIZE - 1)] = { board.zobristHash, scoreToStore, bestMoveFound, depth, flag };

    return alpha;
}

int Search::findBestMove(Board& board, int softLimitMs, int hardLimitMs) {
    startTime_      = std::chrono::high_resolution_clock::now();
    softLimit_      = softLimitMs;
    hardLimit_      = hardLimitMs;
    stopSearch_     = false;
    nodesProcessed_ = 0;

    std::memset(killers_, 0, sizeof(killers_));
    std::memset(historyTable_, 0, sizeof(historyTable_));
    std::memset(counterMoves_, 0, sizeof(counterMoves_));

    int bestMove           = 0;
    int prevGoodMove       = 0;
    int lastCompletedScore = 0;

    for (int depth = 1; depth < MAX_DEPTH; depth++) {
        auto current_now = std::chrono::high_resolution_clock::now();
        auto current_ms  = std::chrono::duration_cast<std::chrono::milliseconds>(current_now - startTime_).count();
        if (depth > 1 && current_ms >= softLimit_ / 2) {
            break;
        }

        MoveList list;
        generateAllMoves(board, &list);

        int scores[256];
        for (int i = 0; i < list.count; i++)
            scores[i] = getMoveScore(board, list.moves[i], prevGoodMove, 0);

        for (int i = 0; i < list.count; i++) {
            for (int j = i + 1; j < list.count; j++) {
                if (scores[j] > scores[i]) {
                    std::swap(scores[i],     scores[j]);
                    std::swap(list.moves[i], list.moves[j]);
                }
            }
        }

        int alpha = -INF;
        int beta  = INF;
        int delta = 50;

        // Setup Aspiration Window
        if (depth >= 4) {
            alpha = std::max(-INF, lastCompletedScore - delta);
            beta  = std::min( INF, lastCompletedScore + delta);
        }

        int goodMove           = 0;
        int bestScoreThisDepth = -INF;

        while (true) {
            goodMove           = 0;
            bestScoreThisDepth = -INF;
            
            int currentAlpha = alpha;
            int currentBeta  = beta;

            for (int i = 0; i < list.count; i++) {
                if (!board.makeMove(list.moves[i])) continue;

                int score;
                
                if (bestScoreThisDepth == -INF) {
                    score = -alphaBeta(board, depth - 1, -currentBeta, -currentAlpha, 1);
                }
                else {
                    score = -alphaBeta(board, depth - 1, -currentAlpha - 1, -currentAlpha, 1);
                    
                    if (score > currentAlpha && score < currentBeta) {
                        score = -alphaBeta(board, depth - 1, -currentBeta, -currentAlpha, 1);
                    }
                }

                board.unmakeMove();

                if (stopSearch_) break;

                if (score > bestScoreThisDepth) {
                    bestScoreThisDepth = score;
                    goodMove = list.moves[i];
                }
                
                if (score > currentAlpha) {
                    currentAlpha = score;
                }
            }

            if (stopSearch_) break;

            // Aspiration Window check
            if (bestScoreThisDepth <= alpha) {
                // Fail low
                alpha = std::max(-INF, alpha - delta);
                delta += delta / 2; // Expand delta logarithmically
            }
            else if (bestScoreThisDepth >= beta) {
                // Fail high
                beta = std::min(INF, beta + delta);
                delta += delta / 2; 
            }
            else {
                // Exact score
                break;
            }
        }

        if (!stopSearch_) {
            bestMove           = goodMove;
            prevGoodMove       = goodMove;
            lastCompletedScore = bestScoreThisDepth;

            auto now = std::chrono::high_resolution_clock::now();
            auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count();

            std::cout << "info depth " << depth
                      << " score cp "  << lastCompletedScore
                      << " time "      << ms
                      << " pv "        << moveToString(bestMove)
                      << std::endl;
        }
        else {
            break;
        }
    }

    return bestMove;
}
