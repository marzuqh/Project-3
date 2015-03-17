#include "p3.h"

int main(int argc, char *argv[])
{
    (void) myMain(argc, argv);
    return 0;
}

void shuffle(Deck &d)
{
    Card tmp[MAXCARDS];

    int mid = d.numCards/2;
    int left = 0, right = mid;

    int i = 0;
    while (i < d.numCards) {
	if (right < d.numCards) tmp[i++] = d.cards[right++];
	if (left < mid) tmp[i++] = d.cards[left++];
    }

    for (i = 0; i < d.numCards; i++) {
	d.cards[i] = tmp[i];
    }
    
    d.curCard = 0;
}
