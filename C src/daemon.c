#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <arpa/inet.h>


#define MAX_DTA_SIZE 16  //defining maximum size to prevent memory overflow
#define SOCKET_PATH "/tmp/daemon.sock" //creating socket file path
#define MAX_ID_SIZE 64
#define MAX_BLOCKS 100
#define PERMISSION_READ 0x1
#define PERMISSION_WRITE 0x2

typedef struct {  //structuring the management of data as a data unit everytime it is called
   char id[MAX_ID_SIZE];
   uint8_t secret [16];  //16 byte password to protect it
   uint32_t length_data;
   void*data;//pointer to where data is stored
   uint8_t permission;
} Block;

Block *blocks[MAX_BLOCKS];
int blocks_count = 0;

//store data function

void storeblock(const char id[], const uint8_t secret[], const uint32_t length_data, const void* data, uint8_t permission) {
   //error handling for data management
   if (blocks_count >= MAX_BLOCKS) {
      syslog(LOG_ERR, "Too many blocks!");
      return;
   }

   Block *new_b = (Block *)malloc(sizeof(Block));  //allocating memory to the
   if (!new_b) {
      syslog(LOG_ERR, "Out of memory!");
      return;
   }
   memcpy(new_b->id, id, MAX_ID_SIZE);
   memcpy(new_b->secret, secret, 16);
   new_b->length_data = length_data;
   new_b->data = malloc(length_data);
   new_b->permission = permission;
   if (!new_b->data) {
      free(new_b);
      syslog(LOG_ERR, "Out of memory!");
      return;
   }
   memcpy(new_b->data, data, length_data);
   blocks[blocks_count++] = new_b;
   syslog(LOG_INFO, "Created new block %s", new_b->id);

}


void retrieveBlock(int client_fd) {
   char id[MAX_ID_SIZE];
   uint8_t secret [16];

   if (read(client_fd, id, sizeof(id)) != sizeof(id)) return;
   if (read(client_fd, secret , 16) != 16) return;

   for (int i = 0; i < blocks_count; i++) {
      if (strncmp(blocks[i]->id, id, MAX_ID_SIZE) == 0&&
         memcmp(blocks[i]->secret, secret, 16) == 0) {
         if (!(blocks[i]->permission &PERMISSION_READ)){
            syslog(LOG_WARNING, "Access denied for block ID:  %s",id);
            return;
         }
         uint32_t data_length = blocks[i]->length_data;
         if (write(client_fd, &data_length, sizeof(data_length))!= sizeof(data_length)) {
            syslog(LOG_ERR, "failed to send data length");
            return;
         }
         write(client_fd, &blocks[i]->data,sizeof(blocks[i]->data));
         write(client_fd, blocks[i]->data, blocks[i]->length_data);
         syslog(LOG_INFO, "Read block ID: %s", blocks[i]->id);
         return;
         }
   }
   syslog(LOG_WARNING, "Could not read block ID: %s", id);
}


//handle process request functions
void requests(int client_fd){
   char command;
   if (read(client_fd, &command, 1 )!=1) return;

   if (command == 'S') {
      char id[MAX_ID_SIZE];
      uint8_t secret[16];
      uint32_t length_data;
      char data[MAX_DTA_SIZE];
      if(read(client_fd, &id, sizeof(id)) != sizeof(id)) return;
      if(read(client_fd, secret, 16) != 16) return;
      //read data length
      if(read(client_fd, &length_data, sizeof(length_data)) != sizeof(length_data)) return;
      if(length_data > MAX_DTA_SIZE) return;

      if(read(client_fd, data, sizeof(data)) != sizeof(data)) return;

      storeblock(id, secret, length_data, data, PERMISSION_READ | PERMISSION_WRITE);

   }else if (command == 'G') {
      retrieveBlock(client_fd);
   }else {
      syslog(LOG_ERR, "Invalid command");
   }
}

int main(void){

   openlog("daemon", LOG_PID |LOG_CONS, LOG_DAEMON);
   syslog(LOG_INFO, "Starting daemon");

   pid_t pid = fork();
   if (pid < 0) {
      exit(EXIT_SUCCESS);
   }
   if (pid > 0) {
      exit(EXIT_SUCCESS);
   }
   //resetting the file mode
   umask(0);
   //create new session
   pid_t sid = setsid();
   if (sid < 0) {
      exit(EXIT_SUCCESS);
         }
   //Next, make the current directory
   if((chdir("/") < 0)) {
      syslog(LOG_ERR, "%s\n", "chdir");
      exit(EXIT_FAILURE);
         }
   //make directory
   if((chdir("/")< 0)) {
      syslog(LOG_ERR, "%s\n", "chdir");
      exit(EXIT_FAILURE);
         }
   /* Close stdin, etc. */
   close(STDIN_FILENO);
   close(STDOUT_FILENO);
   close(STDERR_FILENO);

   //setting up the unix domain socket for IPC communication
   int server_fd, client_fd;
   server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
   struct sockaddr_un addr;
   memset(&addr, 0, sizeof(addr));
   addr.sun_family = AF_UNIX;
   strncpy(addr.sun_path, "/tmp/daemon.sock", sizeof(addr.sun_path)-1);
   unlink(addr.sun_path);

   //binding server error handling
   if(bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      syslog(LOG_ERR, "Binding failed");
      close(server_fd);
      exit(EXIT_FAILURE);
   }

   syslog(LOG_INFO, "Daemon is running at %s", SOCKET_PATH);

   while(1) {
      //used to connect the clients
      int client_fd = accept(server_fd,NULL, NULL);
      if (client_fd < 0) continue;
      requests(client_fd);
      close(client_fd);
   }
   close(server_fd);
   unlink(SOCKET_PATH);
   syslog(LOG_INFO, "Done");
   closelog();
   return 0;
}



