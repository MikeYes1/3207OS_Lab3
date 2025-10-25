#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MENU 1
#define ORDER 2
#define ORDER_CMPL 3

struct msgbuf{
    long mtype;
    char mtext[64]; //message data
};

key_t key = ftok("\mnt\c\Users\Aweso\SchoolArchive\3207OS\exe\key.txt", 'S');
  if(key_t == -1){
        perror("file open failed");
        exit(1);
  }
  
int qid = msgget(key, IPC_CREAT | 0666);
  if(qid == -1){
        perror("IPC open failed");
        exit(1);
  }

//log layter

void waiting(){
    
    int min = 2;
    int max = 6;
    
    srand(time(NULL)); 
    int r = min + rand() % (max - min + 1); 
    sleep(r);
    
}

void waiter(int customers, int orders){

    struct msgbuf msgp;
    
    for(int i = 0; i < (customers * orders); i++){
        msgp.mtype = MENU;
        strcpy(msgp.mtext, "This is message 1");
        msgsnd(qid, msgp, 256, NULL);              //sends out the menu to the IPC
        printf("The waiter is about to pick up the meal...");
        waiting();
        
        msgrcv(qid, msgp, 256, ORDER_CMPL, NULL); //what flag if any
        printf("The waiter is cleaning the table..."); //what do i do with the meal... why do i recieve it?
        waiting();
    }
    
    
    /*
    Waiter, this is an actual process, loops aswell
Send a message with the menu ID 
... (they don't say anything but shouldn't this send an order?)
Then, (organically) wait to recieve a message with the order-complete ID
Wait for some time to simulate cleaning table (idk how this fits in w customer loop resetting)
Send another message with a menu ID
Reset loop until no customers(?)
    */
}

void customer(int orders){
    //fork
    
    srand(time(NULL)); 
    int r = 0 + rand() % (999 - 0 + 1); 
    
    struct msgbuf msgp;
    
    for(int i = 0; i < orders; i++){
        
        srand(time(NULL)); 
        int t = 0 + rand() % (999 - 0 + 1); 
        
        printf("Customer table %d, order %d is waiting for a menu...", r, t);
        waiting(); //actually blocking is all that's needed
        msgrcv(qid, msgp, 256, MENU, NULL);

        printf("Customer table %d, order %d is deciding what to get...", r, t);
        waiting();
        msgp.mtype = ORDER;
        strcpy(msgp.mtext, "This is message 2");
        msgsnd(qid, msgp, 256, NULL); //what's the deal with PID bro
        
        printf("Customer table %d, order %d is either eating or waiting...", r, t);
        waiting(); //should it recieve the food? ...

    }
    
    /*
Customers run in a loop that 
waits for a menu, message with menu ID
once menu got, wait for a couple seconds to simulate choosing
sends a message with the order ID and the customer's PID (from fork?)
a wait a couple seconds for the waiting for food(?) and also for eating the food
once done, repeats the loop (free table for other customer?)
    */
    
}


void chefs(){
    
    //loop but what's the condition "continue when there's another order"
                    //ig maybe when customers are done send some sort of signal out here to say we're done.
        //ftok for debugging purposes
        //msgrcv(order ID struct)
        //waiting()
        //msgsnd(order-complete ID struct)
    
    

}

/* MAIN CONCERN BULLETIN
    are the loops making me overly worried that this just won't work?
    do i bother with making the orders go to the right customer? if so, will it be easy?
    do we need to regenerate an ftok for each process?
     
    Other concerns
    should the parent be a waiter?
    random numbers
    how to stop everyone
    log
*/


int main(int argc, char **argv)
{
    int orders, customers, chefs;
    
    
    printf("Welcome to Professor Pizza's Palace! The best slices on this side of where we are...\n\n"
    "How many orders will each customer table place? ");
    scanf("%d", &orders);
    printf("How many customer tables to fill? ");
    scanf("%d", &customers);
    printf("Input the number of chefs: ");
    scanf("%d", &chefs);
    
    /*
     * functions take care of the message passing
     * main takes care of the forks 
     */
    
    
    
    //fork now, child is waiter and you are dad

    //loop of the number of customers. and just call 
    
    
    
    return 0;
}


/*

what are we doing brody?
remeber producer/consumer? 

So we're going to have to have some buffer for write to and read from.

Will we have to acquire the lock by causing either consumer or producer to fall asleep when 
the buffer is empty or full?

How do I factor in multiple? (maybe one process for each job)
 
what will p's or c's do with recieving message passing

and threads ? it says processes.



main, cc customer(s), cc producer(s), ipc system v commands
both are individually P/C
message queue sends long ID associated messages
ID for the type not specific customer?

msgget() create or get a queue ID
msgsnd() to send a message
    int msgsnd(int msqid, const void msgp[.msgsz], size_t msgsz, int msgflg) 
               struct msgbuf { msgp points to this
               long mtype;       message type, must be > 0 
               char mtext[1];     message data 
           }; queue automatically has MSGMNB amount of bytes
            blocks until more space is made available if full unless IPC_NOWAIT is in msgflg
    
     
msgrcv() recieve msg
msgctl() delete or get info about queue
ftok() gets the key token
 
 
Customers run in a loop that 
waits for a menu, message with menu ID
once menu got, wait for a couple seconds to simulate choosing
sends a message with the order ID and the customer's PID (from fork?)
a wait a couple seconds for the waiting for food(?) and also for eating the food
once done, repeats the loop (free table for other customer?)


Chefs also run in a loop that
recieves an order with an order ID
Wait to simulate preparing the meal (does the customer or chef wait here? what if customer waits less?)
send a message with an order-complete ID
finish loop and continue when there's another order (doesn't sleep in the case of nothing?)


Waiter, this is an actual process, loops aswell
Send a message with the menu ID 
... (they don't say anything but shouldn't this send an order?)
Then, (organically) wait to recieve a message with the order-complete ID
Wait for some time to simulate cleaning table (idk how this fits in w customer loop resetting)
Send another message with a menu ID
Reset loop until no customers(?)

  

declare the customers and chefs outside of main
They will loop for the amount of orders we say
and have message sending capabilities inside
 
    all forks will need some dup2 action
fork to make a messager (also ill declare outside)
fork for the chefs and customers(?)
ftok
call function

 

 
 


*/


    //ftok for key (actually global?)
    //msgget? returns id or -1 on fail. pass key_t key and a flag. can create with IPC_CREAT or get ID w no flag.
    //key is IPC_PRIVATE to create... maybe, or not to get
    //id for queue or type of message?