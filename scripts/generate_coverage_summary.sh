#!/bin/bash
# Generate lightweight coverage summary for git tracking
# This creates a snapshot of coverage without the full HTML report

SUMMARY_FILE="docs/coverage-summary.txt"
BADGE_FILE="docs/coverage-badge.svg"

# Create docs directory if it doesn't exist
mkdir -p docs

# Extract coverage percentages
LINE_COVERAGE=$(lcov --summary coverage.filtered.info 2>/dev/null | grep "lines" | grep -oP '\d+\.\d+%' | head -1)
FUNC_COVERAGE=$(lcov --summary coverage.filtered.info 2>/dev/null | grep "functions" | grep -oP '\d+\.\d+%' | head -1)

# Get detailed file list
FILE_LIST=$(lcov --list coverage.filtered.info 2>/dev/null | grep -E "\.cpp|\.h" | grep -v googletest | grep -v "span-lite")

# Generate summary text file
cat > "$SUMMARY_FILE" << EOF
ESP32 WiFi Serial - Code Coverage Summary
==========================================
Generated: $(date '+%Y-%m-%d %H:%M:%S')

Overall Coverage:
  Lines:      $LINE_COVERAGE
  Functions:  $FUNC_COVERAGE

Coverage Philosophy:
This report includes ALL source files in the project, including
ESP-coupled code that cannot be tested in native environment.

Files at 0% are candidates for policy-based abstraction to enable
native unit testing without hardware dependencies.

Policy-Based Testing Approach:
- Uses template policies instead of virtual functions (zero-cost)
- Test policies provide controllable behavior for verification
- ESP-coupled code shows 0% until abstracted
- Well-abstracted code shows 90%+ coverage

Detailed File Coverage:
$FILE_LIST

Next Steps:
- Abstract ESP-coupled files using policy pattern
- Increase testable surface area
- Maintain high coverage on policy-based code

For full HTML report: make coverage && make view-coverage
EOF

# Generate SVG badge
LINE_PCT=${LINE_COVERAGE%\%}
COLOR="red"
if (( $(echo "$LINE_PCT > 50" | bc -l) )); then COLOR="yellow"; fi
if (( $(echo "$LINE_PCT > 75" | bc -l) )); then COLOR="green"; fi

cat > "$BADGE_FILE" << EOF
<svg xmlns="http://www.w3.org/2000/svg" width="120" height="20">
  <linearGradient id="b" x2="0" y2="100%">
    <stop offset="0" stop-color="#bbb" stop-opacity=".1"/>
    <stop offset="1" stop-opacity=".1"/>
  </linearGradient>
  <mask id="a">
    <rect width="120" height="20" rx="3" fill="#fff"/>
  </mask>
  <g mask="url(#a)">
    <path fill="#555" d="M0 0h70v20H0z"/>
    <path fill="$COLOR" d="M70 0h50v20H70z"/>
    <path fill="url(#b)" d="M0 0h120v20H0z"/>
  </g>
  <g fill="#fff" text-anchor="middle" font-family="DejaVu Sans,Verdana,Geneva,sans-serif" font-size="11">
    <text x="35" y="15" fill="#010101" fill-opacity=".3">coverage</text>
    <text x="35" y="14">coverage</text>
    <text x="95" y="15" fill="#010101" fill-opacity=".3">$LINE_COVERAGE</text>
    <text x="95" y="14">$LINE_COVERAGE</text>
  </g>
</svg>
EOF

echo "Coverage summary generated:"
echo "  - $SUMMARY_FILE"
echo "  - $BADGE_FILE"
echo ""
echo "Coverage: Lines $LINE_COVERAGE, Functions $FUNC_COVERAGE"
