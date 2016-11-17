/*  main.c  - main */

#include <xinu.h>

pid32 subProc1;
pid32 subProc2;
pid32 subProc3;
pid32 pubProc1;
pid32 pubProc2;
pid32 subProc3;
pid32 pubProc4;
pid32 unsubProc;
pid32 broker_id;
/* These two variables are required for implementing pending Queue */
uint32 front  = -1;
uint32 rear = -1;

topic16 getGroup(topic16 topic) {
    return topic/256;
}

topic16 getTopicId(topic16 topic) {
    return topic%256;
}

syscall subscribe(topic16 topic, void(*function_handler)(topic16 topic, uint32 data)) {
    topic16 topicId = getTopicId(topic);
    /* If topicId is less than 0 or greater than TOPIC_TABLE_SIZE then return SYSERROR */
    if(topicId < 0 || topicId >= TOPIC_TABLE_SIZE) {
        return SYSERR;
    }
    /* If the process has already subscribed to a topicId then it cannot do it again */
    if(topicTab[topicId][currpid - 1].group_id != -1) {
        return SYSERR;
    }

    /* Put the function handler and group id in topicTab[][] table */
    topicTab[topicId][currpid - 1].function_handler = function_handler;
    topicTab[topicId][currpid - 1].group_id = (int32)getGroup(topic);

    return OK;
}

syscall publish(topic16 topic, uint32 data) {
    /* Enqueues the topic in the pendingQueue[] Queue*/
    enqueueProc(topic, data);
    return OK;
}

syscall unsubscribe(topic16 topic) {
    topic16 topicId = getTopicId(topic);
    /* Updating group Id as 1 in topicTab[] for the corresponding process unsubscribes the process */
    topicTab[topicId][currpid - 1].group_id = -1;
    return OK;
}

/* Broker process is always running */
process broker() {
    sleep(1);
    /* We will store the topic Id and group Id in the following
    two variables from the given topic taken from pendingQueue queue */
    topic16 topicId;
    topic16 group;
    uint32 i;
    /* This variable checks if any processes have subscribed for any given topic */
    bool8 flag;
    while(1) {
        flag = TRUE;
        /* If queue is not empty means there is some event that is not published */
        if(!isQueueEmpty()) {
            /* Take the data out of the pendingQueue[] and store them in the following two variables */
            topic16 topic = pendingQueue[front].topic;
            uint32 data = pendingQueue[front].data;
            topicId = getTopicId(topic);
            group = getGroup(topic);
            /* If some event needs to be published with group zero,
             * that means it should be published  to all the processes wh have subscribed to this topicId */
            if(group == 0) {
                for(i = 0; i < ARRAY_SIZE; i++) {
                    if(topicTab[topicId][i].group_id != -1) {
                        /* Call the function handler of the subscriber process */
                        topicTab[topicId][i].function_handler(topic, data);
                        /* Send signal to the subscriber process,
                        so that if it can unsubscribe if it has matched it's requirement */
                        send(i+1, 0);
                        /* flag False means some processes were subscribed to this topic Id */
                        flag = FALSE;
                    }
                }
            }
            else {
                for(i = 0; i < ARRAY_SIZE; i++) {
                    /* Match the group Id as group is not equal to 0 before calling the handler function */
                    if(topicTab[topicId][i].group_id == (int32)group) {
                        /* Call the function handler of the subscriber process */
                        topicTab[topicId][i].function_handler(topic, data);
                        /* Send signal to the subscriber process,
                        so that if it can unsubscribe if it has matched it's requirement */
                        send(i+1, 0);
                        /* flag False means some processes were subscribed to this topic Id */
                        flag = FALSE;
                    }
                }
            }
            /* If the code has come here means the one entry from the pendingQueue[] has been processed */
            dequeueProc();
            /*If flag is still true means no function was subscribed to the processed topicId*/
            if(flag) {
                sleep(1);
                printf("No functions are called \n");
            }
        }
    }
    return OK;
}

/* Subscriber process 1 */
process A1(void(*function_handler)(topic16, uint32), char* procName, char* funcName, int32 topic) {
     printf("Process %s subscribes to topic %d with handler %s\n", procName, topic, funcName);
     subscribe(topic, function_handler);
     return OK;
}

/* Subscriber process 2, which also ties to send a subscribe
request to a topicId which have been already been subscribed but different groupId */
process A2(void(*function_handler)(topic16, uint32), char* procName, char* funcName, topic16 topic) {
    printf("Process %s subscribes to topic %d with handler %s \n", procName, topic, funcName);
    subscribe(topic, function_handler);
    if(subscribe(319, function_handler) == SYSERR) {
        printf("Process with Process Id %d could not subscribe to topic %d \n", currpid, 319);
    }
    return OK;
}

/* Subscriber process 3, which handles the case of unsubscribing a topic */
process U1(void(*function_handler)(topic16, uint32), char* procName, char* funcName, topic16 topic, uint32 num) {
    printf("Process %s subscribes to topic %d with handler %s \n", procName, topic, funcName);
    subscribe(topic, function_handler);
    int32 count;
    for(count = 1; count <= num; count++) {
        receive();
    }
    unsubscribe(topic);
    sleep(1);
    printf("Unsubscribed process with processId %d and topic Id %d \n", topic, currpid);
    return OK;
}

/* Publish process 1 */
process P1(char* funcName, topic16 topic, uint32 data) {
    sleep(1);
    printf("Process %s publishes %d to topic %d\n", funcName, data, topic);
    publish(topic, data);
    return OK;
}

/* Publish process 2 */
process P2(char* funcName, topic16 topic, uint32 data) {
    sleep(1);
    printf("Process %s publishes %d to topic %d\n", funcName, data, topic);
    publish(topic, data);
    return OK;
}

/* Publish process 3 */
process P4(char* funcName, topic16 topic, uint32 data) {
    sleep(1);
    printf("Process %s publishes %d to topic %d\n", funcName, data, topic);
    publish(topic, data);
    return OK;
}

/* Function handler 1 */
void foo(topic16 topic, uint32 data) {
    sleep(1);
    printf("Function %s() is called with topic %d and data %d \n", __func__, topic, data);
}

/* Function handler 2 */
void bar(topic16 topic, uint32 data) {
    sleep(1);
    printf("Function %s() is called with topic %d and data %d \n", __func__, topic, data);
}

/* Function handler 3 */
void foo2(topic16 topic, uint32 data) {
    sleep(1);
    printf("Function %s() is called with topic %d and data %d \n", __func__, topic, data);
}

process	main(void) {

    topic16 topic1 = 319;
    topic16 topic2 = 575;
    topic16 topic3 = 63;
    uint32 data1 = 77;
    uint32 data2 = 88;

    subProc1 = create(A1, 4096, 50, "A1", 4, foo, "A1", "foo", topic1);
    subProc2 = create(A2, 4096, 50, "A2", 4, bar, "A2", "bar", topic2);
    unsubProc = create(U1, 4096, 50, "U1", 5, foo2, "U1", "foo2", topic1, 2);
    pubProc1 = create(P1, 4096, 50, "P1", 3, "P1", topic1, data1);
    pubProc2 = create(P2, 4096, 50, "P2", 3, "P2", topic3, data2);
    pubProc4 = create(P4, 4096, 50, "P4", 3, "P4", 989, 66);

    broker_id = create(broker, 4096, 50, "broker", 0);

    resume(subProc1);
    resume(subProc2);
    resume(unsubProc);
    resume(pubProc1);
    resume(pubProc2);
    resume(pubProc4);
    resume(broker_id);


	return OK;
}