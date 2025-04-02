# test_assingment

All the code is in main.cpp divided into functions, no classes were made since it wasnt asked in the assignment

## Solution:  Using linear algebra's Gauss elimination for Ax=b.
              I'll explain what A and b are but first what our vectors here represent?
              A vector represents the matrix we`re working with in a flattened out state, 
              so a x by y input can be represented by a single vector of size x*y.
              - b is the initial state of input. see calcInitState() function
              - A is more interesting, each line of A represents an effect of a toggle,  
              of a single cell on a 0 matrix (see precalculateToggleEffects for specifics)
              - x represents the toggles required to solve the problem,
              in the same vector format.
              
               Why the problem of solving Ax = b is the same problem ?
               The question 'for which x Ax = b' essentially asks what toggles do we need
               to perform on the zero matrix so that we get b (initial state).
               This is derived from the definition of A and x and the matrix multiplication definition.
               The order of toggles doesnt matter so its the same as asking which toggles 
               turn b into the 0 matrix.

## Building

I build VS2022 project files using premake which I put here for convenience.\
I run `premake\premake5.exe vs2022`
