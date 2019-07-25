#!/bin/bash

REPLICA_SOURCE=PhysX-3.4/PhysX_3.4/Samples/Replicas/Source
SYMLINK_DESTINATION=Plugins/Replicas/Source/Replicas/Source

# Does the submodule exist?
sources=($REPLICA_SOURCE/*.cpp)
if [ ! -e $sources ]; then
	echo "Couldn't find sources in $REPLICA_SOURCE - did you checkout the submodule with 'git submodule update --init?'"
	exit 1
fi

# Check if symlink already exists
if [ -L $SYMLINK_DESTINATION ]; then
	echo "$SYMLINK_DESTINATION already exists, or this script has been run already."
	exit 1
fi

# Create a symlink to the Replica sources folder
ln -rsv $REPLICA_SOURCE $SYMLINK_DESTINATION
