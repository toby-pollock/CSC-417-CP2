#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 '<json_string>'"
    exit 1
fi

# Run the interpreter with the provided JSON string
./interpreter "$1"