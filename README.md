## Project of an internet relay chat for academic purposes

###### How to run:
First use the command to compile:
- make all

Run server first, to listen for connections:
- make run_server

Then run the client on another terminal:
- make run_client

You can run up to 30 client programs. Once they're all connected, messages can be exchanged between clients through the server.

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
