#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <time.h>

#ifdef DEBUG
#include <windows.h>
#include <conio.h>
constexpr int SLEEP_AMNT = 0;
#endif // DEBUG

#ifdef DEBUG
inline void gotoxy(uint32_t x, uint32_t y)
{
    std::cout.flush();
    COORD coord = { x, y }; // Create a COORD structure 
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord); // Move the cursor
}

inline void drawBool(uint32_t x, uint32_t y, bool b)
{
    gotoxy(x, y);
    char printed = b ? '1' : '0';
    std::cout << printed;
}

inline void waitForInput()
{
    while (true)
        if (_kbhit()) break;
}
#endif // DEBUG


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
#ifdef DEBUG
        if(shuffled)
        {
            drawBool(x, y, box[y][x]);
            Sleep(SLEEP_AMNT);
        }
#endif // DEBUG

        for (uint32_t i = 0; i < xSize; i++)
        { 
            box[y][i] = !box[y][i];
#ifdef DEBUG
            if(shuffled)
            {
                drawBool(i, y, box[y][i]);
                Sleep(SLEEP_AMNT);
            }
#endif // DEBUG
        }

        for (uint32_t i = 0; i < ySize; i++)
        {
            box[i][x] = !box[i][x];
#ifdef DEBUG
            if(shuffled)
            {
                drawBool(x, i, box[i][x]);
                Sleep(SLEEP_AMNT);
            }
#endif // DEBUG
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
#ifdef DEBUG
    bool shuffled = false;
#endif // DEBUG

    //================================================================================
    // Method: shuffle
    // Description: Randomly toggles cells in the box to 
    // create an initial locked state.
    //================================================================================
    void shuffle()
    {
        for (uint32_t t = rng() % 1000; t > 0; t--)
            toggle(rng() % ySize, rng() % xSize);

#ifdef DEBUG
        shuffled = true;
#endif // DEBUG
    }
};

//================================================================================
// Function: openBox
// Description: Your task is to implement this function to unlock the SecureBox.
//              Use only the public methods of SecureBox (toggle, getState, isLocked).
//              You must determine the correct sequence of toggle operations to make
//              all values in the box 'false'. The function should return false if
//              the box is successfully unlocked, or true if any cell remains locked.
//================================================================================

bool openBoxManual(uint32_t y, uint32_t x)
{
    SecureBox box(y, x);

    auto boxDataCopy = box.getState();

#ifdef DEBUG

    for (uint32_t row = 0; row < y; row++)
    {
        for (uint32_t col = 0; col < x; col++)
        {
            char printed = boxDataCopy[row][col] ? '1' : '0';
            std::cout << printed;
        }
        std::cout << std::endl;
    }
#endif

    uint32_t tX = 0, tY = 0;
    bool status = false;
    std::cin >> status >> tY >> tX;

    while (status)
    {
        box.toggle(tY, tX);
        std::string clear(50, ' ');
        gotoxy(0, y);
        std::cout << clear;
        gotoxy(0, y);

        std::cin >> status >> tY >> tX;
    }

    return box.isLocked();
}

struct Cell
{
    uint32_t y, x;
    bool operator==(Cell other)
    {
        return (x == other.x) && (y == other.y);
    }
    bool operator!=(Cell other)
    {
        return !(*this == other);
    }
};

// Same as toggle but on a copy vector outside the box
// to keep track of changes done to the box
void toggleDataCopy(std::vector<std::vector<bool>>& boxData, Cell cell)
{
    auto x = cell.x, y = cell.y;

    boxData[y][x] = !boxData[y][x];

    for (uint32_t i = 0; i < boxData[0].size(); i++)
    {
        boxData[y][i] = !boxData[y][i];
    }

    for (uint32_t i = 0; i < boxData.size(); i++)
    {
        boxData[i][x] = !boxData[i][x];
    }
}

bool openBoxRec(SecureBox& box, std::stack<Cell>& toggles, 
                std::vector<std::vector<bool>>& boxData,
                uint64_t depth)
{
    if (!box.isLocked())
        return false;
    if (depth > boxData.size() * boxData[0].size())
        return true;

    for (uint32_t row = 0; row < boxData.size(); row++)
    {
        for (uint32_t col = 0; col < boxData[0].size(); col++)
        {
            Cell cell{ row, col };
            // no point in changing the same cell twice in a row
            if (toggles.empty() || toggles.top() != cell)
            {
                box.toggle(row, col);          // internal box toggle
                toggleDataCopy(boxData, cell); // tracking toggle
                toggles.push({ row, col });
                // if we couldn`t open the box recursively
                if (openBoxRec(box, toggles, boxData, depth + 1))
                {
                    toggles.pop();                 // pop last toggle
                    box.toggle(cell.y, cell.x);    // revert it (its this iteration`s cell)
                    toggleDataCopy(boxData, cell); // revert the effect on the copy
                }
                else return false;
            }
        }
    }

    return true;
}

bool openBox(uint32_t y, uint32_t x)
{
    SecureBox box(y, x);
    auto boxDataCopy = box.getState();

#ifdef DEBUG

    for (uint32_t row = 0; row < y; row++)
    {
        for (uint32_t col = 0; col < x; col++)
        {
            char printed = boxDataCopy[row][col] ? '1' : '0';
            std::cout << printed;
        }
        std::cout << std::endl;
    }
#endif

    std::stack<Cell> toggleStack;
    openBoxRec(box, toggleStack, boxDataCopy,0);

    return box.isLocked();
}


int main(int argc, char* argv[])
{
    //DEBUG:
    uint32_t y = 10;
    uint32_t x = 10;

    //uint32_t y = std::atol(argv[1]);
    //uint32_t x = std::atol(argv[2]);
    
    bool state = openBox(y, x);

    if (state)
        std::cout << "BOX: LOCKED!" << std::endl;
    else
        std::cout << "BOX: OPENED!" << std::endl;

    return state;
}

