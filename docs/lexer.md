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

        START -->|"' '"| START
        START -->|"*"| WORD
        START -->|"Operator"| OPERATOR
        START -->|"$"| IN_DOLLAR
        
        WORD -->|"Operator or ' '"| START
        WORD -->|"*"| WORD
        WORD -->|"\"| ESCAPE
        WORD -->|"$"| IN_DOLLAR
        WORD -->|"'"| IN_SG_QUOTE
        WORD -->|"'#quot;'"| IN_DB_QUOTE
        WORD -->|"'#quot;'$"| IN_DOLLAR_IN_DB_QUOTE

        
    ```

## Transition Matrix 

| Initial State | Read Character | Next State | Associated action |
| :--- | :--- | :--- | :--- |
