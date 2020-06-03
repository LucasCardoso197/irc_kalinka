## Project of an internet relay chat for academic purposes

###### How to run:
First use the command to compile:
- make all

Run server first, to liston for connections:
- make run_server

Then run the client on another terminal:
- make run_client

When both the server and client are open, they can exchange messages normally by just typing the message on the terminal and pressing enter.

Client commands:
- connect to server:
  - /connect
- ping server:
  - /ping
- close connection and finish the program:
  - /quit or Ctrl + D  


Operating System:
- Ubuntu 18.04 (Bionic Beaver)

Compiler:
- gcc (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0
