#!/bin/sh

CYTO="../src/cyto"
SITE_DIR='_site'
EXPECTED='_expected'

test_file() {
	test_name="$1"
	actual_file="$2"
	expected_file="$3"
	
	af_bn=`basename $actual_file`
	if [ "$af_bn" = "feed.xml" ]
	then
		# Strip the update tag line because it's always different
		actual_file_tmp=`mktemp`
		expected_file_tmp=`mktemp`
		sed -e '/<updated>/d' "$actual_file" > "$actual_file_tmp"
		sed -e '/<updated>/d' "$expected_file" > "$expected_file_tmp"
		actual_file="$actual_file_tmp"
		expected_file="$expected_file_tmp"
	fi
	d=`diff "$expected_file" "$actual_file" > /dev/null`
	if [ "$?" -ne 0 ]
	then
		printf "FAIL: File is wrong: $actual_file\n"
		diff -u "$expected_file" "$actual_file"
		exit 1
	fi
}

test_directory() {
	test_name="$1"
	test_dir="$2"
	for f in `ls "$test_dir"`
	do
		actual_file="$test_dir/$f"
		expected_file=`echo "$actual_file" \
			| sed -e "s|$SITE_DIR|$EXPECTED|g"`

		if [ ! -f "$actual_file" -a ! -d "$actual_file" ]
		then
			printf "FAIL: actual file missing: $actual_file\n"
			exit 1
		fi

		if [ -f "$f" -a ! -f "$expected_file" ]
		then
			printf "FAIL: Unexpected file: $actual_file\n"
			echo "EXPECTED: $expected_file"
			exit 1
		fi

		if [ -d "$f" -a ! -d "$expected_file" ]
		then
			printf "FAIL: Unexpected directory: $actual_file\n"
			exit 1
		fi

		if [ -f "$actual_file" ]
		then
			test_file "$test_name" "$actual_file" "$expected_file"
			if [ "$?" -eq 1 ]
			then
				exit 1
			fi
		elif [ -d "$actual_file" ]
		then
			(
				test_directory "$test_name" "$actual_file"
			)
			if [ "$?" -eq 1 ]
			then
				exit 1
			fi
		fi
	done
}

run_test() {
	test_name="$1"
	printf "Running $test_name... "
	../$CYTO generate

	if [ ! -d "$EXPECTED" ]
	then
		echo "$test_name: ERROR: No $EXPECTED directory"
		exit 1
	fi

	test_directory "$test_name" "./$SITE_DIR"

	../$CYTO clean

	if [ "$?" -eq 0 ]
	then
		printf "PASS\n"
	else
		exit 1
	fi
}

if [ ! -f "$CYTO" ]
then
	echo 'ERROR: cyto not built'
	exit 1
fi

fail_count=0
for dir in `find . -type 'd' -name '*_test' | sort -n`
do
	(
		cd "$dir"
		run_test "$dir"
	)

	if [ "$?" -ne 0 ]
	then
		fail_count=`dc -e "$fail_count 1 + p"`
	fi
done

echo '----------'
if [ "$fail_count" -ne 0 ]
then
	echo 'TEST(S) FAILED'
	exit 1
else
	echo 'ALL TESTS PASSED'
fi
