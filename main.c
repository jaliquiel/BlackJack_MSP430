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
unsigned char* getCardString(unsigned int card);
int getValue(unsigned char hand[10][3], int lenghtOfHand);
unsigned char calculateBet(int handValue, unsigned char playerBet);
int isOverflow(int value);
bool cpuDraw(int value);
void celebration(void);
void humiliation(void);
void swDelay(char numLoops);
void printHands(unsigned char playerHand[][], unsigned char cpuHand[][], unsigned char dispFour[], unsigned char dispSz, unsigned char dispFourSz);


// Declare globals here
enum GAME_STATE {welcome = 0, cut =1, shuffle=2, bet=3, draw=4, cpu=5, loser=6, winner=7, end=8, reset=9};
enum HAND_STATE {lost = 0, won =1, keepPlaying=2};


void main(void){
    // ... code

    unsigned char currKey=0, dispSz = 3, dispFourSz = 4;
    unsigned char dispThree[3] = {NULL, NULL, NULL};
    unsigned char dispFour[4] = {NULL, NULL, NULL, NULL};
    unsigned int deck[52] = {0};
    unsigned int topDeckIndex = 0; // represents the current index of the top of the deck

    unsigned char playerHand[10][3];
    unsigned char cpuHand[10][3];

    unsigned char playerBet;
    unsigned char cpuBet;

    // variables for lenght of players hand array
    int playerHandLen;
    int cpuHandLen;
    // current number of cards at each players hand
    int playerCards = 0;
    int cpuCards = 0;

    int endGame;

    int cpuHandValue;
    int playerHandValue;

    enum GAME_STATE state = welcome;


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

    while (1)    // Forever loop
    {

        switch(state){
        case welcome: // Start Screen

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

            Graphics_drawStringCentered(&g_sContext, "CUT THE DECK", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "enter number", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "from 0 - 15", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

            // Refresh the display so it shows the new data
            Graphics_flushBuffer(&g_sContext);

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

            // Set the random seed
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
            playerCards += 2;
            strcpy(cpuHand[0], getCardString(deck[topDeckIndex]));
            topDeckIndex++;
            strcpy(cpuHand[1], getCardString(deck[topDeckIndex]));
            topDeckIndex++;
            cpuCards += 2;

            // PRINT OUT PLAYERS HANDS
//            printHands(unsigned char playerHand[][], unsigned char cpuHand[][], unsigned char dispFour[], unsigned char dispSz, unsigned char dispFourSz);
            printHands(&playerHand, &cpuHand, &dispFour, dispSz, dispFourSz);
//            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
//            // print out player's cards
//            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
//            for (i = 0; i< playerHandLen;i++){
//                if(playerHand[i][0] == '\0'){
//                    break;
//                }
////                else if(playerHand[i][0] == '1'){
////                    Graphics_drawStringCentered(&g_sContext, "10", AUTO_STRING_LENGTH, 17 + i*25, 20, OPAQUE_TEXT);
////                    Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
////                    continue;
////                }
//                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
//            }
//            // print out cpu's cards
//            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
//            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
//            for (i = 0; i< cpuHandLen;i++){
//                if(cpuHand[i][0] == '\0'){
//                    break;
//                }
//                if (i == 1){
//                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 10 + i*25, 50, OPAQUE_TEXT);
//                    continue;
//                }
//                // TODO FIX PRINTING OF 10s ******************************************************************************************************10
//                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 10 + i*25, 50, OPAQUE_TEXT);
//            }
//            Graphics_flushBuffer(&g_sContext);

            // get ready to switch state to bet
            state = bet;
            swDelay(1);
            setLeds(0);
            break;

        case bet: // Adding cards state

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
                if(playerHand[i][0] == '\0'){
                    break;
                }
                else if(playerHand[i][0] == '1'){
                    dispFour[0] = '1';
                    dispFour[1] = '0';
                    dispFour[2] = playerHand[i][1]; // add '-'
                    dispFour[3] = playerHand[i][2]; // add suit
                    Graphics_drawStringCentered(&g_sContext, dispFour, dispFourSz, 10 + i*25, 20, OPAQUE_TEXT);
                    continue;
                }
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i][0] == '\0'){
                    break;
                }
                else if(cpuHand[i][0] == '1'){
                    dispFour[0] = '1';
                    dispFour[1] = '0';
                    dispFour[2] = cpuHand[i][1]; // add '-'
                    dispFour[3] = cpuHand[i][2]; // add suit
                    Graphics_drawStringCentered(&g_sContext, dispFour, dispFourSz, 10 + i*25, 20, OPAQUE_TEXT);
                    continue;
                }
                if (i == 1){
                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 10 + i*25, 50, OPAQUE_TEXT);
                    continue;
                }
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 10 + i*25, 50, OPAQUE_TEXT);
            }

            // store the bet of the player
            playerBet = currKey;
            dispThree[0] = ' ';
            dispThree[1] = playerBet;
            dispThree[2] = '\0';

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
            dispThree[2] = '\0';
            Graphics_drawStringCentered(&g_sContext, "CPU's bet is: ", AUTO_STRING_LENGTH, 50, 75, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, dispThree, AUTO_STRING_LENGTH, 90, 75, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);


            // if playerBet is not equal to cpu bet then player needs to put difference
            if (playerBet != cpuBet){
                int difference;
                while(currKey != '*'){
                    currKey = getKey();
                    difference = playerBet + currKey - ('0'*2);
                    if(difference == cpuBet - '0'){
                        setLeds(difference - 0x30);
                        break;
                    }

                }
            }
            if(currKey == '*'){
                // go to reset state
                state = reset;
            }else{
                playerBet = cpuBet;
                state = draw;
                Graphics_clearDisplay(&g_sContext); // Clear the display
            }
            swDelay(1);
            break;

        case draw: // Player's Turn
            endGame = isOverflow(getValue(&playerHand, playerHandLen));

            if(endGame == lost){
                state = loser;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;
            }else if(endGame == won){
                state = winner;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;
            }


            // PRINT OUT PLAYERS HANDS
            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
            // print out player's cards
            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
            for (i = 0; i< playerHandLen;i++){
                if(playerHand[i][0] == '\0'){
                    break;
                }
//                else if(playerHand[i][0] == '1'){
//                    Graphics_drawStringCentered(&g_sContext, "10", AUTO_STRING_LENGTH, 17 + i*25, 20, OPAQUE_TEXT);
//                    Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
//                    continue;
//                }
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i][0] == '\0'){
                    break;
                }
                if (i == 1){
                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 10 + i*25, 50, OPAQUE_TEXT);
                    continue;
                }
                // TODO FIX PRINTING OF 10s ******************************************************************************************************10
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 10 + i*25, 50, OPAQUE_TEXT);
            }
            Graphics_drawStringCentered(&g_sContext, "Press 1 to draw", AUTO_STRING_LENGTH, 48, 70, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Press # to hold", AUTO_STRING_LENGTH, 48, 80, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);

            // ask player to fold or get a card
            while(1){
                currKey = getKey();
                if((currKey == '#') || (currKey == '1') )
                    break;
            }


            if (currKey == '1'){
                // add card to player
                strcpy(playerHand[playerCards], getCardString(deck[topDeckIndex]));
                topDeckIndex++;
                playerCards += 1;

                state = draw;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display

                // get out of switch statement
                break;
            }else if(currKey == '#'){
                state = cpu;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;
            }

            break;

        case cpu: // Win or lost state
            // display celebration or loss statement


            cpuHandValue = getValue(&cpuHand, cpuHandLen);
            endGame = isOverflow(cpuHandValue);

            // CPU lost, so send player to win screen
            if(endGame == lost){
                state = winner;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;
            }else if(endGame == won){
                // CPU won so send player to lost screen
                state = loser;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;
            }

            // PRINT OUT PLAYERS HANDS
            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
            // print out player's cards
            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
            for (i = 0; i< playerHandLen;i++){
                if(playerHand[i][0] == '\0'){
                    break;
                }
//                else if(playerHand[i][0] == '1'){
//                    Graphics_drawStringCentered(&g_sContext, "10", AUTO_STRING_LENGTH, 17 + i*25, 20, OPAQUE_TEXT);
//                    Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
//                    continue;
//                }
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i][0] == '\0'){
                    break;
                }
                if (i == 1){
                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 10 + i*25, 50, OPAQUE_TEXT);
                    continue;
                }
                // TODO FIX PRINTING OF 10s ******************************************************************************************************10
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 10 + i*25, 50, OPAQUE_TEXT);
            }
            Graphics_flushBuffer(&g_sContext);


//            // Do CPU Action
//            // restart state
            if(cpuDraw(cpuHandValue)){
                // draw and reenter state
                // add card to player
                strcpy(cpuHand[cpuCards], getCardString(deck[topDeckIndex]));
                topDeckIndex++;
                cpuCards += 1;

                state = cpu;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                // get out of switch statement
                break;

            }else{
                // go to final game decision state
                state = end;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;

            }

            break;

        case loser: // Lost state
            // display celebration or loss statement
            // ...


            // PRINT OUT PLAYERS HANDS
            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
            // print out player's cards
            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
            for (i = 0; i< playerHandLen;i++){
                if(playerHand[i][0] == '\0'){
                    break;
                }
//                else if(playerHand[i][0] == '1'){
//                    Graphics_drawStringCentered(&g_sContext, "10", AUTO_STRING_LENGTH, 17 + i*25, 20, OPAQUE_TEXT);
//                    Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
//                    continue;
//                }
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i][0] == '\0'){
                    break;
                }
//                if (i == 1){
//                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 10 + i*25, 50, OPAQUE_TEXT);
//                    continue;
//                }
                // TODO FIX PRINTING OF 10s ******************************************************************************************************10
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 10 + i*25, 50, OPAQUE_TEXT);
            }

            dispThree[0] = ' ';
            dispThree[1] = cpuBet;
            dispThree[2] = '\0';
            for(i = 0; i < 4; i ++){
                Graphics_drawStringCentered(&g_sContext, "YOU LOST", AUTO_STRING_LENGTH, 28, 60 + i*10, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, dispThree , AUTO_STRING_LENGTH, 54, 60 + i*10, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "CHIPS" , AUTO_STRING_LENGTH, 78, 60 + i*10, TRANSPARENT_TEXT);
            }
            Graphics_flushBuffer(&g_sContext);

            humiliation();

            while(currKey != '*'){
                currKey = getKey();
            }

            if(currKey == '*'){
                // Go to reset state
                state = reset;
            }

            break;

        case winner: // Win state


            // PRINT OUT PLAYERS HANDS
            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
            // print out player's cards
            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
            for (i = 0; i< playerHandLen;i++){
                if(playerHand[i][0] == '\0'){
                    break;
                }
//                else if(playerHand[i][0] == '1'){
//                    Graphics_drawStringCentered(&g_sContext, "10", AUTO_STRING_LENGTH, 17 + i*25, 20, OPAQUE_TEXT);
//                    Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
//                    continue;
//                }
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i][0] == '\0'){
                    break;
                }
//                if (i == 1){
//                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 10 + i*25, 50, OPAQUE_TEXT);
//                    continue;
//                }
                // TODO FIX PRINTING OF 10s ******************************************************************************************************10
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 10 + i*25, 50, OPAQUE_TEXT);
            }


            // display celebration
            dispThree[0] = ' ';
            dispThree[1] = cpuBet;
            dispThree[2] = '\0';
            for(i = 0; i < 4; i ++){
                Graphics_drawStringCentered(&g_sContext, "YOU WON", AUTO_STRING_LENGTH, 28, 60 + i*10, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, dispThree , AUTO_STRING_LENGTH, 54, 60 + i*10, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "CHIPS" , AUTO_STRING_LENGTH, 78, 60 + i*10, TRANSPARENT_TEXT);
            }
            Graphics_flushBuffer(&g_sContext);

            //proper celebration w/ buzzer and leds
            celebration();

            while(currKey != '*'){
                currKey = getKey();
            }

            if(currKey == '*'){
                // Go to reset state
                state = reset;
            }
            break;

        case end: // cpu and player holded, decide winner

            // PRINT OUT PLAYERS HANDS
            Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
            // print out player's cards
            playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
            for (i = 0; i< playerHandLen;i++){
                if(playerHand[i][0] == '\0'){
                    break;
                }
//                else if(playerHand[i][0] == '1'){
//                    Graphics_drawStringCentered(&g_sContext, "10", AUTO_STRING_LENGTH, 17 + i*25, 20, OPAQUE_TEXT);
//                    Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
//                    continue;
//                }
                Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
            }
            // print out cpu's cards
            cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
            Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
            for (i = 0; i< cpuHandLen;i++){
                if(cpuHand[i][0] == '\0'){
                    break;
                }
//                if (i == 1){
//                    Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 10 + i*25, 50, OPAQUE_TEXT);
//                    continue;
//                }
                // TODO FIX PRINTING OF 10s ******************************************************************************************************10
                Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 10 + i*25, 50, OPAQUE_TEXT);
            }
            Graphics_flushBuffer(&g_sContext);


            // calculate hand values for player and cpu
            cpuHandValue = getValue(&cpuHand, cpuHandLen);
            playerHandValue = getValue(&playerHand, playerHandLen);
            // closest hand to 21 wins
            // In this case, none of the hands are above 21 therefore, the biggest value is the closest to 21
            if (playerHandValue > cpuHandValue){
                // go to win state
                state = winner;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;
            }else if (playerHandValue < cpuHandValue){
                // go to lose state
                state = loser;
                currKey = NULL;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;
            }else{
                // draw
                Graphics_drawStringCentered(&g_sContext, "DRAW", AUTO_STRING_LENGTH, 48, 60, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "DRAW", AUTO_STRING_LENGTH, 48, 70, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "DRAW", AUTO_STRING_LENGTH, 48, 80, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "DRAW", AUTO_STRING_LENGTH, 48, 90, TRANSPARENT_TEXT);
            }

            while(currKey != '*'){
                currKey = getKey();
            }

            if(currKey == '*'){
                // Go to reset state
                state = reset;
            }
            break;

        case reset: // reset state
            // RESTART ALL VARIABLES FOR NEW GAME
            Graphics_clearDisplay(&g_sContext); // Clear the display
            state = welcome;
            currKey = NULL;
            dispThree[0] = NULL;
            dispThree[1] = NULL;
            dispThree[2] = NULL;
            dispFour[0] = NULL;
            dispFour[1] = NULL;
            dispFour[2] = NULL;
            dispFour[3] = NULL;
            for(i = 0; i < 52; i ++ ){
                deck[i] = 0;
            }
            topDeckIndex = 0; // represents the current index of the top of the deck
            playerCards = 0;
            cpuCards = 0;
            int j;
            for(i = 0; i < 10; i ++ ){
                for(j = 0; j < 10; j++){
                    playerHand[i][j] = 0;
                    cpuHand[i][j] = 0;
                }
            }
            state = welcome;
            break;
        } // end switch statement
    }  // end while


}

// Given an int from 1 to 52, return the corresponding card string representation
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
        string[0] = '1';
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

        if( (value == 'J') || (value == 'K') || (value == 'Q') || (value == '1') ){
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


// given a value check if the hand's value is bigger than 21
// return lost if hand overflows
// won if hand == 21
// keepPlaying if hand is less than 21
int isOverflow(int value){
    if(value > 21){
        // end game player lost
        return lost;
    }else if (value == 21){
        // player won
        return won;
    }else if (value < 21){
        // player could keep going
        return keepPlaying;
    }
    // this wont happen
    return lost;
}

// takes the value of the cpu's hand, measures the distance and then decides to fold or draw
// return true if cpu wants to draw card
// false if cpu decides to hold
bool cpuDraw(int value){
    int decision;
    if (value > 17)
        return false;
    else if (value <= 11)
        return true;
    else{
        decision = rand() % 2;
        if (decision > 1){
            return true;
        }else{
            return false;
        }
    }
}

// Play celebration sound with buzzer and turn on LEDs for a set amount of time
void celebration(void){
    unsigned int m = 20000;
    while(1){
        m--;
        if (m > 15000){
            BuzzerOn();
            setLeds(0x03);
        }
        else if ((m > 10000) && (m <= 15000)){
            Buzzer3On();
            setLeds(0x0C);
        }
        else if ((m > 5000) && (m <= 15000)){
            Buzzer1On();
            setLeds(0x06);
        }
        else if ((m > 0) && (m <= 5000)){
            Buzzer2On();
            setLeds(0x0F);
        }
        else if (m == 0){
            BuzzerOff();
            setLeds(0);
            return 0;
        }
    }
}

// Play humiliation sound with buzzer and turn on LEDs for a set amount of time
void humiliation(void){
    unsigned int m = 20000;
    while(1){
        m--;
        if (m > 15000){
            Buzzer3On();
            setLeds(0x01);
        }
        else if ((m > 10000) && (m <= 15000)){
            Buzzer2On();
            setLeds(0x03);
        }
        else if ((m > 5000) && (m <= 15000)){
            Buzzer1On();
            setLeds(0x07);
        }
        else if ((m > 0) && (m <= 5000)){
            BuzzerOn();
            setLeds(0x08);
        }
        else if (m == 0){
            BuzzerOff();
            setLeds(0);
            return 0;
        }
    }
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


// prints to the board, both the player and opponent hands
void printHands(unsigned char playerHand[10][3], unsigned char cpuHand[10][3], unsigned char dispFour[4], unsigned char dispSz, unsigned char dispFourSz){

    // PRINT OUT PLAYERS HANDS
    Graphics_drawStringCentered(&g_sContext, "PLAYER 1:", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
    // print out player's cards

//    int playerHandLen = sizeof(playerHand)/sizeof(playerHand[0]);
    int playerHandLen = 10;
    int i = 0;
    for (i = 0; i< playerHandLen;i++){
        if(playerHand[i][0] == '\0'){
            break;
        }
        else if(playerHand[i][0] == '1'){
            dispFour[0] = '1';
            dispFour[1] = '0';
            dispFour[2] = playerHand[i][1]; // add '-'
            dispFour[3] = playerHand[i][2]; // add suit
            Graphics_drawStringCentered(&g_sContext, dispFour, dispFourSz, 10 + i*25, 20, OPAQUE_TEXT);
            continue;
        }
        Graphics_drawStringCentered(&g_sContext, playerHand[i], dispSz, 10 + i*25, 20, OPAQUE_TEXT);
    }
    // print out cpu's cards
//    int cpuHandLen = sizeof(cpuHand)/sizeof(cpuHand[0]);
    int cpuHandLen = 10;
    Graphics_drawStringCentered(&g_sContext, "CPU", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
    for (i = 0; i< cpuHandLen;i++){
        if(cpuHand[i][0] == '\0'){
            break;
        }
        else if(cpuHand[i][0] == '1'){
            dispFour[0] = '1';
            dispFour[1] = '0';
            dispFour[2] = cpuHand[i][1]; // add '-'
            dispFour[3] = cpuHand[i][2]; // add suit
            Graphics_drawStringCentered(&g_sContext, dispFour, dispFourSz, 10 + i*25, 20, OPAQUE_TEXT);
            continue;
        }
        if (i == 1){
            Graphics_drawStringCentered(&g_sContext, "xxx", dispSz, 10 + i*25, 50, OPAQUE_TEXT);
            continue;
        }
        Graphics_drawStringCentered(&g_sContext, cpuHand[i], dispSz, 10 + i*25, 50, OPAQUE_TEXT);
    }

}



