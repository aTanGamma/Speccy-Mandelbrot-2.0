A 'simple' program for zx spectrum 48k to generate a hi-res mandlebrot set

O----= Memory Map =-----------------------------O
|                                               |
|   $0000   O-----------O   Speccy Rom          |
|           |           |                       |
|   $4000   O-----------O   Bitmap Memory       |
|           |           |                       |
|   $5800   O-----------O   Colour Attributes   |
|           |           |                       |
|   $5B00   O-----------O   Reserved            |
|           |           |                       |
|   $5CCB   O-----------O   Free Space          |
|           |           |                       |
|   $6000   O-----------O   Main Code           |
|           |           |                       |
|           ¦           ¦                       |
|           '           '                       |
O-----------------------------------------------O

O----= 16-bit Fixed Point =---------------------------------O
|                                                           |
|   Format:                                                 |
|                                                           |
|   0b  0000.0000 0000 0000                                 |
|       \__/ \____________/                                 |
|        |    |                                             |
|        |    +---> Fractional part                         |
|        +--------> Whole number part                       |
|                                                           |
|   Range:                                                  |
|                                                           |
|   0b  0111.1111 1111 1111 = 7.99975... = 7 + 4095/4096    |
|                                                           |
|   0b  0000.0000 0000 0000 = 0.0                           |
|                                                           |
|   0b  1000.0000 0000 0000 = -8.0                          |
|                                                           |
|   Maths:                                                  |
|                                                           |
|       Adding/Subtracting - As expected                    |
|                                                           |
|       Multiplication:                                     |
|                                                           |
|           - Multiply as 16-bit integer                    |
|           - Shift answer right 4 bits (Divide by 16)      |
|                                                           |
|       Division:                                           |
|                                                           |
|           - Load Num_A into the high word of 4 byte space |
|           - Divide Big_A by B                             |
|           - Shift result right 4 bits                     |
|                                                           |
O-----------------------------------------------------------O

O----= Multiplication Algorithm =---------------------------O
|                                                           |
|   Standard-ish shift/add algorithm:                       |
|                                                           |
|       - Load Num_A into memory, Num_A in a register       |
|   +---> Shift Num_B right (0 -> B7 ; B0 -> Carry)         |
|   |     +-> If Carry set, add contents of A to a Result   |
|   |   - Shift Num_A left 1 bit (A = 2*A)                  |
|   +---- Decrement loop counter                            |
|                                                           |
|       - Shift Result right 4 bits (Result = Result / 16)  |
|       - Take middle 2 bytes from Result as final answer   |
|       - Remaining upper nibble put in Acc for reference   |
|                                                           |
O-----------------------------------------------------------O

O----= Division Algorithm =-----------------------------------------------------------------O
|                                                                                           |
|   Long-division implementation:                                                           |
|                                                                                           |
|       - Make both numbers positive. Set flag for output conditioning                      |
|       - Store Num_A in high-word of working space                                         |
|   +---> Try subtracting Num_B from the top of Num_A (This will move down during the loop) |
|   |     +-> If underflow, add B back                                                      |
|   |     +-> If no underflow, toggle Answer bit in that position                           |
|   +---- Keep going down the length of A                                                   |
|       - Shift answer right 4 bits                                                         |
|                                                                                           |
|   Not used in this prg                                                                    |
|                                                                                           |
O-------------------------------------------------------------------------------------------O
