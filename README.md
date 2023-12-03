# README

## Brief
This repo contains files and code for a minimal, toy HTTP/1.x server. *nix systems and Linux distros should support the C standard library and UNIX networking headers, but Windows will not. Not for production usage but for learning purposes.

## References
 - [HTTP 1.x Introduction](https://jmarshall.com/easy/http/)

## Usage
 - Run `make all` to build the program.
 - Enter `./h1cserver` to run the server on default port 8080.
 - Enter `./h1cserver n` to run the server on port n where n is at least 1024.
 - Enter `make clean && make all` after changes to refresh the build.

## To Do's
 1. ~~Implement response writer.~~
 2. ~~Implement and fill in main server "object".~~
 3. ~~Refactor server to send `Date: xxx` header.~~
 4. Refactor server to use route handler logic.
    - ~~Implement static resources.~~
    - ~~Implement date utility and resource cache before packing them into a context object.~~
    - ~~Add context argument to callback & fallback function signatures.~~
    - ~~Integrate `RouteMap` into main server logic.~~
    - ~~Possibly refactor server to use thread pools: `BlockedQueue`, `ServerListener`, `ServerWorker`~~
    - Fix server to gracefully exit on `SIGINT`. Currently the CTRL+C keystroke does not cleanly exit: the first time only prints the exiting message. Even then there is a double free almost certainly within the called cleanup code per worker. 
