#include "board.h"
#include "movegen.h"
#include <chrono>
#include <iomanip>
#include <iostream>

long long perft(Board& board, int depth) {
    if (depth == 0) return 1;

    long long nodes = 0;
    MoveList list;
    generateAllMoves(board, &list);

    for (int i = 0; i < list.count; i++) {
        if (board.makeMove(list.moves[i])) {
            nodes += perft(board, depth - 1);
            board.unmakeMove();
        }
    }
    return nodes;
}

void perftTest(Board& board, int depth) {
    std::cout << "Starting Perft Test (Depth " << depth << ")\n";

    auto start = std::chrono::high_resolution_clock::now();
    long long nodes = perft(board, depth);
    auto end   = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration<double>(end - start).count();
    double nps     = (seconds > 0.0) ? (nodes / seconds) : 0.0;

    std::cout << "------------------------------------------\n";
    std::cout << "Depth: " << depth  << "\n";
    std::cout << "Nodes: " << nodes  << "\n";
    std::cout << "Time:  " << std::fixed << std::setprecision(3) << seconds << " s\n";

    if (nps > 1'000'000)
        std::cout << "NPS:   " << std::setprecision(2) << nps / 1'000'000 << " MNPS\n";
    else
        std::cout << "NPS:   " << static_cast<long long>(nps) << "\n";

    std::cout << "------------------------------------------\n";
}
