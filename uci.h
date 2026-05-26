#pragma once

#include "board.h"
#include "search.h"
#include <sstream>
#include <string>

class UCI {
public:
    UCI();

    void run();

private:
    Board  board_;
    Search search_;

    void handleHandshake();
    void handleIsReady();
    void handleNewGame();
    void handlePosition(std::istringstream& is);
    void handleGo(std::istringstream& is);

    int parseMove(const std::string& moveStr);
};
