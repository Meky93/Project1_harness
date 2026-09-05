# Vibe Coding Log: `harness.c`

This log records the prompts, iterations, and AI responses used while creating
the simple C command-line harness.

## Iteration 1

### Prompt

> Hi, I am currently tasked with writing a harness for an LLM in C. "I need to write a simple command-line program in C. I am a beginner, so please keep the
> code as simple as possible. Do not use external libraries, only standard ones like <stdio.h>
> and <string.h>. Here is the specification for the program:
> 1. It should run an infinite while loop that asks for user input using fgets.
> 2. If the user types 'exit', the loop should break and the program should end. 
> 3. If the user types a sentence containing the word 'hello', the program should print a
> hardcoded greeting.
> 4. If the user types anything else, it should echo their input back to them.
> 5. Please add clear, line-by-line comments explaining what the code is doing."

### AI response

The AI inspected the starter project, replaced the C++ starter program with a
standard-library C implementation, and updated the VS Code build task from
`clang++` to `clang` with C17.

The implementation was initially created as `main.c`, then became the project
program `harness.c`:

```c
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
```

### AI final response

> Implemented the C command-line harness in `Project1/main.c`.
>
> - Uses `fgets` inside an infinite loop.
> - Exits when the user enters `exit`.
> - Prints a hardcoded greeting when the input contains `hello`.
> - Echoes all other input.
> - Includes clear comments throughout.
> - Updated the VS Code build task to use `clang` with C17.
> - Removed the original C++ starter file.

## Iteration 2

### Prompt

> Write a very simple Bash script (for
> Linux) that automatically sends the word 'hello', followed by the word 'exit', into the
> program to test if it works.

### AI response and iteration

The AI created `test.sh` with this initial command:

```bash
#!/bin/bash

# Send "hello" and then "exit" to the program.
printf "hello\nexit\n" | ./main
```

The first test attempt revealed that the project program was named
`harness`, not `main`. The script was corrected to use `./harness`.

### AI final response

> Created `Project1/test.sh`:
>
> ```bash
> #!/bin/bash
>
> # Send "hello" and then "exit" to the program.
> printf "hello\nexit\n" | ./harness
> ```
>
> Run it from the `Project1` directory after compiling the program:
>
> ```bash
> chmod +x test.sh
> ./test.sh
> ```

The corrected test produced the greeting for `hello` and then exited when it
received `exit`.

## Result

The completed harness is `Project1/harness.c`. The associated Linux test script
is `Project1/test.sh`. The log documents the prompts, implementation decisions,
test iteration, correction, and AI responses involved in creating the harness.
