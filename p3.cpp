#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <cassert>

#include "p3.h"

/////////////////*My functions*//////////////////

void improp(void);
//EFFECTS: outputs "Improper argument." and exits program
//         if game conditions are not met

template <typename T>
T converter(const string a, const string b[], const int size, T dummy);
// EFFECTS: returns converted string to enum

void well_formed(int argc, char *argv[]);
// EFFECTS: checks if game meets starting requirements
//          else improp() is returned

void shuffler(Game &game, int round);
//REQUIRES: game is well-formed and game.deck is a valid deck
//MODIFIES: game
//EFFECTS:  Checks to see if you have ran out of cards and
//          shuffles the deck if you are, otherwise, nothing

bool iAmAlreadyThere(const Game &game, int square, Color player_num);
// REQUIRES: game is well-formed
// EFFECTS: returns true if there is an allied pawn at the destination square

void myFarthest(const Game &game, const CardType Draw, Outcome &drawn, const Color player, const int distance);
// REQUIRES: game is well-formed
// MODIFIES: drawn.pawnNum and drawn.canMove
// EFFECTS: finds farthest pawn from home square

void closestToHome(const Game &game, Outcome &drawn, const Color player);
// REQUIRES: game is well-formed
// MODIFIES: drawn.opponent.num drawn.opponent.color 
// EFFECTS: finds closest enemy pawn of a player
//          returns their color/pawnNum for SWAP or SORRY

void game_won(Game &game, Color player);
// REQUIRES: game is well-formed
// EFFECTS: checks for a game winner, if so game exits

void collisions(Game &game, int enemyP, int targsquare, int youP);
// REQUIRES: game is well-formed
// MODIFIES: if pawns collide of diff colors collide, adjustments made
//           if not, nothing happens
// EFFECTS: game

void sliding(Game &game, int youP, int enemyP, int &startSquare, Color slideColor);
// REQUIRES: game is well-formed
// MODIFIES: game and startSquare(beginning of slide)
// EFFECTS: slides a player and bumps any opponents along the way

void pawn_Home(Game &game, int square, int homeSquare, int player, int pawn);
// REQUIRES: game is well-formed
// MODIFIES: game
// EFFECTS: places moves for pawn to reach home, also checks to see if player has won

void Announce1(const Game &game, const Color player);
// REQUIRES: game is well-formed
// EFFECTS: performs announcements for eval()
//          announces card that was drawn

void Announce2(const Game &game, const Color player, const CardType Draw, const int pawn, const int start,
		const int currentSquare, const int newSquare, const Outcome move);
// REQUIRES: game is well-formed
// EFFECTS: performs announcements for eval()
//          announces evaluation of card drawn

//////////*functions that I'll implement*///////////////

bool initGame(Game &game, int argc, char *argv[]);
// MODIFIES: game
// EFFECTS: reads the board and deck, as 
//          specified on the command line, and initializes the Game
//          structure.  Returns true on success, false on failure.
//          Should produce no output.

Outcome eval(const Game &game, Color player);
// REQUIRES: game is well-formed
// EFFECTS: evaluates the desired Outcome of "player" drawing the card
//          that is currently at the top of the deck.
//          Must follow the specification's movement rules, and MUST NOT
//          modify game.  Should produce no output.

void apply(Game &game, Color player, Outcome move);
// REQUIRES: game is well-formed
//           given the game state and the card that is
//           currently at the top of the deck.
// MODIFIES: game
// EFFECTS: applies the changes to game that result from player making move.
//          Should produce the appropriate output for the *move*, but not
//          any output relating to drawing cards.  Consumes the card
//          at the top of the deck, but does *not* shuffle/reset if
//          this is the last card.

int myMain(int argc, char *argv[]);
// REQUIRES: argc and argv are the arguments to the program.
// EFFECTS: performs the sorry simulation as specified.

/*my function definitions*/

void improp(void)
{
	cout << "Improper arguments." << endl;
	exit(1);
}

template <typename T>

T converter(const string a, const string b[], const int size, T dummy)
{
	for(int i = 0; i < size; i++)
	{
		if(a == b[i])
		{
			return static_cast<T>(i);
		}
	}
	assert(0);
	return dummy;
}

void well_formed(int argc, char *argv[])
{
	if(argc != 6)//checks argument count
	{
		improp();
	}

	string board = argv[1];
	string deck = argv[2];
	int player_num = atoi(argv[3]);
	int round_num = atoi(argv[4]);
	string shuffle = argv[5];

	//reading in board and deck (check)
	ifstream boardstream;
	ifstream deckstream;
	boardstream.open(argv[1]);
	deckstream.open(argv[2]);
	if(!boardstream.good() || !deckstream.good())//checks if files can be found/exist
	{
		improp();
	}

	//check player count
	else if(player_num > 4 || player_num < 1)
	{
		improp();
	}

	//check round count
	else if(round_num > 1000 || round_num < 1)
	{
		improp();
	}

	//check shuffle response
	else if(shuffle != "yes" && shuffle != "no")
	{
		improp();
	}

	boardstream.close();
	deckstream.close();
}

void shuffler(Game &game, int round_num)
{
	if(game.deck.curCard >= game.deck.numCards || 
	  (game.shuffle && round_num == 0))
	{//if game runs out of cards or first round
		if(game.shuffle)
		{
			shuffle(game.deck);
			game.deck.curCard = 0;	
		}
		else
			game.deck.curCard = 0;
	}
}

bool iAmAlreadyThere(const Game &game, int square, Color player)
{
	if(square < 0)//checks for backward moves
	{
		square = square + game.board.numSquares;
	}

	if (game.board.squares[square].occupant.color == player &&
	    game.board.squares[square].ends.kind != HOMESQ)
	{
		//if the occupant's square's color == player's color and 
		//the square isn't home then move is NOT valid
		return true;
	}
	else return false;
}

void myFarthest(const Game &game, const CardType Draw, Outcome &drawn, const Color player, const int distance)
{
	drawn.canMove = false;//assumes a player cannot move
	int farthest = 0; //"farthest" pawn is at home
	int a = (int) player;//casts player's color as its integer

	for (int b = 0; b <NUMPAWNS; b++)
	//checks all of player's pawns for most distant
	{
		//the following two clauses check to see if a pawn 
		//of the same color is already at the target square
		//if so it skips the pawn in question before continuing 
		//calculating - for forward/backward moves
		if(iAmAlreadyThere(game,(game.players[a].pawns[b].square 
		+ distance) % game.board.numSquares, player)
		&& Draw == FORWARD
		&& game.players[a].pawns[b].state == ON_BOARD)
		   {
		   	b++;
		   }
		else if(iAmAlreadyThere(game,(game.players[a].pawns[b].square 
		- distance)%game.board.numSquares, player)
		&& Draw == BACKWARD 
		&& game.players[a].pawns[b].state == ON_BOARD)
		   {
		   	b++;
		   }

		if(game.players[a].pawns[b].state == ON_BOARD)
		{
			//if the distance from player's home is larger 
			//than farthest, then we set the new pawnNum as the 
			//farthest pawn away from home that is valid to move
			if((game.players[a].homeSquare + game.board.numSquares 
			- game.players[a].pawns[b].square) %
			game.board.numSquares > farthest)
			   {
			   	int c = b;
			   	drawn.pawnNum = c;
			   	//the most distant pawn to move
			   	
			   	farthest = ((game.players[a].homeSquare + 
			   	game.board.numSquares - 
			   	game.players[a].pawns[b].square) % 
			   	game.board.numSquares);
			   }

			drawn.canMove = true;//pawn is movable
		}
		
	}
}

void closestToHome(const Game &game, Outcome &drawn, const Color player)
{
	drawn.canMove = false; 
	//assumes no pawns on board, except for players
	int closest = game.board.numSquares; 
	//assume the closest pawn is a board away

	for(int a = 0; a < game.numPlayers; a++)
	{
		if(a == (int) player)
		{
			a++;//skips player's own pawns
			    //targeting only opponents
		}

		for(int b = 0; b < NUMPAWNS; b++)
		{
			if(game.players[a].pawns[b].state == ON_BOARD)
			{
				if((game.players[(int) player].homeSquare + 
				game.board.numSquares - 
				game.players[a].pawns[b].square) % 
				game.board.numSquares < closest)
				//if distance to another player's 
				//pawn < closest distance pawn
				//to that player's home square
				
				{
					drawn.canMove = true; 
					//there is opponent pawn on the board
					drawn.opponent.color = (Color) a;
					//sets color
					drawn.opponent.num = b; 
					//sets opponent pawnNum to SWAP or SORRY

					closest = (game.players[(int)
					player].homeSquare + 
					game.board.numSquares - 
					game.players[a].pawns[b].square) % 
					game.board.numSquares;
					//sets closest = to closest pawn from 
					//player's home
				}
			}
		}
	}
}

void game_won(Game &game, Color Player)
{
	int alreadyHome = 0;//assume no pawns are at home
	int person = (int) Player;

	for(int a = 0; a < NUMPAWNS; a++) //checks number of pawns at home
	{
		if(game.players[person].pawns[a].state == HOME)
		{
			alreadyHome++;
		}
	}

	if(alreadyHome == 4)//if 4 pawns are home, that player wins
	{
		exit(1);
	}
}

void collisions(Game &game, int Pnum, int targsquare, int youP)
{
	Color colorPTargSquare = game.board.squares[targsquare].occupant.color;
	//color of player at square
	int SquarePnumber = game.board.squares[targsquare].occupant.num;
	//the current pawnNum of the occupant, if any

	if(colorPTargSquare != (Color) youP && colorPTargSquare != NONE &&
	  game.players[colorPTargSquare].pawns[SquarePnumber].state == ON_BOARD)
	   {
	   	//if there is a pawn on that square and the color isn't the same
	   	//as the moving pawn AND the pawn is on_board and not at home

	   	int bumpablePawn = game.board.squares[targsquare].occupant.num;
	   	//pawn currently on square
	   	Color bumpablePawnPlayer = game.board.squares
	   	[targsquare].occupant.color;
	   	//color of pawn on that square

	   	game.players[(int) bumpablePawnPlayer].pawns
	   	[bumpablePawn].state = STARTABLE;
	   	//bumps said pawn off the board
	   	game.players[(int) bumpablePawnPlayer].pawns
	   	[bumpablePawn].square = MAXSQUARES;

	   	cout << colorNames[youP] << " pawn " << Pnum << " bumps " << 
	   	        colorNames[bumpablePawnPlayer] << " pawn " << 
	   	        bumpablePawn << "." << endl;
	   }
}

void pawn_Home(Game &game, int square, int homeSquare, int player, int pawn)
{
	if(square == homeSquare)
		{
			game.players[player].pawns[pawn].state = HOME;
			game.board.squares[square].occupant.color = NONE;
			game.board.squares[square].occupant.num = MAXSQUARES;

			cout << colorNames[player] << " pawn " << pawn << 
			" has reached home." << endl;

			game_won(game, (Color) player);
		}
}

void sliding(Game &game, int youP, int Pnum, int &startSquare, Color slideColor)
{
	int endSquare;//the end of the slide
	bool endNotFound = true;//assuming we haven't found an end square

	//finds the closest end square that has the same color as the beginning square
	for(int a = startSquare; endNotFound; a++)
	{
		//if the slide goes off the board, it loops the square index to 0
		if( a > game.board.numSquares)
		{
			a = 0;
		}

		if(game.board.squares[a].slide.kind == END && 
		   game.board.squares[a].slide.color == slideColor)
		   {
		   	endSquare = a;//initializes square as the end square
		   	endNotFound = false;//there is an end
		   }
	}

	cout << colorNames[youP] << " pawn slides from square " << startSquare << " to square " <<
	     endSquare << "." << endl;

	game.players[youP].pawns[Pnum].square = endSquare;
	//temp place curr. player pawn at end square

	bool playersNotRemoved = true;
	//assume opposing players on slide haven't been removed

	for (int b = startSquare + 1; playersNotRemoved; b++)
	//checks all squares on slide
	{
		if(b >= game.board.numSquares)
		//if it hasn't reached the end and has to loop around, 
		//sets i = 0
		{
			b = 0;
		}

		if(game.board.squares[b].occupant.color != NONE)
		{
			int player = (int) game.board.squares[b].occupant.color;
			//color of occupying pawn

			cout << colorNames[youP] << " pawn " << Pnum 
			<< " bumps " << colorNames[player] << "pawn" 
			<< game.board.squares[b].occupant.num << "." << endl;

			game.players[player].pawns[game.board.squares[b].occupant.num].state = STARTABLE;
			//bumps pawn off board
			
			game.players[player].pawns[game.board.squares[b].occupant.num].square = MAXSQUARES;
			
			game.board.squares[b].occupant.color = NONE;	
			//removes the pawn off the square
			game.board.squares[b].occupant.num = MAXSQUARES;     
		}

		if(b == endSquare)
		//by the time we reach the end square
		//all pawns on slide have been removed
		{
			playersNotRemoved = false;
		}
	}

	//sets for one of two conditions
	//1. pawn slides to home
	//2. pawn slides to a regular end
	if(endSquare == game.players[(int) youP].homeSquare)
	{
		pawn_Home(game, endSquare, game.players[(int) youP].homeSquare, youP, Pnum);
		//checks if pawn has slid home
	}

	else
	{
		//sets the square that the pawn lands on to its color and 
		//pawnNum
		game.board.squares[endSquare].occupant.color = (Color) youP;
		game.board.squares[endSquare].occupant.num = Pnum;
	}

	//changes occupant of slide beginning to NONE
	game.board.squares[startSquare].occupant.color = NONE;
	game.board.squares[startSquare].occupant.num = MAXSQUARES;
}

void Announce1(const Game &game, const Color player)
{
	CardType Draw = game.deck.cards[game.deck.curCard].type;
	//card drawn
	int value = game.deck.cards[game.deck.curCard].value;
	//the value of the card

	if(Draw == START || Draw == SWAP || Draw == SORRY)
	{
		cout << "Player " << colorNames[(int) player] << " draws " << 
		cardNames[(int) Draw] << "." << endl;
	}

	if(Draw == FORWARD || Draw == BACKWARD)
	{
		cout << "Player " << colorNames[(int) player] << " draws " << 
		cardNames[(int) Draw] << " " << value << "." << endl;
	}
}

void Announce2(const Game &game, const Color player, const CardType Draw, const int pawn, const int start, const int currentSquare, const int newSquare, const Outcome move)
{
	if(Draw == START && move.canMove)
	{
		cout << colorNames[(int) player] << " pawn " << pawn << 
		" starts at square " << start << "." << endl;
	}

	else if((Draw == FORWARD || Draw == BACKWARD) && move.canMove)
	{
		cout << colorNames[(int) player] << " pawn " << pawn << 
		" moves from square " << currentSquare << " to square " << 
		newSquare << "." << endl;             
	}

	else if(!move.canMove)
	{
		cout << "Player " << colorNames[(int) player] << 
		" cannot move." << endl;
	}
}

/////////////////////////*Function Implementations*///////////////////////////

///////////////*initiate game*/////////////

bool initGame(Game &game, int argc, char *argv[])
{
	//read in actual board

	ifstream board;
	board.open(argv[1]);

	string response;
	board >> response >> game.board.numSquares;
	//reads in total square number

	int square;
	string types;
	string colors;

	for(int i = 0; i < game.board.numSquares; i++)
	//sets all square kinds and colors to REGULAR and NONE
	{					       
		//and occ. color and num to NONE and INT_MAX
		game.board.squares[i].slide.kind = REGULAR;
		game.board.squares[i].slide.color = NONE;
		game.board.squares[i].ends.kind = REGULAR;
		game.board.squares[i].ends.color = NONE;
		game.board.squares[i].occupant.color = NONE;
		game.board.squares[i].occupant.num = MAXSQUARES;
	}

	while(!board.eof())
	//assign sq. types/colors to squares, converting strings to enums
	{
		if(types == "BEGIN" || types == "END")
		{
			game.board.squares[square].slide.kind = converter(types, kindNames, MAXKINDS, REGULAR);
			game.board.squares[square].slide.color = converter(colors, colorNames, MAXCOLORS, NONE);
		}

		else if(types == "STARTSQ" || types == "HOMESQ")
		{
			game.board.squares[square].ends.kind = converter(types, kindNames, MAXKINDS, REGULAR);
			game.board.squares[square].ends.color = converter(colors, colorNames, MAXCOLORS, NONE);

			int color = (int) game.board.squares[square].ends.color;

			if(types == "STARTSQ") 
			//assigns the player their start square
			{
				game.players[color].startSquare = square;
			}
			else 
			//assigns the player their home squares
			{
				game.players[color].homeSquare = square;
			}
		}
		board >> square >> types >> colors; 
		//takes in info from file
	}

	board.close();
	
	//read in deck	

	ifstream deck;
	deck.open(argv[2]);

	deck >> game.deck.numCards; 

	int a = 0;
	string type;
	int move;

	game.deck.curCard = 0;
	//curCard is intialized to the first card in the deck

	while(!deck.eof() && a < game.deck.numCards)
	{
		deck >> type >> move;

		game.deck.cards[a].type = converter(type, cardNames, MAXTYPES, START);
		game.deck.cards[a].value = move;

		a++;
	}

	deck.close();

	//initializing the rest of the game

	game.numPlayers = atoi(argv[3]);//how many players are playing
	game.rounds = atoi(argv[4]);//number of rounds
	string shuffle = argv[5];//to shuffle deck or not after rounds

	if (shuffle == "yes") // initializing shuffle variable
	{
		game.shuffle = true;
	}
	else
		game.shuffle = false;

	int num1 = 0;
	int pawn;
	//assigns all the pawn states as STARTABLE for all players
	while(num1 < atoi(argv[3]))
	{
		pawn = 0;

		while(pawn < NUMPAWNS)
		{
			game.players[num1].pawns[pawn].state = STARTABLE;
			pawn++;
		}
	num1++;
	}

	return true;
}

///////////////*evaluation*///////////////////

Outcome eval(const Game &game, Color player)
{
	int distance = game.deck.cards[game.deck.curCard].value;
	//distance of movement
	
	CardType Drawn = game.deck.cards[game.deck.curCard].type;
	//top card's type
	
	Outcome outcome;//the drawn card

	int a = (int)player;
	int b = 0;//to indicate first pawn

	outcome.canMove = false;//assume player cannot move at first

	//START card drawn
	if(Drawn == START)//finds if pawn is startable on start square
	{
		if(iAmAlreadyThere(game, game.players[a].startSquare, player))
		{
			//checks to see if player's pawn is on startSquare

			outcome.canMove = false;
			return outcome;
		}

		else
		{
			while(b < NUMPAWNS)
			{
				if(game.players[a].pawns[b].state == STARTABLE)
				{
					outcome.canMove = true;
					outcome.pawnNum = b;
					return outcome;
				}
			b++;
			}
		}

		//if no move can be placed, then there is no possible move
		outcome.canMove = false;
		return outcome;
	}

	//Forward and Backward Cards
	else if(Drawn == FORWARD || Drawn == BACKWARD)
	{
		myFarthest(game, Drawn, outcome, player, distance);
		//finds farthest pawn

		if(outcome.canMove)//i.e. if there is a pawn on board
		{
			if(distance > (game.players[a].homeSquare +
			game.board.numSquares - 
			game.players[a].pawns[outcome.pawnNum].square)%
			game.board.numSquares 
			&& Drawn == FORWARD)
			{
			   	//makes sure that pawn doesn't 
			   	//move past home
			   	
			   	outcome.canMove = false;
			   	return outcome;
			}

			return outcome;
		}
		else 
		//if no pawn is on board, move cannot be made

		outcome.canMove = false;
		return outcome;
	}

	//Swap Card
	else if(Drawn == SWAP)
	{
		myFarthest(game, Drawn, outcome, player, distance);
		//finds farthest pawn for SWAP

		if(outcome.canMove)
		//if pawn on board, then swap
		{
			closestToHome(game, outcome, player);
			//finds closest enemy pawn to player home

			return outcome;
		}

		else
		//if no pawn on board, no move can be made

		outcome.canMove = false;
		return outcome;
	}

	//SORRY card

	else if(Drawn == SORRY)
	{
		while(b < NUMPAWNS)
		//finds the first startable pawn
		{
			if(game.players[a].pawns[b].state == STARTABLE)
				{
					outcome.canMove = true;
					outcome.pawnNum = b;

					b = NUMPAWNS;//stops loop
				}
			b++;
		}

		if(outcome.canMove)
		{
			closestToHome(game, outcome, player);
			//finds closest enemy pawn to bump off board
			return outcome;
		}

		else
			outcome.canMove = false;
			return outcome;
	}
	else
		outcome.canMove = false;
		return outcome;
}

/////////////////////*Apply*///////////////////

void apply(Game &game, Color player, Outcome move)
{
	CardType Drawn = game.deck.cards[game.deck.curCard].type;
	int a = (int) player;

	if(move.canMove)
	{
		int distance = game.deck.cards[game.deck.curCard].value;
		//distance traveled
		int pawnNum = move.pawnNum;
		//pawn index number
		int home = game.players[a].homeSquare;
		//player homeSquare
		int currentSquare = game.players[a].pawns[move.pawnNum].square;
		//player's current square
		int newSquare = 0;

		//START card
		if(Drawn == START)
		{
			game.players[a].pawns[pawnNum].square = game.players[a].startSquare; 
			//changes pawns square
			
			game.players[a].pawns[pawnNum].state = ON_BOARD;
			//changes state to on board
			
			currentSquare = game.players[a].startSquare;
			newSquare = currentSquare;
		}

		//FORWARD card
		else if(Drawn == FORWARD)
		{
			if(currentSquare + distance == home)
			//checks if move takes pawn home
			{
				game.players[a].pawns[pawnNum].square = home;
				newSquare = home;
			}

			else
			{
				game.players[a].pawns[pawnNum].square = 
				(currentSquare + distance)%game.board.numSquares;
				//mods to consider size of board and loops
				newSquare = (currentSquare + distance)%
				game.board.numSquares;
				//pawn has moved here
			}	
		}
		//BACKWARD card
		else if(Drawn == BACKWARD)
		{
			if(currentSquare - distance == home)
			//checks if move takes pawn home
			{
				game.players[a].pawns[pawnNum].square = home;
				newSquare = home;
			}

			else if((currentSquare - distance) < 0)
			//if pawn moves off board in reverse direction
			{
				game.players[a].pawns[pawnNum].square = 
				game.board.numSquares - (distance - 
				currentSquare);
				//allows for "loopage" around board
				//to consider values < 0
				
				newSquare = game.board.numSquares - (distance - 
				currentSquare);

				if(newSquare == home)
				{
				game.players[a].pawns[pawnNum].square = home;
				newSquare = home;
				}
			}

			else //normal backwards move
			{
				game.players[a].pawns[pawnNum].square = 
				currentSquare - distance;
				newSquare = currentSquare - distance;

			}

		}
		//SWAP card
		else if(Drawn == SWAP)
		{
			int tempSquare = game.players[a].pawns[pawnNum].square;
			//square that swapper pawn is on
			int oppSquare = game.players[(int)move.opponent.color].pawns[move.opponent.num].square;
			//square that opponent is on
			
			game.players[a].pawns[pawnNum].square = oppSquare;
			//sets player's pawn to opponent pawn location

			game.board.squares[currentSquare].occupant.color = 
			move.opponent.color;
			//sets player's pawn's old square color to 
			//opponent's color

			game.board.squares[oppSquare].occupant.color = player;
			//vice versa of above

			game.board.squares[currentSquare].occupant.num = 
			move.opponent.num;
			//swaps pawn's numbers

			game.board.squares[oppSquare].occupant.num = pawnNum;

			game.players[(int) move.opponent.color].pawns
			[move.opponent.num].square = tempSquare;
			//sets the location of opponent pawn to player's 
			//original location

			cout << colorNames[a] << " pawn " << pawnNum << 
			" swaps with " << colorNames[(int)move.opponent.color] 
			<< " pawn " << move.opponent.num << "." << endl;

			if(game.players[a].homeSquare == oppSquare)
			//checks if player's pawn is home
			{
				pawn_Home(game, home, home, a, pawnNum);
			}

			bool is_oppHome = false;

			if(game.players[(int)move.opponent.color].homeSquare 
			== tempSquare)
			//checks if opponent pawn is swapped into home
			//if it is, it cannot be bumped
			{
			is_oppHome = true;
			game.board.squares[tempSquare].occupant.color = NONE;
			}

			if(game.board.squares[oppSquare].slide.kind == BEGIN 
			&& game.board.squares[oppSquare].slide.color != player 
			&& game.board.squares[oppSquare].ends.kind != HOMESQ)
			{
				//checks if swapping pawn has landed on slide
				//if so, calls slide
				sliding(game, a, pawnNum, oppSquare, 
				game.board.squares[oppSquare].slide.color);
			}

			if(is_oppHome)
			//
			{
				pawn_Home(game, tempSquare, game.players[(int)
				move.opponent.color].homeSquare, 
				move.opponent.color, pawnNum);
			}

			else if(game.board.squares[tempSquare].ends.kind != 
			HOMESQ && game.players[move.opponent.color].pawns
			[move.opponent.num].state == ON_BOARD && 
			game.board.squares[tempSquare].slide.kind == BEGIN && 
			game.board.squares[tempSquare].slide.color != 
			move.opponent.color)
			//checks to see if opp pawn is on board, landed on a 
			//slide square, if so it calls sliding
			{
				sliding(game, (int) move.opponent.color, move.opponent.num, 
				tempSquare, game.board.squares[tempSquare].slide.color);
			}
		}
		//SORRY Card
		else
		{
			int oppSquare = game.players[(int)
			move.opponent.color].pawns[move.opponent.num].square;
			//intializes opponent square number
			
			game.players[a].pawns[pawnNum].state = ON_BOARD;
			//changes player's pawn's state to ON_BOARD
			
			game.players[a].pawns[pawnNum].square = oppSquare;
			//places player on opponent square number
			
			game.board.squares[oppSquare].occupant.color = player;
			
			game.board.squares[oppSquare].occupant.num = pawnNum;
			
			game.players[(int)move.opponent.color].pawns
			[move.opponent.num].state = STARTABLE;
			
			game.players[(int)move.opponent.color].pawns
			[move.opponent.num].square = MAXSQUARES;

			cout << colorNames[a] << " pawn " << pawnNum 
			<< " bumps " << colorNames[(int) move.opponent.color]
			<< " pawn " << move.opponent.num << "." << endl;

			if(game.players[a].homeSquare == oppSquare)
			//checks if player is swapped into home square
			{
				pawn_Home(game, home, home, a, pawnNum);
			}

			if(game.board.squares[oppSquare].slide.kind == BEGIN && 
			   game.board.squares[oppSquare].slide.color != player &&
			   game.board.squares[oppSquare].ends.kind != HOMESQ)
			{
				sliding(game, a, pawnNum, oppSquare, 
				game.board.squares[oppSquare].slide.color);
			}
		}

		Announce2(game, player, Drawn, pawnNum, 
		game.players[a].startSquare, currentSquare, newSquare, move);

		if(Drawn != SWAP && Drawn != SORRY)
			{
				collisions(game, pawnNum, newSquare, a);
			}
		
		if(game.board.squares[newSquare].slide.kind == BEGIN &&
		   game.board.squares[newSquare].slide.color != player &&
		   game.board.squares[newSquare].ends.kind != HOMESQ &&
		   Drawn != SWAP && Drawn != SORRY)
		   {//checks if FORWARD, BACKWARD, or START starts a slide
		   
		   	game.board.squares[currentSquare].occupant.color = 
		   	NONE;
		   	//sets original square to none
		   	
		   	sliding(game, a, pawnNum, newSquare, game.board.squares
		   	[newSquare].slide.color);
		   	//calls sliding
		   }
		else if(Drawn != SWAP && Drawn != SORRY 
		&& game.players[a].pawns[pawnNum].state != HOME)
		{
		
		game.board.squares[currentSquare].occupant.color = NONE;
		
		game.board.squares[currentSquare].occupant.num = MAXSQUARES;
		
		game.board.squares[newSquare].occupant.color = player;
		
		game.board.squares[newSquare].occupant.num = pawnNum;
		
		pawn_Home(game, newSquare, home, a, pawnNum);
		
		}

		game.deck.curCard++;//Next card!
	}
	else//assuming no move is possible according to eval()
	{
		game.deck.curCard++;
		Announce2(game, player, Drawn, 0, 0, 0, 0, move);
	}
}

//////////////////*myMain*///////////////////

int myMain(int argc, char *argv[])
{	
	Game game;

	well_formed(argc, argv);
	
	initGame(game, argc, argv);

	Outcome shoopDaWoop;

	for(int j=0; j < game.rounds; j++)
	{
		cout << "Round " << j+1 << endl;
		
		for(int i = 0; i < game.numPlayers; i++)
		{
			Color c = (Color)i;
			shuffler(game, j);
			Announce1(game, c);//announces card that was drawn
			shoopDaWoop = eval(game, c);
			apply(game, c, shoopDaWoop);
		}

	}
	
	return 0;
}
