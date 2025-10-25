#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "helpers.h"
#include <sys/wait.h>

#define MENU 1
#define ORDER 2
#define ORDER_CMPL 3
#define CLOSE 4

struct msgbuf{
    long mtype;
    char mtext[256]; //message data
};


void logSeconds(){ //A re-usable function that measures the time
    
    struct timeval timep;
    
    gettimeofday(&timep, NULL);
    printf("\nSeconds: %ld Microseconds: %ld ",(long)timep.tv_sec, (long)timep.tv_usec);
    
}

int waiting(int min, int max){ //A re-usable function to wait for a random, specified amount of time.
    
    return min + rand() % (max - min + 1); 
    
}

void waiter(int customers, int orders, int qid){

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
            msgsnd(qid, &msgp, 256, 0);              //sends out the menu to the IPC
            sleep(waiting(5, 5));
            
            logSeconds();
            printf("\nThe waiter is about to pick up the meal...\n");
            sleep(waiting(2, 6));
        
            msgrcv(qid, &msgp, 256, ORDER_CMPL, 0); //no flag, so we block until message is recieved
            logSeconds();
            printf("\nThe waiter is cleaning the table...\n");
            sleep(waiting(2, 6));
        }
    
        free(array);
        free(line);
    
     
}

void customer(int orders, int qid){
    
    struct msgbuf msgp;
    
    for(int i = 0; i < orders; i++){
        
            pid_t pid = getpid();
            logSeconds();
            printf("\nCustomer %d is waiting for a menu...\n", (int)pid);
            sleep(waiting(2, 4)); //actually blocking is all that's needed here but sleeping makes it predictable
            msgrcv(qid, &msgp, 256, MENU, 0);


            //these 3 lines concatenate a menu item and "customer PID"
            size_t needed = strlen(msgp.mtext) + 48;
            char *result = malloc(needed);
            snprintf(result, needed, "%s from customer %d", msgp.mtext, (int)pid); //ChatGPT made this line


            logSeconds();
            printf("\nCustomer %d is eyeing '%s'...\n", (int)pid, msgp.mtext);
            sleep(waiting(2,6));
            msgp.mtype = ORDER;
            strcpy(msgp.mtext, result); 
            msgsnd(qid, &msgp, 256, 0); 
        
            logSeconds();
            printf("\nCustomer %d is either eating or waiting...\n", (int)pid);
            sleep(waiting(2, 6)); 
            
            free(result);

    }

    
}


void chef(int qid){
    
        int marker = 67;
        struct msgbuf msgp;
    
        while(marker == 67){ 
            
            msgrcv(qid, &msgp, 256, ORDER, IPC_NOWAIT); 
            
            if(msgp.mtype == ORDER){
                pid_t pid = getpid();
                logSeconds();
                printf("\nChef %d is preparing: %s ...\n", (int)pid, msgp.mtext);
                sleep(waiting(2, 6));

                msgp.mtype = ORDER_CMPL;
                msgsnd(qid, &msgp, 256, 0);
                    //perror("chef order not sent");
                
                logSeconds();
                printf("\nChef %d completed: %s ...\n", (int)pid, msgp.mtext);
            }
            
            msgrcv(qid, &msgp, 256, CLOSE, IPC_NOWAIT); //This segment checks for an exit signal in the IPC
            //printf("hi mom");
            if(msgp.mtype == CLOSE){
                marker = 0;
            }
            
        }

    //Skip parent's wait(NULL) to achieve concurrency

}




int main(int argc, char **argv)
{
    int orders, customers, chefs, status;
    struct msgbuf msgp;
    
    
    key_t key = ftok("/mnt/c/Users/Aweso/SchoolArchive/3207OS/exe/key.txt", 'S');
    if(key == -1){
        perror("open file failed");
        exit(1);
    }
  
    int qid = msgget(key, IPC_CREAT | 0666);
    if(qid == -1){
        perror("IPC open failed");
        exit(1);
    }
    
    
    printf("Welcome to Professa Pizza's Palace! The best slices on this side of where we are...\n\n"
    "How many orders will each customer table place? ");
    scanf("%d", &orders);
    printf("How many customer tables to fill? ");
    scanf("%d", &customers);
    printf("Input the number of chefs: ");
    scanf("%d", &chefs);
    
    
    pid_t pid = fork();
    
    if(pid == 0){
        pid = fork(); 
        
        srand(time(NULL)); 
        
        if(pid == 0){
            waiter(customers, orders, qid);
            //This dies after main dies surprisingly, you'll see if you print a statement here
            exit(1);
        } else if (pid < 0){
            perror("fork failed");
        } else {
            for(int i = 0; i < customers; i++){
                pid = fork();
                if(pid == 0){
                    customer(orders, qid); 
                } else if(pid < 0){
                    perror("fork failed");
                } else {
                    //This process dies immedietly
                    exit(1);
                }
            }
        }
        msgp.mtype = CLOSE;
        for(int i = 0; i < chefs; i++){ //I loop because I want each chef to get an exit message
            msgsnd(qid, &msgp, 256, 0);
        }
        
        puts("\nCustomers leave.");
        exit(1);
    } else if (pid < 0){
        perror("fork failed");
    } else {
        
        for(int i = 0; i < chefs; i++){
            pid = fork();
            if(pid == 0){
                chef(qid);
                //puts("chef die");
                exit(1);
            } else if(pid < 0){
                perror("fork failed");
            } /*else {
                exit(1);
            }*/
        }

        while(wait(&status) > 0) {
            //printf("hello");
        }
        /*
        msgp.mtype = CLOSE;
        for(int i = 0; i < chefs; i++){ //I loop because I want each chef to get an exit message
            msgsnd(qid, &msgp, 256, 0);
        }*/
    }

    sleep(1);
    printf("\n\nCongratulations! You made it to the end!\n");
    
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