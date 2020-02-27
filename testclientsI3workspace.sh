#!/bin/bash

i3-msg 'Workspace 4';
xterm -hold -e "./cmake-build-debug/server" &
sleep 1;
i3-msg 'Workspace 5';
i3-msg 'append_layout ~/CLionProjects/networksystems/i3-layout-tui.json'
xterm -xrm "xterm*allowTitleOps: false" -T "client1tui" -hold -e "sleep 1; ./cmake-build-debug/node 1"    &
xterm -xrm "xterm*allowTitleOps: false" -T "client2tui" -hold -e "sleep 2; ./cmake-build-debug/node 2"    &
xterm -xrm "xterm*allowTitleOps: false" -T "client3tui" -hold -e "sleep 3; ./cmake-build-debug/node 3"    &
xterm -xrm "xterm*allowTitleOps: false" -T "client4tui" -hold -e "sleep 4; ./cmake-build-debug/node 4"    &
i3-msg 'Workspace 6';
i3-msg 'append_layout ~/CLionProjects/networksystems/i3-layout-log.json'
xterm -xrm "xterm*allowTitleOps: false" -T "client1log" -e sh -c 'trap sh SIGINT; tail -f ./client_1.txt' &
xterm -xrm "xterm*allowTitleOps: false" -T "client2log" -e sh -c 'trap sh SIGINT; tail -f ./client_2.txt' &
xterm -xrm "xterm*allowTitleOps: false" -T "client3log" -e sh -c 'trap sh SIGINT; tail -f ./client_3.txt' &
xterm -xrm "xterm*allowTitleOps: false" -T "client4log" -e sh -c 'trap sh SIGINT; tail -f ./client_4.txt' &

