#!/usr/bin/env bash
#
# run_tests.sh - olvsh smoke tests
#
# Pipes inputs to the shell and asserts stdout/exit codes.
#
# Usage: ./test/run_tests.sh [path-to-binary]
# Default: bin/olvsh (pass bin/olvsh-debug for ASan runs)

set -uo pipefail

SHELL_BIN="${1:-bin/olvsh}"

if [ ! -x "$SHELL_BIN" ]; then
    echo "error: '$SHELL_BIN' not found or not executable" >&2
    echo "build it first with 'make' or 'make debug', or pass a path as \$1" >&2
    exit 1
fi

PASS=0
FAIL=0
FAILED_TESTS=()

# ---------------------------------------------------------------------------
# run_with_timeout SECONDS CMD...
# Wraps GNU timeout/gtimeout.
# Fallback: manual background + kill for bare macOS/BSD.
# Returns 124 on timeout.
# ---------------------------------------------------------------------------
if command -v timeout >/dev/null 2>&1; then
    TIMEOUT_BIN="timeout"
elif command -v gtimeout >/dev/null 2>&1; then
    TIMEOUT_BIN="gtimeout"
else
    TIMEOUT_BIN=""
fi

# ---------------------------------------------------------------------------
# Cross-platform hidden char reveal (GNU cat -A vs BSD cat -etv)
# ---------------------------------------------------------------------------
if echo "" | cat -A >/dev/null 2>&1; then
    CAT_SHOW_ALL="cat -A"
else
    CAT_SHOW_ALL="cat -etv"
fi

run_with_timeout() {
    local secs="$1"
    shift

    if [ -n "$TIMEOUT_BIN" ]; then
        "$TIMEOUT_BIN" "$secs" "$@"
        return $?
    fi

    # Manual fallback loop
    "$@" <&0 &
    local pid=$!
    local interval="0.1"
    local steps
    steps=$(awk -v s="$secs" -v i="$interval" 'BEGIN { print int(s / i) }')

    local i
    for ((i = 0; i < steps; i++)); do
        if ! kill -0 "$pid" 2>/dev/null; then
            wait "$pid"
            return $?
        fi
        sleep "$interval"
    done

    if kill -0 "$pid" 2>/dev/null; then
        kill -TERM "$pid" 2>/dev/null
        sleep 0.2
        kill -KILL "$pid" 2>/dev/null
        wait "$pid" 2>/dev/null
        return 124
    fi

    wait "$pid"
    return $?
}

# ---------------------------------------------------------------------------
# assert_output NAME INPUT EXPECTED_OUTPUT
# Checks strict stdout match.
# ---------------------------------------------------------------------------
assert_output() {
    local name="$1"
    local input="$2"
    local expected="$3"
    local actual

    actual="$(printf '%s\n' "$input" | run_with_timeout 5 "$SHELL_BIN" 2>/dev/null)"

    if [ "$actual" == "$expected" ]; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
        FAILED_TESTS+=("$name")
        echo "  FAIL - $name"
        echo "         input:    $input"
        echo "         expected: $expected"
        echo -n "         actual:   "
        echo -n "$actual" | $CAT_SHOW_ALL
        echo ""
    fi
}

# ---------------------------------------------------------------------------
# assert_exit_code NAME INPUT EXPECTED_CODE
# Checks shell exit status.
# ---------------------------------------------------------------------------
assert_exit_code() {
    local name="$1"
    local input="$2"
    local expected="$3"
    local actual

    printf '%s\n' "$input" | run_with_timeout 5 "$SHELL_BIN" >/dev/null 2>&1
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
        FAILED_TESTS+=("$name")
        echo "  FAIL - $name"
        echo "         input:    $input"
        echo "         expected: $expected"
        echo -n "         actual:   " 
        echo -n "$actual" | $CAT_SHOW_ALL   
        echo ""
    fi
}

# ---------------------------------------------------------------------------
# assert_no_crash NAME INPUT
# Ensures input doesn't trigger segfault/ASan abort.
# ---------------------------------------------------------------------------
assert_no_crash() {
    local name="$1"
    local input="$2"
    local code

    printf '%s\n' "$input" | run_with_timeout 5 "$SHELL_BIN" >/dev/null 2>&1
    code="$?"

    # 124 = timeout, 134 = SIGABRT (ASan), 139 = SIGSEGV
    if [ "$code" == "124" ] || [ "$code" == "134" ] || [ "$code" == "139" ]; then
        FAIL=$((FAIL + 1))
        FAILED_TESTS+=("$name")
        echo "  FAIL - $name (crashed/hung, exit code $code)"
        echo "         input: $input"
    else
        PASS=$((PASS + 1))
    fi
}

echo "Running olive-sh smoke tests against: $SHELL_BIN"
echo

# --- Basic execution ---------------------------------------------------
echo "== Basic execution =="
assert_output "simple echo"              'echo hello'                 'hello'
assert_output "echo with multiple args"  'echo hello world'           'hello world'
assert_exit_code "true builtin/binary"   'true'                       0
assert_exit_code "false builtin/binary"  'false'                      1

# --- Operators -----------------------------------------------------------
echo "== Operators =="
assert_output "pipe"                     'echo hello | cat'           'hello'
assert_output "multi-stage pipe"         'echo hello | cat | cat'     'hello'
assert_output "&& runs on success"       'true && echo yes'           'yes'
assert_output "&& skips on failure"      'false && echo yes'          ''
assert_output "|| runs on failure"       'false || echo fallback'     'fallback'
assert_output "|| skips on success"      'true || echo fallback'      ''
assert_output "; sequencing"             'echo a ; echo b'            "$(printf 'a\nb')"

# --- Redirections --------------------------------------------------------
echo "== Redirections =="
TMP_FILE="$(mktemp)"
assert_output "redirect output then cat"  "echo redir_test > $TMP_FILE"$'\n'"cat $TMP_FILE" 'redir_test'
rm -f "$TMP_FILE"

assert_no_crash "redirect to nonexistent dir"  'echo hi > /this/path/does/not/exist/file'

# --- Variables -------------------------------------------------------------
echo "== Variables =="
assert_output "variable expansion"        'export FOO=bar'$'\n''echo $FOO'   'bar'
assert_output "unset variable"            'echo $UNSET_VAR_XYZ'              ''

# --- Quoting / lexer edge cases --------------------------------------------
echo "== Quoting & lexer edge cases =="
assert_output "single quotes preserve literal" "echo 'a  b'"          'a  b'
assert_output "double quotes with variable"    'export X=1'$'\n''echo "val=$X"' 'val=1'
assert_no_crash "unterminated double quote"    'echo "unterminated'
assert_no_crash "unterminated single quote"    "echo 'unterminated"
assert_no_crash "unclosed brace expansion"     'echo ${unclosed'
assert_no_crash "long word (near MAX_WORD_LENGTH)" "echo $(printf 'a%.0s' {1..3000})"
assert_no_crash "empty escape at EOF"          'echo \'

# --- Parser edge cases ------------------------------------------------------
# These are syntactically invalid. The shell must reject them cleanly 
# (syntax error) and not crash.
echo "== Parser edge cases (malformed input must error cleanly, not crash) =="
assert_no_crash "leading pipe"             '| echo hi'
assert_no_crash "trailing pipe"            'echo hi |'
assert_no_crash "double pipe operator"     'echo hi | | echo bye'
assert_no_crash "leading &&"               '&& echo hi'
assert_no_crash "chained delimiters"       'echo hi ; ; ; echo bye'
assert_no_crash "long pipeline"            'echo a | cat | cat | cat | cat | cat | cat | cat | cat'

assert_output "leading pipe produces no output"   '| echo hi'                 ''
assert_output "trailing pipe produces no output"  'echo hi |'                 ''
assert_output "double pipe produces no output"    'echo hi | | echo bye'      ''

# --- Job control -------------------------------------------------------------
echo "== Job control =="
assert_no_crash "background job"           'sleep 0.1 &'$'\n''jobs'
assert_no_crash "background then wait"     'sleep 0.1 &'$'\n''wait'

# --- Summary -----------------------------------------------------------------
echo
echo "-----------------------------------"
echo "Passed: $PASS   Failed: $FAIL"
echo "-----------------------------------"

if [ "$FAIL" -ne 0 ]; then
    echo "Failed tests:"
    for t in "${FAILED_TESTS[@]}"; do
        echo "  - $t"
    done
    exit 1
fi

exit 0