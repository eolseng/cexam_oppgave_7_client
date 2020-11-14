#include <stdio.h>
#include <string.h>

int GetIntInRange(int iLower, int iUpper) {
    int iInt, iResult;
    // Loop until valid input
    while (1) {
        // Get input from user
        iResult = scanf("%d", &iInt);
        // Flushes stdin in case of invalid (non-int) input
        fflush(stdin);
        // Validate input - breaks if input is integer and in range 1..5
        if (iResult == 1 && (iInt >= iLower && iInt <= iUpper)) break;
        // Prompt user to try again
        printf("Invalid input. Must be between %d and %d\r\n", iLower, iUpper);
    }
    // Return validated input
    return iInt;
}

int GetStringWithMaxLength(int iMaxLength, char *pszBuffer) {
    // Get user input
    fgets(pszBuffer, iMaxLength, stdin);
    // Strip trailing new lines
    if ((strlen(pszBuffer) > 0) && (pszBuffer[strlen(pszBuffer) - 1] == '\n')) {
        pszBuffer[strlen(pszBuffer) - 1] = '\0';
    }
    // Flush stdin buffer
    fflush(stdin);
    return 0;
}