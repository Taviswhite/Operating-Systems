#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
   char username[30];
   int socket;
   struct node *next;
};



/////////////////// USERLIST //////////////////////////

//insert node at the first location
struct node* insertFirstU(struct node *head, int socket, char *username);

//find a node with given username
struct node* findU(struct node *head, char* username);

//find a node with given socket
struct node* findUBySocket(struct node *head, int socket);

//remove the node with given socket, returns the new head
struct node* removeU(struct node *head, int socket);

//free every node in the user list
void freeAllU(struct node *head);


/////////////////// ROOMLIST //////////////////////////

// a member of a room (users are stored by socket, so a login rename
// shows up automatically in every room and DM)
struct roomUser {
   int socket;
   struct roomUser *next;
};

struct room {
   char name[30];
   struct roomUser *members;
   struct room *next;
};

//insert a room at the first location (ignored if it already exists)
struct room* insertRoom(struct room *rooms, char *name);

//find a room with given name
struct room* findRoom(struct room *rooms, char *name);

//add / remove / check a user (socket) in a room
bool roomAddUser(struct room *room, int socket);
bool roomRemoveUser(struct room *room, int socket);
bool roomHasUser(struct room *room, int socket);

//remove a user from every room
void removeUserFromAllRooms(struct room *rooms, int socket);

//true if both users are in at least one common room
bool usersShareRoom(struct room *rooms, int socket1, int socket2);

//free every room and its member list
void freeAllRooms(struct room *rooms);


/////////////////// DIRECT CONNECTIONS (DMs) //////////////////////////

struct dm {
   int user1;
   int user2;
   struct dm *next;
};

//add a DM between two users (ignored if it already exists)
struct dm* insertDM(struct dm *dms, int socket1, int socket2);

//find the DM between two users (either direction)
struct dm* findDM(struct dm *dms, int socket1, int socket2);

//remove the DM between two users, returns the new head
struct dm* removeDM(struct dm *dms, int socket1, int socket2);

//remove every DM that includes this user, returns the new head
struct dm* removeAllDMs(struct dm *dms, int socket);

//free every DM
void freeAllDMs(struct dm *dms);