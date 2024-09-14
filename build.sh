#!/bin/bash

libs="-luser32 -lopengl32 -lgdi32"
warnings="-Wno-writable-strings -Wno-format-security"
security="-Wnoformat-security"
includes="-Ithird_party -Ithird_party/Include"
clang $includes -g src/main.cpp -omemo.exe $libs $warnings $security
