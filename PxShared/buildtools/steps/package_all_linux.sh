#!/bin/bash +x

set -e

# build docs
pushd "$(dirname "$0")/../../docs/_compile"
./build_all.sh
popd

# remove all zips
pushd "$(dirname "$0")/../.."
rm -f *.zip
popd

# replace packman's config file to use s3 instead of gtl
echo "s3" > "$(dirname "$0")/../packman/source.conf"

# run packager
if [ -z $1 ] ; then
	OPTIONS=""
else 
	OPTIONS="-v $1"
	shift
fi

pushd "$(dirname "$0")/../packager/"
./create_packages.sh linux $OPTIONS $@
popd