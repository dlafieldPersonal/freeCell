/*
    freecell solver
    separator is | for input files
    empty card is xx
*/

#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <ctime>  
#include <queue>

#define ENTER_KEY 10
#define STREAM_LENGTH  53
#define NUM_OF_FREECELLS 4
#define MAX_DEPTH 1000

using namespace std;

typedef int cardType;

/**************************************************************/

typedef struct
{
    cardType freeCells[NUM_OF_FREECELLS];
    vector<cardType> stream[8];
    
    /* home squares */
    int lowestHeart;
    int lowestClub;
    int lowestSpade;
    int lowestDiamond;
}handType;

/**************************************************************/
 
unsigned long long int dSeed;
int lastDisplay = 999;
long long int callCount = 0;
bool byMovable = false;

/* prototypes */
string breadthFirst(handType origin, int depthLimit, int deadline);
bool cardVectorMatch(vector<cardType> a, vector<cardType> b); 
bool compareHands(handType a, handType b);
int coverCount(handType h);
handType dealCards(int gamenumber, int lowestRank);
void displayHand(handType hand1);
int dRand(void);
int dumpingCount(handType h);
char getRank(cardType card);
int getRankInt(cardType card);
char getSuit(cardType card);
handType getThreshold(queue<handType> hQ);
int handCompareForSorting(handType a, handType b);
string handDescent(handType origin, handType h, string journey, int depth, int *depthLimit, int originalLimit, int deadline);
int handPoints(handType h);
string listMoves(handType h);
handType makeMove(handType h, string move);
handType migrateCards(handType h, int source, int dest, int numOfCards);
string noBrainer(handType h, string listOfMoves);
int numOfMovables(handType h);
void outputSolver(handType h, string solution);
handType iCantBelieveItsNotQuicksort(vector<handType> *v, int firstIndex, int lastIndex, int target);
void quickSort(vector<handType> *v, int firstIndex, int lastIndex);
string rankToEnglish(cardType c);
bool sameColors(cardType a, cardType b);
string sortMoves(handType h, string moveList);
string suitToEnglish(cardType c);
string translateMove(handType h, string move);
int underCount(handType h);
int usefulMove(handType h, string move);

/**************************************************************/

int main(int argc, char** argv)
{
	int gameNumber;

	if(argc < 2)
		gameNumber = -1;
	else
		gameNumber = atoi(argv[1]);
		
	cout << "Game number " << gameNumber << endl;

	int lowestRank = 10;
	
	//handType h = dealCards(gameNumber, lowestRank);
	//handType h = dealCards(164, lowestRank);
	handType h = dealCards(169, lowestRank);
	//handType h = dealCards(10913, lowestRank);
	//handType h = dealCards(-51, lowestRank);
	
	
	int depthLimit = MAX_DEPTH;
	
	/* remove cards lower than the lowest rank */
	
	displayHand(h);
	
	int deadline = time(NULL) + 10;
	
	//string solveList = handDescent(h, h, "", 0, &depthLimit, depthLimit, deadline);
	string solveList = breadthFirst(h, depthLimit, deadline);
		
	cout << "callCount = " << callCount << endl;
	
	cout << "Solve list:" << endl << solveList << endl;
	
	outputSolver(h, solveList);
	
	int choice = 100;
	while(choice != 0)
	{
		/*string solveList = handDescent(h, h, "", 0, &depthLimit, depthLimit);
	
		cout << "Solve list:" << endl << solveList << endl;*/
		
		displayHand(h);
		string listOfMoves = listMoves(h);
		listOfMoves = noBrainer(h, listOfMoves);
		listOfMoves = sortMoves(h, listOfMoves);
		
		cout << listOfMoves << endl << endl;
		for(int d = 0; d < listOfMoves.size(); d += 2)
			cout << listOfMoves.substr(d, 2) << " ";
		cout << endl << endl;
	
		if(coverCount(h) == 0)
		{
			cout << "move all remaining cards home" << endl;
			choice = 0;
		}
		else
		{
			cout << "0 : exit." << endl;
			for(int x = 0; x < listOfMoves.size(); x += 2)
				cout << x / 2 + 1	 << " : " << translateMove(h, listOfMoves.substr(x, 2)) << endl;
					
			cout << endl << "Choice> ";
			cin >> choice;
			
			int location = (choice - 1) * 2;
			if(location >= 0 && location < listOfMoves.size())
			{
				string moveString = listOfMoves.substr((choice - 1) * 2, 2);
				h = makeMove(h, moveString);
			}
		}
	} /* while choice is not 0 */
	return 0;
}

/**************************************************************/

void displayHand(handType hand1)
{
    for(int x = 0; x < NUM_OF_FREECELLS; x++)
        if(getSuit(hand1.freeCells[x]) == 'x')
            cout << "    ";
        else
			cout << getRank(hand1.freeCells[x]) << getSuit(hand1.freeCells[x]) << "  ";
			
    cout << endl << endl; 

    bool lastLine = false;
    for(int x = 0; !lastLine && x < 20; x++)
    { /* for each horizontal line of cards */
		
        lastLine = true;
        for(int y = 0; y < 8; y++)
            if(x >= hand1.stream[y].size() || getSuit(hand1.stream[y][x]) == 'x')
                cout << "    ";
            else
            {
				cout << getRank(hand1.stream[y][x]) << getSuit(hand1.stream[y][x]) << "  ";
                lastLine = false;
            }
        cout << endl;
        
	} /* for each horizontal line of cards */

} /* displayHand */

/**************************************************************/

char getSuit(cardType card)
{
	if(card < 0)
		return 'x';
	if(card % 4 == 0)
		return 'c';
	if(card % 4 == 1)
		return 'd';
	if(card % 4 == 2)
		return 'h';
	return 's';
	
} /* get suit */

/**************************************************************/

char getRank(cardType card)
{
	if(card < 0)
		return 'x';
		
	int r = (card / 4) % 13 + 1;
	
	if(r == 10)
		return 'T';
	if(r == 1)
		return 'A';
	if(r == 11)
		return 'J';
	if(r == 12)
		return 'Q';
	if(r == 13)
		return 'K';
	return r + '0';
	
} /* get rank */

/**************************************************************/

handType dealCards(int gamenumber, int lowestRank)
{
	vector<cardType> deck;
	handType h;
	
	h.lowestClub = lowestRank;
    h.lowestDiamond = lowestRank;
    h.lowestHeart = lowestRank;
    h.lowestSpade = lowestRank;

	/* set up free cells */
	for(int i = 0; i < NUM_OF_FREECELLS; i++)
		h.freeCells[i] = -1;
		
	/* put unique card in deck for each location */
    for (int i = 0; i < 52; i++)      
		deck.push_back(i);

	dSeed = gamenumber;
    
    for (int i = 0; i < 52; i++)
    { /* while deck is not empty */
		
		/* pick random card from deck */
		//int r = dRand();
        //int j = r % deck.size();
        int j = ((dRand())) % deck.size();
        
        /* swap selected card with last card */
		cardType temp = deck[j];
		deck[j] = deck[deck.size() - 1];
		deck[deck.size() - 1] = temp;
		
        /* add card to stream */
        if(getRankInt(deck[deck.size() - 1]) >= lowestRank)
			h.stream[i % 8].push_back(deck[deck.size() - 1]);
        
        /* remove card from deck */
        deck.erase(deck.begin() + deck.size() - 1);
        
    } /* while deck is not empty */

    return h;
} /* deal cards */ 

/**************************************************************/

int dRand(void)
{
	/*
	if(dSeed < -1)
	{
		dSeed++;
		return -1 * dSeed;
	}*/
	if(dSeed < 0)
		return 0;

	dSeed = (214013 * dSeed + 2531011) % 2147483648;
	unsigned long long int ret = (dSeed >> 16) & 0x7fff;
	return ret;
} /* doug rand */

/**************************************************************/

int handPoints(handType h)
{
    return h.lowestClub + h.lowestDiamond + h.lowestHeart + h.lowestSpade;
} /* hand points */ 

/**************************************************************/

string listMoves(handType h)
{
    int x, y, i, tempLength;
    bool movable;
    string tempString, smallString;
    cardType smallStringCard;

    tempString = "";
    
    for(x = 0; x < NUM_OF_FREECELLS; x++)
    { /* freecell to home */

        smallString = "a";
        smallString[0] += x;

        if( (getRankInt(h.freeCells[x]) - 1 == h.lowestClub && getSuit(h.freeCells[x]) == 'c') ||
            (getRankInt(h.freeCells[x]) - 1 == h.lowestHeart && getSuit(h.freeCells[x]) == 'h') ||
            (getRankInt(h.freeCells[x]) - 1 == h.lowestDiamond && getSuit(h.freeCells[x]) == 'd') ||
            (getRankInt(h.freeCells[x]) - 1 == h.lowestSpade && getSuit(h.freeCells[x]) == 's')    )
        {    
			if(x == 0) tempString += "ah";
			if(x == 1) tempString += "bh";
			if(x == 2) tempString += "ch";
			if(x == 3) tempString += "dh";
        }

    } /* freecell to home */

    for(x = 0; x < 8; x++)
    { /* stream to home */
		
        if(h.stream[x].size() > 0)
        { /* if stream is not empty */
			smallString = to_string(x + 1);
			smallStringCard = h.stream[x][h.stream[x].size() - 1];
			if((getRankInt(smallStringCard) - 1 == h.lowestClub && getSuit(smallStringCard) == 'c') ||
				(getRankInt(smallStringCard) - 1 == h.lowestHeart && getSuit(smallStringCard) == 'h') ||
				(getRankInt(smallStringCard) - 1 == h.lowestDiamond && getSuit(smallStringCard) == 'd') ||
				(getRankInt(smallStringCard) - 1 == h.lowestSpade && getSuit(smallStringCard) == 's'))
			{
				tempString = tempString + smallString + "h";
			}
			
		} /* if stream is not empty */

    } /* stream to home */

    /* stream to freecell */
    x = 0;
    while(x < NUM_OF_FREECELLS && getSuit(h.freeCells[x]) != 'x')
        x++;
        
    /* if there is an empty freecell, list all streams to cell */  
    if(x < NUM_OF_FREECELLS)
    
		/* for each stream */
        for(y = 0; y < 8; y++)
        
            if(h.stream[y].size() > 0)
            { /* if stream is not empty */
				
				smallString = to_string(y + 1);
				string freeCellString;
				if(x == 0) freeCellString = "a";
				if(x == 1) freeCellString = "b";
				if(x == 2) freeCellString = "c";
				if(x == 3) freeCellString = "d";
				
				tempString = tempString + smallString + freeCellString;
                
            } /* if stream is not empty */

    /* freecell to empty stream */ 
    x = 0;
    while(x < 8 && h.stream[x].size() > 0)
        x++;
        
    /* look for empty stream, if found, add 4 freecell to stream codes */     
    if(x < 8)

		/* for each free cell */
        for(y = 0; y < NUM_OF_FREECELLS; y++)
			
            if(getSuit(h.freeCells[y]) != 'x')
            { /* if freecell is not empty */
				
				string freeCellString;
				if(y == 0) freeCellString = "a";
				if(y == 1) freeCellString = "b";
				if(y == 2) freeCellString = "c";
				if(y == 3) freeCellString = "d";
				
				smallString = to_string(x + 1);
				tempString += freeCellString + smallString;
				
            } /* if freecell is not empty */
            
    /** freecell to stream */ 
    /* go through each cell */
    for(x = 0; x < NUM_OF_FREECELLS; x++)

		/* if the cell is occupied */
        if(getSuit(h.freeCells[x]) != 'x')

			/* go through each stream */
            for(y = 0; y < 8; y++)

				/* if the stream is occupied */
                if(h.stream[y].size() > 0)

                    if(!sameColors(h.freeCells[x], h.stream[y][h.stream[y].size() - 1]) &&
                        getRankInt(h.freeCells[x]) + 1 == getRankInt(h.stream[y][h.stream[y].size() - 1]))
                    { /* check if the move is valid */

						smallString = to_string(y + 1);
						string freeCellString;
						if(x == 0) freeCellString = "a";
						if(x == 1) freeCellString = "b";
						if(x == 2) freeCellString = "c";
						if(x == 3) freeCellString = "d";
						tempString += freeCellString + smallString;
				
                    } /* check if the move is valid */

    /* stream to empty stream */
    x = 0;
    while(x < 8 && h.stream[x].size() > 0)
        x++;
        
    /* if there is an empty stream, do all streams to empty stream */  
    if(x < 8)
        
        /* for each stream */
        for(y = 0; y < 8; y++)
        
			/* if freecell is not empty, add combo */
            if(h.stream[y].size() > 0)
				tempString += to_string(10 * (y + 1) + x + 1);

    /** stream to stream */
    
    /* go through source */
    for(x = 0; x < 8; x++)
        
        if(h.stream[x].size() > 0)
        { /* if not empty */

            i = h.stream[x].size() - 1;
            movable = true;
            while(i >= 0 && h.stream[x].size() - i <= numOfMovables(h) && movable)
            { /* go through cards in current source until top of stream, top of movable, or until max */
                                                                    
                for(y = 0; y < 8; y++)
                { /* go through destination streams */

					/* if x != y and dest. stream is not empty */
                    if(x != y && h.stream[y].size() != 0)

						/* if a valid move */
                        if(getRankInt(h.stream[x][i]) + 1 == getRankInt(h.stream[y][h.stream[y].size() - 1]) && 
                            !sameColors(h.stream[x][i], h.stream[y][h.stream[y].size() - 1]))

							tempString += to_string(10 * (x + 1) + y + 1);
							
                } /* go through destination streams */
                
                i--;
                movable = (i >= 0) && !sameColors(h.stream[x][i], h.stream[x][i + 1]) && 
					getRankInt(h.stream[x][i]) - getRankInt(h.stream[x][i + 1]) == 1;

            } /* go through cards in current source until top of stream, top of movable, or until max */
            
        } /* if not empty */

    return tempString;

} /* list moves */ 

/**************************************************************/

bool sameColors(cardType a, cardType b)
{
	char aS = getSuit(a);
	char bS = getSuit(b);
    return ((aS == 'h' || aS == 'd') && (bS == 'h' || bS == 'd')) ||
           ((aS == 'c' || aS == 's') && (bS == 'c' || bS == 's'));
} /* same colors */ 

/**************************************************************/

int getRankInt(cardType card)
{
	if(card < 0)
		return -1;
	return (card / 4) % 13 + 1;
} /* get rank int */

/**************************************************************/

int numOfMovables(handType h)
{
    int x, y, numOfCells = 0, numOfStreams = 0;

    /* count cells */
    for(x = 0; x < NUM_OF_FREECELLS; x++)
        if(getSuit(h.freeCells[x]) == 'x')
            numOfCells++;

    /* count streams */
    for(x = 0; x < 8; x++)
        if(h.stream[x].size() == 0)
            numOfStreams++;

    /* (1 + number of empty freecells) * 2 ^ (number of empty columns) */
    y = 1;
    for(x = 0; x < numOfStreams; x++)
        y += y;
    return (1 + numOfCells) * y;

} /* number of cards that can move */

/**************************************************************/

string suitToEnglish(cardType c)
{
    switch(getSuit(c))
    { /* switch c.suit */

        case 'c' :
        case 'C' : return "Clubs";
        case 'd' :
        case 'D' : return "Diamonds";
        case 'h' :
        case 'H' : return "Hearts";
        case 's' :
        case 'S' : return "Spades";
    } /* switch c.rank */

	return "error suit";
	
} /* suit to english */

/**************************************************************/

string rankToEnglish(cardType c)
{
    switch(getRankInt(c))
    { /* switch c.rant */
        case 1 : return "Ace of ";
        case 2 : return "Deuce of ";
        case 3 : return "Three of ";
        case 4 : return "Four of ";
        case 5 : return "Five of ";
        case 6 : return "Six of ";
        case 7 : return "Seven of ";
        case 8 : return "Eight of ";
        case 9 : return "Nine of ";
        case 10 : return "Ten of ";
        case 11 : return "Jack of ";
        case 12 : return "Queen of ";
        case 13 : return "King of ";
    } /* switch c.rant */
    
    return "error rank";

} /* rank to english */ 

/**************************************************************/

handType migrateCards(handType h, int source, int dest, int numOfCards)
{ 
    /* move numOfCards number of cards from stream source to stream dest, assume errors checked */

    cardType oneCard;
    vector<cardType> stack;
    int countCards;
    bool Error = false;
	handType ret = h;

    for(countCards = 0; !Error && countCards < numOfCards; countCards++)
    { /* move from source to stack */

		Error = !(ret.stream[source].size() > 0);
		oneCard = ret.stream[source].back();
		ret.stream[source].pop_back();
		stack.push_back(oneCard);

    } /* move from source to stack */

    while(countCards > 0 && !Error)
    { /* move from stack to dest */

        oneCard = stack.back();
        ret.stream[dest].push_back(oneCard);
        stack.pop_back();
        countCards--;

    } /* move from stack to dest */

	if(Error)
		cout << "***Error in migrateCards***" << endl;
		
    return ret;

} /* migrate cards */ 

/**************************************************************/

string translateMove(handType h, string move)
{
    /* take the hand h and the 2 character move and return the same move in english. 
       assume a valid move
    */

    /* 
        possible moves:
        -card to card (ex. Move the Jack of Diamonds on the Queen of Spades)
        -card to cell (ex. kick the Two of Clubs up)
        -card to empty stream (ex. Move the Seven of Hearts to an empty spot)
        -card to home (ex. Send the Four of Diamonds home)

    */

    char chFrom, chTo;
    string rankStr, suitStr;
    int streamLength, freeCellIndex, x, y;
    bool fromCell, toCell;
    cardType curCard, destCard;
    string translation;

    chFrom = move[0];
    chTo = move[1];
    fromCell = ((chFrom >= 'a' && chFrom <= 'd') || (chFrom >= 'A' && chFrom <= 'D'));
    toCell = ((chTo >= 'a' && chTo <= 'd') || (chTo >= 'A' && chTo <= 'D'));
    
    if(chFrom <= '8' && chFrom >= '1' && toCell)
    { /* stream to cell */

        streamLength = h.stream[chFrom - '1'].size();
        suitStr = suitToEnglish(h.stream[chFrom - '1'][streamLength - 1]);
        rankStr = rankToEnglish(h.stream[chFrom - '1'][streamLength - 1]);
        translation = "Kick the " + rankStr + suitStr + " up.";
        
    } /* stream to cell */
    else
    {
        if(fromCell)
        { /* from cell */

            freeCellIndex = (chFrom >= 'a' && chFrom <= 'd') ? chFrom - 'a' : chFrom - 'A';
            suitStr = suitToEnglish(h.freeCells[freeCellIndex]);
            rankStr = rankToEnglish(h.freeCells[freeCellIndex]);

			/* cell to home */
            if(chTo == 'h' || chTo == 'H')
                translation = "Send the " + rankStr + suitStr + " home.";
            else
            { /* not going home */ 

                translation = "Move the " + rankStr + suitStr;
                
                
                if(h.stream[chTo - '1'].size() == 0)
                /* cell to empty stream */
                    translation += " to an empty spot.";
                else
                { /* cell to stream */
                    
                    suitStr = suitToEnglish(h.stream[chTo - '1'][h.stream[chTo - '1'].size() - 1]);
                    rankStr = rankToEnglish(h.stream[chTo - '1'][h.stream[chTo - '1'].size() - 1]);
                    translation += " to the " + rankStr + suitStr + ".";

                } /* cell to stream */
            } /* not going home */
        } /* from cell */
        else
        { /* from stream */

            /* find length of from stream */
            streamLength = h.stream[chFrom - '1'].size();
            translation = "Move the ";

            if(chTo == 'h' || chTo == 'H')
            { /* stream to home */
                
                translation = "Send the ";
                suitStr = suitToEnglish(h.stream[chFrom - '1'][streamLength - 1]);
                rankStr = rankToEnglish(h.stream[chFrom - '1'][streamLength - 1]);
                translation += rankStr + suitStr + " home.";

            } /* stream to home */
            else
            { /* stream to stream */
                
                if(h.stream[chTo - '1'].size() == 0)
                { /* stream to empty stream */
                    
                    /* count up free cells */
                    y = 0;
                    for(x = 0; x < NUM_OF_FREECELLS; x++)
                        y += getSuit(h.freeCells[x]) == 'x' ? 1 : 0;

                    /* start at the bottom of from stream */
                    x = h.stream[chFrom - '1'].size() - 1;
                    curCard = h.stream[chFrom - '1'][x];
                    x--;

                    while(x >= 0 && y != 0 && 
						getRankInt(h.stream[chFrom - '1'][x]) - getRankInt(curCard) == 1 &&
						!sameColors(h.stream[chFrom - '1'][x], curCard))
                    { /* while y != 0 and cards are alternating and decrementing */

                        y--;
                        curCard = h.stream[chFrom - '1'][x];
                        x--;

                    }

                    /* concatinate card */
                    x++;
                    curCard = h.stream[chFrom - '1'][x];
                    suitStr = suitToEnglish(curCard);
                    rankStr = rankToEnglish(curCard);
                    translation += rankStr + suitStr + " to an empty spot.";
                
                } /* stream to empty stream */
                else
                { /* stream to occupied stream */
                    
                    x = h.stream[chFrom - '1'].size() - 1;
                    curCard = h.stream[chFrom - '1'][x];
                    destCard = h.stream[chTo - '1'][h.stream[chTo - '1'].size() - 1];
                    while(getRankInt(curCard) + 1 != getRankInt(destCard))
                    {
                        x--;
                        curCard = h.stream[chFrom - '1'][x];
                    }
                    suitStr = suitToEnglish(curCard);
                    rankStr = rankToEnglish(curCard);
                    translation += rankStr + suitStr + " to the ";
                    suitStr = suitToEnglish(destCard);
                    rankStr = rankToEnglish(destCard);
                    translation += rankStr + suitStr + ".";

                } /* stream to occupied stream */

            } /* stream to stream */

        } /* from stream */

    } /* else not stream to cell */

	return translation;
	
} /* translate move */ 

/**************************************************************/

handType makeMove(handType h, string move)
{
    char chFrom, chTo, cardSuit;
    int i, cardRank, cellNum, streamNum1, streamNum2, streamLength, 
		numOfFreeCells, cardsToMove;
    cardType card;
    handType ret = h;

    chFrom = move[0];
    chTo = move[1];

    if(chTo == 'h' || chTo == 'H')
    { /* to home */
        
        if((chFrom >= 'a' && chFrom <= 'd') || (chFrom >= 'A' && chFrom <= 'D'))
        { /* cell to home */
            if(chFrom >= 'a' && chFrom <= 'd')
                cellNum = chFrom - 'a';
            else
                cellNum = chFrom - 'A';
            
            cardSuit = getSuit(ret.freeCells[cellNum]);
            cardRank = getRankInt(ret.freeCells[cellNum]);
        
            if(cardSuit == 'h' && cardRank - 1 == ret.lowestHeart)
            { /* heart */

                ret.freeCells[cellNum] = -1;
                ret.lowestHeart++;
                return ret;

            } /* heart */

            if(cardSuit == 'c' && cardRank - 1 == ret.lowestClub)
            { /* club */
                
                ret.freeCells[cellNum] = -1;
                ret.lowestClub++;
                return ret;

            } /* club */

            if(cardSuit == 's' && cardRank - 1 == ret.lowestSpade)
            { /* spade */

                ret.freeCells[cellNum] = -1;
                ret.lowestSpade++;
                return ret;

            } /* spade */

            if(cardSuit == 'd' && cardRank - 1 == ret.lowestDiamond)
            { /* diamond */

                ret.freeCells[cellNum] = -1;
                ret.lowestDiamond++;
                return ret;

            } /* diamond */

            /* not valid */
            return ret;
            
        } /* cell to home */

        if(chFrom >= '1' && chFrom <= '8')
        { /* stream to home */

            cellNum = chFrom - '1';
            cardSuit = getSuit(ret.stream[cellNum][ret.stream[cellNum].size() - 1]);
            cardRank = getRankInt(ret.stream[cellNum][ret.stream[cellNum].size() - 1]);

			ret.stream[cellNum].pop_back();
            if(cardSuit == 'h' && cardRank - 1 == ret.lowestHeart)
            { /* heart */

				ret.lowestHeart++;
                return ret;

            } /* heart */

            if(cardSuit == 'c' && cardRank - 1 == ret.lowestClub)
            { /* club */
                
				ret.lowestClub++;
				return ret;

            } /* club */

            if(cardSuit == 's' && cardRank - 1 == ret.lowestSpade)
            { /* spade */

				ret.lowestSpade++;
				return ret;
		   
            } /* spade */

            if(cardSuit == 'd' && cardRank - 1 == ret.lowestDiamond)
            { /* diamond */

				ret.lowestDiamond++;
				return ret;

            } /* diamond */

            /* not valid */
            return ret;
            
        } /* stream to home */

        return ret;
        
    } /* to home */

    if(chFrom <= '8' && chFrom >= '1' && ((chTo <= 'd' && chTo >= 'a') || (chTo <= 'D' && chTo >= 'A')))
    { /* stream to cell */

        streamNum1 = chFrom - '1';
        if(chTo <= 'd' && chTo >= 'a')
            cellNum = chTo - 'a';
        else
            cellNum = chTo - 'A';

        if(ret.stream[streamNum1].size() > 0 && getSuit(ret.freeCells[cellNum]) == 'x')
        {
			card = ret.stream[streamNum1].back();
			ret.stream[streamNum1].pop_back();
			ret.freeCells[cellNum] = card;
            return ret;
        }

        return ret;

    } /* stream to cell */

    if(((chFrom <= 'd' && chFrom >= 'a') || (chFrom <= 'D' && chFrom >= 'A')) && (chTo <= '8' && chTo >= '1'))
    { /* cell to stream */

        cellNum = chFrom - ((chFrom <= 'd' && chFrom >= 'a') ? 'a' : 'A');
        streamNum1 = chTo - '1';
        if(ret.stream[streamNum1].size() == 0)
        { /* cell to empty stream */
            
            if(getSuit(ret.freeCells[cellNum]) == 'x')
                return ret;
            card = ret.freeCells[cellNum];
            ret.stream[streamNum1].push_back(card);
            ret.freeCells[cellNum] = -1;
            return ret;

        } /* cell to empty stream */
        else
        { /* cell to full stream */
            
            streamLength = ret.stream[streamNum1].size();
            if(getSuit(ret.freeCells[cellNum]) == 'x' ||
                sameColors(ret.freeCells[cellNum], ret.stream[streamNum1][streamLength - 1]) ||
                getRankInt(ret.stream[streamNum1][streamLength - 1]) - getRankInt(ret.freeCells[cellNum]) != 1)
                return ret;
            card = ret.freeCells[cellNum];
            ret.stream[streamNum1].push_back(card);
            ret.freeCells[cellNum] = -1;
            return ret;

        } /* cell to full stream */

    } /* cell to stream */

    if(chFrom >= '1' && chFrom <= '8' && chTo >= '1' && chTo <= '8')
    { /* stream to stream */

        streamNum1 = chFrom - '1';
        streamNum2 = chTo - '1';
        if(ret.stream[streamNum1].size() == 0)
            return ret;

        if(ret.stream[streamNum2].size() == 0)    /* not moving multiple cards to an empty string correctly */
        { /* stream to empty stream */

            numOfFreeCells = 0;
            for(i = 0; i < NUM_OF_FREECELLS; i++)
                numOfFreeCells += getSuit(ret.freeCells[i]) == 'x' ? 1 : 0;

            /* count the movable cards */
            cardsToMove = 1;
            i = ret.stream[streamNum1].size() - 2;
            while(i > 0 && cardsToMove <= numOfFreeCells &&
				!sameColors(ret.stream[streamNum1][i], ret.stream[streamNum1][i + 1]) &&
				getRankInt(ret.stream[streamNum1][i]) - 1 == getRankInt(ret.stream[streamNum1][i + 1])
				)
                    
            {
                i--;
                cardsToMove++;
            }

            return migrateCards(ret, streamNum1, streamNum2, cardsToMove);

        } /* stream to empty stream */

        else
        { /* stream to occupied stream */
            
            /* check if source has card that can land in dest */
            i = ret.stream[streamNum1].size() - 1;
            while(i >= 0 && 
				(getRankInt(ret.stream[streamNum1][i]) + 1 != 
				getRankInt(ret.stream[streamNum2][ret.stream[streamNum2].size() - 1]) ||
				sameColors(ret.stream[streamNum1][i], 
				ret.stream[streamNum2][ret.stream[streamNum2].size() - 1]))
			)
                i--;
            if(getRankInt(ret.stream[streamNum1][i]) + 1 != 
                getRankInt(ret.stream[streamNum2][ret.stream[streamNum2].size() - 1]) ||
                sameColors(ret.stream[streamNum1][i], 
                ret.stream[streamNum2][ret.stream[streamNum2].size() - 1])
              )
                return ret;

            /* check if number of cards under top card is <= movable */ 
            if(ret.stream[streamNum1].size() - i > numOfMovables(h))
                return ret;

            /* check if alternating and descending */
            i++;
            while(i < ret.stream[streamNum1].size() &&
				!sameColors(ret.stream[streamNum1][i], ret.stream[streamNum1][i - 1]) &&
				getRankInt(ret.stream[streamNum1][i - 1]) - getRankInt(ret.stream[streamNum1][i]) == 1
			)
                i++;
            if(i != ret.stream[streamNum1].size())
                return ret;

            /* migrate cards (num of cards = numOfMovables) */
            i = ret.stream[streamNum1].size() - 1;
            while(getRankInt(ret.stream[streamNum1][i]) + 1 != 
				getRankInt(ret.stream[streamNum2][ret.stream[streamNum2].size() - 1]) ||
				sameColors(ret.stream[streamNum1][i], 
				ret.stream[streamNum2][ret.stream[streamNum2].size() - 1])
			)
                i--;
            return migrateCards(ret, streamNum1, streamNum2, ret.stream[streamNum1].size() - i);

        } /* stream to occupied stream */
        
    } /* stream to stream */

    return ret;

} /* make move */ 

/**************************************************************/

string noBrainer(handType h, string listOfMoves)
{
	cardType fromCard;
	int nbRed = ((h.lowestClub > h.lowestSpade) ? h.lowestSpade : h.lowestClub) + 2;
    int nbBlack = ((h.lowestDiamond > h.lowestHeart) ? h.lowestHeart : h.lowestDiamond) + 2;

    for(int x = 1; x < listOfMoves.size(); x += 2)
    {
        
        if(listOfMoves[x] == 'h')
        { /* if moving home */

            /* get card */
            if(listOfMoves[x - 1] >= 'a' && listOfMoves[x - 1] <= 'd')
                fromCard = h.freeCells[listOfMoves[x - 1] - 'a'];
            if(listOfMoves[x - 1] >= 'A' && listOfMoves[x - 1] <= 'A')
                fromCard = h.freeCells[listOfMoves[x - 1] - 'A'];
            if(listOfMoves[x - 1] >= '1' && listOfMoves[x - 1] <= '8')
				fromCard = h.stream[listOfMoves[x - 1] - '1'].back();

			/* if noBrainer, return shortened list */
            if(((getSuit(fromCard) == 'd' || getSuit(fromCard) == 'h') 
				&& getRankInt(fromCard) <= nbRed) 
				|| ((getSuit(fromCard) == 's' || getSuit(fromCard) == 'c') 
				&& getRankInt(fromCard) <= nbBlack)
			)
                return listOfMoves.substr(x - 1, 2);
                
        } /* if moving home */
    } /* for x goes trough list */

	return listOfMoves;
} /* no brainer */

/**************************************************************/

string sortMoves(handType h, string moveList)
{ /* moveList is already populated */

    int numberOfMoves, x, streamLength, i, j, diff1, diff2;
    bool swapped;
    cardType fromCard;
    handType swapHand; // if 40 needs to change, so does the comparison to numberOfMoves
    vector<handType> handList;
    char ch;

    /* get number of moves */
    numberOfMoves = moveList.size() / 2;

	for(x = 0; x < numberOfMoves; x++)
	{ /* make list of hands based on moveList */

		swapHand = makeMove(h, moveList.substr(2 * x, 2));
		handList.push_back(swapHand);

	} /* make list of hands based on moveList */   
	
	/*    first, sort points */
	do
	{ /* while swapped */

		swapped = false;
		for(i = 0; i < numberOfMoves - 1; i++)
		{
			if(handCompareForSorting(handList[i], handList[i + 1]) < 0)
			{
				swapped = true;
				swapHand = handList[i];
				handList[i] = handList[i + 1];
				handList[i + 1] = swapHand;
				ch = moveList[2 * i];
				moveList[2 * i] = moveList[2 * i + 2];
				moveList[2 * i + 2] = ch;
				ch = moveList[2 * i + 1];
				moveList[2 * i + 1] = moveList[2 * i + 3];
				moveList[2 * i + 3] = ch;
			} /* if hand points is out of order, then swap */

		} /* for i goes from 0 to number of moves - 2 */

	} while(swapped);

    return moveList;
    
} /* sort moves */ 

/**************************************************************/

int underCount(handType h)
{
	return 1;
	int x, y, count;

    count = 0;
    for(x = 0; x < 8; x++)
    { /* go through streams */
		
		int tempCount = 0;
		
        y = h.stream[x].size() - 1;
        while(y > 0 && getRankInt(h.stream[x][y - 1]) >= getRankInt(h.stream[x][y]))
        { /* go through cards */
			
            y--;
            tempCount++;
            
        } /* go through cards */
        
        count += y >= 0 ? tempCount : 0;
        
    } /* go through streams */
    
    return count;
    
} /* under count */

/**************************************************************/

int coverCount(handType h)
{
	int x, y, count;

    count = 0;
    for(x = 0; x < 8; x++)
    { /* go through streams */

		/* go through cards */
        y = h.stream[x].size() - 1;
        while(y > 0 && getRankInt(h.stream[x][y - 1]) >= getRankInt(h.stream[x][y]))
            y--;
        count += y >= 0 ? y : 0;
        
    } /* go through streams */
    
    return count;
    
} /* cover count */

/**************************************************************/

string handDescent(handType origin, handType h, string journey, int depth, int *depthLimit, int originalLimit, int deadline)
{ /*find most efficient win, and return the future path in return list */

/*	
if(depth % 10 == 0)
{
if(rand() % 10 == 0) cout << endl;
cout << "depth = " << depth << " with a limit of " << (*depthLimit) << endl;
}

if((*depthLimit) != MAX_DEPTH) return "";

if((*depthLimit) < originalLimit) return "";
if((*depthLimit) < 93) return "";
* */

callCount++;

if(deadline - time(NULL) != lastDisplay /*&& (deadline - time(NULL)) % 10 == 0*/)
{
lastDisplay = deadline - time(NULL);
cout << lastDisplay << " seconds remain callCount = " << callCount << endl;
}
if(time(NULL) >= deadline) return "";


	/* check if deep max - cards count is met (fail) */
	int cardsCount = 0;
	for(int x = 0; x < 8; x++)
		cardsCount += h.stream[x].size();
	if((*depthLimit) - cardsCount <= depth)
		return "";
		
	/* check if cover count is zero (success) */
	if(coverCount(h) == 0)
	{
		(*depthLimit) = cardsCount + depth;
cout << "depthLimit adjusted to " << (*depthLimit) << "; " << (deadline - time(NULL)) << " seconds remaining" << endl;
		if(cardsCount > 0)
			return " move remaining cards home";
		else
			return " this concludes move list.";
	}
	
	/* get list of possible moves */
	string listOfMoves = listMoves(h);
	listOfMoves = noBrainer(h, listOfMoves);
	if(listOfMoves.size() > 2)
		listOfMoves = sortMoves(h, listOfMoves);

	/* check if move list is empty (fail) */
	if(listOfMoves.size() == 0)
		return "";
		
	string adjMoveList = "";
	
	if(true)
	{ /* start of scope for journey hands */
		
		/* get list of journey hands */
		vector<handType> journeyHands;
		
		for(int x = 0; x < journey.size(); x += 2)
		{ /* for each move in journey list */
			
			string tempMove = journey.substr(x, 2);
			
			/* generate temp hand */
			handType tempHand, lastHand;
			if(journeyHands.size() == 0)
				lastHand = origin;
			else
				lastHand = journeyHands.back();
				
			tempHand = makeMove(lastHand, tempMove);
			journeyHands.push_back(tempHand);
			
		} /* for each move in journey list */
		
		for(int x = 0; x < listOfMoves.size(); x += 2)
		{ /* for each move in move list */
			
			/* generate new hand */
			string tempMove = listOfMoves.substr(x, 2);
			handType tempHand = makeMove(h, tempMove);
			
			/* search for new hand in journey hand list */
			bool foundit = false;
			
			for(int y = 0; !foundit && y < journeyHands.size(); y++)
				foundit = compareHands(tempHand, journeyHands[y]);
				
			/* if new hand exists in journey hand list, remove that move from list */	
			if(!foundit)
				adjMoveList += tempMove;
				
		} /* for each move in move list */
		
		/* get rid of journey list (automatically done at end of scope) */
		
	} /* end of scope for journey hands */
	
	/* check if move list is empty (fail) */
	if(adjMoveList.size() == 0)
		return "";
/*		
if(adjMoveList.size() > 10)
adjMoveList = adjMoveList.substr(0, 10);
*/
	/* initialize suffix string to empty string */
	string suffix = "";
	string curMove;
	string retMove = "";
	
	for(int moveIndex = 0; moveIndex < adjMoveList.size(); moveIndex += 2)
	{ /* for each move in move list */
		
		/* generate new hand */
		curMove = adjMoveList.substr(moveIndex, 2);
		handType tempHand = makeMove(h, curMove);
		
		/* recursively call hand decent with new hand into temp string */
		string tempString = handDescent(origin, tempHand, journey + curMove, depth + 1, depthLimit, originalLimit, deadline);
		
		/* if temp string is not empty */
		if(tempString.size() > 0)
		
			/* if suffix string is empty, or temp string is shorter than suffix string, update it */
			if(suffix.size() == 0 || tempString.size() < suffix.size())
			{
				suffix = tempString;
				retMove = curMove;
			}
				
	} /* for each move in move list */
	
	/* return concatinated string */
	return retMove + suffix;
	
} /*find most efficient win, and return the future path in return list */

/**************************************************************/

bool compareHands(handType a, handType b)
{ /* return true if a = b, false otherwise */
	
	/*
ifstream f;
f.open("flag");
int flag;
f >> flag;
f.close();
if(flag == 1)
{
cout << "a:" << endl;
displayHand(a);
cout << "b:" << endl;
displayHand(b);
cout << endl;
}
	*/
	/* check home matching */
	if(a.lowestHeart != b.lowestHeart || 
		a.lowestClub != b.lowestClub || 
		a.lowestSpade != b.lowestSpade || 
		a.lowestDiamond != b.lowestDiamond)
		return false;
		
	/* check if each has the same number of freecells */
	int aFreeNum = 0;
	int bFreeNum = 0;
	
	for(int x = 0; x < NUM_OF_FREECELLS; x++)
	{
		if(getSuit(a.freeCells[x]) == 'x')
			aFreeNum++;
			
		if(getSuit(b.freeCells[x]) == 'x')
			bFreeNum++;
	} /* for each freecell */
	
	if(aFreeNum != bFreeNum)
		return false;
		
	/* check if all a's free cards are in b's free cards */
	for(int x = 0; aFreeNum > 0 && x < NUM_OF_FREECELLS; x++)
	{ /* for each of a's free cells */
		
		if(getSuit(a.freeCells[x]) != 'x')
		{ /* if there is a card in the freecell */
			
			bool foundIt = false;
			for(int y = 0; !foundIt && y < NUM_OF_FREECELLS; y++)
				foundIt = a.freeCells[x] == b.freeCells[y];
			if(!foundIt)
				return false;
				
			aFreeNum--;
			
		} /* if there is a card in the freecell */
		
	} /* for each of a's free cells */
	
	/* check if a and b have same number of empty streams */
	int aFreeStream = 0;
	int bFreeStream = 0;
	for(int x = 0; x < 8; x++)
	{
		if(a.stream[x].size() == 0)
			aFreeStream++;
		if(b.stream[x].size() == 0)
			bFreeStream++;
	}
	if(aFreeStream != bFreeStream)
		return false;
		
	/* for each stream in a */	
	for(int x = 0; x < 8; x++)
		
		/* check if that stream exists in b */
		if(a.stream[x].size() > 0)
		{ /* a's stream is not empty */
			
			bool foundit = false;
			for(int y = 0; !foundit && y < 8; y++)
				foundit = cardVectorMatch(a.stream[x], b.stream[y]);
			if(!foundit)
				return false;
				
		} /* a's stream is not empty */
		
	return true;
	
} /* compare hands */

/**************************************************************/

bool cardVectorMatch(vector<cardType> a, vector<cardType> b)
{ /* return true if vectors match, false otherwise */
	
	/* check sizes */
	int size = a.size();
	if(size != b.size())
		return false;
		
	/* check each value */
	for(int x = 0; x < size; x++)
		if(a[x] != b[x])
			return false;
	
	return true;
	
} /* card vector match */

/**************************************************************/

int dumpingCount(handType h)
{
	int ret = 0;
	
	for(int x = 0; x < 8; x++)
	{ /* for each column */
		
		bool descending = true;
		
		for(int y = 1; descending && y < h.stream[x].size(); y++)
			descending = getRankInt(h.stream[x][y]) >= getRankInt(h.stream[x][y - 1]);

		/* if descending, add size of column to total */
		if(descending)
			ret += h.stream[x].size();
			
	} /* for each column */
			
	return ret;
}

/**************************************************************/

void outputSolver(handType h, string solution)
{
	/* send to solution.txt a plain text translation of the solution */
	handType h2 = h;
	
	int x = 0;
	ofstream f;
	f.open("solution.txt");
	string move;
		
	while(x < solution.size() && solution[x] != ' ')
	{ /* for each move in the shorterString */
		
		/* get english translation */
		move = solution.substr(x, 2);
		string outString = translateMove(h2, move);
		
		/* output to file */
		int index = x / 2 + 1;
		f << index << " : (" << move << ") " << outString << "(" << coverCount(h2) << ")" << endl;
	
		x += 2;
		h2 = makeMove(h2, move);
		
	} /* for each move in the shorterString */
		
	/* close file */
	f << solution.substr(x, solution.size() - x) << endl;
	f.close();
	
} /* output solver */

/**************************************************************/

int usefulMove(handType h, string move)
{
	/* if move to home, then useful */
	if(move[1] == 'h')
		return 1;
		
	/* if move from or to freecell, then useful */
	if((move[0] <= 'd' && move[0] >= 'a') || (move[1] <= 'd' && move[1] >= 'a'))
		return 1;
		
	/* if move changes number of movables, then useful */
	handType h2 = makeMove(h, move);
	if(coverCount(h) != coverCount(h2))
		return 1;
		
	/* if move to empty stream, then useful */
	int toIndex = move[1] - '1';
	if(h.stream[toIndex].size() == 0)
		return 1;
		
	/* get value of bottom card in dest dc */
	cardType dc = h.stream[toIndex].back();
	
	/* find card in source that is one less than value of dc (sc) */
	int fromIndex = move[0] - '1';
	int cardIndex = h.stream[fromIndex].size() - 1;
	cardType sc = h.stream[fromIndex][cardIndex];
	while(getRankInt(sc) + 1 > getRankInt(dc))
	{
		cardIndex--;
		sc = h.stream[fromIndex][cardIndex];
	}
	
	/* if card above sc has same rank and color as dc, then not useful */
	cardIndex--;
	sc = h.stream[fromIndex][cardIndex];
	if(getRankInt(sc) == getRankInt(dc) && sameColors(sc, dc))
		return 0;
		
	/* useful */
	return 1;
	
} /* useful move */

/**************************************************************/

string breadthFirst(handType origin, int depthLimit, int deadline)
{
int lastDepth = -1;
int queueCount = -1;
	queue<handType> hQ;
	queue<string> journeyQ;
	queue<int> depthQ;
	bool cleanup = false;
	handType threshold;
	
	/* push starting position */
	hQ.push(origin);
	journeyQ.push("");
	depthQ.push(0);
	
	handType h;
	string journey;
	int depth;
	
	while(hQ.size() > 0)
	{ /* while queue is not empty */
		
		/* pop queue into current position */
		h = hQ.front();
		hQ.pop();
		journey = journeyQ.front();
		journeyQ.pop();
		depth = depthQ.front();
		depthQ.pop();
		
		if(depth != lastDepth)
		{
			lastDepth = depth;
			queueCount = depthQ.size();
			cout << "depth = " << depth << " and size of queue = " << depthQ.size() << endl;
			
			if(queueCount > 10000)
			{
				cleanup = true;
				threshold = getThreshold(hQ);
				cout << "cleaning up" << endl;
			}
			else
				cleanup = false;
		}
		
queueCount--;
if(queueCount % 1000 == 0)
cout << "depth = " << depth << "; current depth count = " << queueCount 
<< "; coverCount = " << coverCount(h) << "; movables = " 
<< numOfMovables(h) << endl;

		bool fail = false;
		
		/* check if deep max - cards count is met (fail) */
		int cardsCount = 0;
		for(int x = 0; x < 8; x++)
			cardsCount += h.stream[x].size();
		if(depthLimit - cardsCount <= depth)
			fail = true;
			
		/* check if need to clean up */
		int handComparison = handCompareForSorting(h, threshold);
		if(cleanup && handComparison > 0)
			fail = true;
		if(!fail && cleanup && handComparison == 0 && rand() % 100 != 0)
			fail = true;
			
		if(!fail)
		{ /* first chance to fail */
					
			/* check if cover count is zero (success) */
			if(coverCount(h) == 0)
			{
				if(cardsCount > 0)
					return journey + " move remaining cards home";
				else
					return journey + " this concludes move list.";
			}
		
			/* get list of possible moves */
			string listOfMoves = listMoves(h);
			listOfMoves = noBrainer(h, listOfMoves);
			if(listOfMoves.size() > 2)
				listOfMoves = sortMoves(h, listOfMoves);
			
			/* check if move list is empty (fail) */
			if(listOfMoves.size() == 0)
				fail = true;
		
			if(!fail)
			{ /* second chance at fail */
				
				string adjMoveList = "";
				
				if(true)
				{ /* start of scope for journey hands */

					/* get list of journey hands */
					vector<handType> journeyHands;
					
					for(int x = 0; x < journey.size(); x += 2)
					{ /* for each move in journey list */
						
						string tempMove = journey.substr(x, 2);
						
						/* generate temp hand */
						handType tempHand, lastHand;
						if(journeyHands.size() == 0)
							lastHand = origin;
						else
							lastHand = journeyHands.back();
				
						tempHand = makeMove(lastHand, tempMove);
						journeyHands.push_back(tempHand);
						
					} /* for each move in journey list */
					
					
					for(int x = 0; x < listOfMoves.size(); x += 2)
					{ /* for each move in move list */
						
						/* generate new hand */
						string tempMove = listOfMoves.substr(x, 2);
						handType tempHand = makeMove(h, tempMove);
			
						/* search for new hand in journey hand list */
						bool foundit = false;
			
						for(int y = 0; !foundit && y < journeyHands.size(); y++)
							foundit = compareHands(tempHand, journeyHands[y]);
							
						/* if new hand exists in journey hand list, remove that move from list */
						if(!foundit)
							adjMoveList += tempMove;
				
					} /* for each move in move list */
					
					/* get rid of journey list (automatically done at end of scope) */
					
				} /* start of scope for journey hands */
				
				/* check if move list is empty (fail) */
				if(adjMoveList.size() == 0)
					fail = true;
				
				if(!fail)
				{ /* third chance at fail */
					
					/* initialize suffix string to empty string */
					string suffix = "";
					string curMove;
	
					for(int moveIndex = 0; moveIndex < adjMoveList.size(); moveIndex += 2)
					{ /* for each move in move list */
						
						/* generate new hand */
						curMove = adjMoveList.substr(moveIndex, 2);
						handType tempHand = makeMove(h, curMove);
						
						/* push new position */
						hQ.push(tempHand);
						journeyQ.push(journey + curMove);
						depthQ.push(depth + 1);
						
					} /* for each move in move list */
						
				} /* third chance at fail */
				
			} /* second chance at fail */
			
		} /* first chance to fail */
		
	} /* while queue is not empty */
	
	/* fail */
	return "";
	
} /* breadth first */

/**************************************************************/

int handCompareForSorting(handType a, handType b)
{
	/* return -1 if a is more desireable than b, 0 if equal, 1 otherwise */
	int x, y;
	
	/* compare cover count */
	x = coverCount(a);
	y = coverCount(b);
	if(x < y)
		return -1;
	if(x > y)
		return 1;
		
	/* compare num of movable */
	x = numOfMovables(a);
	y = numOfMovables(b);
	if(x < y)
		return 1;
	if(x > y)
		return -1;
		
	/* compare dumping ground value */
	x = dumpingCount(a);
	y = dumpingCount(b);
	if(x < y)
		return 1;
	if(x > y)
		return -1;
		
	/* compare under count */
	x = underCount(a);
	y = underCount(b);
	if(x < y)
		return -1;
	if(x > y)
		return 1;
		
	/* compare points */
	x = handPoints(a);
	y = handPoints(b);
	if(x < y)
		return 1;
	if(x > y)
		return -1;
	
	/* do discernable difference in desirability */	
	return 0;
	
} /* hand compare for sorting */

/**************************************************************/

handType getThreshold(queue<handType> hQ)
{
	/* sort hand, and return hand at 1% mark */
	
	vector<handType> ccList;
	queue<handType> tempQ = hQ;
	
	while(!tempQ.empty())
	{ /* for each hand in hQ */
		
		/* push cover count onto ccList */
		handType h = tempQ.front();
		ccList.push_back(h);
		tempQ.pop();
		
	} /* for each hand in hQ */
	
	if(ccList.size() > 100000)
		return iCantBelieveItsNotQuicksort(&ccList, 0, ccList.size() - 1, 1000);
	else
		return iCantBelieveItsNotQuicksort(&ccList, 0, ccList.size() - 1, ccList.size() / 500);

	/* sort ccList */
	quickSort(&ccList, 0, ccList.size());
	
	/* calculate threshold index */
	int tIndex = ccList.size() / 100;
	
	cout << "coverCount for threshold = " << coverCount(ccList[tIndex]) << endl;
	
	/* return threshold */
	return ccList[tIndex];
	
} /* get threshhold */

/**************************************************************/

void quickSort(vector<handType> *v, int firstIndex, int lastIndex)
{
if(lastIndex - firstIndex > 10000) 
cout << "quicksort: " << firstIndex << " - " << lastIndex << endl;
	/* if last index - first index < 2, then done */
	if(lastIndex - firstIndex < 2)
		return;
		
	if(lastIndex - firstIndex < 10)
	{ /* if last index - first index < 10 */
		
		/* bubble sort */
		bool swapped = true;
		
		do
		{ /* while swapped */
			
			swapped = false;
			for(int x = firstIndex; x < lastIndex; x++)
			{
				if(handCompareForSorting((*v)[x], (*v)[x + 1]) > 0)
				{
					swapped = true;
					handType temp = (*v)[x];
					(*v)[x] = (*v)[x + 1];
					(*v)[x + 1] = temp;
					
				} /* swap if out of order */
				
			} /* for x goes through all elements */
			
		} while(swapped);
		
		/* done */
		return;
		
	} /* if last index - first index < 10 */
	
	/* let pivot be in the middle */
	int pIndex = (firstIndex + lastIndex) / 2;
	
	/* set indecies */
	int lowIndex = firstIndex;
	int highIndex = pIndex + 1;
	
	/* advance indecies until end or find something that belongs to the other side */
	while(lowIndex < pIndex && handCompareForSorting((*v)[lowIndex], (*v)[pIndex]) <= 0)
		lowIndex++;
	while(highIndex <= lastIndex && handCompareForSorting((*v)[highIndex], (*v)[pIndex]) >= 0)
		highIndex++;
	
	while(lowIndex < pIndex && highIndex <= lastIndex)
	{ /* while low < pivot and high < last */
		
		/* swap low and high elements */
		handType temp = (*v)[lowIndex];
		(*v)[lowIndex] = (*v)[highIndex];
		(*v)[highIndex] = temp;
		
		/* advance indecies until end or find something that belongs to the other side */
		while(lowIndex < pIndex && compareHands((*v)[lowIndex], (*v)[pIndex]) <= 0)
			lowIndex++;
		while(highIndex <= lastIndex && compareHands((*v)[highIndex], (*v)[pIndex]) >= 0)
			highIndex++;
		
	} /* while low < pivot and high < last */
	
	while(lowIndex < pIndex)
	{ /* while low < pivot */
		
		/* do double swap */
		handType temp = (*v)[lowIndex];
		(*v)[lowIndex] = (*v)[pIndex - 1];
		(*v)[pIndex - 1] = temp;
		temp = (*v)[pIndex - 1];
		(*v)[pIndex - 1] = (*v)[pIndex];
		(*v)[pIndex] = temp;
		pIndex--;
		
		/* advance low until pivot or find something too big */
		while(lowIndex < pIndex && compareHands((*v)[lowIndex], (*v)[pIndex]) <= 0)
			lowIndex++;
			
	} /* while low < pivot */
	
	while(highIndex <= lastIndex)
	{ /* while high < last */
		
		/* do double swap */
		handType temp = (*v)[highIndex];
		(*v)[highIndex] = (*v)[pIndex + 1];
		(*v)[pIndex + 1] = temp;
		temp = (*v)[pIndex + 1];
		(*v)[pIndex + 1] = (*v)[pIndex];
		(*v)[pIndex] = temp;
		pIndex++;
		
		/* advance until end or find something too small */
		while(highIndex <= lastIndex && compareHands((*v)[highIndex], (*v)[pIndex]) >= 0)
			highIndex++;
			
	} /* while high < last */
	
	/* quick sort lower end */
	quickSort(v, firstIndex, pIndex - 1);
	
	/* quick sort higher end */
	quickSort(v, pIndex + 1, lastIndex);
	
} /* quickSort */

/**************************************************************/

handType iCantBelieveItsNotQuicksort(vector<handType> *v, int firstIndex, int lastIndex, int target)
{
//if(lastIndex - firstIndex > 10000) 
cout << "quickselect: " << firstIndex << " - " << lastIndex << " looking for " << target << endl;
	/* let pivot be in the middle */
	int pIndex = (firstIndex + lastIndex) / 2;
	
	/* set indecies */
	int lowIndex = firstIndex;
	int highIndex = pIndex + 1;
	
	/* advance indecies until end or find something that belongs to the other side */
	while(lowIndex < pIndex && handCompareForSorting((*v)[lowIndex], (*v)[pIndex]) <= 0)
		lowIndex++;
	while(highIndex <= lastIndex && handCompareForSorting((*v)[highIndex], (*v)[pIndex]) >= 0)
		highIndex++;
	
	while(lowIndex < pIndex && highIndex <= lastIndex)
	{ /* while low < pivot and high < last */
		
		/* swap low and high elements */
		handType temp = (*v)[lowIndex];
		(*v)[lowIndex] = (*v)[highIndex];
		(*v)[highIndex] = temp;
		
		/* advance indecies until end or find something that belongs to the other side */
		while(lowIndex < pIndex && compareHands((*v)[lowIndex], (*v)[pIndex]) <= 0)
			lowIndex++;
		while(highIndex <= lastIndex && compareHands((*v)[highIndex], (*v)[pIndex]) >= 0)
			highIndex++;
		
	} /* while low < pivot and high < last */
	
	while(lowIndex < pIndex)
	{ /* while low < pivot */
		
		/* do double swap */
		handType temp = (*v)[lowIndex];
		(*v)[lowIndex] = (*v)[pIndex - 1];
		(*v)[pIndex - 1] = temp;
		temp = (*v)[pIndex - 1];
		(*v)[pIndex - 1] = (*v)[pIndex];
		(*v)[pIndex] = temp;
		pIndex--;
		
		/* advance low until pivot or find something too big */
		while(lowIndex < pIndex && compareHands((*v)[lowIndex], (*v)[pIndex]) <= 0)
			lowIndex++;
			
	} /* while low < pivot */
	
	while(highIndex <= lastIndex)
	{ /* while high < last */
		
		/* do double swap */
		handType temp = (*v)[highIndex];
		(*v)[highIndex] = (*v)[pIndex + 1];
		(*v)[pIndex + 1] = temp;
		temp = (*v)[pIndex + 1];
		(*v)[pIndex + 1] = (*v)[pIndex];
		(*v)[pIndex] = temp;
		pIndex++;
		
		/* advance until end or find something too small */
		while(highIndex <= lastIndex && compareHands((*v)[highIndex], (*v)[pIndex]) >= 0)
			highIndex++;
			
	} /* while high < last */
	
	/* check if found it */
	if(pIndex == target)
	{
		cout << "coverCount: " << coverCount((*v)[pIndex]) << "\tmovables: " << numOfMovables((*v)[pIndex]) << endl;
		return (*v)[pIndex];
	}
	
	if(target < pIndex)
	
		/* quick select lower end */
		return iCantBelieveItsNotQuicksort(v, firstIndex, pIndex - 1, target);
	
	/* quick select higher end */
	return iCantBelieveItsNotQuicksort(v, pIndex + 1, lastIndex, target);
	
} /* iCantBelieveItsNotQuicksort */

/**************************************************************/

handType readFile(void)
{
	ifstream f;
	f.open("handFile.txt");
	/***/
	/* initialize hand */
	/* load freecells */
	/* load streams */
	/* load lowest */
	/* freecell loop -*/
	/* streams */
	
	f.close();
} /* read file */

/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
