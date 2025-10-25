#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "helpers.h"

#define MENU 1
#define ORDER 2
#define ORDER_CMPL 3

struct msgbuf{
    long mtype;
    char mtext[64]; //message data
};

struct timeval{
    time_t tv_sec;
    suseconds_t tv_usec;
};

key_t key = ftok("\mnt\c\Users\Aweso\SchoolArchive\3207OS\exe\key.txt", 'S');
  if(key_t == -1){
        perror("open file failed");
        exit(1);
  }
  
int qid = msgget(key, IPC_CREAT | 0666);
  if(qid == -1){
        perror("IPC open failed");
        exit(1);
  }

//log layter

void logSeconds(){
    
    struct timeval timep;
    
    gettimeofday(&timep, NULL);
    printf("Seconds: %ld Microseconds: %ld ",(long)timep.tv_sec, (long)timep.tv_usec);
    
}

int waiting(int min, int max){
    
    srand(time(NULL)); 
    int r = min + rand() % (max - min + 1); 
    return r;
    
}

void waiter(int customers, int orders){

    int element;
    struct msgbuf msgp;
    
    //Send a random food item to the customer
    char _line[1000] = "Pepino's Signature Pie|TyeDye Surprise|That's Amore Brunch|Pizza Ball|7-Up\n";
    char *line = strdup(_line);
    char **array = parse(line ,"|");
    
    for(int i = 0; i < (customers * orders); i++){
        
        element = waiting(0, 4);
        
        msgp.mtype = MENU;
        strcpy(msgp.mtext, array[element]);
        msgsnd(qid, msgp, 256, NULL);              //sends out the menu to the IPC
        logSeconds();
        printf("\nThe waiter is about to pick up the meal...");
        sleep(waiting(2, 6));
        
        msgrcv(qid, msgp, 256, ORDER_CMPL, NULL); //no flag, so we block until message is recieved
        logSeconds();
        printf("\nThe waiter is cleaning the table...");
        sleep(waiting(2, 6));
    }
    
    free(array);
    free(line);
    
    
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
    
    struct msgbuf msgp;
    pid_t pid = getpid();
    
    for(int i = 0; i < orders; i++){
        
        logSeconds();
        printf("\nCustomer %d is waiting for a menu...", (int)pid);
        sleep(waiting(2, 6)); //actually blocking is all that's needed
        msgrcv(qid, msgp, 256, MENU, NULL);

        //next 3 lines are to concatenate a menu item and "customer PID"
        size_t needed = strlen(msgp.mtext + 48);
        char *result = malloc(needed);
        snprintf(result, needed, "%s from customer %d", msgp.mtext, (int)pid);

        logSeconds();
        printf("\nCustomer %d is eyeing '%s'...", (int)pid, msgp.mtext);
        sleep(waiting(2,6));
        msgp.mtype = ORDER;
        strcpy(msgp.mtext, result); //this PID and then 
        msgsnd(qid, msgp, 256, NULL); //what's the deal with PID bro
        
        logSeconds();
        printf("\nCustomer %d is either eating or waiting...", (int)pid);
        sleep(waiting(2, 6)); //should it recieve the food? ...
        
        free(result);

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


void chef(){
    //fork
    
    struct msgbuf msgp;
    pid_t pid = getpid();
    
    //loop but what's the condition "continue when there's another order"
                    //ig maybe when customers are done send some sort of signal out here to say we're done.

    
    while(1){ //find way to terminate loop
        
        msgrcv(qid, msgp, 256, ORDER, NULL);
        logSeconds();
        printf("\nChef %d is preparing: %s ...", (int)pid, msgp.mtext);
        sleep(waiting(2, 6));
        
        msgp.mtype = ORDER_CMPL;
        msgsnd(qid, msgp, 256, NULL);
        logSeconds();
        printf("\nChef %d completed: %s ...", (int)pid, msgp.mtext);
    }
    

}

/* MAIN CONCERN BULLETIN
    fork?
    are the loops making me overly worried that this just won't work?
    how do i log the time of the event? gettimeofday()
    "order content msg from customer PID" menu will be long string parsed by '\n' and random food items?
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