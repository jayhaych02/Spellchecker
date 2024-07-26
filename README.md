# Jaden Howard 3207 Project-3-f23 Networked Spell Checker   

### Project Overview
Server-Client design to spell check words for an arbitrary number of clients

### Setup
Organization: Header Files/Structs
   Utilized Header Files & Structs to keep similar data organized

Server Configuration Struct
   A live server can be run with many different configurations, therefore I utilized a struct containing all possible configuration flags.
   This allowed me to maintain a reference to the user's server flag configurations from the makefile command when running the server.
   
The Makefile Command:
   run: server.out
      ./server.out -k $(PORT) -w $(THREADS) -j $(BUFFER) -b $(DICT) -e $(PRIO)",
      
Makefile
   Utilized to compile the server/client files & test different command configurations

### Server & Client Interaction
   
Running the Server
   The user runs the server with the make command:
      run: server.out
      ./server.out -k $(PORT) -w $(THREADS) -j $(BUFFER) -b $(DICT) -e $(PRIO)

   ALL FLAGS must be specified for the server to function.
   The only exception is the DICT (dictionary).
   If the user enters, " -b DICT=", the program uses the dictionary, "/usr/share/dict/words", provided by my College's Linux Server

Running the Server:

   (1)

   Start multiple client terminal instances by SSHing to the same node as the server being run. 
   If the server is "lclient01", I would SSH to the correct node by typing: "ssh tun85812@cis-lclient01". 
   Doing so connects us to the server, allowing us to type a word to be spell-checked.

   Upon connection, the server assigns client-specific attributes to a Struct & inserts that Client into the connection buffer.
   That client is then handled by an assigned worker thread, that communicates with the client.
   The worker thread prompts the client for a word to spell check and the client responds.
   The worker thread scans the dictionary chosen when running the server, and assigns unique properties to that client.
   Those properties include: Word arrival time, Spellcheck completion time, and the status of the client's word; whether it was found or misspelled.
   The worker then inserts the newly updated client into a log buffer for the log thread to extract client data to write to a log file describing the clients' attributes.
   


   (2)
   
   Run the make command to simulate 5 Clients attempting to connect to the server with predefined words to be spell-checked.
   All you need is 2 terminals: 1 terminal for the server & 1 for the client simulations that are run by the make command.
   Running the client simulation works similarly, except the worker thread messaging format in the terminal can be messy due to multiple server connections
   being initialized at the exact same time.