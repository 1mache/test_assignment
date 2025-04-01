#include <iostream>
#include <vector>
#include <optional>
#include <random>
#include <time.h>


/*
You are given a locked container represented as a two-dimensional grid of boolean values (true = locked, false = unlocked).
Your task is to write an algorithm that fully unlocks the box, i.e.,
transforms the entire matrix into all false.

Implement the function:
bool openBox(uint32_t y, uint32_t x);
This function should:
    - Use the SecureBox public API (toggle, isLocked, getState).
    - Strategically toggle cells to reach a state where all elements are false.
    - Return true if the box remains locked, false if successfully unlocked.
You are not allowed to path or modify the SecureBox class.

Evaluation Criteria:
    - Functional correctness
    - Computational efficiency
    - Code quality, structure, and comments
    - Algorithmic insight and clarity
*/

class SecureBox
{
private:
    std::vector<std::vector<bool>> box;

public:

    //================================================================================
    // Constructor: SecureBox
    // Description: Initializes the secure box with a given size and 
    //              shuffles its state using a pseudo-random number generator 
    //              seeded with current time.
    //================================================================================
    SecureBox(uint32_t y, uint32_t x) : ySize(y), xSize(x)
    {
        rng.seed(time(0));
        box.resize(y);
        for (auto& it : box) 
            it.resize(x);
        shuffle();
    }

    //================================================================================
    // Method: toggle
    // Description: Toggles the state at position (x, y) and also all cells in the
    //              same row above and the same column to the left of it.
    //================================================================================
    void toggle(uint32_t y, uint32_t x)
    {
        box[y][x] = !box[y][x];

        for (uint32_t i = 0; i < xSize; i++)
        { 
            box[y][i] = !box[y][i];
        }

        for (uint32_t i = 0; i < ySize; i++)
        {
            box[i][x] = !box[i][x];
        }
    }

    //================================================================================
    // Method: isLocked
    // Description: Returns true if any cell 
    //              in the box is true (locked); false otherwise.
    //================================================================================
    bool isLocked()
    {
        for (uint32_t x = 0; x < xSize; x++)
            for (uint32_t y = 0; y < ySize; y++)
                if (box[y][x])
                    return true;

        return false;
    }

    //================================================================================
    // Method: getState
    // Description: Returns a copy of the current state of the box.
    //================================================================================
    std::vector<std::vector<bool>> getState()
    {
        return box;
    }

private:
    std::mt19937_64 rng;
    uint32_t ySize, xSize;

    //================================================================================
    // Method: shuffle
    // Description: Randomly toggles cells in the box to 
    // create an initial locked state.
    //================================================================================
    void shuffle()
    {
        for (uint32_t t = rng() % 1000; t > 0; t--)
            toggle(rng() % ySize, rng() % xSize);
    }
};

struct Cell
{
    uint32_t y, x;

    // Conversion to hashable type
    operator uint64_t() const {
        return (static_cast<uint64_t>(x) << 32) | y;
    }

    bool operator==(Cell other)
    {
        return (x == other.x) && (y == other.y);
    }
    bool operator!=(Cell other)
    {
        return !(*this == other);
    }
};

// translates a Cell object to an index for a flattened out matrix
uint64_t cellToId(Cell cell, uint32_t xSize)
{
    return cell.y * xSize + cell.x;
}

// Does the same that SecureBox::toggle does but with a vector of effects
std::vector<bool> calcToggleEffect(Cell toggled, uint32_t ySize, uint32_t xSize)
{
    uint64_t fSize = ySize * xSize;
    auto effectVector = std::vector<bool>(fSize);

    for (uint32_t i = 0; i < xSize; i++)
    {
        Cell c{ toggled.y, i };
        effectVector[cellToId(c, xSize)] = true;
    }

    for (uint32_t i = 0; i < ySize; i++)
    {
        Cell c{ i, toggled.x };
        effectVector[cellToId(c, xSize)] = true;
    }

    return effectVector;
}

// Calculates a matrix where line i is a flattened out matrix that represents the effects of a
// toggle on cell x, y (such that cellToId(y,x) = i) on the zero matrix
/* ==========Example:=======================================================================
*  A = 2x2 matrix, the effects of toggle of 0,0 are |1 1| (1 on cells that are affected)
*                                                   |1 0|
*  Flattened out vector of this will be (1, 1, 1, 0)
============================================================================================ */
std::vector<std::vector<bool>> precalculateToggleEffects(uint32_t ySize, uint32_t xSize)
{
    // flattened size
    uint64_t fSize = ySize * xSize;
    auto toggleMatrix = std::vector<std::vector<bool>>(fSize, std::vector<bool>(fSize));

    for (uint32_t row = 0; row < ySize; row++)
    {
        for (uint32_t col = 0; col < xSize; col++)
        {
            Cell cell{ row,col };

            toggleMatrix[cellToId(cell, xSize)] = calcToggleEffect(cell, ySize, xSize);
        }
    }

    return toggleMatrix;
}

// Returns a vector that represents the flattened matrix that is the boxData at its init. state
std::vector<bool> calcInitState(std::vector<std::vector<bool>> boxData)
{

    uint64_t fSize = boxData.size() * boxData[0].size();
    auto res = std::vector<bool>(fSize);

    uint32_t ySize = boxData.size(), xSize = boxData[0].size();

    for (uint32_t row = 0; row < ySize; row++)
    {
        for (uint32_t col = 0; col < xSize; col++)
            res[cellToId({ row, col }, xSize)] = boxData[row][col];
    }

    return res;
}

// Adds second vector to first using XOR which is + for bits
// assumes they're same size of course
void addBoolVectors(std::vector<bool>& to, const std::vector<bool>& added)
{
    for (uint32_t i = 0; i < added.size(); i++)
        to[i] = to[i] ^ added[i];
}

bool isZeroVector(const std::vector<bool>& v)
{
    for (auto b : v)
        if (b) return false;

    return true;
}

// Finds a row with a pivot at given position, searches starting at given row and downwards
// returns its index (std::optional)
std::optional<uint32_t> findPivotRow(const std::vector<std::vector<bool>>& A,
                                              uint32_t forColumn, uint32_t fromRow)
{
    for (auto i = fromRow; i < A.size(); i++)
    {
        auto& row = A[i];
        if (row[forColumn]) // true at given column => we found it
            return i;
    }
    // in our case shold never get here
    return std::nullopt; 
}

// Solves Ax = b. (A is square) if the solution exists returns x, if not, returns empty vector
// Note: we assume A can be multiplied by b so linear algebra laws are true and this function makes sense 
std::vector<bool> gaussianElimination(std::vector<std::vector<bool>>& A, std::vector<bool>&& b)
{
    uint32_t pivot = 0; // pivot in which column are we looking for
    // Note: since we're talking about square matrix A, the pivot also represents the id of the current line

    for (; pivot < A[0].size() - 1; pivot++)
    {
        // Note: we assume there will be a pivot for each line, in our case this should be 
        // true given the definition of our matrix. 
        auto pivotRowId = findPivotRow(A, pivot, pivot);

        // if we didnt find a pivot (this shouldnt happen in our case)
        if (!pivotRowId.has_value()) 
            break; // something went wrong, this is a debug breakpoint

        auto& pivotRow = A[pivotRowId.value()];
        // if the pivot wasnt on the current row, swap the rows
        if (pivotRowId != pivot)
        {
            std::swap(A[pivot], pivotRow);     // in A
            bool tmp = b[pivotRowId.value()];  // in b
            b[pivotRowId.value()] = b[pivot];  // (std::swap didnt work as intended here)
            b[pivot] = tmp;                    
        }

        // zero out every 1 below pivot
        for (uint32_t row = pivot + 1; row < A.size(); row++)
        {
            // if there is a 1 add rows which will zero it out
            if (A[row][pivot])
            {
                addBoolVectors(A[row], A[pivot]); // in A
                b[row] = b[row] ^ b[pivot];       // in b
            }
        }
    }

    // go over the pivots again in the reverse order
    for (; pivot >= 1; pivot--)
    {
        std::vector<bool>& pivotRow = A[pivot];

        // zero out every 1 above pivot
        for (int64_t row = pivot - 1; row >= 0; row--)
        {
            if (A[row][pivot])
            {
                addBoolVectors(A[row], pivotRow);
                b[row] = b[row] ^ b[pivot];
            }
        }
    }


    // Check for a contradiction: line i is all zeroes but the elem. i of b is 1 
    for (uint32_t i = 0; i < A.size(); i++) 
    {
        if (isZeroVector(A[i]) && b[i])
            return std::vector<bool>(0); // no solution exists 
    }

    return b;
}

//================================================================================
// Function: openBox
// Description: Your task is to implement this function to unlock the SecureBox.
//              Use only the public methods of SecureBox (toggle, getState, isLocked).
//              You must determine the correct sequence of toggle operations to make
//              all values in the box 'false'. The function should return false if
//              the box is successfully unlocked, or true if any cell remains locked.
//================================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Solution:  Using linear algebra's Gauss elimination for Ax=b.
//            I'll explain what A and b are but first what our vectors here represent?
//            A vector represents the matrix we`re working with in a flattened out state,
//            so a x by y input can be represented by a single vector of size x*y.
//            - b is the initial state of input. see calcInitState() function
//            - A is more interesting, each line of A represents an effect of a toggle, 
//              of a single cell on a 0 matrix (see precalculateToggleEffects for specifics)
//            - x represents the toggles required to solve the problem,
//              in the same vector format.
// 
//             Why the problem of solving Ax = b is the same problem ?
//             The question 'for which x Ax = b' essentially asks what toggles do we need
//             to perform on the zero matrix so that we get b (initial state).
//             This is derived from the definition of A and x and the matrix multiplication definition.
//             The orded 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool openBox(uint32_t y, uint32_t x)
{
    SecureBox box(y, x);
    auto boxDataCopy = box.getState();
    
    auto toggleEffectsMatrix = precalculateToggleEffects(y,x);

    auto solution = gaussianElimination(toggleEffectsMatrix, calcInitState(boxDataCopy));

    if (solution.empty())
        return true; // opening is not possible

    for (uint32_t row = 0; row < y; row++)
    {
        for (uint32_t col = 0; col < x; col++)
        {
            if(solution[cellToId({ row, col }, x)])
                box.toggle(row, col); // toggle cell if its in the solution
        }
    }
    
    std::cout << std::endl;
    return box.isLocked();
}


int main(int argc, char* argv[])
{
    uint32_t y = std::atol(argv[1]);
    uint32_t x = std::atol(argv[2]);
    
    bool state = openBox(y, x);

    if (state)
        std::cout << "BOX: LOCKED!" << std::endl;
    else
        std::cout << "BOX: OPENED!" << std::endl;

    return state;
}

