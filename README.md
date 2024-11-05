# read–eval–print loop (REPL) sqlite based database

## Description

A simple read–eval–print loop ("REPL") c based db.

It contains basic sqlite based commands (detailed below), functioning in a command line interface when ran as "db\".


It also supports disk persistence, meaning instead of being temporarily stored in the memory (stack), it will be stored into the disk instead. 
This means we can access our data when we re-boot, similar to a regular db.


```
[test]
```

What is REPL?


What is this based on?

sqlite
For a reference, check out [here](https://www.sqlite.org/cli.html).


## Commands

| Command name    | Description |  Usage |
| -------- 		  | -------     | -------     |
| .exit |  Closes command prompt. Must be used to save onto disk  | .exit   |
| insert  |  Inserts entry into db by appending to row |  insert [row # as integer] [username] [email@emailcom] |
| select  |    | -------     |
| .tables |   | -------     |


# How this works:


Front End:
```
print_prompt() -> read_input() -> 
```
This goes on forever until .exit is typed.


Back End:
```
print_prompt() -> read_input() -> 
```

[ Main ] 
Checks for 




# Todo:

- Move side functions to header file for readability.
- Keep main in main.c

- Switch printf to cout<<

## Side Notes:
This is built for windows.
Windows does not support POSIX. Some workarounds were used to replicate getline(), refer to code.

Several design considerations:
Too long strings.

Tips to debug:
Figuring out where memory problems? Just simply insert print statement at places where think code is breaking. 
Sometimes, no error messages come out because of pointer misuse, so don't know where code goes wrong.
Insert print statements in certain places in main to figure out where it breaks.

Testing?
Consider rspec?