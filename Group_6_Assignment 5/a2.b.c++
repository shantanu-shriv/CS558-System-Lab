#include <iostream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <chrono>
#include <semaphore.h>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <condition_variable>
#define MAX_THREADS 10
#define BRIDGE_CAPACITY 3
#define WAIT_ON_BRIDGE 2
#define VEHICLE_FREQUENCY 1
#define NORTHERN 0
#define SOUTHERN 1

using namespace std;

mutex bridge;
mutex NtS, StN;
sem_t on_bridge;
uint32_t nts=0, stn=0;
mutex w_queue;
mutex p_queue;

queue<pair<int, int>> waiting_list;
queue<pair<int, int>> process_list;

void manage_queue(){
    pair<int,int> curr;

    w_queue.lock();
    p_queue.lock();
    process_list.push( waiting_list.front());
    waiting_list.pop();
    w_queue.unlock();
    p_queue.unlock();

    p_queue.lock();
    curr = process_list.front();
    if(curr.first==NORTHERN ){
        while(stn!=0);
    }
    if(curr.first==SOUTHERN ){
        while(nts!=0);
    }
    process_list.pop();
    p_queue.unlock();

    if(curr.first==NORTHERN){
        sem_wait(&on_bridge);
        NtS.lock();
        if(nts==0){
            bridge.lock();
        }
        nts++;
        cout<<"Person "<<curr.second<<" going from North to South\n";
        NtS.unlock();

        sleep(WAIT_ON_BRIDGE);

        NtS.lock();
        
        nts--;
        cout<<"Person "<<curr.second<<" reached South \n";
        if(nts==0){
            bridge.unlock();
        }
        NtS.unlock();
        sem_post(&on_bridge);
    }
        
    else if(curr.first==SOUTHERN){
        sem_wait(&on_bridge);
        StN.lock();
        if(stn==0){
            bridge.lock();
        }
        stn++;
        cout<<"Person "<<curr.second<<" going from South to North\n";
        StN.unlock();
        
        sleep(WAIT_ON_BRIDGE);

        StN.lock();
        
        stn--;
        cout<<"Person "<<curr.second<<" reached North\n";
        if(stn==0){
            bridge.unlock();
        }
        StN.unlock();
        sem_post(&on_bridge);
    }
}

void north_bound(int id){
    int delay_time = rand() % VEHICLE_FREQUENCY ;
    sleep(delay_time);
    w_queue.lock();
    waiting_list.push({NORTHERN, id});
    w_queue.unlock();
    manage_queue();
    
}

void south_bound(int id){
    int delay_time = rand() % VEHICLE_FREQUENCY ;
    sleep(delay_time);
    w_queue.lock();
    waiting_list.push({SOUTHERN, id});
    w_queue.unlock();
    manage_queue();
    
}

int main()
{
    sem_init(&on_bridge,0,BRIDGE_CAPACITY);
    
    // Create threads for northbound people
    thread northbound_threads[MAX_THREADS];
    thread southbound_threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        northbound_threads[i] = thread(north_bound, i+101);
        southbound_threads[i] = thread(south_bound, i+201);
    }

    // Wait for all threads to finish
    for (int i = 0; i < MAX_THREADS; i++) {
        northbound_threads[i].join();
        southbound_threads[i].join();
    }
}
