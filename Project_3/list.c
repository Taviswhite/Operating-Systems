#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

//insert link at the first location
struct node* insertFirstU(struct node *head, int socket, char *username) {
   if(findU(head,username) == NULL) {
       //create a link
       struct node *link = (struct node*) malloc(sizeof(struct node));

       link->socket = socket;
       strcpy(link->username,username);
       //point it to old first node
       link->next = head;

       //point first to new first node
       head = link;
   }
   else
       printf("Duplicate: %s\n", username);
   return head;
}

//find a link with given user
struct node* findU(struct node *head, char* username) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
    while(strcmp(current->username, username) != 0) {
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
   //if username found, return the current Link
   return current;
}

//find a link with given socket
struct node* findUBySocket(struct node *head, int socket) {
   struct node *current = head;
   while(current != NULL) {
      if(current->socket == socket)
         return current;
      current = current->next;
   }
   return NULL;
}

//remove the link with given socket
struct node* removeU(struct node *head, int socket) {
   struct node *current = head, *prev = NULL;
   while(current != NULL) {
      if(current->socket == socket) {
         if(prev == NULL)
            head = current->next;
         else
            prev->next = current->next;
         free(current);
         return head;
      }
      prev = current;
      current = current->next;
   }
   return head;
}

//free every link in the user list
void freeAllU(struct node *head) {
   while(head != NULL) {
      struct node *next = head->next;
      free(head);
      head = next;
   }
}

/////////////////// ROOMLIST //////////////////////////

struct room* insertRoom(struct room *rooms, char *name) {
   if(findRoom(rooms, name) != NULL)
      return rooms;
   struct room *r = (struct room*) malloc(sizeof(struct room));
   strncpy(r->name, name, sizeof(r->name) - 1);
   r->name[sizeof(r->name) - 1] = '\0';
   r->members = NULL;
   r->next = rooms;
   return r;
}

struct room* findRoom(struct room *rooms, char *name) {
   while(rooms != NULL) {
      if(strcmp(rooms->name, name) == 0)
         return rooms;
      rooms = rooms->next;
   }
   return NULL;
}

bool roomHasUser(struct room *room, int socket) {
   struct roomUser *m;
   if(room == NULL)
      return false;
   for(m = room->members; m != NULL; m = m->next)
      if(m->socket == socket)
         return true;
   return false;
}

bool roomAddUser(struct room *room, int socket) {
   if(room == NULL || roomHasUser(room, socket))
      return false;
   struct roomUser *m = (struct roomUser*) malloc(sizeof(struct roomUser));
   m->socket = socket;
   m->next = room->members;
   room->members = m;
   return true;
}

bool roomRemoveUser(struct room *room, int socket) {
   struct roomUser *m, *prev = NULL;
   if(room == NULL)
      return false;
   for(m = room->members; m != NULL; prev = m, m = m->next) {
      if(m->socket == socket) {
         if(prev == NULL)
            room->members = m->next;
         else
            prev->next = m->next;
         free(m);
         return true;
      }
   }
   return false;
}

void removeUserFromAllRooms(struct room *rooms, int socket) {
   for(; rooms != NULL; rooms = rooms->next)
      roomRemoveUser(rooms, socket);
}

bool usersShareRoom(struct room *rooms, int socket1, int socket2) {
   for(; rooms != NULL; rooms = rooms->next)
      if(roomHasUser(rooms, socket1) && roomHasUser(rooms, socket2))
         return true;
   return false;
}

void freeAllRooms(struct room *rooms) {
   while(rooms != NULL) {
      struct room *next = rooms->next;
      struct roomUser *m = rooms->members;
      while(m != NULL) {
         struct roomUser *mnext = m->next;
         free(m);
         m = mnext;
      }
      free(rooms);
      rooms = next;
   }
}

/////////////////// DIRECT CONNECTIONS (DMs) //////////////////////////

struct dm* findDM(struct dm *dms, int socket1, int socket2) {
   for(; dms != NULL; dms = dms->next)
      if((dms->user1 == socket1 && dms->user2 == socket2) ||
         (dms->user1 == socket2 && dms->user2 == socket1))
         return dms;
   return NULL;
}

struct dm* insertDM(struct dm *dms, int socket1, int socket2) {
   if(findDM(dms, socket1, socket2) != NULL)
      return dms;
   struct dm *d = (struct dm*) malloc(sizeof(struct dm));
   d->user1 = socket1;
   d->user2 = socket2;
   d->next = dms;
   return d;
}

struct dm* removeDM(struct dm *dms, int socket1, int socket2) {
   struct dm *current = dms, *prev = NULL;
   while(current != NULL) {
      if((current->user1 == socket1 && current->user2 == socket2) ||
         (current->user1 == socket2 && current->user2 == socket1)) {
         if(prev == NULL)
            dms = current->next;
         else
            prev->next = current->next;
         free(current);
         return dms;
      }
      prev = current;
      current = current->next;
   }
   return dms;
}

struct dm* removeAllDMs(struct dm *dms, int socket) {
   struct dm *current = dms, *prev = NULL;
   while(current != NULL) {
      struct dm *next = current->next;
      if(current->user1 == socket || current->user2 == socket) {
         if(prev == NULL)
            dms = next;
         else
            prev->next = next;
         free(current);
      } else {
         prev = current;
      }
      current = next;
   }
   return dms;
}

void freeAllDMs(struct dm *dms) {
   while(dms != NULL) {
      struct dm *next = dms->next;
      free(dms);
      dms = next;
   }
}