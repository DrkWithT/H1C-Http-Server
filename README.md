# README

## Brief
This repo contains files and code for a minimal, toy HTTP/1.x server. Not intended for production usage, and it is not fully portable since it's made from scratch on Mac OS.

## References
 - [HTTP Crash Course](https://jmarshall.com/easy/http/)

## Usage
 - Run `make all` to build the program.
 - Enter `./h1cserver` to run the server on default port 8080.
 - Enter `./h1cserver n` to run the server on port n where n is a positive integer literal.
 - Enter `make clean && make all` after changes to refresh the build.

## To Do's
 1. ~~Implement response writer.~~
 2. ~~Implement and fill in main server "object".~~
 3. ~~Refactor server to send `Date: xxx` header.~~
 4. Refactor server to use threads later.
