#!/bin/sh
set -ex
echo "mov eax, 0x4141" | ../bin/rappel | grep -q "00004141"
