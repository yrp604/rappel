#!/bin/sh
set -ex
echo "mov x0, #0x4141" | ../bin/rappel | grep -q "0000000000004141"
