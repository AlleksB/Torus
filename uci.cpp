#include "uci.h"
#include "movegen.h"
#include <iostream>
#include <string>

std::string moveToString(int move) {
    const int from  = getMoveSource(move);
    const int to    = getMoveTarget(move);
    const int flags = getMoveFlags(move);

    std::string s;
    s += static_cast<char>('a' + (from % 8));
    s += static_cast<char>('1' + (from / 8));
    s += static_cast<char>('a' + (to   % 8));
    s += static_cast<char>('1' + (to   / 8));

    if (flags & MOVE_PROMOTION) {
        const int promo = flags & ~MOVE_CAPTURE;
        if      (promo == MOVE_PROMO_N) s += 'n';
        else if (promo == MOVE_PROMO_B) s += 'b';
        else if (promo == MOVE_PROMO_R) s += 'r';
        else if (promo == MOVE_PROMO_Q) s += 'q';
    }

    return s;
}

UCI::UCI() {
    board_.parseFEN(START_FEN);
}

void UCI::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream is(line);
        std::string command;
        is >> command;

        if      (command == "uci")        handleHandshake();
        else if (command == "isready")    handleIsReady();
        else if (command == "setoption")  {}
        else if (command == "ucinewgame") handleNewGame();
        else if (command == "position")   handlePosition(is);
        else if (command == "go")         handleGo(is);
        else if (command == "quit")       break;
    }
}

void UCI::handleHandshake() {
    std::cout << "id name Torus v1.0\n";
    std::cout << "id author Alexandru Benescu\n";

    std::cout << "option name Move Overhead type spin default 100 min 0 max 5000\n";
    std::cout << "option name Threads type spin default 1 min 1 max 128\n";
    std::cout << "option name Hash type spin default 16 min 1 max 1048576\n";

    std::cout << "uciok\n";
}

void UCI::handleIsReady() {
    std::cout << "readyok\n";
}

void UCI::handleNewGame() {
    board_.parseFEN(START_FEN);
    search_.clearTT();
}

void UCI::handlePosition(std::istringstream& is) {
    std::string token;
    is >> token;

    if (token == "startpos") {
        board_.parseFEN(START_FEN);
        is >> token;
    }
    else if (token == "fen") {
        std::string fen;
        while (is >> token && token != "moves")
            fen += token + ' ';
        board_.parseFEN(fen);
    }

    if (token == "moves") {
        while (is >> token) {
            int move = parseMove(token);
            if (move != 0) {
                board_.makeMove(move);
            }
            else {
                std::cout << "info string CRITICAL ERROR: move not understood " << token << '\n';
            }
        }
    }
}

void UCI::handleGo(std::istringstream& is) {
    int wtime = -1, btime = -1;
    int winc = 0, binc = 0;
    int movetime = -1;

    std::string token;
    while (is >> token) {
        if      (token == "wtime")    is >> wtime;
        else if (token == "btime")    is >> btime;
        else if (token == "winc")     is >> winc;
        else if (token == "binc")     is >> binc;
        else if (token == "movetime") is >> movetime;
    }

    int softLimit = 1000;
    int hardLimit = 1000;
    
    const int moveOverhead = 50; 

    if (movetime != -1) {
        softLimit = std::max(1, movetime - moveOverhead);
        hardLimit = softLimit;
    }
    else {
        int myTime = (board_.sideToMove == WHITE) ? wtime : btime;
        int myInc  = (board_.sideToMove == WHITE) ? winc  : binc;

        if (myTime != -1) {
            myTime = std::max(1, myTime - moveOverhead);
            
            softLimit = myTime / 30 + (myInc * 3) / 4;
            hardLimit = myTime / 10 + (myInc * 3) / 4;
            
            if (softLimit > hardLimit) softLimit = hardLimit;
        }
    }

    int bestMove = search_.findBestMove(board_, softLimit, hardLimit);
    std::cout << "bestmove " << moveToString(bestMove) << '\n';
}

int UCI::parseMove(const std::string& moveStr) {
    MoveList list;
    generateAllMoves(board_, &list);

    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        if (board_.makeMove(move)) {
            board_.unmakeMove();
            if (moveToString(move) == moveStr) return move;
        }
    }
    return 0;
}
