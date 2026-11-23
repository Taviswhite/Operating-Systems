
#include "server.h"

#define DEFAULT_ROOM "Lobby"

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern struct room *rooms;
extern struct dm   *dms;

extern char *server_MOTD;


/*
 *Main thread for each client.  Receives all messages
 *and passes the data off to the correct function.  Receives
 *a pointer to the file descriptor for the socket the thread
 *should listen on
 */

/////////////////////////////////////////////////////
// Reader-writer locking (readers-preference, as in the textbook):
//  - many readers (users, rooms, sending messages) may run together
//  - a writer (create, join, leave, login, connect, ...) runs alone
//  - mutex protects numReaders; rw_lock is held by the writer or
//    by the group of readers as a whole
void start_read(void) {
   pthread_mutex_lock(&mutex);
   numReaders++;
   if(numReaders == 1)               // first reader locks out writers
      pthread_mutex_lock(&rw_lock);
   pthread_mutex_unlock(&mutex);
}

void end_read(void) {
   pthread_mutex_lock(&mutex);
   numReaders--;
   if(numReaders == 0)               // last reader lets writers in
      pthread_mutex_unlock(&rw_lock);
   pthread_mutex_unlock(&mutex);
}

void start_write(void) {
   pthread_mutex_lock(&rw_lock);
}

void end_write(void) {
   pthread_mutex_unlock(&rw_lock);
}

// Remove a user from all rooms, all direct connections, and the user list.
void remove_user(int client) {
   start_write();
   removeUserFromAllRooms(rooms, client);
   dms = removeAllDMs(dms, client);
   head = removeU(head, client);
   end_write();
}

// Send a short reply to one client.
void reply(int client, const char *msg) {
   send(client, msg, strlen(msg), 0);
}

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;  // socket
   free(ptr);                  // main() allocated this copy of the socket for us
  
   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  //data buffer of 2K  
   char tmpbuf[MAXBUFF];  //data temp buffer of 1K  
   char cmd[MAXBUFF], username[20];
   char *arguments[80];
   char msg[MAXBUFF];
   struct room *room;
   struct node *target;

   struct node *currentUser;
    
   send(client  , server_MOTD , strlen(server_MOTD) , 0 ); // Send Welcome Message of the Day.

   // Creating the guest user name
  
   sprintf(username,"guest%d", client);
   start_write();
   head = insertFirstU(head, client , username);
   
   // Add the GUEST to the DEFAULT ROOM (i.e. Lobby)
   roomAddUser(findRoom(rooms, DEFAULT_ROOM), client);
   end_write();

   while (1) {
       
      received = read(client , buffer, MAXBUFF - 1);
      if (received <= 0) {             // client closed the connection (or an error)
            remove_user(client);
            close(client);
            break;
      }
      else {
      
            buffer[received] = '\0'; 
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);
         
            /////////////////////////////////////////////////////
            // we got some data from a client

            // 1. Tokenize the input in buf (split it on whitespace)

            // get the first token 

             arguments[0] = strtok(cmd, delimiters);

            // walk through other tokens 

             i = 0;
             while( arguments[i] != NULL ) {
                arguments[++i] = strtok(NULL, delimiters); 
                strcpy(arguments[i-1], trimwhitespace(arguments[i-1]));
             } 

             // Arg[0] = command
             // Arg[1] = user or room

             // blank line: just show the prompt again
             if (arguments[0] == NULL || arguments[0][0] == '\0') {
                reply(client, "chat>");
                continue;
             }

             // commands that need an argument (room or user name)
             if ((strcmp(arguments[0], "create") == 0 || strcmp(arguments[0], "join") == 0 ||
                  strcmp(arguments[0], "leave") == 0 || strcmp(arguments[0], "connect") == 0 ||
                  strcmp(arguments[0], "disconnect") == 0 || strcmp(arguments[0], "login") == 0) &&
                 (arguments[1] == NULL || arguments[1][0] == '\0')) {
                snprintf(msg, sizeof(msg), "Usage: %s <name>\nchat>", arguments[0]);
                reply(client, msg);
                continue;
             }
             
             /////////////////////////////////////////////////////
             // 2. Execute command: TODO


            if(strcmp(arguments[0], "create") == 0)
            {
               printf("create room: %s\n", arguments[1]); 
              
               // perform the operations to create room arg[1]
               start_write();
               if (findRoom(rooms, arguments[1]) != NULL)
                  snprintf(msg, sizeof(msg), "Room %s already exists.", arguments[1]);
               else {
                  rooms = insertRoom(rooms, arguments[1]);
                  snprintf(msg, sizeof(msg), "Room %s created.", arguments[1]);
               }
               end_write();
               reply(client, msg);
              
               sprintf(buffer, "\nchat>");
               send(client , buffer , strlen(buffer) , 0 ); // send back to client
            }
            else if (strcmp(arguments[0], "join") == 0)
            {
               printf("join room: %s\n", arguments[1]);  

               // perform the operations to join room arg[1]
               start_write();
               room = findRoom(rooms, arguments[1]);
               if (room == NULL)
                  snprintf(msg, sizeof(msg), "Room %s does not exist.", arguments[1]);
               else if (!roomAddUser(room, client))
                  snprintf(msg, sizeof(msg), "You are already in room %s.", arguments[1]);
               else
                  snprintf(msg, sizeof(msg), "Joined room %s.", arguments[1]);
               end_write();
               reply(client, msg);
              
               sprintf(buffer, "\nchat>");
               send(client , buffer , strlen(buffer) , 0 ); // send back to client
            }
            else if (strcmp(arguments[0], "leave") == 0)
            {
               printf("leave room: %s\n", arguments[1]); 

               // perform the operations to leave room arg[1]
               start_write();
               room = findRoom(rooms, arguments[1]);
               if (room == NULL)
                  snprintf(msg, sizeof(msg), "Room %s does not exist.", arguments[1]);
               else if (!roomRemoveUser(room, client))
                  snprintf(msg, sizeof(msg), "You are not in room %s.", arguments[1]);
               else
                  snprintf(msg, sizeof(msg), "Left room %s.", arguments[1]);
               end_write();
               reply(client, msg);

               sprintf(buffer, "\nchat>");
               send(client , buffer , strlen(buffer) , 0 ); // send back to client
            } 
            else if (strcmp(arguments[0], "connect") == 0)
            {
               printf("connect to user: %s \n", arguments[1]);

               // perform the operations to connect user with socket = client from arg[1]
               start_write();
               target = findU(head, arguments[1]);
               if (target == NULL)
                  snprintf(msg, sizeof(msg), "User %s not found.", arguments[1]);
               else if (target->socket == client)
                  snprintf(msg, sizeof(msg), "You cannot connect to yourself.");
               else if (findDM(dms, client, target->socket) != NULL)
                  snprintf(msg, sizeof(msg), "Already connected to %s.", arguments[1]);
               else {
                  dms = insertDM(dms, client, target->socket);
                  snprintf(msg, sizeof(msg), "Connected to %s.", arguments[1]);
               }
               end_write();
               reply(client, msg);

               sprintf(buffer, "\nchat>");
               send(client , buffer , strlen(buffer) , 0 ); // send back to client
            }
            else if (strcmp(arguments[0], "disconnect") == 0)
            {             
               printf("disconnect from user: %s\n", arguments[1]);
               
               // perform the operations to disconnect user with socket = client from arg[1]
               start_write();
               target = findU(head, arguments[1]);
               if (target == NULL)
                  snprintf(msg, sizeof(msg), "User %s not found.", arguments[1]);
               else if (findDM(dms, client, target->socket) == NULL)
                  snprintf(msg, sizeof(msg), "You are not connected to %s.", arguments[1]);
               else {
                  dms = removeDM(dms, client, target->socket);
                  snprintf(msg, sizeof(msg), "Disconnected from %s.", arguments[1]);
               }
               end_write();
               reply(client, msg);
                
               sprintf(buffer, "\nchat>");
               send(client , buffer , strlen(buffer) , 0 ); // send back to client
            }                  
            else if (strcmp(arguments[0], "rooms") == 0)
            {
                printf("List all the rooms\n");
              
                // must add put list of users into buffer to send to client
                start_read();
                strcpy(buffer, "Rooms:\n");
                for (room = rooms; room != NULL; room = room->next) {
                   int count = 0;
                   for (struct roomUser *m = room->members; m != NULL; m = m->next)
                      count++;
                   snprintf(msg, sizeof(msg), "  %s (%d user%s)%s\n", room->name, count,
                            count == 1 ? "" : "s", roomHasUser(room, client) ? " *joined*" : "");
                   if (strlen(buffer) + strlen(msg) < MAXBUFF - 10)
                      strcat(buffer, msg);
                }
                end_read();
       
              
                strcat(buffer, "\nchat>");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client                            
            }   
            else if (strcmp(arguments[0], "users") == 0)
            {
                printf("List all the users\n");
              
                // must add put list of users into buffer to send to client
                start_read();
                strcpy(buffer, "Users:\n");
                for (target = head; target != NULL; target = target->next) {
                   snprintf(msg, sizeof(msg), "  %s%s%s\n", target->username,
                            target->socket == client ? " (you)" : "",
                            findDM(dms, client, target->socket) ? " [DM]" : "");
                   if (strlen(buffer) + strlen(msg) < MAXBUFF - 10)
                      strcat(buffer, msg);
                }
                end_read();
                
                strcat(buffer, "\nchat>");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client
            }                           
            else if (strcmp(arguments[0], "login") == 0)
            {
                
                //rename their guestID to username. Make sure any room or DMs have the updated username.
                // (rooms and DMs store the socket, so they pick up the new name automatically)
                start_write();
                target = findU(head, arguments[1]);
                currentUser = findUBySocket(head, client);
                if (target != NULL && target->socket != client)
                   snprintf(msg, sizeof(msg), "Username %s is already taken.", arguments[1]);
                else if (currentUser != NULL) {
                   strncpy(currentUser->username, arguments[1], sizeof(currentUser->username) - 1);
                   currentUser->username[sizeof(currentUser->username) - 1] = '\0';
                   snprintf(msg, sizeof(msg), "Logged in as %s.", currentUser->username);
                }
                else
                   snprintf(msg, sizeof(msg), "Login failed.");
                end_write();
                reply(client, msg);
                
                sprintf(buffer, "\nchat>");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client
            } 
            else if (strcmp(arguments[0], "help") == 0 )
            {
                sprintf(buffer, "login <username> - \"login with username\" \ncreate <room> - \"create a room\" \njoin <room> - \"join a room\" \nleave <room> - \"leave a room\" \nusers - \"list all users\" \nrooms -  \"list all rooms\" \nconnect <user> - \"connect to user\" \nexit - \"exit chat\" \n");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client 
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0)
            {
    
                //Remove the initiating user from all rooms and direct connections, then close the socket descriptor.
                reply(client, "Goodbye!\n");
                remove_user(client);
                close(client);
                break;
            }                         
            else { 
                 /////////////////////////////////////////////////////////////
                 // 3. sending a message
           
                 // send a message in the following format followed by the promt chat> to the appropriate receipients based on rooms, DMs
                 // ::[userfrom]> <message>
              
                 start_read();
                 struct node *fromUser = findUBySocket(head, client);
                 snprintf(tmpbuf, sizeof(tmpbuf), "\n::%s> %s\nchat>",
                          fromUser ? fromUser->username : username, trimwhitespace(sbuffer));
                 strcpy(sbuffer, tmpbuf);
                       
                 currentUser = head;
                 while(currentUser != NULL) {
                     
                     // dont send to yourself; only send to users who share a room or a DM with the sender
                     if(client != currentUser->socket &&
                        (usersShareRoom(rooms, client, currentUser->socket) ||
                         findDM(dms, client, currentUser->socket) != NULL)){
                       
                         send(currentUser->socket , sbuffer , strlen(sbuffer) , 0 ); 
                     }
                     currentUser = currentUser->next;
                 }
                 end_read();

                 reply(client, "chat>");   // prompt the sender again
          
            }
 
         memset(buffer, 0, sizeof(1024));
      }
   }
   return NULL;
}