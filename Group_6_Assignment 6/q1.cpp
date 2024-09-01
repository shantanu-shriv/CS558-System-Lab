#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>
#include <utility>
#include <map>
#include <queue>
#include <set>

using namespace std;

#define ll long long
#define INF 0x3f3f3f3f
int V;
vector<int> idstore;

class Router
{
    public:
        int id;
        vector<Router *> neighbors;
        map<int, int> routing_table;

		// Constructor
        Router()
        {
            this->id = V;
            V++;
        }
		
		// Add router to network
        void add_neighbor(Router *neighbor)
        {
            neighbors.push_back(neighbor);
        }
		
		// Print network
        void print_neighbor()
        {
            for (auto j : neighbors)
                cout << j->id << endl;
        }

		// Updating routing tables using Dijkstra's Algorithm
        void update_routing_table()
        {
            priority_queue<pair<int, Router *>, vector<pair<int, Router *>>, greater<pair<int, Router *>>>
                pq;
            map<int, int> parentmap;
            vector<int> dist(V, INF);
            pq.push(make_pair(0, this));
            dist[this->id] = 0;
            while (!pq.empty())
            {
                Router *u = pq.top().second;
                pq.pop();
                for (auto j : u->neighbors)
                {
                    Router *v = j;
                    if (dist[v->id] > dist[u->id] + 1)
                    {
                        dist[v->id] = dist[u->id] + 1;
                        parentmap[j->id] = u->id;
                        pq.push(make_pair(dist[v->id], v));
                    }
                }
            }

            for (int i = 0; i < V; i++)
            {
                if (idstore[i] != this->id)
                {
                    int destination = idstore[i];
                    while (parentmap[destination] != this->id)
                    {
                        destination = parentmap[destination];
                    }
                    routing_table[idstore[i]] = destination;
                }
            }
        }
		
		// Printing final updated routing tables
        void print_routing_table()
        {
            cout<<endl;
            cout << "Routing table node " << this->id << " : " << endl;
            cout<<"\t|";
            cout << "    Destination"<< "    |    Next Hop    |" << endl;
            for (int i = 0; i < V; i++)
            {

                if (idstore[i] != this->id)
                {
                    
                    cout << "\t|        " << idstore[i] << "          |        " << routing_table[idstore[i]] <<"       |"<< endl;
                }
            }

        }
};

int main()
{
    V = 0;
    int nod = 7;
    
    Router router[nod];

	// Initialize vector to store router IDs
    for(int i=0;i<V;i++){
        idstore.push_back(i);
    }

	// Adding neighbor relationships
    router[2].add_neighbor(&router[1]); // Router 2 is connected to Router 1
    router[1].add_neighbor(&router[2]); // Router 1 is connected to Router 2

    router[1].add_neighbor(&router[3]); // Router 1 is connected to Router 3
    router[3].add_neighbor(&router[1]); // Router 3 is connected to Router 1

    router[3].add_neighbor(&router[2]); // Router 3 is connected to Router 2
    router[2].add_neighbor(&router[3]); // Router 2 is connected to Router 3

    router[3].add_neighbor(&router[4]); // Router 3 is connected to Router 4
    router[4].add_neighbor(&router[3]); // Router 4 is connected to Router 3

    router[3].add_neighbor(&router[0]); // Router 3 is connected to Router 0
    router[0].add_neighbor(&router[3]); // Router 0 is connected to Router 3

    router[2].add_neighbor(&router[4]); // Router 2 is connected to Router 4
    router[4].add_neighbor(&router[2]); // Router 4 is connected to Router 2

    router[4].add_neighbor(&router[5]); // Router 4 is connected to Router 5
    router[5].add_neighbor(&router[4]); // Router 5 is connected to Router 4

    router[5].add_neighbor(&router[6]); // Router 5 is connected to Router 6
    router[6].add_neighbor(&router[5]); // Router 6 is connected to Router 5

/*
       [0]
        |
       [3]
      / | \
    [1] | [2]
      \ | /
       [4]
      /   \
    [5]   [6]

*/

    for(int i=0;i<V;i++){
        router[i].update_routing_table();
        router[i].print_routing_table();
    }

    int startingnode, endingnode;
	cout<<"\n";
    cout << "Enter valid starting node id : ";
    cin >> startingnode;
    cout << endl;

    cout << "Enter valid destination node id : ";
    cin >> endingnode;
    cout << endl;

    if(startingnode<0 || startingnode>=nod || endingnode<0 || endingnode>=nod)
    {
        cout<<"Incorrect node id";
        return 0;
    }

	// Simulate source to destination path
    cout << "Source to destination path simulation : ";



    int currnode = startingnode;
    while (currnode != endingnode)
    {
        Router *curr_router = &router[currnode];
        cout << curr_router->id << " --> ";
        currnode = curr_router->routing_table[endingnode];
    }
    cout << endingnode << endl;

    return 0;
}
