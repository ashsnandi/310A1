#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Shell version 1.5 created Dec 2025\n");

    // us code
    int interactive = isatty(fileno(stdin));

    char prompt = '$';              // Shell prompt
    char userInput[MAX_USER_INPUT]; // user's input stored here
    int errorCode = 0;              // zero means no error, default

    // init user input
    for (int i = 0; i < MAX_USER_INPUT; i++)
    {
        userInput[i] = '\0';
    }

    // init shell memory
    mem_init();
    while (1)
    {
        if (interactive)
        {
            printf("%c ", prompt);
        }
        // here you should check the unistd library
        // so that you can find a way to not display $ in the batch mode
        // -- done, now need to figure out a way to make it quit. this means that the hwile 1 shouldn't happen anyumor
        if (fgets(userInput, MAX_USER_INPUT - 1, stdin) == NULL)
        {
            // eof reached in batch mode
            break;
        }
        errorCode = parseInput(userInput);
        if (errorCode == -1)
            exit(99); // ignore all other errors
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

int wordEnding(char c)
{
    // added ';' to stop parsing a command when in multi command single line mode
    return c == '\0' || c == '\n' || c == ' ' || c == ';';
}

int parseInput(char inp[])
{
    char tmp[200], *words[100];
    int ix = 0, w = 0;
    int wordlen;
    int errorCode;

    for (ix = 0; inp[ix] == ' ' && ix < 1000; ix++)
        ; // skip white spaces

    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000)
    {
        // current character is either semicolon or non semicolon
        // if current character is semicolon, send array of words to interpreter- this is one command to be executed
        if (inp[ix] == ';')
        {
            if (w > 0)
            {
                errorCode = interpreter(words, w);
                // free word space and reset w index-> prep for next command to be parsed
                for (int i = 0; i < w; i++)
                {
                    free(words[i]);
                }
                w = 0;
            }
            ix++;
            // skip spaces
            while (inp[ix] == ' ' && ix < 1000)
            {
                ix++;
            }
        }

        // extract a word
        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++)
        {
            tmp[wordlen] = inp[ix];
        }

        tmp[wordlen] = '\0';

        if (wordlen > 0)
        {
            words[w] = strdup(tmp);
            w++;
        }

        // skip spaces
        while (inp[ix] == ' ' && ix < 1000)
        {
            ix++;
        }

        // reach end of input -> leave while loop
        if (inp[ix] == '\0')
            break;
    }

    // free up word array space after use
    if (w > 0)
    {
        errorCode = interpreter(words, w);
        for (int i = 0; i < w; i++)
        {
            free(words[i]);
        }
    }
    return errorCode;
}
