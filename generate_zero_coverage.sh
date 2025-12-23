#!/bin/bash
# Generate zero-coverage baseline for all source files not in coverage report
#
# This script ensures ALL source files appear in the coverage report,
# even if they weren't compiled/tested (they'll show 0% coverage)

COVERAGE_FILE="$1"
OUTPUT_FILE="$2"

if [ -z "$COVERAGE_FILE" ] || [ -z "$OUTPUT_FILE" ]; then
    echo "Usage: $0 <input_coverage_file> <output_file>"
    exit 1
fi

# Get list of files already in coverage
FILES_IN_COVERAGE=$(lcov --list "$COVERAGE_FILE" 2>/dev/null | grep -E "\.cpp|\.h" | awk '{print $1}' | sed 's/.*src\///' || true)

# Create temp file for zero coverage entries
ZERO_COV_FILE=$(mktemp)

echo "TN:" > "$ZERO_COV_FILE"

# Find all source files and add zero coverage for missing ones
find src -type f \( -name "*.cpp" -o -name "*.h" \) | while read -r file; do
    # Get relative path from src/
    rel_path="${file#src/}"

    # Check if this file is already in coverage
    if ! echo "$FILES_IN_COVERAGE" | grep -q "$rel_path"; then
        # Add zero coverage entry for this file
        abs_path=$(realpath "$file" 2>/dev/null || echo "$PWD/$file")

        # Add file entry with zero coverage
        echo "SF:$abs_path" >> "$ZERO_COV_FILE"
        echo "DA:1,0" >> "$ZERO_COV_FILE"  # At least one line to show 0%
        echo "LF:1" >> "$ZERO_COV_FILE"     # 1 line found
        echo "LH:0" >> "$ZERO_COV_FILE"     # 0 lines hit
        echo "end_of_record" >> "$ZERO_COV_FILE"
    fi
done

# If we have actual coverage, merge it; otherwise just use zero coverage
if [ -f "$COVERAGE_FILE" ] && [ -s "$COVERAGE_FILE" ]; then
    lcov --add-tracefile "$COVERAGE_FILE" \
         --add-tracefile "$ZERO_COV_FILE" \
         --output-file "$OUTPUT_FILE" 2>/dev/null || cp "$COVERAGE_FILE" "$OUTPUT_FILE"
else
    cp "$ZERO_COV_FILE" "$OUTPUT_FILE"
fi

# Cleanup
rm -f "$ZERO_COV_FILE"
