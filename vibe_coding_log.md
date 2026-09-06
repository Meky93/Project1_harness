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
```

The new code generated is seen below:

```c

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 256
#define MAX_RESPONSE 512
#define MAX_HISTORY 5

typedef struct {
    char user[MAX_INPUT];
    char assistant[MAX_RESPONSE];
} Turn;

typedef struct {
    Turn turns[MAX_HISTORY];
    size_t count;
    size_t next;
} Conversation;

/*
 * The calculator is a tool rather than part of the mock model's response
 * generation. It parses a small arithmetic language without invoking a shell
 * or an external process.
 */
typedef struct {
    const char *input;
    size_t position;
    int error;
} Calculator;

static void skip_spaces(Calculator *calculator)
{
    while (isspace((unsigned char)calculator->input[calculator->position])) {
        calculator->position++;
    }
}

static double parse_expression(Calculator *calculator);

static double parse_number(Calculator *calculator)
{
    char *end;
    double value;

    skip_spaces(calculator);
    errno = 0;
    value = strtod(calculator->input + calculator->position, &end);
    if (end == calculator->input + calculator->position || errno == ERANGE) {
        calculator->error = 1;
        return 0.0;
    }

    calculator->position = (size_t)(end - calculator->input);
    return value;
}

static double parse_factor(Calculator *calculator)
{
    double value;

    skip_spaces(calculator);
    if (calculator->input[calculator->position] == '(') {
        calculator->position++;
        value = parse_expression(calculator);
        skip_spaces(calculator);
        if (calculator->input[calculator->position] != ')') {
            calculator->error = 1;
        } else {
            calculator->position++;
        }
        return value;
    }

    if (calculator->input[calculator->position] == '-') {
        calculator->position++;
        return -parse_factor(calculator);
    }

    return parse_number(calculator);
}

static double parse_term(Calculator *calculator)
{
    double value = parse_factor(calculator);

    while (!calculator->error) {
        char operator;
        double right;

        skip_spaces(calculator);
        operator = calculator->input[calculator->position];
        if (operator != '*' && operator != '/') {
            break;
        }

        calculator->position++;
        right = parse_factor(calculator);
        if (operator == '/' && right == 0.0) {
            calculator->error = 1;
            break;
        }
        value = operator == '*' ? value * right : value / right;
    }

    return value;
}

static double parse_expression(Calculator *calculator)
{
    double value = parse_term(calculator);

    while (!calculator->error) {
        char operator;
        double right;

        skip_spaces(calculator);
        operator = calculator->input[calculator->position];
        if (operator != '+' && operator != '-') {
            break;
        }

        calculator->position++;
        right = parse_term(calculator);
        value = operator == '+' ? value + right : value - right;
    }

    return value;
}

static int calculate_tool(const char *expression, char *response, size_t response_size)
{
    Calculator calculator = {expression, 0, 0};
    double result = parse_expression(&calculator);

    skip_spaces(&calculator);
    if (calculator.error || expression[calculator.position] != '\0') {
        snprintf(response, response_size,
                 "Calculator error: use numbers, parentheses, and + - * /.");
        return 0;
    }

    snprintf(response, response_size, "Tool result: %.10g", result);
    return 1;
}

static void conversation_init(Conversation *conversation)
{
    memset(conversation, 0, sizeof(*conversation));
}

static void conversation_add(Conversation *conversation,
                             const char *user,
                             const char *assistant)
{
    Turn *turn = &conversation->turns[conversation->next];

    snprintf(turn->user, sizeof(turn->user), "%s", user);
    snprintf(turn->assistant, sizeof(turn->assistant), "%s", assistant);
    conversation->next = (conversation->next + 1) % MAX_HISTORY;
    if (conversation->count < MAX_HISTORY) {
        conversation->count++;
    }
}

static int starts_with(const char *text, const char *prefix)
{
    return strncmp(text, prefix, strlen(prefix)) == 0;
}

/*
 * This mock model is deterministic and intentionally local: it uses the
 * current prompt and reports the number of retained turns instead of calling
 * an LLM API. A prompt beginning with "calc " invokes the calculator tool.
 */
static void mock_model(const Conversation *conversation,
                       const char *prompt,
                       char *response,
                       size_t response_size)
{
    if (starts_with(prompt, "calc ")) {
        calculate_tool(prompt + 5, response, response_size);
        return;
    }

    if (strcmp(prompt, "hello") == 0) {
        snprintf(response, response_size,
                 "Hello! I am a mock model. I remember %zu turn%s.",
                 conversation->count, conversation->count == 1 ? "" : "s");
        return;
    }

    snprintf(response, response_size,
             "Mock response: \"%s\" (history: %zu/%d turns).",
             prompt, conversation->count, MAX_HISTORY);
}

int main(void)
{
    Conversation conversation;
    char input[MAX_INPUT];
    char response[MAX_RESPONSE];

    conversation_init(&conversation);
    printf("Mock harness ready. Try 'hello', 'calc 2 + 3 * 4', or 'exit'.\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        if (strchr(input, '\n') == NULL && !feof(stdin)) {
            int character;
            while ((character = getchar()) != '\n' && character != EOF) {
                /* Discard the remainder of an overlong input line. */
            }
            printf("Input too long; please keep requests under %d characters.\n",
                   MAX_INPUT);
            continue;
        }

        input[strcspn(input, "\r\n")] = '\0';
        if (strcmp(input, "exit") == 0) {
            break;
        }
        if (input[0] == '\0') {
            printf("Please enter a request.\n");
            continue;
        }

        mock_model(&conversation, input, response, sizeof(response));
        printf("%s\n", response);
        conversation_add(&conversation, input, response);
    }

    return 0;
}
```

