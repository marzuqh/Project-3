#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <cassert>

#include "p3.h"

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

int main(int argc, char *argv[])
{
    Game game;

    assert(initGame(game, argc, argv));

    printGame(game);

    return 0;
}

/* Should never be called, but must be defined */
void shuffle(Deck &d)
{
    assert(0);
}
