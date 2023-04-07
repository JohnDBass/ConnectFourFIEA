**ConnectFourMiniMax.cpp contains all functional code for this submission**

This is a ConnectFour command line game that pits you against an AI opponent that utilizes a MiniMax algorithm to choose an optimal move based on a Depth First Search of potential future board states. Board states are given a score based on an analysis of every grouping of 4 tokens on the board:

4 own tokens: Score + 100
3 own and 1 empty token: Score + 10
2 own and 2 empty tokens: Score + 2
3 opponent and 1 empty tokens: Score - 8

The algorithm alternates between creating a maximizing score: the highest possible board score for the AI, and a minimizing score: the lowest possible board score for the player. The algorithm also uses Alpha Beta pruning to greatly reduce the total number of function calls by breaking the recursive call early when a new high or low score is found.