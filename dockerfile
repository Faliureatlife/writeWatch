FROM debian

# Install minimal required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    vim \
    inotify-tools \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /app

# We'll mount our source code here instead of copying it
# This allows for easier development/testing cycle
