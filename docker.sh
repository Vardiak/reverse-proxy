#!/bin/bash

docker build . -t epita -f ./Dockerfile
docker run --rm --privileged -p 8000:8000 -v $(pwd):/root/data -v $(pwd)/.zsh_history:/root/.zsh_history --workdir /root/data -it epita zsh
