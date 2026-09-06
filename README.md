# Project 1: An LLM Mini-Harness in C via Vibe Coding

## Description

This project creates a simple C command-line program that reads input from a
human user. The program responds to the specific inputs `hello` and `exit`, and
echoes any other input back to the user.

The program uses `fgets` to read user input and terminates when the user enters
`exit`.

## Features

- Uses `fgets` to read input from the user.
- Prints a greeting when the input contains `hello`.
- Terminates when the user enters `exit`.
- Echoes any other input back to the user.
- Uses only standard C libraries.

## Development Tools

The code was created using the student version of GitHub Copilot and
Copilot CLI within Code - OSS, an open-source version of Visual Studio Code.

## Compilation

GCC was used to compile the program:

```bash
gcc harness.c -o harness
```

## Running the Program

Run the compiled program with:

```bash
./harness
```

Example interaction:

```text
> hello
Hello! It is nice to meet you.
> another message
another message
> exit
```

## Automated Test

The `test.sh` script automatically sends `hello` and `exit` to the program as
if they had been entered by the user.

Make the script executable and run it with:

```bash
chmod +x test.sh
./test.sh
```
