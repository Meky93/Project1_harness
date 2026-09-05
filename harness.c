#include <stdio.h>
#include <string.h>

int main(void)
{
    // Create a character array to store each line entered by the user.
    char input[256];

    // Keep asking for input until the user enters "exit" or input ends.
    while (1)
    {
        // Display a prompt so the user knows the program is waiting.
        printf("> ");

        // Read one line of input, including spaces, from the user.
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // Stop cleanly if the user sends an end-of-file signal.
            break;
        }

        // Remove the newline that fgets normally stores at the end.
        input[strcspn(input, "\n")] = '\0';

        // Stop the program when the user enters exactly "exit".
        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        // Check whether the input contains the word "hello".
        if (strstr(input, "hello") != NULL)
        {
            // Print the hardcoded greeting when "hello" is found.
            printf("Hello! It is nice to meet you.\n");
        }
        else
        {
            // Echo the input when it does not contain "hello".
            printf("%s\n", input);
        }
    }

    // Return zero to indicate that the program ended successfully.
    return 0;
}
