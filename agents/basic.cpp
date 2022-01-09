#include<bits/stdc++.h>
#define fi first
#define se second
#define pb push_back
using namespace std;

/*
*       Basic bot
*       Finds first troop that can conquer any factory and sends it.
*       Doesn't use bombs or increments.
*/

typedef pair<int,int> par;
typedef pair<int, par> three;

vector<par> graph[109];
int n, m;

par my_fact[109], op_fact[109];
int my_decrease[109], op_decrease[109], neutr_fact[109];

int main()
{
    int f1, f2, dist, entity_cnt;
    int arg_1, arg_2, arg_3, arg_4, arg_5, id;
    string type;
    par fact1, fact2;
    par p, p1, p2;
    int action_cnt, cost;
    cin>>n>>m;
    for(int i=1; i<=m; i++)
    {
        cin>>f1>>f2>>dist; cin.ignore();
        graph[f1].pb(par(f2, dist));
        graph[f2].pb(par(f1, dist));
    }


    // game loop
    while(true)
    {
        for(int i=0; i<n; i++)
        {
            my_decrease[i] = 0;
            op_decrease[i] = 0;
            my_fact[i] = par(-1, -1);
            op_fact[i] = par(-1, -1);
            neutr_fact[i] = 0;
        }
        action_cnt = 0;

        cin>>entity_cnt; cin.ignore(); // The number of entities (e.g. factories and troops).
        for(int i=0; i<entity_cnt; i++) 
        {
            cin>>id>>type>>arg_1>>arg_2>>arg_3>>arg_4>>arg_5; cin.ignore();
            if(type == "FACTORY")
            {
                if(arg_1 == 1) my_fact[id] = par(arg_2, arg_3);
                else op_fact[id] = par(arg_2, arg_3);
                if(arg_1 == 0) neutr_fact[i] = 1;
            }
            else
            {
                if(arg_1 == -1) my_decrease[arg_3] += arg_4;
                else op_decrease[arg_3] += arg_4;
            }
        }

        for(int i=0; i<n; i++)
        {
            my_fact[i].fi -= my_decrease[i];
            op_fact[i].fi -= op_decrease[i];
        }

        for(int i=0; i<n; i++)
        {
            fact1 = my_fact[i];
            for(int j=0; j<graph[i].size(); j++)
            {
                p = graph[i][j];
                fact2 = op_fact[p.fi];
                if(neutr_fact[p.fi] == 1)
                {
                    if(fact2.fi >= 0 && fact1.fi >= 0 && fact1.fi > fact2.fi)
                    {
                        if(action_cnt > 0) cout<<";";
                        cost = fact2.fi + 1;
                        cout<<"MOVE "<<i<<" "<<p.fi<<" "<<cost;
                        my_fact[i].fi -= cost;
                        op_fact[p.fi].fi -= cost;
                        action_cnt++;
                    }
                }
                else
                {
                    if(fact2.fi >= 0 && fact1.fi >= 0 && fact1.fi > fact2.fi + p.se * fact2.se)
                    {
                        if(action_cnt > 0) cout<<";";
                        cost = fact2.fi + p.se * fact2.se + 1;
                        cout<<"MOVE "<<i<<" "<<p.fi<<" "<<cost;
                        my_fact[i].fi -= cost;
                        op_fact[p.fi].fi -= cost;
                        action_cnt++;
                    }
                }
            }
        }
        if(action_cnt == 0) cout<<"WAIT";
        cout<<endl;
    }
}
