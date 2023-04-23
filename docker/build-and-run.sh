#!/bin/bash

cd /home/build && \
    cmake /home/project && \
    make && \
    ctest && \
    ./bin/run_app 0.0.0.0 8080 /home/volume 1
