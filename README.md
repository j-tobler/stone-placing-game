# csse2310-assignment1
My first c assignment for uni.

## Game Instructions
The game is played on an R × C grid of cells (where R is the number of rows and C is the number of columns). The corners of the board are removed. Each cell has a point value. The value of the border cells is zero, interior cells have values between 1 and 9 (inclusive). Empty cells are indicated with a dot.

Players take turns placing “stones” on empty cells. The game ends when the interior of the board is full. Each player gets points for each cell they have a stone in.

Playing a stone on one of the edges, is different. Such stones will be pushed into the interior moving other stones as needed. This may result in a stone being pushed into the opposite edge.

Stones can only be played in an edge cell if there is an empty cell in the direction it would be pushed, and there is a stone to be pushed immediately next to the edge cell.

## Scoring
A player’s score at the end of the game is the total of the points for all cells which have that player’s stones in them.
