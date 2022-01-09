#include<bits/stdc++.h>
#define fi first
#define se second
#define pb push_back
using namespace std;

/*
*       Random bot
*       Number of moves in turn and number of cyborgs in troop capped at 10.
*/

// ---------------------------- Hyperparamaters ----------------------------

int moves_in_turn = 10, max_cyborgs = 10;

// -------------------------------------------------------------------------

typedef pair<int,int> par;
typedef pair<int, par> three;
typedef pair<int, three> quad;

vector<par> graph[109];
int n, m, bombs = 2;

par my_fact[109], op_fact[109];
int my_decrease[109], op_decrease[109], neutr_fact[109], opponent_fact[109];
vector<quad> options;

void rand_moves()
{
    int x, action_cnt = 0, cost;
    quad q;
    par p;
    while(true)
    {
        options.clear();
        
        // Finding all possible moves
        for(int i=0; i<n; i++)
        {
            if(neutr_fact[i] == 1 || opponent_fact[i] == 1) continue;
            if(my_fact[i].fi >= 10 && my_fact[i].se < 3) options.pb(quad(3, three(i, par(-1, -1))));
            for(int j=0; j<graph[i].size(); j++)
            {
                p = graph[i][j];
                if(op_fact[p.fi].fi > 0 && bombs > 0) options.pb(quad(2, three(-1, par(i, p.fi))));
                for(int k=1; k<=max_cyborgs; k++)        // Caping movement of troops by 10
                {
                    if(k > my_fact[i].fi) break;
                    if((opponent_fact[p.fi] == 1 || neutr_fact[p.fi] == 1) && op_fact[p.fi].fi > 0) options.pb(quad(1, three(k, par(i, p.fi))));
                    if(opponent_fact[p.fi] == 0 && neutr_fact[p.fi] == 0) options.pb(quad(1, three(k, par(i, p.fi))));
                }
            }
        }

        // Choosing random move
        if(options.size() == 0) break;
        x = rand() % options.size();
        if(action_cnt > 0) cout<<";";
        action_cnt++;
        q = options[x];
        if(q.fi == 1)
        {
            cout<<"MOVE "<<q.se.se.fi<<" "<<q.se.se.se<<" "<<q.se.fi;
            my_fact[q.se.se.fi].fi -= q.se.fi;
            op_fact[q.se.se.se].fi -= q.se.fi;
        }
        else if(q.fi == 2)
        {
            cout<<"BOMB "<<q.se.se.fi<<" "<<q.se.se.se;
            cost = max(10, op_fact[q.se.se.se].fi / 2);
            op_fact[q.se.se.se].fi -= cost;
            bombs--;
        }
        else
        {
            cout<<"INC "<<q.se.fi;
            my_fact[q.se.fi].se++;
            my_fact[q.se.fi].fi -= 10;
        }
        x = rand() % 3;
        if(x == 0 || action_cnt == moves_in_turn || options.size() == 1) break;
    }
    if(action_cnt == 0) cout<<"WAIT";
    cout<<endl;
    return;
}

int main()
{
    int f1, f2, dist, entity_cnt;
    int arg_1, arg_2, arg_3, arg_4, arg_5, id;
    string type;
    par fact1, fact2;
    par p, p1, p2;
    int action_cnt, cost;

    srand(time(NULL));

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
            opponent_fact[i] = 0;
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
                else if(arg_1 == -1) opponent_fact[i] = 1;
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

        rand_moves();
    }
}
