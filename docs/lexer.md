This document details the implementation of the lexical analyser (lexer).

## FSM 
The complete flowchart of the Finite State Machine (FSM) used to build the lexer is represented below. 

```mermaid
    %%{init: {"flowchart": {"htmlLabels": true}} }%%
    flowchart TD

        START["START\n*inbetween words*"]
        WORD["WORD"]
        OPERATOR["OPERATOR"]
        ESCAPE["ESCAPE"]
        IN_DOLLAR["IN_DOLLAR"]
        IN_BRACES["IN_BRACES"]
        IN_SG_QUOTE["IN_SG_QUOTE"]
        IN_DB_QUOTE["IN_DB_QUOTE"]
        ESCAPE_IN_DB_QUOTE["ESCAPE_IN_DB_QUOTE"]
        IN_DOLLAR_IN_DB_QUOTE["IN_DOLLAR_IN_DB_QUOTE"]
        IN_BRACES_IN_DB_QUOTE["IN_BRACES_IN_DB_QUOTE"]
        WAIT_CLOSE((" "))
        CHECK_AFTER_BRACE((" "))
        WAIT_CLOSE_DB((" "))

        START -->|"' '"| START
        START -->|"*"| WORD
        START -->|"Operator"| OPERATOR
        START -->|"$"| IN_DOLLAR

        WORD -->|"Operator or ' '"| START
        WORD -->|"*"| WORD
        WORD -->|"\"| ESCAPE
        WORD -->|"$"| IN_DOLLAR
        WORD -->|"'"| IN_SG_QUOTE
        WORD -->|"#quot;"| IN_DB_QUOTE
        WORD -->|"#quot; $"| IN_DOLLAR_IN_DB_QUOTE

        OPERATOR -->|"Operator"| OPERATOR
        OPERATOR -->|"Not Operator"| START

        ESCAPE -->|"*"| WORD
        ESCAPE_IN_DB_QUOTE -->|"*"| IN_DB_QUOTE

        IN_DOLLAR -->|"{"| IN_BRACES
        IN_DOLLAR -->|"?, $ or 0"| WORD
        IN_DOLLAR -->|"Not a variable char"| WORD
        IN_DOLLAR -->|"*"| IN_DOLLAR

        IN_DOLLAR_IN_DB_QUOTE -->|"{"| IN_BRACES_IN_DB_QUOTE
        IN_DOLLAR_IN_DB_QUOTE -->|"#quot;"| WORD
        IN_DOLLAR_IN_DB_QUOTE -->|"?, $ or 0"| IN_DB_QUOTE
        IN_DOLLAR_IN_DB_QUOTE -->|"Not a variable char"| IN_DB_QUOTE
        IN_DOLLAR_IN_DB_QUOTE -->|"*"| IN_DOLLAR_IN_DB_QUOTE

        IN_BRACES -->|"*"| IN_BRACES
        IN_BRACES -->|"?, $ or 0"| WAIT_CLOSE
        WAIT_CLOSE -->|"}"| WORD
        IN_BRACES -->|"}"| CHECK_AFTER_BRACE
        CHECK_AFTER_BRACE -->|"Not operator or space"| WORD
        CHECK_AFTER_BRACE -->|"Operator or space"| START

        IN_BRACES_IN_DB_QUOTE -->|"*"| IN_BRACES_IN_DB_QUOTE
        IN_BRACES_IN_DB_QUOTE -->|"?, $ or 0"| WAIT_CLOSE_DB
        WAIT_CLOSE_DB -->|"}"| IN_DB_QUOTE
        IN_BRACES_IN_DB_QUOTE -->|"}"| IN_DB_QUOTE

        IN_SG_QUOTE -->|"'"| WORD
        IN_SG_QUOTE -->|"*"| IN_SG_QUOTE

        IN_DB_QUOTE -->|"#quot;"| WORD
        IN_DB_QUOTE -->|"\"| ESCAPE_IN_DB_QUOTE
        IN_DB_QUOTE -->|"$"| IN_DOLLAR_IN_DB_QUOTE
        IN_DB_QUOTE -->|"*"| IN_DB_QUOTE
```

## Transition Matrix 

| Initial State | Read Character | Next State           | Associated action |
| ------------- | -------------- | -------------------- | ----------------- |
| START         | ' '            | START                |                   |
| START         | *              | WORD                 | Buffers character (beginning of a new word) |
| START         | Operator       | OPERATOR             |                   |
| START         | $              | IN_DOLLAR            |                   |
| WORD          | Operator or ' '| START                | Starts new token |
| WORD          | \              | ESCAPE               |                   |
| WORD          | $              | IN_DOLLAR            | Creates new VAR segment | 
| WORD          | '              | IN_SG_QUOTE          |                   | 
| WORD          | "              | IN_DB_QUOTE          |                   | 
| WORD          | "$             | IN_DOLLAR_IN_DB_QUOTE| Creates new VAR segment | 
| WORD          | *              | WORD                 |                   |
| OPERATOR      | Operator       | OPERATOR             |                   |
| OPERATOR      | Not an operator| START                | Starts new token  |
| ESCAPE        | *              | WORD                 | Specific symbol matching |
| ESCAPE_IN_DB_QUOTE | *         | IN_DB_QUOTE          | Specific symbol matching |
| IN_DOLLAR     | {              | IN_BRACES            |                   |
| IN_DOLLAR     | ?, $ or 0      | WORD                 | Ends VAR segment and creates new literal segment |
| IN_DOLLAR     | Not a variable char | WORD            | Ends VAR segment and creates new literal segment | 
| IN_DOLLAR     | *              | IN_DOLLAR            |                   |
| IN_DOLLAR_IN_DB_QUOTE | {                   | IN_BRACES_IN_DB_QUOTE | |
| IN_DOLLAR_IN_DB_QUOTE | "                   | WORD                  | Ends VAR segment and creates new literal segment | 
| IN_DOLLAR_IN_DB_QUOTE | ?, $ or 0           | IN_DB_QUOTE           | Ends VAR segment and creates new literal segment | 
| IN_DOLLAR_IN_DB_QUOTE | Not a variable char | IN_DB_QUOTE           | Ends VAR segment and creates new literal segment | 
| IN_DOLLAR_IN_DB_QUOTE | *                   | IN_DOLLAR_IN_DB_QUOTE | |
| IN_BRACES     | ?, $ or 0 followed by } | START if followed by operator or ' ', WORD otherwise | |
| IN_BRACES     | }                       | START if followed by operator or ' ', WORD otherwise | |
| IN_BRACES     | *                       | IN_BRACES                                            | |
| IN_BRACES_IN_DB_QUOTE | ?, $ or 0 followed by } | IN_DB_QUOTE           | |
| IN_BRACES_IN_DB_QUOTE | }                       | IN_DB_QUOTE           | |
| IN_BRACES_IN_DB_QUOTE | *                       | IN_BRACES_IN_DB_QUOTE | |
| IN_SG_QUOTE   | ' | WORD        | | 
| IN_SG_QUOTE   | * | IN_SG_QUOTE | | 
| IN_DB_QUOTE   | " | WORD                  | |
| IN_DB_QUOTE   | \ | ESCAPE_IN_DB_QUOTE    | |
| IN_DB_QUOTE   | $ | IN_DOLLAR_IN_DB_QUOTE | Ends literal segment and creates new VAR segment |
| IN_DB_QUOTE   | * | IN_DB_QUOTE           | |