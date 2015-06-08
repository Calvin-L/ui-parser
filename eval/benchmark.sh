#!/bin/bash

set -e

PARSER="$1"
CORRECTNESS="$2"
export TIMEFORMAT="%R"
shift 2

function debug {
    echo "$@" >&2
}


echo "\\begin{tabular}{lrl}"
echo "\\multicolumn{1}{c}{\\textbf{INPUT}} &"
echo "\\multicolumn{1}{c}{\\textbf{TIME (s)}} &"
echo "\\multicolumn{1}{c}{\\textbf{QUALITY}}\\\\"
echo "\\hline\\\\"

for MODE in dry real; do
    for EXAMPLE in $@; do
        debug "benchmarking $EXAMPLE [$MODE]"
        TIME="$( (time $PARSER "$EXAMPLE" >/dev/null 2>/dev/null) 2>&1)"
        QUALITY="$(fgrep "$(basename "$EXAMPLE")" <"$CORRECTNESS" | awk '{print $2}')"
        if [[ "$MODE" == "real" ]]; then
            echo "\verb+$(basename "$EXAMPLE")+ & $TIME & $QUALITY\\\\"
        fi
    done
done

echo "\\end{tabular}"
