/*

Author: MANISH SONI
Assignment 6
Description: This application first discovers neighboring nodes and then finds local leader using local m value first as a 
local minima and then it keepslooking for other local minima from every node in the topology to find global minima so that 
using global minima a leader could be elected. 

*/

#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "shell.h"
#include "net/sock/udp.h"
#include "net/ipv6/addr.h"
#include "thread.h"
#include "random.h"

#define SERVER_MSG_QUEUE_SIZE   (32)
#define SERVER_BUFFER_SIZE      (64)
#define PORT_S                  (12913)
#define IPV6_ADDRESS_LEN        (46)
#define MAX_NEIGHBORS           (20)
#define MAX_IPC_MESSAGE_SIZE    (256)
#define MAIN_QUEUE_SIZE     (8)
#define K_VAL                  (5)
#define T1_VAL                 (5000292)
#define T2_VAL                  (2000292)

static bool server_running = false;
static sock_udp_t sock;
static char server_buffer[SERVER_BUFFER_SIZE];
static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];




char myipv6[IPV6_ADDRESS_LEN] = { 0 };
int neighborsVal[MAX_NEIGHBORS] = { 0 };
int neighborsLocalVal[MAX_NEIGHBORS] = { 0 };
int expectedincomingcount=0;
char leader[IPV6_ADDRESS_LEN]={ 0 };
int localmin=256;





// substr uses s point array to put the substring in t array from index a with length b
void substr(char *s, int a, int b, char *t) 
{
    memset(t, 0, b);
    strncpy(t, s+a, b);
}

int alreadyANeighbor(char **neighbors, char *ipv6) {
    for(int i = 0; i < MAX_NEIGHBORS; i++) {
        if(strcmp(neighbors[i], ipv6) == 0) return 1;
    }
    return 0;
}

int getNeighborIndex(char **neighbors, char *ipv6) {
    for(int i = 0; i < MAX_NEIGHBORS; i++) {
        if(strcmp(neighbors[i], ipv6) == 0) return i;
    }
    return -1;
}


int indexOfChar(char *ipv6, char charval) {
    for (uint i = 0; i < strlen(ipv6); i++) {
        if (ipv6[i]  == charval) {
            return i+1; // start of second id
        }
    }
    return -1;
}



int say_hello(void)
{
    
    int res;
    sock_udp_ep_t remote = { .family = AF_INET6 };
    char ipv6[IPV6_ADDRESS_LEN] = { 0 };



    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t *)&remote.addr.ipv6, IPV6_ADDR_MCAST_SCP_LINK_LOCAL);

    if (ipv6_addr_is_link_local((ipv6_addr_t *)&remote.addr)) {
        gnrc_netif_t *netif = gnrc_netif_iter(NULL);
        remote.netif = (uint16_t)netif->pid;
    }
    remote.port =PORT_S ;
    ipv6_addr_to_str(ipv6, (ipv6_addr_t *)&remote.addr.ipv6, IPV6_ADDRESS_LEN);
    if((res = sock_udp_send(NULL, "hi?", strlen("hi?"), &remote)) < 0) {
       
        return -1;
    }
    else {
        
    }
    return 0;
}




int udp_send(char* target_addr, char * msg)
{
    int res;
    sock_udp_ep_t remote = { .family = AF_INET6 };

    
    if (ipv6_addr_from_str((ipv6_addr_t *)&remote.addr, target_addr) == NULL) {
        puts("Error: unable to parse destination address");
        return -1;
    }
    if (ipv6_addr_is_link_local((ipv6_addr_t *)&remote.addr)) {
        /* choose first interface when address is link local */
        gnrc_netif_t *netif = gnrc_netif_iter(NULL);
        remote.netif = (uint16_t)netif->pid;
    }
    remote.port = PORT_S;
    if((res = sock_udp_send(NULL, msg, strlen(msg), &remote)) < 0) {
        puts("could not send");
        return -1;
    }
    else {
        
    }
    return 0;
}




int indexOfplus(char *ipv6) {
    for (uint32_t i = 0; i < strlen(ipv6); i++) {
        if (ipv6[i]  == '+') {
            return i+1; // start of second id
        }
    }
    return -1;
}


void printallneighbors(char** neig, int numcount)
{
    printf("Neighbor list started\n");
    printf("-------------------------------------------------\n");
    for(int i = 0; i < numcount; i++) {
        printf("IPV6 address is: %s\n",neig[i]);
    }
    printf("-------------------------------------------------\n");


}

void printallneighborsVal(char** neig, int numcount, int* valList,int mval)
{

    printf("Neighbor list started\n");
    printf("-------------------------------------------------\n");
    for(int i = 0; i < numcount; i++) {
       // neighbors[i] = (char*)calloc(IPV6_ADDRESS_LEN, sizeof(char));
        printf("IPV6 address is: %s and Value is:%d\n",neig[i],valList[i]);
    }
    printf("-------------------------------------------------\n");
    printf("MyIP :: %s ; m value:%d ; and local min value: %d\n",myipv6,mval,localmin );
    printf("-------------------------------------------------\n");


}


void updaterelativeminval(char **neighbors, char *ipv6, int val)
{
    int indexofn=getNeighborIndex(neighbors,ipv6);

    if(neighborsVal[indexofn]!=val){
        neighborsVal[indexofn]=val;
    }

}

void updaterelativelocalminval(char **neighbors, char *ipv6, int val)
{
    int indexofn=getNeighborIndex(neighbors,ipv6);

    if(neighborsLocalVal[indexofn]!=val){
        neighborsLocalVal[indexofn]=val;
    }

}


int sendleaderdetail(char** neig,int numcount, char *ipv6, uint32_t minval)
{
    int res;
    char localmsg[MAX_IPC_MESSAGE_SIZE] = "leader?"; 
    char Minstr[4] = { 0 };                   
                        if(minval < 10) {
                            sprintf(Minstr, "00%d",minval);
                            } else if (minval < 100) {
                                sprintf(Minstr, "0%d",minval);
                            } else {
                                sprintf(Minstr, "%d",minval);
                            }

    strcat(localmsg,Minstr);
    strcat(localmsg,"+");
    strcat(localmsg,ipv6);

  for(int i = 0; i < numcount; i++) {
         res=udp_send(neig[i],localmsg);
        if(res<0)
        {
            printf("error occured while sending the who has min query\n");
            return -1;
        }
    }

    return 0;
  
}

// return -1 if a<b, 1 if a>b, 0 if a==b
int minIPv6(char *ipv6_a, char *ipv6_b) {
    uint32_t minLength = strlen(ipv6_a);
    if (strlen(ipv6_b) < minLength) minLength = strlen(ipv6_b);
    
    for (uint32_t i = 0; i < minLength; i++) {
        if (ipv6_a[i] < ipv6_b[i]) {
            return -1;
        } else if (ipv6_b[i] < ipv6_a[i]) {
            return 1;
        }
    }
    return 0;
}

int broadcast_msg(char* msg)
{
    //multicast: FF02::1
    int res;
    sock_udp_ep_t remote = { .family = AF_INET6 };
    char ipv6[IPV6_ADDRESS_LEN] = { 0 };

    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t *)&remote.addr.ipv6, IPV6_ADDR_MCAST_SCP_LINK_LOCAL);

    if (ipv6_addr_is_link_local((ipv6_addr_t *)&remote.addr)) {
        /* choose first interface when address is link local */
        gnrc_netif_t *netif = gnrc_netif_iter(NULL);
        remote.netif = (uint16_t)netif->pid;
    }
    remote.port =PORT_S ;
    ipv6_addr_to_str(ipv6, (ipv6_addr_t *)&remote.addr.ipv6, IPV6_ADDRESS_LEN);
    if((res = sock_udp_send(NULL, msg, strlen(msg), &remote)) < 0) {
       // printf("UDP: Error - could not send message \"%s\" to %s\n", argv[2], ipv6);
        return -1;
    }
    else {
        
    }
    return 0;
}

int askmin(char** neig, int numcount, uint32_t mval)
{
   
    int res;
    char localmsg[MAX_IPC_MESSAGE_SIZE] = "whohasmin?"; 
    expectedincomingcount=0;
    char Mvalstr[4] = { 0 };                   
                        if(mval < 10) {
                            sprintf(Mvalstr, "00%d",mval);
                            } else if (mval < 100) {
                                sprintf(Mvalstr, "0%d",mval);
                            } else {
                                sprintf(Mvalstr, "%d",mval);
                            }

    strcat(localmsg,Mvalstr);

  for(int i = 0; i < numcount; i++) {
       
         res=udp_send(neig[i],localmsg);
        if(res<0)
        {
            printf("error occured while sending the who has min query\n");
            return -1;
        }
        //printf("Sent a message to:%s and the msg is:%s\n", neig[i],localmsg);
    }
   // printf("sent askmin message: %s\n",localmsg);

    expectedincomingcount=numcount;

    return 0;
  
}


int main(void)
{
    char ipv6[IPV6_ADDRESS_LEN] = { 0 };
    

    uint32_t mval = (random_uint32() % 254)+1; // my leader election value, range 1 to 255

    // set the flags
    bool nd=true;
    bool le=false;
    char msg[MAX_IPC_MESSAGE_SIZE] = ""; 
    //uint64_t sendercmd = xtimer_now_usec64();
    //uint64_t nd=0;
    uint64_t MAX_ND_SAMPLE_TIME=2000000; // node discovery time
  //  uint64_t MAX_ALIVE_SAMPLE_TIME=5000292;
    uint64_t MAX_LE_SAMPLE_TIME=T2_VAL;        // it is t2 timer here
     uint64_t MAX_T1_SAMPLE_TIME=T1_VAL;


  

// timers here
    xtimer_ticks64_t ndstart;
    xtimer_ticks64_t lestart;
    xtimer_ticks64_t t1start;
 

    uint64_t startTimeLE = 0;
    uint64_t endTimeLE = 0;
    uint64_t convergenceTimeLE;


 
//count value
    int counter=K_VAL;

  

     // array of MAX neighbors
    int numNeighbors = 0;
    char **neighbors = (char**)calloc(MAX_NEIGHBORS, sizeof(char*));
    for(int i = 0; i < MAX_NEIGHBORS; i++) {
        neighbors[i] = (char*)calloc(IPV6_ADDRESS_LEN, sizeof(char));
    }

// socket variables
	sock_udp_ep_t server = { .port = PORT_S, .family = AF_INET6 };
    sock_udp_ep_t remote = { .family = AF_INET6 };

    msg_init_queue(server_msg_queue, SERVER_MSG_QUEUE_SIZE);

     printf("Initializing the server: starting out UDP server on port %u\n", server.port);

    if(sock_udp_create(&sock, &server, NULL, 0) < 0) {
        return -1;
    }

    server_running = true;
    printf("Success: started UDP server on port %u\n", server.port);

   // set timers 
    ndstart = xtimer_now64();
    lestart = xtimer_now64();
    t1start=xtimer_now64();

    // set incomin packet count
    int incomingpacketcount=0;

// only set the nd flag for now

    nd=false;
    le=false;
    
    xtimer_usleep(5000000); // wait 5 seconds to start the application

// send a hello broadcast message
     say_hello();
            
     bool ndproc=true; // ndproc is neighbor discovery process flag
     bool leaderproc=false;   // leaderproc is leader discovery process flag
     bool converge=false;
     int nddiscoverycount=0;   // neighbordiscovery count

    while (1) {

    	
        int res;
        int nd_status;
      
      // initial timer for neighbor discovery

        // main sequence for timers

        if(!converge && xtimer_diff64(xtimer_now64(), ndstart).ticks64 > MAX_ND_SAMPLE_TIME) {
            if(nddiscoverycount<3){
            nd=true;
            }
            else if(nddiscoverycount>2 && nddiscoverycount<6)
            {
                le=true;
            }
            else if(nddiscoverycount==6)
            {
                le=false;
                ndproc=false;
               // leaderproc=true;
                t1start=xtimer_now64();
                startTimeLE = (uint64_t)(xtimer_now_usec64()/1000000); // it starts at this counter 

            }
            nddiscoverycount=nddiscoverycount+1;

           
        }

        // t1 start

         if(!converge && !ndproc && (xtimer_diff64(xtimer_now64(), t1start).ticks64 > MAX_T1_SAMPLE_TIME)) {
            leaderproc=true;
            t1start=xtimer_now64();
            lestart=xtimer_now64();
            le=true;


           
        }

       

        if(leaderproc && (xtimer_diff64(xtimer_now64(), lestart).ticks64 > MAX_LE_SAMPLE_TIME)) {
            
                le=false;
            
        }


        // nd flag for sending the hello broadcast messages
        if(nd)
        {
           
            nd_status=say_hello();
            if(nd_status<0)
                printf("ND:Failed\n");

            nd=false;
            ndstart = xtimer_now64();
           

        }

        if(le)
        {
          
            int tosendval=mval;

            if(leaderproc)
            {
                if(localmin!=256)// default case of local min in that case send mval
                {
    
                    tosendval=localmin;
                }

            }
            nd_status=askmin(neighbors,numNeighbors,tosendval);

           
                le=false;

        }


 if(ndproc){
       

        uint32_t waitflag=SOCK_NO_TIMEOUT;
            waitflag= 1 * US_PER_SEC;

      
        res = sock_udp_recv(&sock, server_buffer,
                                     sizeof(server_buffer) - 1, waitflag,
                                     &remote);
        if (res == -ETIMEDOUT) {
                       
                }
       
            if (res < 0) {
                //puts("Error while receiving");
            }
            else if (res == 0) {
                puts("No data received");
            }
        else {

            server_buffer[res] = '\0';
            ipv6_addr_to_str(ipv6, (ipv6_addr_t *)&remote.addr.ipv6, IPV6_ADDRESS_LEN);
             

                    if(strncmp(server_buffer,"acknowledged+",13)==0)
                    {
                      //  printf("acknowledged started\n");


        				if (numNeighbors < MAX_NEIGHBORS && strncmp(server_buffer, "acknowledged+", 13) == 0) {
                           
                             int delimiterind=indexOfChar(server_buffer,'+');   
                             substr(server_buffer,delimiterind,IPV6_ADDRESS_LEN,myipv6);
                               
        	                if (!alreadyANeighbor(neighbors, ipv6)) {
        	                    strcpy(neighbors[numNeighbors], ipv6); // record their ID
                                printf("-----------------------------------------------------\n");
        	                    printf("Neighbor Discovery: Found new neighbor, %s\n\n", (char*)neighbors[numNeighbors]);
                                printf("-----------------------------------------------------\n");
        	                    numNeighbors++;
      	                  
        	                } else {
        	                   
        	                   }  
                    	}


                    }

                     if(strcmp(server_buffer,"hi?")==0)
                    {
                       


        				if (numNeighbors < MAX_NEIGHBORS && strncmp(server_buffer, "hi?", 3) == 0) {
                          
        	                if (!alreadyANeighbor(neighbors, ipv6)) {
        	                    strcpy(neighbors[numNeighbors], ipv6); // record their ID
        	                   printf("-----------------------------------------------------\n");
                                printf("Neighbor Discovery: Found new neighbor, %s\n\n", (char*)neighbors[numNeighbors]);
                                printf("-----------------------------------------------------\n");
        	                    numNeighbors++;

        	                   
        	                } else {
        	                   
        	                }

        	                strcat(msg,"acknowledged+");
                            strcat(msg,ipv6);
        	                
        	                udp_send(ipv6,msg);
                            memset(msg,0,MAX_IPC_MESSAGE_SIZE);


                    	}
                    }
                


            
                    if(strncmp(server_buffer,"whohasmin?",10)==0)
                    {
                        //printf("check1");
                        int delimiterind=indexOfChar(server_buffer,'?');
                               char remainingstring[4];
                                int remsize=4;
                                substr(server_buffer,delimiterind,remsize,remainingstring);
                               
                                int rcvminval = atoi(remainingstring);
                              
                                updaterelativeminval(neighbors, ipv6, rcvminval);
                                updaterelativelocalminval(neighbors, ipv6, rcvminval); // keep the local value of neighbors same as of their m_vals while discovering neighbors

                                char Mvalstr[4] = { 0 };                   
                             if(mval < 10) {
                            sprintf(Mvalstr, "00%d",mval);
                            } else if (mval < 100) {
                                sprintf(Mvalstr, "0%d",mval);
                            } else {
                                sprintf(Mvalstr, "%d",mval);
                            }

                            
                        strcat(msg,"mymin?");
                        strcat(msg,Mvalstr);

                        udp_send(ipv6,msg);
                       
                        memset(msg,0,MAX_IPC_MESSAGE_SIZE);          
                        
                    }
                    if(strncmp(server_buffer,"mymin?",6)==0)
                    {
                        int delimiterind=indexOfChar(server_buffer,'?');
                        char remainingstring[4];
                        int remsize=4;
                        substr(server_buffer,delimiterind,remsize,remainingstring);
                       
                        int rcvminval = atoi(remainingstring);
                         
                        incomingpacketcount=incomingpacketcount+1; 

                        updaterelativeminval(neighbors, ipv6, rcvminval);
                        updaterelativelocalminval(neighbors, ipv6, rcvminval);
                        if(expectedincomingcount==incomingpacketcount){


                            printallneighborsVal(neighbors,numNeighbors,neighborsVal,mval);
                            incomingpacketcount=0;
                            expectedincomingcount=0;
                        }
                                
                        
                    }

                }
      
         }       

                    
           if(leaderproc)
            {
                uint32_t waitflag=SOCK_NO_TIMEOUT;
                waitflag= 1 * US_PER_SEC;
                res = sock_udp_recv(&sock, server_buffer,
                                             sizeof(server_buffer) - 1,  waitflag,
                                             &remote);
                
                if (res == -ETIMEDOUT) {
                        //puts("Timed out");
                }
                if (res < 0) {
                    //puts("Error while receiving");
                }
                else if (res == 0) {
                    puts("No data received");
                }
                else 
                {

                        server_buffer[res] = '\0';
                        ipv6_addr_to_str(ipv6, (ipv6_addr_t *)&remote.addr.ipv6, IPV6_ADDRESS_LEN);
                        
                           
                            
                            if(strncmp(server_buffer,"whohasmin?",10)==0)
                            {
                               
                                int delimiterind=indexOfChar(server_buffer,'?');
                                       char remainingstring[4];
                                        int remsize=4;
                                        substr(server_buffer,delimiterind,remsize,remainingstring);
                                        int rcvminval = atoi(remainingstring);
                                       
                                        updaterelativelocalminval(neighbors, ipv6, rcvminval);
                                       
                                        char Mvalstr[4] = { 0 }; 
                                        if(localmin==256)
                                        {
                                            localmin=mval;
                                           
                                        }                  
                                     if(localmin < 10) {
                                         sprintf(Mvalstr, "00%d",localmin);
                                         } else if (mval < 100) {
                                        sprintf(Mvalstr, "0%d",localmin);
                                        } else {
                                            sprintf(Mvalstr, "%d",localmin);
                                        }

                                       
                                        strcat(msg,"mymin?");
                                        strcat(msg,Mvalstr);

                                        

                                udp_send(ipv6,msg);
                               
                                memset(msg,0,MAX_IPC_MESSAGE_SIZE);
                               
                                
                            }
                            if(strncmp(server_buffer,"mymin?",6)==0)
                            {
                               

                                int delimiterind=indexOfChar(server_buffer,'?');
                                char remainingstring[4];
                                int remsize=4;

                                incomingpacketcount=incomingpacketcount+1;
                               

                                substr(server_buffer,delimiterind,remsize,remainingstring);
                                int rcvminval = atoi(remainingstring);

                                

                               

                                updaterelativelocalminval(neighbors, ipv6, rcvminval);

                                if(expectedincomingcount==incomingpacketcount)
                                {   //// now select the leader
                                    /// perform the leader election things
                                    // now initiate the leader process as per professor ali's algorithm

                                    
                                   
                                    int i=0;
                                    int temp=256;

                                    
                                    if(localmin==256)
                                    {
                                        localmin=mval;
                                    }
                                    

                                  
                                    int index=0;

                                    // get the min values from neighbors and place it in leader ip detail
                                   
                                   
                                    for(i=0;i<numNeighbors;i++)
                                    {
                                        
                                        if(temp>neighborsLocalVal[i])
                                        {
                                           
                                            temp=neighborsLocalVal[i];
                                            index=i;
                                        }
                                    }

                                     // if any neighbor has a minimum value than set that minimum value to local minimum with the leader ip detail
                                    if(temp<localmin)
                                    {

                                        localmin=temp;  // got the min val
                                    strcpy(leader,neighbors[index]); // got the leader
                                    counter=K_VAL;

                                    }
                                    else if((temp==localmin )&&counter>0)
                                    {
                                        counter--;
                                        char* neigborip=neighbors[index];
                                        int minres=minIPv6(neigborip, leader);
                                        if(minres<0)
                                        {
                                                strcpy(leader,neigborip);
                                        }
                                        else
                                        {
                                            strcpy(leader,myipv6);

                                        }

                                    }

                                    
                                    sendleaderdetail(neighbors, numNeighbors, leader,  localmin);

                                    if(counter==0)
                                    {
                                        printallneighborsVal(neighbors,numNeighbors,neighborsLocalVal,mval);
                                        printf("Lets set the Global leader found true\n");
                                         printf("Data got converged\n");

                                         printf("-----------------------------------------\n");
                                         printf("Globalmin=%d\n",localmin );
                                        printf("leader=%s\n",leader);
                                        printf("------------------------------------------\n");
                                            leaderproc=false;
                                            broadcast_msg("ndover");
                                            converge=true;
                                            endTimeLE = (uint64_t)(xtimer_now_usec64()/1000000);
                                            convergenceTimeLE = endTimeLE - startTimeLE;
                                            printf("Leader Election: leader election took %" PRIu64 " seconds to converge\n", convergenceTimeLE);

                                    }

                                   
                                    expectedincomingcount=0;
                                    incomingpacketcount=0;
                                    leaderproc=false;


                                }
                                
                            }
                            

                            
                            if(strncmp(server_buffer,"leader?",7)==0)
                            {
                                int delimiterind=indexOfChar(server_buffer,'?');
                                char remainingstring[4];
                                int remsize=4;
                                substr(server_buffer,delimiterind,remsize,remainingstring);
                                int rcvminval = atoi(remainingstring);
                                if(rcvminval<localmin )
                                {
                                    localmin=rcvminval;
                                    delimiterind=indexOfChar(server_buffer,'+');
                                    substr(server_buffer,delimiterind,IPV6_ADDRESS_LEN,leader);
                                }
                                else if(rcvminval==localmin)
                                {

                                    char rcvip[IPV6_ADDRESS_LEN] = { 0 };
                                    delimiterind=indexOfChar(server_buffer,'+');
                                    substr(server_buffer,delimiterind,IPV6_ADDRESS_LEN,rcvip);
                                    int minres=minIPv6(rcvip, leader);
                                        if(minres<0)
                                        {
                                                strcpy(leader,rcvip);
                                        }
                                        else
                                        {
                                            strcpy(leader,myipv6);

                                        }

                                }
                                
                            }
                            if(strcmp(server_buffer,"ndover")==0)
                            {
                                printf("Recieved the convergence command\n");
                                broadcast_msg("ndover");
                                leaderproc=false;
                                converge=true;        
                                printallneighborsVal(neighbors,numNeighbors,neighborsLocalVal,mval);
                                printf("Lets set the Global leader found true\n");
                                         printf("Data got converged\n");

                                         printf("-----------------------------------------\n");
                                         printf("Globalmin=%d\n",localmin );
                                        printf("leader=%s\n",leader);
                                        printf("------------------------------------------\n");
                                        endTimeLE = (uint64_t)(xtimer_now_usec64()/1000000);
                                convergenceTimeLE = endTimeLE - startTimeLE;
                                printf("LE: leader election took %" PRIu64 " seconds to converge\n", convergenceTimeLE);
                                
                            }

                }
         }




        if(converge)
        {
            ndproc=false;
            leaderproc=false;
            // this condition stops both the processes nd and le and keeps the while loop going so that port should not get busy if we exit the application  


        }


    }




    return 0;
}
