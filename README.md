# Torus Chess Engine

Yet another chess engine written in C++.

It definitely won't beat Stockfish, but it runs fast and does talk standard UCI.

Hopefully it will get better at chess soon.

---

## Features
* Magic Bitboards
* Alpha-Beta Pruning
* Principal Variation Search (PVS)
* Quiescence Search
* Aspiration Windows
* Null Move Pruning
* Late Move Reductions (LMR)
* Futility Pruning
* Reverse Futility Pruning
* MVV-LVA (Most Valuable Victim - Least Valuable Aggressor)
* Killer Moves
* History Heuristic
* Zobrist Hashing
* PeSTO Evaluation
* Piece-Square Tables
* Static Exchange Evaluation (SEE)

---

## Build & Installation

### Prerequisites
* A C++ compiler supporting C++17 or later
* `make` build utility

### Compiling from Source
1. Clone the repository:
```bash
   git clone https://github.com/AlleksB/Torus.git
   cd Torus
```
2. Build the project using the Makefile:
```bash
   make
```
