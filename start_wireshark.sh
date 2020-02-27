#!/bin/bash

wireshark -X lua_script:debug.lua -Y "jbp" -i any -k
