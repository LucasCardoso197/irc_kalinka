## Project of an internet relay chat for academic purposes

###### How to run:
First use the command to compile:
- make all

Run server first, to listen for connections:
- make run_server

Then run the client on another terminal:
- make run_client

You can run up to 50 client programs. Once they're all connected, messages can be exchanged between clients through the server.

General commands:
- connect to server:
  - /connect
- ping server:
  - /ping
- close connection and finish the program:
  - /quit or Ctrl + D  
- join or create a channel:
  - /join #channelName
- create or change your nickname on the server:
  - /nickname userName

Some commands help the admin of a channel to moderate it:
- mute a user in your channel indefinetely, not allowing they to send any messages:
  - /mute userName
- unmute an user in your channel:
  - /unmute userName
- see user's ip address:
  - /whois userName
- remove user from channel:
  - /kick userName
- toggle invite-only mode on channel, to stop non-invited users to join:
  - /mode channelName [+|-]i
- invite user to a channel, allowing him to join even on invite-only mode:
  - /invite channelName userName

Operating System:
- Ubuntu 18.04 (Bionic Beaver)

Compiler:
- gcc (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0
