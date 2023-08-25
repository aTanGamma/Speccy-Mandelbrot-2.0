A 'simple' program for zx spectrum 48k to generate a hi-res mandlebrot set

Files required for proper building:

    - Main.z80 - Main code loop
    - Maths.z80 - Maths functions
    - Timer_Msg.z80 - Prints words on screen
    - Working_Msg.z80 - Prints a word on screen
    - Iterator.z80 - Logic for iterations
    - Interrupt_Stuff.z80 - Interrupt setup and timer
    - Title_Binary.bin - Title screen
    - Font.z80 - Font data
    - Scale_Packed.bin - Image of graph scale

Box drawing characters:

↑ ↓ → ← 

░ ▒ ▓ █ 

▀ ▄ ▌▐ ▬ 

║ │ ═ ─

╣ ╢ ╡ ┤ 

╠ ╟ ╞ ├ 

╗ ╖ ╕ ┐ 

╝ ╜ ╛ ┘ 

╔ ╓ ╒ ┌

╚ ╙ ╘ └

╦ ╥ ╤ ┬

╩ ╨ ╧ ┴

╬ ╫ ╪ ┼

Docs:

Credits:

    All Code and 'GFX' by

               |¯¯¯¯¯¯¯¯¯¯¯¯|
               |/¯¯¯|  |¯¯¯\|
                    |  |        /\       ¯\¯\ ¯||¯
        ____ /¯/    |  |       /\ \       |\ \ ||
       /    \ |     |  |      /  \ \      ||\ \||
       |\___| |     |  |     /¯¯¯¯\ \     || \ \|
        \____/\_   _|__|_  _/_    _\_\_  _||_ \_\  (or aTanGamma depending on the platorm)

    Images converted to Speccy format via 'Spectra Image Converter'


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
O----= Byte Plotter =---------------------------------------O
|                                                           |
|   This routine takes an xy byte-coord in bc and plots     |
|       the contents of a.                                  |
|                                                           |
|       - Push af                                           |
|                                                           |
|       - Get Y5, 4, 3, shift to position                   |
|       - Add X potition                                    |
|       - Send finished lo-byte to l                        |
|                                                           |
|       - Get Y 2, 1, 0                                     |
|       - Get Y7, 6 and shift to position                   |
|       - Add above two to get finished hi-byte             |
|       - Send to h                                         |
|                                                           |
|       - Pop af                                            |
|       - Put a at (hf)                                     |
|                                                           |
O----= Character Plotter =----------------------------------O
|                                                           |
|   Repeats the Byte Plotter with each line of a character  |
|                                                           |
O-----------------------------------------------------------O

O----= Interrupt Timer =------------------------------------------------O
|                                                                       |
|   Times how long the plot took to calculate                           |
|                                                                       |
|       - Set interrupt mode 2, point i to $39xx                        |
|       - On interrupt, Z80 jumpts through address at $39xx             |
|           All bytes in this page are $FF (48K Speccy only)            |
|       - Relative jump instruction at $FFFF, with first byte of ROM    |
|           Sends PC back to $FFF4 where another jump is waiting        |
|                                                                       |
|       Interrupt Main:                                                 |
|                                                                       |
|       - Push af                                                       |
|       - Increment 3-byte timer                                        |
|       - Pop af                                                        |
|       - Return from interrupt                                         |
|                                                                       |
O-----------------------------------------------------------------------O

O----= Speedhax =-------------------------------------------O
|                                                           |
|   1.  Screen is mirrored to halve calculations            |
|                                                           |
|   2.  If coord is in one of two blocks ↓, it's            |
|           guaranteed to iterate forever                   |
|                                                           |
|              (-0.3, 0.6) X─────┐                          |
|                          │     │                          |
|   (-0.5, 0.5) X──────────┼─────┤                          |
|               │          │     │                          |
|               │          │     │                          |
|               │          │     │                          |
|               │          │     │                          |
|               │          │     │                          |
|               └──────────┼─────X (0, -0.5)                |
|                          │     │                          |
|                          └─────X (0, -0.6)                |
|                                                           |
|   3.  If X^2 >= 4, exit without calculating Y^2:          |
|           Radius guaranteed to be too large               |
|                                                           |
|   4.  In multiplication routine, the top 2 bits of        |
|           any Num_B are always 0, so skip 2 shift/adds.   |
|                                                           |
|   5.  Multiplication loop unrolled (saved 30 minutes !!)  |
|                                                           |
|   6.  If Next X/Y >= 2, exit iterations early:            |
|           Next X^2 / Y^2 will be >4. (see 3)              |
|                                                           |
O-----------------------------------------------------------O