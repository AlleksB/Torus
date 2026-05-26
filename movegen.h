#pragma once

#include "board.h"

bool isSquareAttacked(const Board& board, int sq, int attackerColor);

U64 getBishopAttacks(int sq, U64 occupancy);
U64 getRookAttacks(int sq, U64 occupancy);
U64 getQueenAttacks(int sq, U64 occupancy);

U64 rookAttacksSlow(int sq, U64 blocks);
U64 bishopAttacksSlow(int sq, U64 blocks);

void generateAllMoves(const Board& board, MoveList* list);
void generateCaptures(const Board& board, MoveList* list);

int seeCapture(const Board& board, int move);

void initAttackTables();
