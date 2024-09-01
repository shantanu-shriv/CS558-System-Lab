#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <list>
#include <mutex>
#include <algorithm>
#include <unistd.h>
#include <chrono>
using namespace std;


mutex signal[10][20];
vector<list<pair<uint32_t,uint32_t>>> requests;
vector<vector<int>> allocation;
vector<vector<uint32_t>> resource_matrix;
vector<vector<int64_t>> stats;
mutex print_l;
chrono::high_resolution_clock::time_point u;

vector<bool> stop(10, false);

void print_stats() {
    int64_t total_waiting_time = 0, total_turnaround_time = 0;
    int n = stats[0].size();
    for(int i = 0; i < n; i++) {
        total_waiting_time += stats[0][i];
        total_turnaround_time += stats[1][i];
    }
    double avg_wt, avg_tt;
    avg_wt = (double)total_waiting_time / n;
    avg_tt = (double)total_turnaround_time / n;
    cout << "Average waiting time: " << avg_wt << " ms \nAverage total time: " << avg_tt << " ms" << endl;
}

void worker(int service_type, int id){
    cout<<"Worker for"<<service_type<<" with priority :"<<id<<endl;
    int curr=-1;
    while(!stop[service_type])
        if(allocation[service_type][id]!=-1) {
            // print_l.lock();
            // cout << "Check:" << allocation[service_type][id] << endl;
            // print_l.unlock();
            signal[service_type][id].lock();
            // if(signal[service_type][id].try_lock()){
                curr = allocation[service_type][id];
                print_l.lock();
                cout<<"Transaction : "<<curr<<" being processed by "<<id<<" of service "<<service_type<<endl;
                print_l.unlock();
                allocation[service_type][id]=-1;
                signal[service_type][id].unlock();
                auto t = chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t - u);
                stats[1][curr] = duration.count();
            // }
            // cout<<"Transaction : "<<curr<<" done"<<endl;
        }
    return;
    
}

void service_manager(int service_type, uint32_t workers){
    cout<<"Service manager for "<<service_type<<endl;
    pair<uint32_t,uint32_t> curr;
    bool flag;
    while(true){
        flag = false;
        if(requests[service_type].empty()){
            cout<<service_type<<" List empty\n";
            stop[service_type]=true;
            return;

        }
        curr = requests[service_type].front();
        for(int i=0;i<workers;i++){     
            if(signal[service_type][i].try_lock()){
                if(allocation[service_type][i]==-1 && resource_matrix[service_type][i]>= curr.second){
                    allocation[service_type][i] = curr.first;
                    print_l.lock();
                    cout << "Service "<<service_type<<" assigned tr "<<curr.first<<" to "<<i<<endl;
                    print_l.unlock();
                    signal[service_type][i].unlock();
                    requests[service_type].pop_front();
                    auto t = chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t - u);
                    stats[0][curr.first] = duration.count();
                    flag = true;
                    break;
                }
                signal[service_type][i].unlock();
            }
        }
        if(flag == false && !requests[service_type].empty()) {
            requests[service_type].pop_front();
            list<pair<uint32_t,uint32_t>>::iterator it = requests[service_type].begin();
            advance(it,1);
            requests[service_type].insert(it, curr);
        }
    }
}


int main(){
    int services = 0, workers = 0, transactions=0;
    cout<<"No of services : ";
    cin>>services;
    cout<<"No of worker threads: ";
    cin>>workers;
    resource_matrix = vector<vector<uint32_t>>(services, vector<uint32_t>(workers));
    
    allocation = vector<vector<int>>(services, vector<int>(workers, -1));
    

    thread thread_manager[services];
    thread matrix[services][workers];
    uint32_t temp=0;

    for(int i=0;i<services;i++){
        cout<<"Provide the priority_level and resources for each worker for service "<<i<<" : <priority_level> <resources>"<<endl;
        for(int j=0;j<workers;j++){
            cin>>temp;
            cin>>resource_matrix[i][temp];
        }
    }
   
    cout<<"No of transactions to be performed :";
    cin>>transactions;
    stats = vector<vector<int64_t>>(2, vector<int64_t>(transactions));
    int s_type, r_req;
    // vector<pair<uint32_t,uint32_t>> requests(transactions, pair<uint32_t,uint32_t>());
    requests = vector<list<pair<uint32_t,uint32_t>>>(transactions);
    cout<<"Provide the transactions : <transaction_type> <resources_required>"<<endl;
    for(int i=0;i<transactions;i++){
        // cin>>requests[i].first>>requests[i].second;
        cin>>s_type>>r_req;
        requests[s_type].push_back({i,r_req});
    }

    for(int i =0;i<services;i++)
    {
        cout<<"----------"<<endl;
        for(auto it : requests[i])
        {
            cout<<it.first<<" "<<it.second<<endl;
        }
    }

    u = chrono::high_resolution_clock::now();
    for(int i=0;i<services;i++){
        for(int j=0;j<workers;j++){
            matrix[i][j] = thread(worker, i, j);
        }
    }


    for(int i=0;i<services;i++){
        thread_manager[i] = thread(service_manager, i, workers);
        
    }
    

    for(int i=0;i<services;i++){
        for(int j=0;j<workers;j++){
            matrix[i][j].join();
        }
    }


    for(int i=0;i<services;i++){
        thread_manager[i].join();

    }

    print_stats();
}

