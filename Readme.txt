A 'simple' program for zx spectrum 48k to generate a hi-res mandlebrot set

Files required for proper building:

    - Main.z80 - Main code loop
    - Maths.z80 - Maths functions

Box drawing characters:

░ ▒ ▓ █ │ ┤ ┘ ┌ ┐ └ ┴ ┬ ├ ─ ┼ ╣ ║ ╗ ╝ ╚ ╔ ╩ ╦ ╠ ═ ╬ ▄ ¦ ■

Docs:

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
|   Relatively standard shift/add routine                   |
|                                                           |
|       - Check the signs of 1 Num_A and Num_B              |
|       - Make both positive and count number of negatives  |
|                                                           |
|   +---> B >> 1 to C (Carry = B0)                          |
|   |     +-> If C = 1, add Num_A to Result                 |
|   |   - A << 1                                            |
|   +---- Decrement counter (from 16)                       |
|                                                           |
|       - Result >> 4                                       |
|       - No_Negatives >> 1                                 |
|       +-> If C = 1, negate answer:                        |
|           - xor $FFFFFFFF                                 |
|           - add 1                                         |
|       - Return answer in abc                              |
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

O----= Spectrum Screen Addresses =--------------------------O
|                                                           |
|   Addressing byte in screen memory for a given X and Y:   |
|                                                           |
|   Ob 0100 0000 0000 0000                                  |
|      \_/\_/\_/ \_/\____/                                  |
|       │  │  │   │  │                                      |
|       │  │  │   │  └────> X potition                      |
|       │  │  │   └───────> Y5, Y4, Y3                      |
|       │  │  └───────────> Y2, Y1, Y0                      |
|       │  └──────────────> Y6, Y7                          |
|       └─────────────────> Fixed @ 010                     |
|                                                           |
O-----------------------------------------------------------O