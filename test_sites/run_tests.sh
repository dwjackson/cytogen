#!/bin/sh

CYTO="../src/cyto"
SITE_DIR='./_site'
EXPECTED='./_expected'

run_test() {
	test_name="$1"
	../$CYTO generate
	for f in `ls "$SITE_DIR"`
	do
		if [ ! -d "$EXPECTED" ]
		then
			echo "$test_name: ERROR: No $EXPECTED directory"
			exit 1
		fi

		expected_file="$EXPECTED/$f"
		if [ ! -f "$expected_file" ]
		then
			echo "$test_name: FAIL: Unexpected file: $f"
			exit 1
		fi

		d=`diff "$expected_file" "$SITE_DIR/$f" > /dev/null`
		if [ "$?" -ne 0 ]
		then
			echo "$test_name: FAIL: File is wrong: $f"
			echo 'EXPECTED'
			cat "$expected_file"
			echo 'ACTUAL'
			cat "$f"
			exit 1
		fi
	done
	../$CYTO clean
	echo "$test_name: PASSED"
}

if [ ! -f "$CYTO" ]
then
	echo 'ERROR: cyto not built'
	exit 1
fi

for dir in `find . -type 'd' -name '*_test' | sort -n`
do
	(
		cd "$dir"
		run_test "$dir"
	)
done
