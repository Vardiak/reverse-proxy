#!/bin/bash

docker build . -t epita -f ./Dockerfile
docker run --rm --privileged -v $(pwd):/root/data --workdir /root/data -it epita zsh
