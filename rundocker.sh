#!/bin/bash
# Build the Docker image
docker build -t inotify-test .

# Run container with source code mounted
docker run -it --rm \
    -v "$(pwd):/app" \
    inotify-test \
    /bin/bash
