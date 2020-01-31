/*
 * lab1.c
 *
 *  Created on: Jan 29, 2020
 *      Author: Jose Li
 */

/************** ECE2049 DEMO CODE ******************/
/**************  13 March 2019   ******************/
/***************************************************/

#include <msp430.h>
# include <stdio.h>

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"

// Function Prototypes
void swDelay(char numLoops);
unsigned char* getCardString(unsigned int card);
enum GAME_STATE {welcome = 0, cut =1, shuffle=2, bet=3, draw=4};

// Declare globals here


void main(void){
    // ... code
    unsigned char currKey=0, dispSz = 3;
    unsigned char dispThree[3] = {NULL, NULL, NULL};
    unsigned int deck[52] = {0};
    unsigned int topDeckIndex = 0; // represents the current index of the top of the deck

    unsigned char playerHand[10][3];
    unsigned char cpuHand[10][3];

    unsigned char playerBet;
    unsigned char cpuBet;

    // variables for lenght of players hand
    int playerHandLen;
    int cpuHandLen;


    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
                                     // You can then configure it properly, if desired

        // Useful code starts here
        initLeds();

        configDisplay();
        configKeypad();

        // *** Intro Screen ***
        Graphics_clearDisplay(&g_sContext); // Clear the display

        // Draw a box around everything because it looks nice
        Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
        Graphics_drawRectangle(&g_sContext, &box);
        // Refresh the display so it shows the new data
        Graphics_flushBuffer(&g_sContext);

    enum GAME_STATE state = welcome;


    while (1)    // Forever loop
    {

//        currKey = getKey();

        switch(state){
        case 0: // Start Screen

            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, "MSP430", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "BLACKJACK", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Press *", AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "to start", AUTO_STRING_LENGTH, 48, 75, TRANSPARENT_TEXT);

            // Refresh the display so it shows the new data
            Graphics_flushBuffer(&g_sContext);

            while(currKey != '*'){
                currKey = getKey();
            }
            if(currKey == '*'){
                Graphics_clearDisplay(&g_sContext); // Clear the display
                state = cut;
                currKey = NULL;
                dispThree[0] = NULL;
                dispThree[1] = NULL;
                dispThree[2] = NULL;

            }
            break;

        case cut: // Shuffle deck when user presses '*'
            // get input from 1 to 15 for cutting the deck
            // shuffle the deck using rand()
            // display to LCD current hand and opponent's hand
            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, "CUT THE DECK", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "enter number", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "from 0 - 15", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

            // Refresh the display so it shows the new data
            Graphics_flushBuffer(&g_sContext);

            // TODO SANITIZE INPUT if number is outside range *****************************************************************************************
            while(currKey != '*'){
                currKey = getKey();

                // if user has put input before, save input into second index
                if (dispThree[0] == NULL){
                    dispThree[0] = currKey;
                    Graphics_drawStringCentered(&g_sContext, dispThree, dispSz, 48, 65, OPAQUE_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                    Graphics_flushBuffer(&g_sContext);
                }else{
                    dispThree[1] = currKey;
                }
                dispThree[2] = ' ';
                if (dispThree[1]){
                    break;
                }
            }

            Graphics_drawStringCentered(&g_sContext, dispThree, dispSz, 48, 65, OPAQUE_TEXT);
            Graphics_flushBuffer(&g_sContext);

            // get ready to switch state to shuffle
            if (dispThree[1])
            {
                // wait awhile before clearing LEDs
                currKey = NULL;
                state = shuffle;
                Graphics_clearDisplay(&g_sContext); // Clear the display

            }
            else if(currKey == '*'){
                Graphics_clearDisplay(&g_sContext); // Clear the display
                state = welcome;
                currKey = NULL;
            }
            break;


        case shuffle: // Wait for input from player
            // read input from player regarding bet
            // light up LED
            // Calculate CPU bet
            // display to LCD
            // let player input difference of bet or fold (*)
            // once bets are done, start new state of adding cards

            // Set the random seed
            // TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO change true
            if (true){
                int a = dispThree[0] - '0';
                int b = dispThree[1] - '0';
                unsigned int seed = a*10 + b;
                srand(seed);
                unsigned char seedChar = seed + '0';

                if ((seed >= 0) && (seed <= 15)){
                    setLeds(seedChar - 0x30);
                }
            }

            unsigned int deckIndex;
            int i;
            // randomly assign a number from 1 to 52 into deck array
            for (i = 1; i < 53; i++){
                // find random unoccupied location
                do{
                    deckIndex = rand() % 52;
                }while(deck[deckIndex] != 0);

                // put card on deck
                deck[deckIndex] = i;
            }

            // Give 2 cards to player and cpu from deck
            strcpy(playerHand[0], getCardString(deck[topDeckIndex]));
            topDeckIndex++;
            strcpy(playerHand[1], getCardString(deck[topDeckIndex]));
            topDeckIndex++;
            strcpy(playerHand[2], "\0");

            strcpy(cpuHand[0], getCardString(deck[topDeckIndex]));
            topDeckIndex++;
            strcpy(cpuHand[1], getCardString(deck[topDeckIndex]));
            topDeckIndex++;
            strcpy(cpuHand[2], "\0");

            // PRINT OUT PLAYERS HANDS
            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
            // print out player's cards
            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
            for (i = 0; i< playerHandLen;i++){
                if(playerHand[i] == '\0')
                    break;
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 20 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i] == '\0')
                    break;
                if (i == 1){
                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 20 + i*25, 50, OPAQUE_TEXT);
                    continue;
                }
                // TODO FIX PRINTING OF 10s ******************************************************************************************************10
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 20 + i*25, 50, OPAQUE_TEXT);
            }
            Graphics_flushBuffer(&g_sContext);

            // get ready to switch state to bet
            state = bet;
            swDelay(2);
            setLeds(0);
            break;

        case bet: // Adding cards state
            // read input from player either 1 or #
            // got check both hands of players (check for losers)
            // move to cpu's turn

            // Write some text to the display
            Graphics_drawStringCentered(&g_sContext, "Bet:1-2-4-8", AUTO_STRING_LENGTH, 40, 60, TRANSPARENT_TEXT);
            // Refresh the display so it shows the new data
            Graphics_flushBuffer(&g_sContext);

            // wait until player puts a bet 1, 2, 4, 8
            while(1){
                currKey = getKey();
                if((currKey == '1') || (currKey == '2') || (currKey == '4') || (currKey == '8'))
                    break;
            }


            Graphics_clearDisplay(&g_sContext); // Clear the display

            // PRINT OUT PLAYERS HANDS
            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
            // print out player's cards
            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
            for (i = 0; i< playerHandLen;i++){
                if(playerHand[i] == '\0')
                    break;
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 20 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i] == '\0')
                    break;

                // TODO UNCOMMENTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTtttttttttttttttttttttttttttttttttttttttttttttt
//                if (i == 1){
//                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 20 + i*25, 50, OPAQUE_TEXT);
//                    continue;
//                }
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 20 + i*25, 50, OPAQUE_TEXT);
            }

            // store the bet of the player
            playerBet = currKey;
            dispThree[0] = ' ';
            dispThree[1] = playerBet;
            dispThree[2] = ' ';

            // display players's bet
            Graphics_drawStringCentered(&g_sContext, "Your bet is: ", AUTO_STRING_LENGTH, 45, 60, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, dispThree, AUTO_STRING_LENGTH, 80, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);

            // light up LDE based on bet
            setLeds(playerBet - 0x30);

            // calculate and display cpu's bet
            cpuBet = calculateBet(getValue(&cpuHand, cpuHandLen) , playerBet);
            dispThree[0] = ' ';
            dispThree[1] = cpuBet;
            dispThree[2] = ' ';
            Graphics_drawStringCentered(&g_sContext, "CPU's bet is: ", AUTO_STRING_LENGTH, 50, 75, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, dispThree, AUTO_STRING_LENGTH, 90, 75, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);

            int difference;
            while(currKey != '*'){
                currKey = getKey();
                difference = playerBet - currKey;
                if(difference == cpuBet)
                    break;
            }
            if(currKey == '*'){
                // TODO send to you loose state ********************************************************************************
                state = welcome;
                Graphics_clearDisplay(&g_sContext); // Clear the display
            }else{
                // TODO DOES IT NEED TO DISPLAY THE NEW BET VALUE????? ****************************************
                playerBet = cpuBet;
                state = draw;
                Graphics_clearDisplay(&g_sContext); // Clear the display
            }
            break;

        case draw: // CPU turn
            // decide if hold or draw
            // check for loss
            // move to player's turn

            // display hands
            // wait for input either 1 or #
            //      add card
            // reenter state


            // PRINT OUT PLAYERS HANDS
            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
            // print out player's cards
            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
            for (i = 0; i< playerHandLen;i++){
                if(playerHand[i] == '\0')
                    break;
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 20 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i] == '\0')
                    break;

                // TODO UNCOMMENTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTtttttttttttttttttttttttttttttttttttttttttttttt
//                if (i == 1){
//                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 20 + i*25, 50, OPAQUE_TEXT);
//                    continue;
//                }
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 20 + i*25, 50, OPAQUE_TEXT);
            }










            break;
        case 5: // Win or lost state
            // display celebration or loss statement
            break;
        }
    }  // end while (1)


}


unsigned char* getCardString(unsigned int card){

    // card string
    unsigned char* string = malloc(3);

    unsigned int reminder = card % 13 + 1;

    string[1] = '-';

    if (card >= 1 && card <= 13){
        // hearts
        string[2] = 'H';
    }else if (card >= 14 && card <= 26){
        // Diamonds
        string[2] = 'D';
    }else if (card >= 27 && card <= 39){
        // Clubs
        string[2] = 'C';
    }else if(card >= 40 && card <= 52){
        // Spade
        string[2] = 'S';
    }

    if(reminder == 11){
        // J
        string[0] = 'J';
    }else if(reminder == 12){
        // Q
        string[0] = 'Q';
    }else if (reminder == 13){
        // King
        string[0] = 'K';
    }else if (reminder == 1){
        // Ace
        string[0] = 'A';
    }else if(reminder == 10){
        string[0] = '10';
    }else{
        // return reminder
        string[0] = reminder + '0';
    }
    return string;
}

// given an player's hand (array of strings)
// return the corresponding value of the card
int getValue(unsigned char hand[10][3], int lenghtOfHand){
    int i;
    int totalVal = 0;
    int numberOfAs = 0; // keep track of As in our hand
    unsigned char value;

    for (i = 0; i < lenghtOfHand; i++){
        value = hand[i][0];
        if (value == 0)
            break;

        if( (value == 'J') || (value == 'K') || (value == 'Q') ){
            // J
            totalVal += 10;
        }else if (value == 'A'){
            // Ace
            totalVal += 1;
            numberOfAs += 1;
        }else{
            // return reminder
             totalVal += value - '0';
        }
    }

    int distance = 21 - totalVal;
    // if one of the As can become 11 without going over 21, then update totalVal
    if ( (distance >= 10) && (numberOfAs > 0) ){
        // make one A be valued 11 instead of 10
        totalVal = totalVal + 10;
    }

    return totalVal;
}

// given hand value, calculate the bet
unsigned char calculateBet(int handValue, unsigned char playerBet){

    unsigned char bet;
    if (handValue >= 2 & handValue <= 6 ){
        bet = '1';
    }else if (handValue >= 7 & handValue <= 11 ){
        bet = '2';
    }else if (handValue >= 12 & handValue <= 17 ){
        bet = '4';
    }else if (handValue >= 17 & handValue <= 21 ){
        bet = '8';
    }

    // if cpu's bet is less than opponent's bet, then equal opponent's
    if (playerBet > bet)
        bet = playerBet;

    return bet;
}




void swDelay(char numLoops)
{
    // This function is a software delay. It performs
    // useless loops to waste a bit of time
    //
    // Input: numLoops = number of delay loops to execute
    // Output: none
    //
    // smj, ECE2049, 25 Aug 2013

    volatile unsigned int i,j;  // volatile to prevent removal in optimization
                                // by compiler. Functionally this is useless code

    for (j=0; j<numLoops; j++)
    {
        i = 50000 ;                 // SW Delay
        while (i > 0)               // could also have used while (i)
           i--;
    }
}



