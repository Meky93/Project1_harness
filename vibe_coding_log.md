# Vibe Coding Log: `harness.c`

This log records the prompts, iterations, and AI responses used while creating
the simple C command-line harness.

## Iteration 1

### Prompt

"Hi, I am currently tasked with writing a harness for an LLM in C. "I need to write a simple command-line program in C. I am a beginner, so please keep the
code as simple as possible. Do not use external libraries, only standard ones like <stdio.h>
and <string.h>. Here is the specification for the program:
1. It should run an infinite while loop that asks for user input using fgets.
2. If the user types 'exit', the loop should break and the program should end. 
3. If the user types a sentence containing the word 'hello', the program should print a
hardcoded greeting.
4. If the user types anything else, it should echo their input back to them.
5. Please add clear, line-by-line comments explaining what the code is doing.""

### AI response

The AI inspected the starter project, replaced the C++ starter program with a
standard-library C implementation, and updated the VS Code build task from
`clang++` to `gcc` with C17.

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

Implemented the C command-line harness in `Project1/harness.c`.

- Uses `fgets` inside an infinite loop.
- Exits when the user enters `exit`.
- Prints a hardcoded greeting when the input contains `hello`.
- Echoes all other input.
- Includes clear comments throughout.
- Updated the VS Code build task to use `gcc` with C17.
- Removed the original C++ starter file.


Created `Project1/test.sh`:

```bash
 #!/bin/bash

 # Send "hello" and then "exit" to the program.
 printf "hello\nexit\n" | ./harness
 ```

 Run it from the `Project1` directory after compiling the program:

 ```bash
 chmod +x test.sh
 ./test.sh
 ```

The corrected test produced the greeting for `hello` and then exited when it
received `exit`.

## Iteration 2

Obviously the prewvious iteration does not fulfill many of the project requirements for the harness. Github-copilat was asked once again to implement add these requirements with the following prompt and response:

### User Request

"The initial C harness only implemented a basic terminal loop that recognized
`hello` and echoed other input. I asked the AI assistant to implement the
remaining requirements:

1. Separate the mock model from the terminal loop.
2. Add safe conversation context management that retains the last five turns.
3. Add a tool-execution capability for mathematical calculations.
4. Preserve safe memory behavior and handle invalid or oversized input.
5. Keep the implementation suitable for validation by the separate `test.sh`
   script."

### Architectural Rules and SDD Decisions

- Use a deterministic local mock model instead of calling an external LLM API.
- Keep the terminal loop responsible for input/output and delegate response
  generation to `mock_model()`.
- Represent each conversation turn as a user message and assistant response.
- Use a fixed-size ring buffer for conversation history with a maximum of five
  turns.
- Avoid dynamic memory allocation so the harness has a simple, predictable
  memory model and can be checked for leaks.
- Keep calculator execution separate from the mock model's general response
  logic.
- Never invoke a shell or external process to perform calculations.
- Parse calculator expressions directly in C and support numbers, parentheses,
  addition, subtraction, multiplication, division, and unary negative values.
- Handle invalid expressions and division by zero with an explicit error
  response.
- Use bounded input functions and fixed-size response buffers.
- Handle EOF, empty input, and input lines longer than the buffer safely.

### AI-Generated Changes

The AI assistant created `harness.c` with the following components:

- `Conversation` and `Turn` structures for state management.
- A five-entry circular history buffer.
- `conversation_init()` and `conversation_add()` functions.
- A separate deterministic `mock_model()` function.
- A calculator tool implemented using expression parsing functions:
  `parse_expression()`, `parse_term()`, `parse_factor()`, and
  `calculate_tool()`.
- The `calc ` command prefix to invoke the calculator tool.
- Safe handling for overlong input lines and empty input.
- A clear `exit` command and EOF handling.

### Example Prompts Used

The implementation was tested with prompts including:

```text
hello
calc 2 + 3 * 4
calc (2 + 3) * 4
exit


