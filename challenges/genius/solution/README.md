Flag: CTF{game_genie_killed_the_nintendo_star}

For my solution, I create `sh;` in-memory using blocks, then jump to it by
modifying `memset()` to point to `system()` and adding an offset to the
argument.

# Game Genius codes

This requires two bytes of changes with the "Game Genius".

The first one changes `call memset` to `call system`:

    0x08048622 (or 0x622 on disk) 0x30 -> 0x20

    Code: AZZAZT

The second one changes the memset argument from the start of the board to the
offset where our code starts (0x15 in):

    0x0804954c (or 0x154c) 0xA0 -> 0xB5

    Code: SLGOGI

# The game

Once those two changes are made, all you have to do is spell out "sh;" at the
offset of 0x15 bytes into the gameboard. Easy, right?

Here is the solution is in 16 moves:

* as
* qaas
* eddds
* qdddds
* ds
* ddddds
* qaas
* eaaaas
* as
* ddddds
* edddds
* aaaaas
* eas
* s
* s
* s
* s
* s

That'll create the following grid after 13 moves (numbering the pieces in hex):

     C      B
     C      BB
    CC      BA
    8 99    AA
    8899    A6
    877     66
      77D   64
      22D    4
     22 DD 344
      1155 33
      1155 3

The important part is:

           <64
      22D    4
     22 DD 344
      >       

or

           <11
    0011100001
    0110110111
    00>       

If you take those as bytes, left to right and top to bottom, you get:

    11001110 00010110 11011100

We have to reverse each byte, because of how it's encoded in-memory:

    01110011 01101000 00111011

And that converts to:

    73 68 3B -> 'sh;'

Once that's done, just play out the game by repeatedly playing `s` till it's
over, at which point a shell should pop!
