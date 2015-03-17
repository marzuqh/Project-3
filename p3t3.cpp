#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <cassert>
#include <cstring>

#include "p3.h"



// Testing "apply()"
// 
// General strategy:
//
// Use a "standard" board, and reads a "scenario" from standard input
//
// A scenario input file has the number of players on the first line.
// Then, numPlayers lines of four pawn locations each
//    -1 is startable, -2 is home.
// Then a single line with the player who is to play.
// then a single line with the current card
// then a single line with the outcome.canMove from eval (as 0 or 1)
// then a single line with the outcome.pawnNum from eval
// then a single line with the outcome.opponent from eval
//	  <player#> <pawn#>
//
// Note, we use enum values directly, not string labels
// So, the entire scenario file is numbers.
// 

static void printGame(Game &g)
{

    cout << "--------------------------------------------------\n";

    cout << "Sorry simulation: " << g.rounds << " rounds with";
    if (!g.shuffle) { cout << "out"; }
    cout << " shuffling\n\n";

    cout << "Current Board State\n";
    cout << "Number of squares: " << g.board.numSquares << endl;

    for (int i=0; i<g.board.numSquares; i++) {
	Square &s = g.board.squares[i];
	if (s.slide.kind != REGULAR) {
	    cout << i << " " << colorNames[s.slide.color] << " "
		 << kindNames[s.slide.kind] << endl;
	}
	if (s.ends.kind != REGULAR) {
	    cout << i << " " << colorNames[s.ends.color] << " "
		 << kindNames[s.ends.kind] << endl;
	}
	if (s.occupant.color != NONE) {
	    cout << i << " occupied by " 
		 << colorNames[s.occupant.color] << " "
		 << s.occupant.num << endl;
	}
    }

    cout << "\n\nCurrent Deck State\n";
    cout << "Size of deck: " << g.deck.numCards << endl;
    cout << "Next card to deal: " << g.deck.curCard << endl;
    for (int i=0; i<g.deck.numCards; i++) {
	Card &c = g.deck.cards[i];
	cout << i << " " << cardNames[c.type];
	if (c.type == FORWARD || c.type == BACKWARD) {
	    cout << " " << c.value;
	}
	cout << endl;
    }

    cout << "\n\n" << g.numPlayers << " players\n";

    for (int i=0; i<g.numPlayers; i++) {
	Player &p = g.players[i];
	cout << "Player " << colorNames[i] << " starts on "
	     << p.startSquare << " and ends on " 
	     << p.homeSquare << endl;
	for (int j=0; j<NUMPAWNS; j++) {
	    cout << "Pawn " << j << " is "
		 << stateNames[p.pawns[j].state];
	    if (p.pawns[j].state == ON_BOARD) {
		cout << " " << p.pawns[j].square;
	    }
	    cout << endl;
	}
    }
	    

    cout << "--------------------------------------------------\n";
}

static void fillBoard(Game &g)
{
    bzero(&g, sizeof(g));

    g.board.numSquares = MAXSQUARES;
    for (int i = 0; i<MAXSQUARES; i++) 	{
	Square &s = g.board.squares[i];
	s.slide.kind = REGULAR;
	s.ends.kind = REGULAR;
	s.occupant.color = NONE;
    }

    for (int i = BLUE; i <= RED; i++) {
	Color c = static_cast<Color>(i);
	const int offset = c * 15;
	Square *sp = g.board.squares;

	sp[1+offset].slide.kind = BEGIN;
	sp[1+offset].slide.color = c;

	sp[2+offset].ends.kind = HOMESQ;
	sp[2+offset].ends.color = c;

	sp[4+offset].slide.kind = END;
	sp[4+offset].slide.color = c;
	sp[4+offset].ends.kind = STARTSQ;
	sp[4+offset].ends.color = c;
	
	sp[9+offset].slide.kind = BEGIN;
	sp[9+offset].slide.color = c;

	sp[13+offset].slide.kind = END;
	sp[13+offset].slide.color = c;

	g.players[c].startSquare = 4+offset;
	g.players[c].homeSquare = 2+offset;
	
    }
	
}

void readScenario(Game &g, Color &p, Outcome& r)
{
    int val;

    cin >> g.numPlayers;
    assert(g.numPlayers>0 && g.numPlayers<=MAXPLAYERS && cin);

    for (int i=0; i<g.numPlayers; i++) {
	for (int j=0; j<NUMPAWNS; j++) {
	    cin >> val;
	    assert(cin && val >= -2 && val <= g.board.numSquares);

	    PawnLocation &pl = g.players[i].pawns[j];
	    switch (val) {
	    case -1:
		pl.state = STARTABLE;
		pl.square = 0;
		break;
	    case -2:
		pl.state = HOME;
		pl.square = 0;
		break;
	    default:
		pl.state = ON_BOARD;
		pl.square = val;
		g.board.squares[val].occupant.color 
		    = static_cast<Color>(i);
		g.board.squares[val].occupant.num = j;
	    }
	}
    }
	
	//current player
    cin >> val;
    assert(cin && val >= BLUE && val <= RED);
    p = static_cast<Color>(val);

    g.deck.curCard = 0;
    g.deck.numCards = 1;
    Card &c = g.deck.cards[0];

	//card type
    cin >> val;
    assert(cin && val >= START && val <= SORRY);
    c.type = static_cast<CardType>(val);

	//value of card
    cin >> val;
    assert(cin && val >= 0 && val <= g.board.numSquares);
    c.value = val;
    
	//canmove?
	cin >> val;
	assert(cin && val >=0 && val < 2);
	r.canMove = val;
	
	//pawnnum
	cin >> val;
	assert(cin && val >=0 && val < NUMPAWNS);
	r.pawnNum = val;
	
	//opponent player#
	cin >> val;
	assert(cin && val >= 0 && val < MAXCOLORS);
	r.opponent.color = static_cast<Color>(val);
	
	//opponent player's pawn #
	cin >> val;
	assert(cin && val >= 0 && val < NUMPAWNS);
	r.opponent.num = val;
}

Outcome clearResult()
{
    Outcome result;

    result.canMove = true;
    result.pawnNum = -1;

    result.opponent.color = NONE;
    result.opponent.num = -1;

    return result;
}

void printOutcome(Color p, Card &c, Outcome &result)
{
    cout << "Move is";
    cout << ((!result.canMove) ? " not " : " ");
    cout << "legal\n";

    if (result.canMove) {
	cout << colorNames[p] << " pawn " << result.pawnNum << endl;
	if (c.type == SWAP || c.type == SORRY) {
	    cout << "Victim: " << colorNames[result.opponent.color]
		 << " pawn " << result.opponent.num << endl;
	}
    }
}

void checkScenario(Game &game)
{
    Outcome result;
    Color   player;

    result = clearResult();

    readScenario(game,player,result);
    Card &c = game.deck.cards[0];

	cout << "--STARTING GAME STATE-----------------------------" << endl;
    printGame(game);
    cout << "Player " << colorNames[player] << " draws "
	 << cardNames[c.type];
    if (c.type == FORWARD || c.type == BACKWARD) {
	cout << " " << c.value;
    }
    cout << endl;
    cout << "--APPLY MESSAGES----------------------------------" << endl;
    apply(game,player,result);
    cout << "--ENDING GAME STATE-------------------------------" << endl;
    printGame(game);
}


int main()
{
    Game game;

    fillBoard(game);
    checkScenario(game);

    return 0;
}


/* Should never be called, but must be defined */
void shuffle(Deck &d)
{
    assert(0);
}
