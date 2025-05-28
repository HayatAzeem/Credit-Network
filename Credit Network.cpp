#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <windows.h>
#include <ctime>
#include <bits/stdc++.h>
#include <memory>
#include <cctype>

using namespace std;

void LoadingBar();
int timeANDdate();

//location function to place texts in the screen at specific co-orinates
//basically used for placing time and day at the top left of the screen

int location (int x, int y)
{
   COORD sach;
   sach.X=x;
   sach.Y=y;
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),sach);
}


const int INF = 1e9; //very large number or infinity

class DebtGraph {
    unordered_map<string, int> nameToId;   // Maps person name to unique ID
    vector<string> idToName;               // Reverse mapping: ID to name
    vector<vector<int>> capacity;          // Stores debt amounts between people
    vector<vector<int>> adj;               // Adjacency list for graph traversal
    vector<vector<int>> weight;            // Edge weights for Dijkstra's algorithm
    int personCount = 0;                   // Total people added

public:
    // Adds a new person to the graph
    void addPerson(const string& name) {
        if (nameToId.find(name) == nameToId.end()) {
            nameToId[name] = personCount++;
            idToName.push_back(name);
            for (auto& row : capacity) row.push_back(0);
            capacity.push_back(vector<int>(personCount, 0));
            for (auto& row : weight) row.push_back(INF);
            weight.push_back(vector<int>(personCount, INF));
            adj.push_back({});
        }
    }

    // Adds a debt between two people
    void addDebt(const string& from, const string& to, int amount) {
        addPerson(from);
        addPerson(to);
        int u = nameToId[from];
        int v = nameToId[to];
        capacity[u][v] += amount;
        weight[u][v] = min(weight[u][v], amount);
        adj[u].push_back(v);
    }

    // Removes a debt between two people
    void removeDebt(const string& from, const string& to) {
        if (nameToId.find(from) != nameToId.end() && nameToId.find(to) != nameToId.end()) {
            int u = nameToId[from];
            int v = nameToId[to];
            capacity[u][v] = 0;
            weight[u][v] = INF;
            adj[u].erase(remove(adj[u].begin(), adj[u].end(), v), adj[u].end());
            cout << "\n✅ Debt removed successfully.\n";
        } else {
            cout << "\n❌ Invalid names.\n";
        }
    }

    // Displays all current debts
    void displayDebts() {
        cout << "\n=======================\n";
        cout << "    📄 Current Debts\n";
        cout << "=======================\n";
        bool found = false;
        for (int u = 0; u < personCount; ++u) {
            for (int v = 0; v < personCount; ++v) {
                if (capacity[u][v] > 0) {
                    found = true;
                    cout << " - " << idToName[u] << " owes " << idToName[v]
                         << ": $" << capacity[u][v] << "\n";
                }
            }
        }
        if (!found) cout << "No debts recorded.\n";
    }

    // Displays the net balance summary for all individuals
    void displaySummary() {
        vector<int> balance = netBalance();
        cout << "\n==========================\n";
        cout << "    💼 Net Balance Summary\n";
        cout << "==========================\n";
        for (int i = 0; i < personCount; ++i) {
            cout << " - " << idToName[i] << ": ";
            if (balance[i] > 0)
                cout << "+$" << balance[i] << " (Creditor)\n";
            else if (balance[i] < 0)
                cout << "-$" << -balance[i] << " (Debtor)\n";
            else
                cout << "$0 (Settled)\n";
        }
    }

    // Computes the net balance of each person
    vector<int> netBalance() {
        vector<int> balance(personCount, 0);
        for (int u = 0; u < personCount; ++u) {
            for (int v = 0; v < personCount; ++v) {
                balance[u] -= capacity[u][v];
                balance[v] += capacity[u][v];
            }
        }
        return balance;
    }

    // Greedy debt settlement algorithm
    void greedySettle() {
        cout << "\n===========================\n";
        cout << "    ⚖️  Greedy Settlement\n";
        cout << "===========================\n";
        vector<int> balance = netBalance();
        vector<pair<int, int>> creditors, debtors;

        for (int i = 0; i < personCount; ++i) {
            if (balance[i] > 0)
                creditors.push_back({balance[i], i});
            else if (balance[i] < 0)
                debtors.push_back({-balance[i], i});
        }

        sort(creditors.begin(), creditors.end(), greater<>());
        sort(debtors.begin(), debtors.end(), greater<>());

        int i = 0, j = 0;
        while (i < creditors.size() && j < debtors.size()) {
            int pay = min(creditors[i].first, debtors[j].first);
            cout << " - " << idToName[debtors[j].second]
                 << " pays " << idToName[creditors[i].second]
                 << ": $" << pay << "\n";

            creditors[i].first -= pay;
            debtors[j].first -= pay;

            if (creditors[i].first == 0) i++;
            if (debtors[j].first == 0) j++;
        }
    }

    // Breadth-first search for Ford-Fulkerson
    int bfsForFF(const vector<vector<int>>& residual, int s, int t, vector<int>& parent) {
        fill(parent.begin(), parent.end(), -1);
        parent[s] = -2;
        queue<pair<int, int>> q;
        q.push({s, INF});
    
        while (!q.empty()) {
            int u = q.front().first;
            int flow = q.front().second;
            q.pop();
    
            for (int v : adj[u]) {
                if (parent[v] == -1 && residual[u][v] > 0) {
                    parent[v] = u;
                    int new_flow = min(flow, residual[u][v]);
                    if (v == t)
                        return new_flow;
                    q.push({v, new_flow});
                }
            }
        }
        return 0;
    }

    // Ford-Fulkerson max flow algorithm
    int fordFulkerson(const string& source, const string& sink) {
        if (nameToId.find(source) == nameToId.end() || nameToId.find(sink) == nameToId.end()) {
            cout << "❌ Invalid source or sink.\n";
            return 0;
        }
    
        int s = nameToId[source];
        int t = nameToId[sink];
        int flow = 0;
        vector<int> parent(personCount);
    
        vector<vector<int>> residual = capacity;
    
        int new_flow;
        while ((new_flow = bfsForFF(residual, s, t, parent))) {
            flow += new_flow;
            int u = t;
            while (u != s) {
                int p = parent[u];
                residual[p][u] -= new_flow;
                residual[u][p] += new_flow;
                u = p;
            }
        }
    
        return flow;
    }

    // Dijkstra's algorithm to find the shortest path (in terms of debt)
    void dijkstra(const string& srcName, const string& destName) {
        if (nameToId.find(srcName) == nameToId.end() || nameToId.find(destName) == nameToId.end()) {
            cout << "❌ Invalid source or destination name.\n";
            return;
        }
        int src = nameToId[srcName];
        int dest = nameToId[destName];
        vector<int> dist(personCount, INF);
        vector<int> prev(personCount, -1);
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;

        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            pair<int, int> top = pq.top(); pq.pop();
            int d = top.first;
            int u = top.second;
            if (d > dist[u]) continue;
            for (int v : adj[u]) {
                if (weight[u][v] != INF && dist[u] + weight[u][v] < dist[v]) {
                    dist[v] = dist[u] + weight[u][v];
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        if (dist[dest] == INF) {
            cout << "⚠️  No path exists from " << srcName << " to " << destName << ".\n";
            return;
        }

        cout << "\n============================\n";
        cout << "    🛣️  Shortest Debt Path\n";
        cout << "============================\n";
        vector<int> path;
        for (int at = dest; at != -1; at = prev[at])
            path.push_back(at);
        reverse(path.begin(), path.end());

        for (size_t i = 0; i < path.size(); ++i) {
            cout << idToName[path[i]];
            if (i + 1 < path.size()) cout << " -> ";
        }
        cout << "\n💰 Total debt: $" << dist[dest] << "\n";
    }

    // Lists all registered people
    void listPeople() {
        cout << "\n=======================\n";
        cout << "    👥 Registered People\n";
        cout << "=======================\n";
        for (int i = 0; i < personCount; ++i) {
            cout << " - " << idToName[i] << "\n";
        }
    }

    // Menu-driven interface
    void menu() {
        int choice;
        string name1, name2;
        int amount;

        do {
            cout << "\n====================================\n";
            cout << "     💳 Debt Settlement System\n";
            cout << "====================================\n";
            cout << " 1. Add Person\n";
            cout << " 2. Add Debt\n";
            cout << " 3. Remove Debt\n";
            cout << " 4. Show Debts\n";
            cout << " 5. Show People\n";
            cout << " 6. Show Balance Summary\n";
            cout << " 7. Greedy Settlement\n";
            cout << " 8. Max Flow (Ford-Fulkerson)\n";
            cout << " 9. Shortest Debt Path (Dijkstra)\n";
            cout << "10. Exit\n";
            cout << "====================================\n";
            cout << "Choose an option: ";
            cin >> choice;

            switch (choice) {
                case 1:
                    cout << "Enter person name: ";
                    cin >> name1;
                    addPerson(name1);
                    break;
                case 2:
                    cout << "Enter debtor name: ";
                    cin >> name1;
                    cout << "Enter creditor name: ";
                    cin >> name2;
                    cout << "Enter amount: ";
                    cin >> amount;
                    addDebt(name1, name2, amount);
                    break;
                case 3:
                    cout << "Enter debtor name: ";
                    cin >> name1;
                    cout << "Enter creditor name: ";
                    cin >> name2;
                    removeDebt(name1, name2);
                    break;
                case 4:
                    displayDebts();
                    break;
                case 5:
                    listPeople();
                    break;
                case 6:
                    displaySummary();
                    break;
                case 7:
                    greedySettle();
                    break;
                case 8:
                    cout << "Enter source: ";
                    cin >> name1;
                    cout << "Enter sink: ";
                    cin >> name2;
                    cout << "💧 Max Flow from " << name1 << " to " << name2 << ": $"
                         << fordFulkerson(name1, name2) << "\n";
                    break;
                case 9:
                    cout << "Enter source: ";
                    cin >> name1;
                    cout << "Enter destination: ";
                    cin >> name2;
                    dijkstra(name1, name2);
                    break;
                case 10:
                    cout << "\n👋 Exiting the system. Goodbye!\n";
                    break;
                default:
                    cout << "\n❌ Invalid choice.\n";
            }
        } while (choice != 10);
    }
};

void DecorateTopic()
{
    system("cls");//clears the screen after the loadout
    location(2,1);//location to place the time and day
    timeANDdate();//time and day
}

//function to get the time and day
int timeANDdate()
{
    time_t t;
    t=time(NULL);
    cout<<asctime(localtime(&t));
}

//a simple loading bar function that prints char 219 as a loading bar
void LoadingBar()
{
    // system("color 46");
    location(2,1);
    timeANDdate();
    char des=219;
    system("cls");
    location(80,21);
    cout<<"loading...";
    for(int j=0;j<=100;j++)
    {
        location(115,22);
        cout<<j<<"%"<<endl;
        location(0,21);

        cout<<"\n\t\t\t\t\t\t\t\t\t\t";
        for(int i=1;i<=j;i++)
        {

            if(i%3==0)
            {
                cout<<des;
            }
        }
        cout<<endl;
        Sleep(1);//refresh the screen after every 0.001 second
    }
    Sleep(1000);//gap of 1 sec
}

int main() {
    LoadingBar();//loading bar callout
    DecorateTopic();//design callout
    DebtGraph dg;
    dg.menu();
    return 0;
}
