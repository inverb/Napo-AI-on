#!/usr/bin/env bash

set -euo pipefail

AGENTS="random basic heuristic1 heuristic2 flat_mc flat_mc_extended"
SEED=42
MATCHES=80

DATE=`date +"%y-%m-%d__%H-%M-%S"`

REPORT_DIR="reports/$DATE"

mkdir -p "$REPORT_DIR"

for a1 in $AGENTS; do
    for a2 in $AGENTS; do
        echo "${a1} VS ${a2}"

        if [ "$a1" = "$a2" ]; then
            echo "Skipping..."
            continue
        fi

        FIGHT_DIR="${REPORT_DIR}/${a1}_vs_${a2}"
        LOGS_SUBDIR="$FIGHT_DIR/logs"
        HASH_FILE="$FIGHT_DIR/hashes.txt"
        AGENT1="../agents/artifacts/$a1"
        AGENT2="../agents/artifacts/$a2"

        mkdir -p $FIGHT_DIR
        mkdir -p $LOGS_SUBDIR
        
        echo "Program hashes:" >> $HASH_FILE
        sha256sum "$AGENT1" >> $HASH_FILE
        sha256sum "$AGENT2" >> $HASH_FILE

        java -jar cg-brutaltester.jar -o \
            -r "java -jar referee.jar" -l "$LOGS_SUBDIR" -s -i $SEED -n $MATCHES \
            -p1 "$AGENT1" -p2 "$AGENT2" \
            1> "${FIGHT_DIR}/report.out" 2> "${FIGHT_DIR}/report.err"
    done
done