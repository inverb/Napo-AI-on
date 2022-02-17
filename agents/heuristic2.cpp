#include<bits/stdc++.h>
#define fi first
#define se second
#define pb push_back
using namespace std;

/*
*       Heuristic2 bot
*       No named algorithm, just a bunch of heuristics for bombs, defence, attack and upgrading.
*/

typedef pair<int,int> par;
typedef pair<int, par> three;
typedef pair<par, par> quad;
typedef pair<int, quad> event;

vector<par> graph[109];
int n, m;

par my_fact[109], op_fact[109], my_fact_simul[109], op_fact_simul[109];
int fact_possession[109], bombs = 2, stopped[109], bombed[109], stopped_simul[109], leave[109], moves_made[109], spares[109];
vector<event> events, events_now;
vector<quad> attacks;
set<int> known_bomb_id;

int closest[109][109], col[109];
priority_queue<quad, vector<quad>, greater<quad> > qu;

void calc_dists()
{
    par p;
    quad q;
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<graph[i].size(); j++)
        {
            p = graph[i][j];
            if(i == p.fi) continue;
            col[p.fi] = 0;
            qu.push(quad(par(p.se, -1), par(p.fi, p.fi)));
        }
        while(!qu.empty())
        {
            q = qu.top();
            qu.pop();
            if(col[q.se.fi] == 1) continue;
            col[q.se.fi] = 1;
            closest[i][q.se.fi] = q.se.se;
            for(int j=0; j<graph[q.se.fi].size(); j++)
            {
                p = graph[q.se.fi][j];
                if(col[p.fi] == 0 && p.fi != i) qu.push(quad(par(q.fi.fi+p.se+1, q.fi.se-1), par(p.fi, q.se.se)));
            }
        }
    }
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++) cerr<<closest[i][j]<<" ";
        cerr<<"\n";
    }
    return;
}

int place_bombs(int action_cnt, int timer)
{
    if(bombs == 0) return 0;
    par p;
    vector<three> v;
    for(int i=0; i<n; i++)
    {
        if(fact_possession[i] == -1 && bombed[i] == 0)
        {
            if(op_fact[i].se < 2) continue;
            int dist = 1e9, fact_id;
            for(int j=0; j<graph[i].size(); j++)
            {
                p = graph[i][j];
                // HEURISTIC : only bomb factories with many cyborgs, so bomb destroys at least 10
                if(timer <= 10 && op_fact[i].fi + op_fact[i].se * p.se < 8) continue;
                if(fact_possession[p.fi] == 1 && p.se < dist)
                {
                    dist = p.se;
                    fact_id = p.fi;
                }
            }
            if(dist < 1e9) v.pb(three(-op_fact[i].fi, par(fact_id, i)));
        }
    }
    sort(v.begin(), v.end());
    if(!v.empty())
    {
        if(action_cnt > 0) cout<<";";
        cout<<"BOMB "<<v[0].se.fi<<" "<<v[0].se.se;
        return 1;
    }
    return 0;
}

void simul()
{
    int curr_time = 0, target_id, lost, troops_sent, actions = 0;
    event e;
    par p;

    for(int i=0; i<events.size(); i++)
    {
        for(int i=0; i<n; i++) if(fact_possession[i] == 1) spares[i] = min(spares[i], my_fact_simul[i].fi);
        e = events[i];
        if(e.fi > curr_time)
        {
            for(int j=curr_time+1; j<=e.fi; j++)
            {
                for(int k=0; k<n; k++)
                {
                    if(stopped_simul[k] == 0) my_fact_simul[k].fi += my_fact_simul[k].se;
                    if(stopped_simul[k] == 0) op_fact_simul[k].fi += op_fact_simul[k].se;
                    else stopped_simul[k]--;
                }
            }
            curr_time = e.fi;
        }

        target_id = e.se.se.se;
        if(e.se.fi.se == -1)
        {
            // Event is a bomb sent by us
            stopped_simul[target_id] = 5;
        }
        else
        {
            // Event is a troop
            if(e.se.fi.fi == 1)
            {
                // My troop
                if(fact_possession[target_id] == 1) my_fact_simul[target_id].fi += e.se.fi.se;
                else
                {
                    op_fact_simul[target_id].fi -= e.se.fi.se;
                    if(op_fact_simul[target_id].fi < 0)
                    {
                        my_fact_simul[target_id].fi = -op_fact_simul[target_id].fi;
                        my_fact_simul[target_id].se = op_fact_simul[target_id].se;
                        op_fact_simul[target_id].fi = 0;
                        //fact_possession[target_id] = 1;
                    }
                }
            }
            else
            {
                // Opponents troop...
                if(fact_possession[target_id] == 1)
                {
                    // ... attacks my factory
                    my_fact_simul[target_id].fi -= e.se.fi.se;
                    if(my_fact_simul[target_id].fi < 0)
                    {
                        lost = 1;
                        if(lost == 1)
                        {
                            op_fact_simul[target_id].fi = -my_fact_simul[target_id].fi;
                            op_fact_simul[target_id].se = my_fact_simul[target_id].se;
                            my_fact_simul[target_id].fi = 0;
                            //fact_possession[target_id] = -1;
                        }
                    }
                }
                else
                {
                    // ... attacks neutral factory
                    if(op_fact_simul[target_id].fi >= e.se.fi.se) op_fact_simul[target_id].fi -= e.se.fi.se;
                    else
                    {
                        op_fact_simul[target_id].fi = e.se.fi.se - op_fact_simul[target_id].fi;
                        //fact_possession[target_id] = -1;
                    }
                }
            }
        }
    }
}

int defend(int action_cnt)
{
    int curr_time = 0, target_id, lost, troops_sent, actions = 0;
    event e;
    par p;

    for(int i=0; i<n; i++)
    {
        my_fact_simul[i] = my_fact[i]; 
        op_fact_simul[i] = op_fact[i];
        stopped_simul[i] = stopped[i];
        spares[i] = 1e9;
    }

    simul();

    for(int i=0; i<n; i++)
    {
        my_fact_simul[i] = my_fact[i]; 
        op_fact_simul[i] = op_fact[i];
        stopped_simul[i] = stopped[i];
    }

    for(int i=0; i<events.size(); i++)
    {
        e = events[i];
        if(e.fi > curr_time)
        {
            for(int j=curr_time+1; j<=e.fi; j++)
            {
                for(int k=0; k<n; k++)
                {
                    if(stopped_simul[k] == 0) my_fact_simul[k].fi += my_fact_simul[k].se;
                    if(stopped_simul[k] == 0) op_fact_simul[k].fi += op_fact_simul[k].se;
                    else stopped_simul[k]--;
                }
            }
            curr_time = e.fi;
        }

        target_id = e.se.se.se;
        if(e.se.fi.se == -1)
        {
            // Event is a bomb sent by us
            stopped_simul[target_id] = 5;
        }
        else
        {
            // Event is a troop
            if(e.se.fi.fi == 1)
            {
                // My troop
                if(fact_possession[target_id] == 1) my_fact_simul[target_id].fi += e.se.fi.se;
                else
                {
                    op_fact_simul[target_id].fi -= e.se.fi.se;
                    if(op_fact_simul[target_id].fi < 0)
                    {
                        my_fact_simul[target_id].fi = -op_fact_simul[target_id].fi;
                        my_fact_simul[target_id].se = op_fact_simul[target_id].se;
                        op_fact_simul[target_id].fi = 0;
                        //fact_possession[target_id] = 1;
                    }
                }
            }
            else
            {
                // Opponents troop...
                if(fact_possession[target_id] == 1)
                {
                    // ... attacks my factory
                    my_fact_simul[target_id].fi -= e.se.fi.se;
                    if(my_fact_simul[target_id].fi < 0)
                    {
                        lost = 1;
                        // HEURISTIC : not saving factories that currently don't produce
                        if(my_fact_simul[target_id].se > 0)
                        {
                            // Trying to prevent losing the factory by sending there troops ASAP
                            for(int j=0; j<graph[target_id].size(); j++)
                            {
                                p = graph[target_id][j];
                                if(fact_possession[p.fi] == 1 && my_fact[p.fi].fi > 5 && my_fact_simul[p.fi].fi > 5 && p.se <= curr_time)
                                {
                                    troops_sent = min(spares[p.fi], min(my_fact[p.fi].fi - 5, -my_fact_simul[target_id].fi));
                                    if(troops_sent == 0) continue;
                                    if(action_cnt + actions != 0) cout<<";";
                                    cout<<"MOVE "<<p.fi<<" "<<closest[p.fi][target_id]<<" "<<troops_sent;
                                    actions++;
                                    my_fact_simul[p.fi].fi -= troops_sent;
                                    my_fact_simul[p.fi].fi += troops_sent;
                                    my_fact[p.fi].fi -= troops_sent;            // Really sending troops, so have to cont them in real counter
                                    spares[p.fi] -= troops_sent;
                                }
                            }
                        }
                        if(lost == 1)
                        {
                            op_fact_simul[target_id].fi = -my_fact_simul[target_id].fi;
                            op_fact_simul[target_id].se = my_fact_simul[target_id].se;
                            my_fact_simul[target_id].fi = 0;
                            //fact_possession[target_id] = -1;
                        }
                    }
                }
                else
                {
                    // ... attacks neutral factory
                    if(op_fact_simul[target_id].fi >= e.se.fi.se) op_fact_simul[target_id].fi -= e.se.fi.se;
                    else
                    {
                        op_fact_simul[target_id].fi = e.se.fi.se - op_fact_simul[target_id].fi;
                        //fact_possession[target_id] = -1;
                    }
                }
            }
        }
    }
    return actions;
}

int attack(int action_cnt)
{
    int actions = 0, troops_sent, target_id, damage;
    quad q;
    par p;
    event e;
    attacks.clear();

    for(int i=0; i<n; i++)
    {
        my_fact_simul[i] = my_fact[i]; 
        op_fact_simul[i] = op_fact[i];
        moves_made[i] = 0;
    }

    for(int i=0; i<events.size(); i++)
    {
        // Simulating all object currently in the air.
        e = events[i];
        target_id = e.se.se.se;
        if(e.se.fi.se == -1)
        {
            // Event is a bomb sent by us
            damage = max(10, op_fact_simul[target_id].fi/2);
            op_fact_simul[target_id].fi -= damage;
        }
        else
        {
            // Event is a troop
            if(e.se.fi.fi == 1)
            {
                // My troop
                op_fact_simul[target_id].fi -= e.se.fi.se;
                if(op_fact_simul[target_id].fi < 0)
                {
                    my_fact_simul[target_id].fi = -op_fact_simul[target_id].fi;
                    my_fact_simul[target_id].se = op_fact_simul[target_id].se;
                    //fact_possession[target_id] = 1;
                }
            }
            else
            {
                // Opponents troop...
                if(fact_possession[target_id] == 1)
                {
                    // ... attacks my factory
                    my_fact_simul[target_id].fi -= e.se.fi.se;
                    if(my_fact_simul[target_id].fi < 0)
                    {
                        op_fact_simul[target_id].fi = -my_fact_simul[target_id].fi;
                        op_fact_simul[target_id].se = my_fact_simul[target_id].se;
                        my_fact_simul[target_id].fi = 0;
                    }
                }
                else
                {
                    // ... attacks neutral factory
                    op_fact_simul[target_id].fi = max(e.se.fi.se - op_fact_simul[target_id].fi, op_fact_simul[target_id].fi - e.se.fi.se);
                }
            }
        }
    }

    for(int i=0; i<n; i++)
    {
        if(fact_possession[i] == 1 && my_fact_simul[i].fi > 0)
        {
            for(int j=0; j<graph[i].size(); j++)
            {
                p = graph[i][j];
                // HEURISTIC : Only attack factories close to you. Prioritize those that produce a lot.
                //if(p.se > 6) continue;
                if(fact_possession[p.fi] != 1 && op_fact_simul[p.fi].fi <= my_fact[i].fi && op_fact_simul[p.fi].fi <= my_fact_simul[i].fi && op_fact_simul[p.fi].fi >= 0)
                {
                    // HEURISTIC : Until you're rich, don't attack factories that don't provide production.
                    if(op_fact_simul[p.fi].se == 0 && my_fact_simul[i].fi < 20) continue;
                    if(fact_possession[p.fi] == 0) attacks.pb(quad(par(1000 * (3 - op_fact_simul[p.fi].se) + op_fact_simul[p.fi].fi, p.se), par(i, p.fi)));
                    else if(op_fact_simul[p.fi].fi + (p.se + 1 - stopped[i]) * op_fact_simul[p.fi].se < my_fact[i].fi) attacks.pb(quad(par(1000 * (3 - op_fact_simul[p.fi].se) + op_fact_simul[p.fi].fi + (p.se + 1 - stopped[i]) * op_fact_simul[p.fi].se, p.se), par(i, p.fi)));
                }
            }
        }
    }

    sort(attacks.begin(), attacks.end());
    for(int i=0; i<attacks.size(); i++)
    {
        //if(actions == 10) break;
        q = attacks[i];
        if(my_fact_simul[q.se.fi].fi < 5) continue;
        if(moves_made[q.se.fi] >= 2) continue;
        troops_sent = min(min(my_fact[q.se.fi].fi, my_fact_simul[q.se.fi].fi), (q.fi.fi % 1000) + 1);
        if(action_cnt + actions != 0) cout<<";";
        cout<<"MOVE "<<q.se.fi<<" "<<closest[q.se.fi][q.se.se]<<" "<<troops_sent;
        actions++;
        my_fact[q.se.fi].fi -= troops_sent;
        // Only restricted numer of moves from each factory in each round
        my_fact_simul[q.se.fi].fi -= troops_sent;
        moves_made[q.se.fi]++;
    }
    return actions;
}

int upgrade(int action_cnt)
{
    int actions = 0;
    par p;
    for(int i=0; i<n; i++)
    {
        if(fact_possession[i] == 1 && my_fact[i].se < 3)
        {
            if(my_fact[i].fi > 15)
            {
                if(action_cnt + actions != 0) cout<<";";
                cout<<"INC "<<i;
                actions++;
                my_fact[p.fi].fi -= 10;
            }
            else
            {
                for(int j=0; j<graph[i].size(); j++)
                {
                    p = graph[i][j];
                    if(fact_possession[p.fi] == 1 && my_fact[p.fi].fi > 30 && my_fact_simul[p.fi].fi > 30)
                    {
                        if(action_cnt + actions != 0) cout<<";";
                        cout<<"MOVE "<<p.fi<<" "<<closest[p.fi][i]<<" 15";
                        my_fact[p.fi].fi -= 15;
                        actions++;
                    }
                }
            }
        }
    }
    return actions;
}

int main()
{
    int f1, f2, dist, entity_cnt, timer = 0;
    int arg_1, arg_2, arg_3, arg_4, arg_5, id;
    string type;
    par fact1, fact2;
    par p, p1, p2;
    quad q;
    event e;
    int action_cnt, cost, target_id, sending_id, damage, time, d_cyborg;
    cin>>n>>m;
    for(int i=1; i<=m; i++)
    {
        cin>>f1>>f2>>dist; cin.ignore();
        graph[f1].pb(par(f2, dist));
        graph[f2].pb(par(f1, dist));
    }

    calc_dists();

    // game loop
    while(true)
    {
        timer++;
        for(int i=0; i<n; i++)
        {
            my_fact[i] = par(-1, -1);
            op_fact[i] = par(-1, -1);
            fact_possession[i] = 0;
            bombed[i] = 0;
        }
        action_cnt = 0;
        d_cyborg = 0;
        events.clear();
        events_now.clear();

        cin>>entity_cnt; cin.ignore(); // the number of entities (e.g. factories and troops)
        for(int i=0; i<entity_cnt; i++) 
        {
            cin>>id>>type>>arg_1>>arg_2>>arg_3>>arg_4>>arg_5; cin.ignore();
            if(type == "FACTORY")
            {
                if(arg_1 == 1) my_fact[id] = par(arg_2, arg_3);
                else op_fact[id] = par(arg_2, arg_3);
                if(arg_1 == 1) d_cyborg += arg_3; 
                fact_possession[id] = arg_1;
                stopped[id] = arg_4;
                if(arg_4 > 0) bombed[id] = 1;
            }
            else if(type == "TROOP")
            {
                if(arg_5 >= 0) events.pb(event(arg_5, quad(par(arg_1, arg_4), par(arg_2, arg_3))));
            }
            else if(type == "BOMB")
            {
                if(arg_1 == 1)
                {
                    if(arg_4 >= 0)
                    {
                        events.pb(event(arg_4, quad(par(arg_1, -1), par(arg_2, arg_3))));
                        bombed[arg_3] = 1;
                    }
                }
                else
                {
                    if(known_bomb_id.find(id) != known_bomb_id.end()) continue;
                    damage = -1;
                    target_id = -1;
                    for(int j=0; j<graph[arg_2].size(); j++)
                    {
                        p = graph[arg_2][j];
                        if(fact_possession[p.fi] == 1 && my_fact[p.fi].fi > damage && leave[p.fi] <= 0)
                        {
                            damage = my_fact[p.fi].fi;
                            target_id = p.fi;
                            time = p.se;
                        }
                    }
                    if(damage > -1) leave[target_id] = time;
                }
                known_bomb_id.insert(id);
            }
        }
        sort(events.begin(), events.end());

        // HEURISTIC : First round is special.
        if(timer == 1)
        {
            // HEURISTIC : If it's first round and you have no cyborg production, create some.
            if(d_cyborg < 1)
            {
                for(int i=0; i<n; i++)
                {
                    if(fact_possession[i] == 1 && my_fact[i].fi >= 10)
                    {
                        if(action_cnt != 0) cout<<";";
                        cout<<"INC "<<i;
                        action_cnt++;
                        my_fact[i].fi -= 10;
                    }
                }
            }

            attacks.clear();

            for(int i=0; i<n; i++)
            {
                if(fact_possession[i] == 1 && my_fact[i].fi > 0)
                {
                    for(int j=0; j<graph[i].size(); j++)
                    {
                        p = graph[i][j];
                        if(fact_possession[p.fi] != 1 && op_fact[p.fi].fi < my_fact[i].fi && op_fact[p.fi].se > 0) attacks.pb(quad(par(p.se, op_fact[p.fi].fi), par(i, p.fi)));
                    }
                }
            }

            sort(attacks.begin(), attacks.end());
            int actions = 0, troops_sent;

            for(int i=0; i<attacks.size(); i++)
            {
                if(actions == 2) break;
                q = attacks[i];
                if(my_fact[q.se.fi].fi <= q.fi.se) continue;
                troops_sent = min(my_fact[q.se.fi].fi, q.fi.se + 1);
                if(action_cnt != 0) cout<<";";
                cout<<"MOVE "<<q.se.fi<<" "<<closest[q.se.fi][q.se.se]<<" "<<troops_sent;
                actions++;
                action_cnt++;
                my_fact[q.se.fi].fi -= troops_sent;
            }
            cout<<endl;
            continue;
        }

        action_cnt += place_bombs(action_cnt,timer);
        action_cnt += defend(action_cnt);
        action_cnt += attack(action_cnt);
        action_cnt += upgrade(action_cnt);

        for(int i=0; i<n; i++)
        {
            leave[i]--;
            if(fact_possession[i] != 1) continue;
            if(leave[i] == 0)
            {
                target_id = -1;
                damage = -1;
                for(int j=0; j<graph[i].size(); j++)
                {
                    p = graph[i][j];
                    if(fact_possession[p.fi] == 1 && my_fact[p.fi].se * 10 + my_fact[p.fi].fi > damage)
                    {
                        damage = my_fact[p.fi].se * 10 + my_fact[p.fi].fi;
                        target_id = p.fi;
                    }
                }
                if(target_id == -1) target_id = graph[i][0].fi;
                if(action_cnt != 0) cout<<";";
                cout<<"MOVE "<<i<<" "<<closest[i][target_id]<<" "<<my_fact[i].fi;
                action_cnt++;
            }
        }

        if(action_cnt == 0) cout<<"WAIT";
        cout<<endl;
    }
}
