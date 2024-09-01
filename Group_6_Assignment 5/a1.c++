#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <cstdlib>

using namespace std;

bool front_door_open = false, restaurant_full = false, service_running = false, service_complete = false, back_door_open = false, all_diners_left = false;
int diner_count = 0, diners_inside = 0, N, total_diners;
mutex front_door_open_mutex, diner_count_mutex, restaurant_full_mutex, service_running_mutex, service_complete_mutex, back_door_open_mutex, all_diners_left_mutex, diners_inside_mutex, print_mutex;
condition_variable front_door_open_cv, restaurant_full_cv, service_running_cv, service_complete_cv, back_door_open_cv, all_diners_left_cv;


// thread to simulate diner
void diner_thread(int id) {
    while(true) {
        unique_lock<mutex> lock(front_door_open_mutex);
        print_mutex.lock();
        cout << "Diner " << id << " is waiting outside restaurant" << endl;
        print_mutex.unlock();
        // diner waits outside till front door of restaurant is opened
        front_door_open_cv.wait(lock, [](){return front_door_open;});
        print_mutex.lock();
        cout << "Diner " << id << " entered restaurant" << endl;
        print_mutex.unlock();
        diner_count_mutex.lock();
        diners_inside_mutex.lock();

        // increment diner count by 1 after entering the restaurant
        diner_count++;
        diners_inside++;

        // close front door if diner count equals max capacity, N
        if(diner_count == N) {
            print_mutex.lock();
            cout << "Closing front door" << endl;
            print_mutex.unlock();
            front_door_open = false;
            restaurant_full_mutex.lock();
            restaurant_full = true;
            restaurant_full_mutex.unlock();

            // signal restaurant thread to start service
            restaurant_full_cv.notify_one();
        }
        diner_count_mutex.unlock();
        diners_inside_mutex.unlock();
        lock.unlock();

        // wait for  service to start
        print_mutex.lock();
        cout << "Diner " << id << " is waiting to be served" << endl;
        print_mutex.unlock();
        unique_lock<mutex> lock2(service_running_mutex);
        service_running_cv.wait(lock2,[](){return service_running;});

        // eat for random time, 1 - 3 sec
        print_mutex.lock();
        cout << "Diner " << id << " started eating" << endl;
        print_mutex.unlock();
        sleep(rand() % 3);
        print_mutex.lock();
        cout << "Diner " << id << " finished eating" << endl;
        print_mutex.unlock();

        // reduce diner count after eating is complete
        diner_count_mutex.lock();
        diner_count--;

        // all diners finished eating
        if(diner_count == 0) {
            service_complete_mutex.lock();
            service_complete = true;
            service_complete_mutex.unlock();

            // notify restaurant thread on completion of service of all diners
            service_complete_cv.notify_one();
        }
        diner_count_mutex.unlock();
        lock2.unlock();

        // wait for back door to open
        unique_lock<mutex> lock3(back_door_open_mutex);
        back_door_open_cv.wait(lock3,[](){return back_door_open;});
        print_mutex.lock();
        cout << "Diner " << id << " left the restaurant" << endl;
        print_mutex.unlock();
        diners_inside_mutex.lock();

        // leave the restaurant
        diners_inside--;

        // signal to restaurant thread all diners left
        if(diners_inside == 0) {
            all_diners_left_mutex.lock();
            all_diners_left = true;
            all_diners_left_mutex.unlock();
            all_diners_left_cv.notify_one();
        }
        diners_inside_mutex.unlock();
    }
}

void restaurant_thread() {
    while(true) {
        unique_lock<mutex> lock(restaurant_full_mutex);
        print_mutex.lock();
        cout << "Opening front door" << endl; 
        print_mutex.unlock();
        // open front door
        front_door_open_mutex.lock();
        front_door_open = true;
        front_door_open_mutex.unlock();
        front_door_open_cv.notify_all();

        // wait for restaurant to fill up
        restaurant_full_cv.wait(lock, [](){return restaurant_full;});
        print_mutex.lock();
        cout << "Restaurant is full, starting service" << endl;
        print_mutex.unlock();
        service_running_mutex.lock();
        service_running = true;
        service_running_mutex.unlock();

        // signal to diners when service starts
        service_running_cv.notify_all();
        unique_lock<mutex> lock2(service_complete_mutex);

        // wait for all diners to finish eating
        service_complete_cv.wait(lock2, [](){ return service_complete; });
        print_mutex.lock();
        cout << "All customers serviced" << endl;
        print_mutex.unlock();

        // stop the service when all diners have finished
        service_running_mutex.lock();
        service_running = false;
        service_running_mutex.unlock();

        // open back door
        print_mutex.lock();
        cout << "Opening back door" << endl;
        print_mutex.unlock();
        back_door_open_mutex.lock();
        back_door_open = true;
        back_door_open_mutex.unlock();

        // notify all diners when back door opens
        back_door_open_cv.notify_all();
        unique_lock<mutex> lock3(all_diners_left_mutex);

        // wait for all diners to leave
        all_diners_left_cv.wait(lock3,[](){return all_diners_left;});
        print_mutex.lock();
        cout << "All customers left" << endl;
        print_mutex.unlock();

        // reset all condition variables for next batch of diners
        all_diners_left = false;
        service_complete = false;
        restaurant_full = false;

        // close back door
        back_door_open_mutex.lock();
        back_door_open = false;
        back_door_open_mutex.unlock();
        print_mutex.lock();
        cout << "Backdoor closed" << endl;
        print_mutex.unlock();
        
        lock.unlock();
        lock2.unlock();
        lock3.unlock();
    }
}

int main() {
    cout << "Enter max capacity: ";
    cin >> N;
    cout << "Enter total number of diners: ";
    cin >> total_diners;
    thread diners[total_diners];
    for (int i = 0; i < total_diners; i++)
        diners[i] = thread (diner_thread, i+1);
    thread restaurant(restaurant_thread);
    restaurant.join();
    for (int i = 1; i <= total_diners; i++)
        diners[i].join();
    return 0;
}
