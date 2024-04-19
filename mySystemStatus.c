#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "stats.function.h"

/**
 * This function is called when the program receives a SIGINT signal (e.g., when the user presses Ctrl+C).
 * It will ask the user whether it wants to quit or not the program.
 *
 * @param sig The signal number (should be SIGINT).
 *
 * @return void
 */
void sigint_handler(int sig)
{
    char response;
    printf("\nDo you want to quit? (y/n): ");
    scanf(" %c", &response);
    if (response == 'y' || response == 'Y')
    {
        exit(EXIT_SUCCESS);
    }
}

/**
 * This function is called when the program receives a SIGTSTP signal (e.g., when the user presses Ctrl+Z) and
 * ignore it as the program should not be run in the background while running interactively
 *
 * @param sig The signal number (should be SIGTSTP).
 *
 * @return void
 */
void sigtstp_handler(int sig)
{
}

int main(int argc, char **argv)
{
    int flags[FLAGS_LENGTH] = {0};
    int samples = 10;
    int tdelay = 1;

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    clearScreen();
    moveCursorTo(0, 0);
    if (argc != 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (strncmp(argv[i], "--samples=", 10) == 0)
            {
                samples = atoi(argv[i] + 10);
            }
            if (strncmp(argv[i], "--tdelay=", 9) == 0)
            {
                tdelay = atoi(argv[i] + 9);
            }
            if (strcmp(argv[i], "--system") == 0)
            {
                flags[0] = 1;
            }
            if (strcmp(argv[i], "--user") == 0)
            {
                flags[1] = 1;
            }
            if (strcmp(argv[i], "--graphics") == 0)
            {
                flags[2] = 1;
            }
            if (strcmp(argv[i], "--sequential") == 0)
            {
                flags[3] = 1;
            }
        }
    }
    else
    {
        flags[0] = 1;
        flags[1] = 1;
    }
    if (flags[3] == 1 && flags[0] == 0 && flags[1] == 0)
    {
        flags[0] = 1;
        flags[1] = 1;
    }
    print_system_status(flags, samples, tdelay);
    return 0;
}