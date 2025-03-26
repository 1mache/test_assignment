#include <iostream>
#include <vector>
#include <random>
#include <time.h>

#ifdef DEBUG
#include <windows.h>
#include <conio.h>
constexpr int SLEEP_AMNT = 10;
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

void openLine(SecureBox& box ,uint32_t lineId ,const std::vector<bool>& lineData)
{
    uint32_t i = 0;
    // kind of a sliding window
    bool left  = false, 
         mid   = lineData[i], 
         right = lineData.size() > 1 ? lineData[i+1] : false;

    for( ; i < lineData.size(); i++)
    {   
        //       l m r
        // [..., 0,1,1,...]
        if (!left && mid && right
                  && i > 0) // and if its not l[m,r,...]
            box.toggle(lineId, i - 1);
        
        //       l m r
        // [..., 1,1,0,...]
        else if (left && mid && !right)
            box.toggle(lineId, i);

        //       l m r
        // [..., 0,1,0,...]
        else if(!left && mid && !right)
        {
            if (i > 0)
                box.toggle(lineId, i - 1);

            box.toggle(lineId, i);
        }
        
        // update window
        left = lineData[i];
        mid = lineData[i+1];
        if (i >= lineData.size() - 2)
            right = false;
        else
            right = lineData[i+2];
    }
}

bool openBox(uint32_t y, uint32_t x)
{
    SecureBox box(y, x);

    
    using BoxData = decltype(box.getState());
    BoxData boxDataCopy = box.getState();

#ifdef DEBUG

    for (int row = 0; row < y; row++)
    {
        for (int col = 0; col < x; col++)
        {
            char printed = boxDataCopy[row][col] ? '1' : '0';
            std::cout << printed;
        }
        std::cout << std::endl;
    }
    waitForInput();
#endif 

    // iterate over the lines, bottom to top
    // since box.toggle() also toggles everything above the cell
    for(int64_t row = y-1; row >= 0; row--)
    {
        auto line = boxDataCopy[row];
        openLine(box, row, line);

#ifdef DEBUG
        waitForInput();
#endif // DEBUG
    }

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

