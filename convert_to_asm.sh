#!/bin/bash
# Convert OMR test output to GNU assembler format
# Usage: ./convert_to_asm.sh input.txt > output.s
#
# This script converts the OMR compiler test output format to GNU assembler syntax.
# Input format:  "        0x23b43180 00000000 [        0x23b6b160] 7c221814          0    addc    gr1, gr2, gr3"
# Output format: "addc 1,2,3"

if [ $# -eq 0 ]; then
    echo "Usage: $0 <input_file>" >&2
    echo "Example: $0 all_instructions.txt > output.s" >&2
    exit 1
fi

INPUT_FILE="$1"

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: File '$INPUT_FILE' not found" >&2
    exit 1
fi

# Process the file with awk and sed
awk 'BEGIN{print ".text"} /\[/{for(i=7;i<=NF;i++) printf "%s%s", $i, (i<NF?" ":""); print ""}' "$INPUT_FILE" | \
    sed 's/gr//g' | \
    sed 's/, */,/g'