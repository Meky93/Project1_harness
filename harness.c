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
