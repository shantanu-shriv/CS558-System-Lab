#include <iostream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <chrono>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <condition_variable>
#define MAX_THREADS 10
#define BRIDGE_CAPACITY 3
#define WAIT_ON_BRIDGE 2
#define VEHICLE_FREQUENCY 1

using namespace std;

mutex bridge,print;
sem_t NtS, StN;
uint32_t nts=0, stn=0;
mutex N_in, S_in;
mutex N_out, S_out;

void north_bound(int id){
    int delay_time = rand() % VEHICLE_FREQUENCY ;
    sleep(delay_time);

    N_in.lock();
    if(nts==0)
        bridge.lock();
    sem_wait(&NtS);
    nts++;
    print.lock();
    cout<<"Person "<<id<<" going from North to South \n";
    print.unlock();
    N_in.unlock();
    
    sleep(WAIT_ON_BRIDGE);
    print.lock();
    cout<<"Person "<<id<<" reached South\n";
    print.unlock();
	
    N_out.lock();
    sem_post(&NtS);
    
    nts--;
    if(nts==0){
        bridge.unlock();
    }
    N_out.unlock();

}

void south_bound(int id){
    int delay_time = rand() % VEHICLE_FREQUENCY ;
    sleep(delay_time);

    S_in.lock();
    if(stn==0)
        bridge.lock();
    sem_wait(&StN);
    stn++;
    print.lock();
    cout<<"Person "<<id<<" going from South to North \n";
    print.unlock();
    S_in.unlock();
    
    sleep(WAIT_ON_BRIDGE);
    print.lock();
    cout<<"Person "<<id<<" reached North\n";
    print.unlock();

    S_out.lock();
    sem_post(&StN);
    stn--;
    if(stn==0){
        bridge.unlock();
    }
    S_out.unlock();
}

int main()
{
    sem_init(&NtS, 0, BRIDGE_CAPACITY);
    sem_init(&StN, 0, BRIDGE_CAPACITY);
    
    cout<<"Persons going from North --> South\n";
    cout<<"----------------------------------\n";
    
    // Create threads for northbound people
    thread northbound_threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        northbound_threads[i] = thread(north_bound, i+101);
    }
    // Wait for all northbound_threads to finish
    for (int i = 0; i < MAX_THREADS; i++) {
        northbound_threads[i].join();
    }
	
    cout<<"\nAll Persons successfully reached South!!\n\n";
    cout<<"Persons going from South --> North\n";
    cout<<"----------------------------------\n";

    // Create threads for southbound people
    thread southbound_threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        southbound_threads[i] = thread(south_bound, i+201);
    }

    // Wait for all southbound_threads to finish
    for (int i = 0; i < MAX_THREADS; i++) {
        southbound_threads[i].join();
    }
    cout<<"\nAll Persons successfully reached North!!\n\n";
}
