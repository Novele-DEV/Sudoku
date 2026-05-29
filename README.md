# Sudoku Solver — Generalised N×N Engine

A high-performance, generalised Sudoku solver written in C++ supporting grids from 1×1 up to **25×25**. Built around a constraint-propagation engine with bitmask candidate tracking and a Minimum Remaining Values (MRV) heuristic-guided backtracking search.

---

## Features

- **Generalised grid support** — handles any valid N×N grid where N has an integer square root (4×4, 9×9, 16×16, 25×25)
- **Bitmask constraint propagation** — row, column, and block candidate sets maintained as `uint64_t` bitmasks for O(1) constraint checks
- **MRV heuristic** — always branches on the cell with the fewest remaining candidates, dramatically reducing the search space
- **Naked & hidden singles elimination** — pre-solve propagation fills deterministic cells before any backtracking begins (see earlier version)
- **Character-encoded input** — digits `1–9` and letters `A–P` cover values 1–25, enabling compact representation of large grids
- **Dynamic allocation** — grid and mask arrays sized at runtime; no fixed buffer limits

---

## Algorithm Overview

### 1. Input Parsing
The solver reads an N×N grid from stdin, inferring the grid size from the number of tokens on the first line. Cells are encoded as:

| Input | Value |
|-------|-------|
| `0` or `.` | Empty cell |
| `1`–`9` | Values 1–9 |
| `A`–`P` | Values 10–25 |

### 2. Constraint Initialisation
Before solving, the initial grid is validated. Each placed value sets a bit in its row, column, and block bitmask. A conflict at this stage immediately returns `No Solution`.

### 3. MRV-Guided Backtracking (`solveMRV`)
The core solver loop:
1. Scans all empty cells to find the one with the minimum number of valid candidates (`fullMask & ~(rowMask | colMask | blockMask)`)
2. If any cell has zero candidates, the current branch is abandoned immediately
3. If no empty cells remain, the puzzle is solved
4. Iterates candidate values using bit manipulation (`choices & -choices` to isolate the lowest set bit), placing and unplacing each via bitmask updates

This approach avoids explicit candidate lists — all constraint information lives in three arrays of `uint64_t`.

---

## Complexity

| Phase | Behaviour |
|-------|-----------|
| Constraint check | O(1) per cell via bitmask |
| MRV scan | O(N²) per branching step |
| Worst-case search | Exponential, bounded in practice by MRV pruning |

For well-formed 9×9 puzzles, the solver typically resolves without any backtracking due to naked/hidden singles propagation.

---

## Building

Requires a C++11-compatible compiler.

```bash
g++ -O2 -o sudoku sudoku.cpp
```

For best performance on GCC/Clang (enables `__builtin_popcountll` and `__builtin_ctzll`):

```bash
g++ -O2 -march=native -o sudoku sudoku.cpp
```

---

## Usage

Pipe a grid to stdin. Cells are space-separated, rows are newline-separated.

```bash
./sudoku < puzzle.txt
```

### Example — 9×9

**Input (`puzzle.txt`):**
```
5 3 0 0 7 0 0 0 0
6 0 0 1 9 5 0 0 0
0 9 8 0 0 0 0 6 0
8 0 0 0 6 0 0 0 3
4 0 0 8 0 3 0 0 1
7 0 0 0 2 0 0 0 6
0 6 0 0 0 0 2 8 0
0 0 0 4 1 9 0 0 5
0 0 0 0 8 0 0 7 9
```

**Output:**
```
5 3 4 6 7 8 9 1 2
6 7 2 1 9 5 3 4 8
1 9 8 3 4 2 5 6 7
8 5 9 7 6 1 4 2 3
4 2 6 8 5 3 7 9 1
7 1 3 9 2 4 8 5 6
9 6 1 5 3 7 2 8 4
2 8 7 4 1 9 6 3 5
3 4 5 2 8 6 1 7 9
```

If no solution exists, the solver outputs:
```
No Solution
```

### Example — 16×16

For a 16×16 grid, values 10–15 are encoded as `A`–`F`:

```
1 2 3 4 5 6 7 8 9 A B C D E F G
...
```

---

## File Structure

```
sudoku-solver/
├── sudoku.cpp      # Full solver implementation
└── README.md
```

---

## Implementation Notes

- **Portability** — `__builtin_popcountll` and `__builtin_ctzll` are used for bit counting and trailing-zero detection. These are available on GCC and Clang; a fallback loop is trivial to substitute for MSVC.
- **Memory** — all heap allocations are freed before exit. Grid and mask arrays are allocated once at runtime based on the parsed grid size.
- **No STL containers in the hot path** — the solver uses raw arrays throughout to minimise overhead during recursive search.

---

## Author

**Ailton Ilidio Eugenio Novele**  
BSc Computer Science & Computational Applications — University of the Witwatersrand  
ailtonnovele24@gmail.com
