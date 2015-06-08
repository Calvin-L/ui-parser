#!/bin/bash

set -e

PARSER="$1"
export TIMEFORMAT="%R"
shift

function debug {
    echo "$@" >&2
}


echo "\\begin{tabular}{lr}"
echo "\\multicolumn{1}{c}{\\textbf{INPUT}} & \\multicolumn{1}{c}{\\textbf{TIME (s)}}\\\\"
echo "\\hline\\\\"

for MODE in dry real; do
    for EXAMPLE in $@; do
        debug "benchmarking $EXAMPLE [$MODE]"
        TIME="$( (time $PARSER "$EXAMPLE" >/dev/null 2>/dev/null) 2>&1)"
        if [[ "$MODE" == "real" ]]; then
            echo "\verb+$(basename "$EXAMPLE")+ & $TIME\\\\"
        fi
    done
done

echo "\\end{tabular}"
