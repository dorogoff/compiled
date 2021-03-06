#include <pspkernel.h> 
#include <pspdebug.h> 
#include <pspsdk.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <psputility_netmodules.h> 
#include <psputility_netparam.h> 
#include <pspwlan.h> 
#include <pspnet.h> 
#include <pspnet_apctl.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#define printf pspDebugScreenPrintf 
#define PORT 5555

int create_socket(void);

PSP_MODULE_INFO("test", 0, 1, 0); 

/* Exit callback */ 
int exit_callback(int arg1, int arg2, void *common) 
{ 
   sceKernelExitGame(); 
   return 0; 
} 

/* Callback thread */ 
int CallbackThread(SceSize args, void *argp) 
{ 
   int cbid; 

   cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL); 
   sceKernelRegisterExitCallback(cbid); 
   sceKernelSleepThreadCB(); 

   return 0; 
} 

/* Sets up the callback thread and returns its thread id */ 
int SetupCallbacks(void) 
{ 
   int thid = 0; 

   thid = sceKernelCreateThread("update_thread", CallbackThread, 
                 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0); 
   if(thid >= 0) 
   { 
      sceKernelStartThread(thid, 0, 0); 
   } 

   return thid; 
} 

int connect_to_apctl(int config) { 
  int err; 
  int stateLast = -1; 
  //int tmp = sceWlanGetSwitchState();

 // if (tmp != 1) 
  //  pspDebugScreenClear(); 

 // while (sceWlanGetSwitchState() != 1) { 
   // pspDebugScreenSetXY(0, 0); 
   // printf("Please enable WLAN to continue.\n"); 
  //  sceKernelDelayThread(1000 * 1000); 
 // } 

  err = sceNetApctlConnect(config); 
  if (err != 0) { 
    printf("sceNetApctlConnect returns %08X\n", err); 
    return 0; 
  } 

  printf("Connecting...\n"); 
  while (1) { 
    int state; 
    err = sceNetApctlGetState(&state); 
    if (err != 0) { 
      printf("sceNetApctlGetState returns $%x\n", err); 
      break; 
    } 
    if (state != stateLast) { 
      printf("  Connection state %d of 4.\n", state); 
      stateLast = state; 
    } 
    if (state == 4) { 
      break; 
    } 
    sceKernelDelayThread(50 * 1000); 
  } 
  printf("Connected!\n"); 
  sceKernelDelayThread(3000 * 1000); 

  if (err != 0) { 
    return 0; 
  } 

  return 1; 
} 

char *getconfname(int confnum) { 
  static char confname[128]; 
  sceUtilityGetNetParam(confnum, PSP_NETPARAM_NAME, (netData *)confname); 
  return confname; 
} 

int net_thread(SceSize args, void *argp) 
{ 
  int selComponent = 2; 
  
  printf("Using connection %d (%s) to connect...\n", selComponent, getconfname(selComponent)); 

  if (connect_to_apctl(selComponent)) 
  { 
    char tmp[32]; 
	SceNetApctlInfo szMyIPAddr[32];

    if (sceNetApctlGetInfo(8, szMyIPAddr) != 0) {
		strcpy(tmp, "unknown IP address"); 
	}

	printf("IP: %s\n", szMyIPAddr); 
	// connect to server trhuth socket
	int n = create_socket();
	printf("\nSocket conenction: %d status", n);
    sceKernelSleepThread(); 
  } 
  return 0; 
} 

int InitialiseNetwork(void) 
{ 
  int err; 

  printf("load network modules..."); 
  err = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON); 
  if (err != 0) 
  { 
    printf("Error, could not load PSP_NET_MODULE_COMMON %08X\n", err); 
    return 1; 
  } 
  err = sceUtilityLoadNetModule(PSP_NET_MODULE_INET); 
  if (err != 0) 
  { 
    printf("Error, could not load PSP_NET_MODULE_INET %08X\n", err); 
    return 1; 
  } 
  printf("done\n"); 

  err = pspSdkInetInit(); 
  if (err != 0) 
  { 
    printf("Error, could not initialise the network %08X\n", err); 
    return 1; 
  } 
  return 0; 
} 

int create_socket()
{

	int sock;
            struct sockaddr_in echoserver;
            char buffer[1024];
            unsigned int echolen;
            int received = 0;


            /* Create the TCP socket */
            if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
              printf("Failed to create socket");
            }

			/* Construct the server sockaddr_in structure */
            memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
            echoserver.sin_family = AF_INET;                  /* Internet/IP */
            echoserver.sin_addr.s_addr = inet_addr("192.168.1.4");  /* IP address */
            echoserver.sin_port = htons(5555);       /* server port */
            /* Establish connection */
            if (connect(sock,                (struct sockaddr *) &echoserver,
                        sizeof(echoserver)) < 0) {
              printf("Failed to connect with server");
            } else {
				printf("\nSocket created");
			}

			// send message and receive response from server
			char msg[10];
			strcpy(msg, "from psp");

            if (send(sock, "hello from psp", 10, 0)==0) {
              printf("Mismatch in number of sent bytes");
            } else {
				printf("Message sended");
			}

            /* Receive the word back from the server */
            printf("\nReceived: ");
			printf("\nMessage from serer: %s" , buffer);



	 return 0;

}



/* Simple thread */ 
int main(int argc, char **argv) 
{ 
   SceUID thid; 

   SetupCallbacks(); 

   pspDebugScreenInit(); 

  if (InitialiseNetwork() != 0) 
  { 
    sceKernelSleepThread(); 
  } 

  thid = sceKernelCreateThread("net_thread", net_thread, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL);

  if (thid < 0) { 
    printf("Error! Thread could not be created!\n"); 
    sceKernelSleepThread(); 
  } 

  

  sceKernelStartThread(thid, 0, NULL); 

  sceKernelExitDeleteThread(0); 

   return 0; 
} 