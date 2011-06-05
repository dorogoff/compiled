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
#include <pspctrl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <fstream>

#define printf pspDebugScreenPrintf 
#define PORT 5555

int memAddr=0x0000;	// 2 bytes

void timer();

int create_socket(void);
int flagNo;	// номер того что изменяем
char comm; // команд
char data; // что отправляем
int flagCommand;

#define writeRam 0x55
#define writeMem 0x33
#define readRam 0xAA
#define readMem 0xCC
#define runCom 0x6A

PSP_MODULE_INFO("test", 0, 1, 0); 

/* Exit callback */ 
int exit_callback(int arg1, int arg2, void *common) 
{ 
   sceKernelExitGame(); 
   return 0; 
} 

void changeAddr(int comm){
	if (comm==1) {
		if (memAddr!=0xFFFF){
			memAddr++;
			timer();
		} 
	} else {
		if (memAddr!=0x0000){
			memAddr--;
			timer();
		} 
	}
}

void changeComm(int tmp) {	
	if (flagCommand==4) {
		flagCommand=0;
	} else {
		flagCommand++;
	}
	char arr[5];
	arr[0]=writeRam;
	arr[1]=writeMem;
	arr[2]=readRam;
	arr[3]=readMem;
	arr[4]=runCom;
	comm=arr[flagCommand];
}

void changeWrValue(int comm) {
	if (comm==1) {
		data++;
	} else {
		data--;
	}
}
// требуется для плавного инкремента и декремента значений
void timer() {
	int z=0;
	while(z<1500000) {
		z++;
	}
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
	//int n = create_socket();
	//printf("\nSocket conenction: %d status", n);
    //sceKernelSleepThread(); 
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
			char buf[1];
			char send_byte[]={27, '\0'};
			char snd[1];

			char recv_byte[1];

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

			// init
            if (send(sock, "\x27", 1, 0)==0) {
              printf("Mismatch in number of sent bytes");
            } else {
				printf("Message sended");
			}

            /* Receive the word back from the server */
			// it must be  #80h
			recv(sock,buf,1,0);
            printf("\nReceived: ");
			printf("%X", buf);
			
			// main loop
			int hiAddr, loAddr, comm;
			hiAddr=0x00;
			loAddr=0x00;
			int j=0;
				sprintf(snd, "%X",comm);
			//while(j<128) {
				
				// send command
				send(sock, send_byte, 1,0);
				// send hi address
				sprintf(snd, "%X",hiAddr);
				send(sock, send_byte, 1,0);
				//send lo address
				sprintf(snd, "%X",loAddr);
				send(sock, send_byte, 1,0);
				//receive byte
				recv(sock, recv_byte,1,0);
				//print this received byte on screen
				printf("\nRCV: %X", recv_byte);
				// increment lo address
				loAddr++;
				j++;
			//}

			//printf("\nMessage from serer: %s" , buffer);

				/*

				ofstream out;
				out.("file.type");
				if (in==NULL) return 0;
				in<<"this string was wrote to file by c++;)"<<endl;

				*/
				close(sock);
	 return 0;

}

int pad_thread(SceSize args, void *argp){

	printf("\nPad_thread into");
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	SceCtrlData pad;

	while (1)
	{
		sceCtrlReadBufferPositive(&pad, 1);
		
		if (pad.Buttons != 0){
			
			// обработка флага, определяем что будем инкрементировать
			// выбор функции
			if (pad.Buttons & PSP_CTRL_LTRIGGER) {
				if (flagNo==3) {
					flagNo=0;
				} else {
					flagNo++;
				}
				timer();
				
			}
			// отправка данных по сети
			if (pad.Buttons & PSP_CTRL_CROSS) {
				printf("\nCross button pressed");
				create_socket();
				printf("\nSocket must be closed already");
			}
			// инкремент
			if (pad.Buttons & PSP_CTRL_UP) {
				pspDebugScreenClear();
				printf("flagNo %d \n", flagNo);
				printf("Memory address:");
				printf("%X \n", memAddr);

				printf("Command:");
				printf("%X \n", comm);

				printf("Data:");
				printf("%X \n", data);

				printf("Responce:");
				//printf("%X \n", recv);

				switch(flagNo) {
					case 0: changeAddr(1);break;
					case 1: changeComm(1);break;
					case 2: changeWrValue(1);break;
					default: changeAddr(1);break;
				}

				timer();
			}

			// декремент
			if (pad.Buttons & PSP_CTRL_DOWN) {
				pspDebugScreenClear();
				printf("flagNo %d \n", flagNo);

				printf("Memory address:");
				printf("%X \n", memAddr);

				printf("Command:");
				printf("%X \n", comm);

				printf("Data:");
				printf("%X \n", data);

				printf("Responce:");
				//printf("%X \n", recv);

				switch(flagNo) {
					case 0: changeAddr(0);break;
					case 1: changeComm(0);break;
					case 2: changeWrValue(0);break;
					default: changeAddr(0);break;
				}
				timer();
			}
		}
	}
	return 0;
}


/* Simple thread */ 
int main(int argc, char **argv) 
{ 
   SceUID thid, th_pad; 

   

   SetupCallbacks(); 

   pspDebugScreenInit(); 
   sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  if (InitialiseNetwork() != 0) 
  { 
    sceKernelSleepThread(); 
  } 

  thid = sceKernelCreateThread("net_thread", net_thread, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL);

  if (thid < 0) { 
    printf("Error! Thread could not be created!\n"); 
    sceKernelSleepThread(); 
  } else {
	  printf("\nNet Thread created\n");
  }

  // create socket and send bytes by triangle

  th_pad = sceKernelCreateThread("pad_thread", pad_thread, 0x19, 0x10000, PSP_THREAD_ATTR_USER, NULL);
  if (th_pad < 0) { 
	  printf("Error! Thread listen pad not created\n"); 
	  sceKernelSleepThread(); 
  } else {
	  printf("\nPad Thread created \n");
	  
  }


  

  

  sceKernelStartThread(thid, 0, NULL); 
  sceKernelStartThread(th_pad, 0, 0);

  sceKernelExitDeleteThread(0); 

   return 0; 
} 

