#!/bin/bash -e

BINDIR=`readlink -f "$(dirname "${BASH_SOURCE[0]}")/../bin/"`
SORT="$BINDIR/sort"
GENERATE="$BINDIR/generateRandomUint64File"

UNSORTED=`tempfile`
SORTEDBIN=`tempfile`
SORTED=`tempfile`

for SIZE in 12 100 1032 4096 100000
do
  "$GENERATE" "$UNSORTED" $SIZE
  "$SORT" "$UNSORTED" "$SORTEDBIN" 1
  od -A n -t u8 -w8 -v "$SORTEDBIN" > "$SORTED"
  if ! od -A n -t u8 -w8 -v "$UNSORTED" | sort -n | cmp - "$SORTED" -s; then
    echo "numbers where not sorted correctly"
    exit 1;
  fi
  rm "$UNSORTED" "$SORTEDBIN" "$SORTED"
done
