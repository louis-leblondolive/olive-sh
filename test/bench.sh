#!/usr/bin/env bash
#
# bench_olvsh.sh — benchmark olvsh against another shell across a few scenarios
#
# Usage:
#   ./bench_olvsh.sh [-s shell] [olvsh_path] [n_commands]
#
# Examples:
#   ./bench_olvsh.sh bin/olvsh 100
#   ./bench_olvsh.sh -s zsh bin/olvsh 200
#
set -euo pipefail

COMPARE_SHELL="bash"

while getopts "s:" opt; do
    case "$opt" in
        s) COMPARE_SHELL="$OPTARG" ;;
        *) echo "Usage: $0 [-s shell] [olvsh_path] [n_commands]" >&2; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

OLVSH="${1:-bin/olvsh}"
N="${2:-100}"
WARMUP=5
MIN_RUNS=20
OUTDIR="/tmp/olvsh_bench"
RESULTS_MD="${OUTDIR}/results.md"
WARN_LOG="${OUTDIR}/warnings.log"

# unit separator, won't collide with anything a scenario label might contain
SEP=$'\x1f'

if ! command -v hyperfine >/dev/null 2>&1; then
    echo "hyperfine not found (brew install hyperfine)" >&2
    exit 1
fi

if [ ! -x "$OLVSH" ]; then
    echo "olvsh binary not found or not executable: $OLVSH" >&2
    exit 1
fi

if ! command -v "$COMPARE_SHELL" >/dev/null 2>&1; then
    echo "comparison shell not found: $COMPARE_SHELL" >&2
    exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "python3 not found (needed to parse hyperfine's json output)" >&2
    exit 1
fi

mkdir -p "$OUTDIR"
: > "$RESULTS_MD"
: > "$WARN_LOG"
echo "# olvsh vs $COMPARE_SHELL ($N commands, warmup=$WARMUP)" >> "$RESULTS_MD"
echo "" >> "$RESULTS_MD"

echo "olvsh:    $OLVSH"
echo "compare:  $COMPARE_SHELL"
echo "commands: $N"
echo ""

# scenario files, generated once per run

{ for _ in $(seq 1 "$N"); do echo "true"; done; echo "exit"; } > "${OUTDIR}/builtin_true.txt"
{ for _ in $(seq 1 "$N"); do echo "echo hello > /dev/null"; done; echo "exit"; } > "${OUTDIR}/builtin_echo.txt"
{ for _ in $(seq 1 "$N"); do echo "/usr/bin/true"; done; echo "exit"; } > "${OUTDIR}/external_true.txt"
{ for _ in $(seq 1 "$N"); do echo "true && true"; done; echo "exit"; } > "${OUTDIR}/operator_and.txt"
{ for _ in $(seq 1 "$N"); do echo "false || true"; done; echo "exit"; } > "${OUTDIR}/operator_or.txt"
{ for _ in $(seq 1 "$N"); do echo "echo hi | cat > /dev/null"; done; echo "exit"; } > "${OUTDIR}/pipeline_simple.txt"
{ for _ in $(seq 1 "$N"); do echo "export X=42; echo \$X > /dev/null"; done; echo "exit"; } > "${OUTDIR}/variables.txt"
{ for _ in $(seq 1 "$N"); do echo "echo test > /dev/null"; done; echo "exit"; } > "${OUTDIR}/redirection.txt"

declare -a SCENARIOS=(
    "builtin_true.txt:builtin (true)"
    "builtin_echo.txt:builtin (echo)"
    "external_true.txt:external command"
    "operator_and.txt:&& operator"
    "operator_or.txt:|| operator"
    "pipeline_simple.txt:pipeline"
    "variables.txt:variables"
    "redirection.txt:redirection"
)

SUMMARY_ROWS=()

for entry in "${SCENARIOS[@]}"; do
    file="${entry%%:*}"
    label="${entry##*:}"
    filepath="${OUTDIR}/${file}"
    json_out="${OUTDIR}/result_${file%.txt}.json"

    echo "running: $label"

    # feed stdin via --input instead of shell redirection, so hyperfine can
    # run the binaries directly (--shell=none) instead of wrapping them in
    # sh -c. that removes a whole extra fork+exec from every measurement.
    hyperfine \
        --shell=none \
        --input "$filepath" \
        --warmup "$WARMUP" \
        --min-runs "$MIN_RUNS" \
        --export-markdown "${OUTDIR}/result_${file%.txt}.md" \
        --export-json "$json_out" \
        --command-name "olvsh" "$OLVSH" \
        --command-name "$COMPARE_SHELL" "$COMPARE_SHELL" \
        >> "$WARN_LOG" 2>&1 || true

    {
        echo "## ${label}"
        echo ""
        cat "${OUTDIR}/result_${file%.txt}.md"
        echo ""
    } >> "$RESULTS_MD"

    read -r olvsh_mean other_mean <<< "$(python3 -c "
import json
data = json.load(open('$json_out'))
means = {r['command']: r['mean'] * 1000 for r in data['results']}
print(f\"{means['olvsh']:.2f} {means['$COMPARE_SHELL']:.2f}\")
")"

    ratio=$(python3 -c "print(f'{$olvsh_mean / $other_mean:.2f}')")
    SUMMARY_ROWS+=("${label}${SEP}${olvsh_mean}${SEP}${other_mean}${SEP}${ratio}")
done

echo ""
printf "%-22s %12s %14s %10s\n" "scenario" "olvsh (ms)" "$COMPARE_SHELL (ms)" "ratio"
printf "%-22s %12s %14s %10s\n" "--------" "----------" "------------" "-----"
for row in "${SUMMARY_ROWS[@]}"; do
    IFS="$SEP" read -r label olvsh_mean other_mean ratio <<< "$row"
    printf "%-22s %12s %14s %10s\n" "$label" "$olvsh_mean" "$other_mean" "${ratio}x"
done

echo ""
echo "see more details in $RESULTS_MD"
echo "hyperfine warnings (if any) logged to $WARN_LOG"