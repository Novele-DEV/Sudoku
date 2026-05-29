#include <iostream>
#include <stack>
#include <cmath>
#include <utility>
#include <sstream>
#include <string>
#include <cstdint>
#include <climits>

using namespace std;

// Convert character to number
int charToNum(char c) {
    // 0 or '.' => empty
    if (c == '0' || c == '.') return 0;
    // digits 1-9
    if (c >= '1' && c <= '9') return c - '0';
    // letters A-P / a-p => 10..25 (supports 25x25)
    if (c >= 'A' && c <= 'P') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'p') return 10 + (c - 'a');
    return 0;
}

// Convert number to character
char numToChar(int num) {
    if (num <= 9) return num + '0';
    return num - 10 + 'A';
}

// popcount helper (portable-ish)
static inline int popcount64(uint64_t x) {
#if defined(__GNUG__) || defined(__clang__)
    return __builtin_popcountll(x);
#else
    // fallback
    int c = 0;
    while (x) { x &= x - 1; ++c; }
    return c;
#endif
}

// Recursive solver using bitmasks + MRV heuristic
bool solveMRV(int* grid, int n, uint64_t* rowMask, uint64_t* colMask, uint64_t* blockMask, uint64_t fullMask) {
    int k = (int) sqrt(n);

    // Find the empty cell with Minimum Remaining Values (MRV)
    int bestR = -1, bestC = -1;
    int bestCount = INT_MAX;
    uint64_t bestCandidates = 0;

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (grid[r * n + c] != 0) continue;
            int bidx = (r / k) * k + (c / k);
            uint64_t avail = fullMask & ~(rowMask[r] | colMask[c] | blockMask[bidx]);
            int cnt = popcount64(avail);
            if (cnt == 0) return false; // dead end
            if (cnt < bestCount) {
                bestCount = cnt;
                bestR = r;
                bestC = c;
                bestCandidates = avail;
                if (bestCount == 1) break; // can't do better
            }
        }
        if (bestCount == 1) break;
    }

    // If no empty cells -> solved
    if (bestR == -1) return true;

    int bidx = (bestR / k) * k + (bestC / k);

    // Try candidates (iterate set bits)
    uint64_t choices = bestCandidates;
    while (choices) {
        uint64_t pick = choices & -choices; // lowest set bit
        int num = __builtin_ctzll(pick) + 1; // num in [1..n]
        // place
        grid[bestR * n + bestC] = num;
        rowMask[bestR] |= pick;
        colMask[bestC] |= pick;
        blockMask[bidx] |= pick;

        if (solveMRV(grid, n, rowMask, colMask, blockMask, fullMask)) return true;

        // undo
        grid[bestR * n + bestC] = 0;
        rowMask[bestR] &= ~pick;
        colMask[bestC] &= ~pick;
        blockMask[bidx] &= ~pick;

        choices &= choices - 1; // clear lowest set bit
    }

    return false;
}

// Function to print the grid
void printGrid(const int* grid, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << numToChar(grid[i * n + j]);
            if (j < n - 1) cout << " ";
        }
        cout << endl;
    }
}

int main() {
    string line;
    int n = 0;
    int* grid = nullptr;
    int rows = 0;

    // Read first non-empty line to determine n and fill first row
    while (getline(cin, line)) {
        if (line.empty()) continue;

        // Count tokens in the line
        stringstream ss(line);
        string token;
        int count = 0;
        while (ss >> token) count++;

        if (count == 0) continue;

        n = count;
        // allocate grid
        grid = new int[n * n];
        // initialize to 0
        for (int i = 0; i < n * n; ++i) grid[i] = 0;

        // fill first row
        stringstream ss2(line);
        int col = 0;
        while (ss2 >> token && col < n) {
            grid[0 * n + col] = charToNum(token[0]);
            col++;
        }
        rows = 1;
        break;
    }

    // If we didn't get any input
    if (n == 0) {
        cout << "No Solution" << endl;
        return 0;
    }

    // Read remaining rows until we have n rows
    while (rows < n && getline(cin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token;
        int col = 0;
        while (ss >> token && col < n) {
            grid[rows * n + col] = charToNum(token[0]);
            col++;
        }
        rows++;
    }

    // Prepare masks
    uint64_t* rowMask = new uint64_t[n];
    uint64_t* colMask = new uint64_t[n];
    uint64_t* blockMask = new uint64_t[n];
    for (int i = 0; i < n; ++i) { rowMask[i] = colMask[i] = blockMask[i] = 0ULL; }

    uint64_t fullMask;
    if (n >= 64) fullMask = ~0ULL;
    else fullMask = (n == 64) ? ~0ULL : ((1ULL << n) - 1ULL);

    int k = (int) sqrt(n);
    bool validInitial = true;

    for (int r = 0; r < n && validInitial; ++r) {
        for (int c = 0; c < n && validInitial; ++c) {
            int val = grid[r * n + c];
            if (val == 0) continue;
            if (val < 1 || val > n) { validInitial = false; break; }
            uint64_t bit = 1ULL << (val - 1);
            int bidx = (r / k) * k + (c / k);
            if ((rowMask[r] & bit) || (colMask[c] & bit) || (blockMask[bidx] & bit)) {
                validInitial = false; break;
            }
            rowMask[r] |= bit;
            colMask[c] |= bit;
            blockMask[bidx] |= bit;
        }
    }

    bool solved = false;
    if (validInitial) {
        solved = solveMRV(grid, n, rowMask, colMask, blockMask, fullMask);
    }

    if (solved) printGrid(grid, n);
    else cout << "No Solution" << endl;

    delete[] grid;
    delete[] rowMask;
    delete[] colMask;
    delete[] blockMask;
    return 0;
}
