#!/bin/bash

xterm -hold -e ./cmake-build-debug/server	&
xterm -hold -e "sleep 1;./cmake-build-debug/node 1"	&
xterm -hold -e "sleep 2;./cmake-build-debug/node 2" &
xterm -hold -e "sleep 3;./cmake-build-debug/node 3"	&
xterm -hold -e "sleep 4;./cmake-build-debug/node 4"	&
