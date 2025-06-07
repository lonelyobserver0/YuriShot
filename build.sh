#!/bin/bash

cd build
qmake ../src/yurishot.pro
make
mv yurishot ../bin
