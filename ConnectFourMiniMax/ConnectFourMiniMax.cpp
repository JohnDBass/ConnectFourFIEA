
#include <iostream>
#include <vector>
#include <array>;

using std::vector;
using std::array;
using std::cout;
using std::cin;
using std::endl;


void CreateBoard();
void PlayGame();
const void PrintBoard(vector<vector<int>>&);
void MakeChosenMove(vector<vector<int>>&, int, int);
int PlayerMove();
int AIMove();
array<int, 2> MiniMax(vector<vector<int>>&, const int, int, int, int);
int TabScore(vector<vector<int>>&, int);
int HeuristicScoreOfSet(vector<int>, int);
vector<vector<int>>CopyBoard(vector<vector<int>>);
const bool IsThereAWinner(vector<vector<int>>&, int);


const int TotalRows = 6;	//Board Height
const int TotalColumns = 7; //Board Width
const int MaxTurns = TotalRows * TotalColumns;
const int Player = 1;	//Player ID
const int AI = 2;		//Computer ID
const int MaxTreeDepth = 5;		//Maximum search tree depth for the MiniMax() algorithm
vector<vector<int>> Board(TotalRows, vector<int>(TotalColumns)); //Game Board
bool GameOver = false;
bool MaxTurnsReached = false;
int WinningPlayer = 0;
int CurrentPlayer = Player;
int CurrentTurn = 0;


/*	Initialize an 'empty' board or reset the board to this initial state
*/
void CreateBoard()
{
	for (int row = 0; row < TotalRows; row++)
	{
		for (int col = 0; col < TotalColumns; col++)
		{
			Board[row][col] = 0;
		}
	}
}

/*	Main game loop where we swap between player and AI moves, check for win/draw states,
		and optionally restart the game
*/
void PlayGame()
{
	PrintBoard(Board); 
	while (!GameOver) 
	{
		//If we've reached the max number of turns, the game is a draw
		if (CurrentTurn >= MaxTurns)	
		{
			MaxTurnsReached = true;
			GameOver = true;
		}
		else if (CurrentPlayer == Player)
		{
			MakeChosenMove(Board, PlayerMove(), Player);
		}
		else if (CurrentPlayer == AI)
		{
			MakeChosenMove(Board, AIMove(), AI);
		}
		GameOver = IsThereAWinner(Board, CurrentPlayer);
		if (GameOver) 
		{ 
			WinningPlayer = CurrentPlayer; 
		}
		//Flip CurrentPlayer between the AI and Player IDs to switch turns
		CurrentPlayer = (CurrentPlayer == 1) ? 2 : 1;
		CurrentTurn++;
		cout << endl;
		PrintBoard(Board);
	}
	if (MaxTurnsReached)
	{
		cout << "The Game is a Draw!" << endl;
	}
	else 
	{
		cout << "The Game is Over! ";
		cout << ((WinningPlayer == Player) ? "You Win!" : "The AI Wins!") << endl;
	}
	char RestartChoice;
	cout << "Would you like to play again? (y or n)" << endl;
	cin >> RestartChoice;
	if (!std::cin.fail() && (RestartChoice == 'y' || RestartChoice == 'Y'))
	{
		GameOver = false;
		CurrentPlayer = 1;
		CurrentTurn = 0;
		CreateBoard();
		PlayGame();
	}
}

/*	Prints to the command line the contents of the passed Board vector
		from the final row down to the first row - i.e. from Row[5] to Row[0] 
*/
const void PrintBoard(vector<vector<int>> &Board)
{
	cout << "----Columns----" << endl;
	for (int i = 1; i <= TotalColumns; i++)
	{
		cout << " " << i;
	}
	cout << endl << "---------------" << endl;
	for (int row = 0; row < TotalRows; row++)
	{
		for (int col = 0; col < TotalColumns; col++)
		{
			cout << "|";
			switch (Board[TotalRows - row - 1][col])
			{
				case 0: cout << " "; break;  
				case 1: cout << "0"; break;		
				case 2: cout << "X"; break;		
			}
			if (col + 1 == TotalColumns)
			{
				cout << "|";
			}
		}
		cout << endl;
	}
	cout << "---------------" << endl;
	cout << endl;
}

/*	Place token into the lowest available row of the column chosen by CurrentPlayer
*/
void MakeChosenMove(vector<vector<int>> &Board, int ChosenColumn, int CurrentPlayer)
{
	/*	Check for unlikely scenario where the AI has reached max depth and returns the default (invalid) move
		See MiniMax() function, Line 199
	*/
	if (ChosenColumn < 0 || ChosenColumn > TotalColumns)
	{
		cout << "You have confounded the AI" << endl;
		return;
	}
	for (int row = 0; row < TotalRows; row++)
	{
		if (Board[row][ChosenColumn] == 0)
		{
			Board[row][ChosenColumn] = CurrentPlayer;
			break;
		}
	}
}

/*	Prompt the user for a column, then sanitize that input by checking for invalid entries or columns
*/
int PlayerMove()
{
	int TempMove = -1;
	while (true)
	{
		cout << "Please choose a column: ";
		cin >> TempMove;
		//Check if user input is not an integer
		if (!cin) 
		{
			cin.clear();
			cin.ignore(INT_MAX, '\n');
			cout << "Please choose a value from 1 to " << TotalColumns << endl;
		}
		//Check if input is outside the bounds of the board
		else if (TempMove < 1 || TempMove > TotalColumns)	
		{
			cout << "Please choose a valid column: " << endl;
		}
		//Check if the chosen column is full
		else if (Board[TotalRows - 1][TempMove - 1] != 0) 
		{
			cout << "The chosen column is full, please choose again: " << endl;
		}
		else	
		{
			break;
		}
		cout << endl << endl;
	}
	return TempMove - 1;
}

/*	Serves as the first call of the MiniMax recursive algorithm
*/
int AIMove()
{
	cout << "AI is choosing a column..." << endl;
	int AIColumn = MiniMax(Board, MaxTreeDepth, INT_MIN, INT_MAX, AI)[1];
	cout << "AI has chosen column: " << AIColumn + 1 << endl;
	return AIColumn;
}

/*	The MiniMax algorithm generates a decision tree of potential future moves, performing a Depth First Search,
		with an initial max depth equal to MaxTreeDepth. The algorithm will score each node using the values contained in HeuristicScoreOfSet(), 
		alternatively maximizing or minimizing the score based on the current PlayerToCheck - Depth 1 maximizes the potential AI moves by 
		comparing their scores to INT_MIN, Depth 2 minimizes potential player moves by comparing those scores to INT_MAX, and so on until 
		we reach max terminal depth or the last possible turn of the game.
	The algorithm also utilizes Alpha Beta pruning to reduce the total number of node comparisons, saving time and memory by breaking the 
		recursion early when a greater (maximizing) or smaller (minimizing) score is found.
*/
array<int, 2> MiniMax(vector<vector<int>> &Board, const int CurrentDepth, int Alpha, int Beta, int PlayerToCheck)
{
	/*	If the algorithm is called at its terminal depth or at the max turn limit, return heuristic value of this board state early.
		In the unlikely scenario that this node is chosen as the AI's move (with an invalid column of -1), MakeChosenMove() contains 
			logic that essentially skips the AI turn
	*/
	if (CurrentDepth == 0 || CurrentDepth >= MaxTurns - CurrentTurn)
	{
		return array<int, 2>{ TabScore(Board, AI), -1};
	}
	// Maximizing AI moves
	if (PlayerToCheck == AI)
	{
		array<int, 2> CurrentMoveScore = { INT_MIN, -1 };
		// If the player is set to win the game, we want to minimize the AI score at this decision node to promote a block
		if (IsThereAWinner(Board, Player))
		{
			return CurrentMoveScore;
		}
		for (int col = 0; col < TotalColumns; col++)
		{
			if (Board[TotalRows - 1][col] == 0)
			{
				vector<vector<int>> BoardCopy = CopyBoard(Board);
				MakeChosenMove(BoardCopy, col, PlayerToCheck);
				int Score = MiniMax(BoardCopy, CurrentDepth - 1, Alpha, Beta, Player)[0];
				if (Score > CurrentMoveScore[0])
				{
					CurrentMoveScore = { Score, col };
				}
				// Alpha pruning
				Alpha = std::max(Alpha, CurrentMoveScore[0]);
				if (Alpha >= Beta)
				{
					break;
				}
			}
		}
		return CurrentMoveScore;
	}
	// Minimizing Player moves
	else
	{
		array<int, 2> CurrentMoveScore = { INT_MAX, -1 };
		if (IsThereAWinner(Board, AI))
		{
			return CurrentMoveScore;
		}
		for (int col = 0; col < TotalColumns; col++)
		{
			if (Board[TotalRows - 1][col] == 0)
			{
				vector<vector<int>> BoardCopy = CopyBoard(Board);
				MakeChosenMove(BoardCopy, col, PlayerToCheck);
				int Score = MiniMax(BoardCopy, CurrentDepth - 1, Alpha, Beta, AI)[0];
				if (Score < CurrentMoveScore[0])
				{
					CurrentMoveScore = { Score, col };
				}
				// Beta pruning
				Beta = std::min(Beta, CurrentMoveScore[0]);
				if (Beta <= Alpha)
				{
					break;
				}
			}
		}
		return CurrentMoveScore;
	}
}

/*	Calculates current board 'value' for the chosen player by scoring every combination of 4 slots on the board
*/
int TabScore(vector<vector<int>> &Board, int PlayerToCheck)
{
	int Score = 0;
	vector<int> SlotsToScore(4);
	// Horizontal Checks
	for (int col = 0; col < TotalColumns - 3; col++)
	{
		for (int row = 0; row < TotalRows; row++)
		{
			for (int i = 0; i < 4; i++)
			{
				SlotsToScore[i] = Board[row][col + i];
			}
			Score += HeuristicScoreOfSet(SlotsToScore, PlayerToCheck);
		}
	}
	// Vertical Checks
	for (int col = 0; col < TotalColumns; col++)
	{
		for (int row = 0; row < TotalRows - 3; row++)
		{
			for (int i = 0; i < 4; i++)
			{
				SlotsToScore[i] = Board[row + i][col];
			}
			Score += HeuristicScoreOfSet(SlotsToScore, PlayerToCheck);
		}
	}
	// Upward Diagonal Checks
	for (int col = 0; col < TotalColumns - 3; col++)
	{
		for (int row = 0; row < TotalRows - 3; row++)
		{
			for (int i = 0; i < 4; i++)
			{
				SlotsToScore[i] = Board[row + i][col + i];
			}
			Score += HeuristicScoreOfSet(SlotsToScore, PlayerToCheck);
		}
	}
	// Downward Diagonal Checks
	for (int col = 0; col < TotalColumns - 3; col++)
	{
		for (int row = 3; row < TotalRows; row++)
		{
			for (int i = 0; i < 4; i++)
			{
				SlotsToScore[i] = Board[row - i][col + i];
			}
			Score += HeuristicScoreOfSet(SlotsToScore, PlayerToCheck);
		}
	}
	return Score;
}

/*	Generates a score for a set of four pieces -SlotsToScore- on the board from the perspective of PlayerToCheck by
		incentivizing a winning move or a stronger board state, and avoiding a potential win for the opponent.
*/
int HeuristicScoreOfSet(vector<int> SlotsToScore, int PlayerToCheck)
{
	int HeuristicScore = 0;
	int BadPiece = Player;
	if (PlayerToCheck == Player)
	{
		BadPiece = AI;
	}
	int MySlots = std::count(SlotsToScore.begin(), SlotsToScore.end(), PlayerToCheck);
	int OpponentSlots = std::count(SlotsToScore.begin(), SlotsToScore.end(), BadPiece);
	int EmptySlots = std::count(SlotsToScore.begin(), SlotsToScore.end(), 0);

	if (MySlots == 4) { HeuristicScore += 100; }
	else if (MySlots == 3 && EmptySlots == 1) { HeuristicScore += 10; }
	else if (MySlots == 2 && EmptySlots == 2) { HeuristicScore += 2; }
	else if (OpponentSlots == 3 && EmptySlots == 1) { HeuristicScore += -8; }
	return HeuristicScore;
}

/*	Return a copy of the current board state.
	Used extensively in the MiniMax algorithm to mutate potential board states without touching the main Board
*/
vector<vector<int>> CopyBoard(vector<vector<int>> Board)
{
	vector<vector<int>> BoardCopy(TotalColumns, vector<int>(TotalColumns));
	for (int row = 0; row < TotalRows; row++)
	{
		for (int col = 0; col < TotalColumns; col++)
		{
			BoardCopy[row][col] = Board[row][col];
		}
	}
	return BoardCopy;
}

/*	Check for a winner by looping through all horizontal, vertical, and diagonal combinations of 4 adjacent pieces
*/
const bool IsThereAWinner(vector<vector<int>> &Board, int CurrentPlayer)
{
	int AdjacentPieces = 0;
	// Check for 4 Horizontal Pieces
	for (int col = 0; col < TotalColumns - 3; col++)
	{
		for (int row = 0; row < TotalRows; row++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (Board[row][col + i] == CurrentPlayer) { AdjacentPieces++; }
				if (AdjacentPieces == 4) { return true; }
			}
			AdjacentPieces = 0;
		}
	}
	//Check for 4 Vertical Pieces
	for (int col = 0; col < TotalColumns; col++)
	{
		for (int row = 0; row < TotalRows - 3; row++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (Board[row + i][col] == CurrentPlayer) { AdjacentPieces++; }
				if (AdjacentPieces == 4) { return true; }
			}
			AdjacentPieces = 0;
		}
	}
	//Check for 4 diagonal downward pieces
	for (int col = 0; col < TotalColumns - 3; col++)
	{
		for (int row = 3; row < TotalRows; row++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (Board[row - i][col + i] == CurrentPlayer) { AdjacentPieces++; }
				if (AdjacentPieces == 4) { return true; }
			}
			AdjacentPieces = 0;
		}
	}
	//Check for 4 diagonal upward pieces 
	for (int col = 0; col < TotalColumns - 3; col++)
	{
		for (int row = 0; row < TotalRows - 3; row++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (Board[row + i][col + i] == CurrentPlayer) { AdjacentPieces++; }
				if (AdjacentPieces == 4) { return true; }
			}
			AdjacentPieces = 0;
		}
	}
	return false; //Not a winner
}

int main(int argc, char *argv[])
{
    CreateBoard();
	PlayGame();
	cout << "Thank You for Playing!";
	return 0;
}