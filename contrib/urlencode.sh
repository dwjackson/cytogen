## This Source Code Form is subject to the terms of the Mozilla Public
## License, v. 2.0. If a copy of the MPL was not distributed with this
## file, You can obtain one at https://mozilla.org/MPL/2.0/.

## Copyright (c) 2020 David Jackson


# Encode a URL to escape parentheses

if [ "$#" -ne 1 ]
then
	echo 'USAGE: urlencode.sh [URL]'
	exit 1
fi

url="$1"
echo "$url" | sed -e 's/(/%28/g; s/)/%29/g'
