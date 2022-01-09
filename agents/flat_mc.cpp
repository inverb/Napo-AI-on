#include<bits/stdc++.h>
#define fi first
#define se second
#define pb push_back
using namespace std;

/*
*       Flat_MC bot
*       In each round it performs fixed number of Flat MC iterations.
*/

typedef pair<int,int> par;
typedef pair<int, par> three;
typedef pair<par, par> quad;
typedef pair<int, quad> event;
typedef pair<double, int> scoring;

// ------------------------- Hyperparameters -------------------------

int moves_in_turn = 10, num_sims = 200, sim_depth = 16;

// -------------------------------------------------------------------

int n, m;
vector<par> graph[109];

par factory[109], sim_factory[109];
int factory_owner[109], bombs = 2, opp_bombs = 2, stopped[109], bomb_target[109], bombed[109], bombed_now[109], targeted_factories[20][20];
int sim_bombs[109], sim_bombed_now[109], sim_factory_owner[109], sim_stopped[109], curr_moves[109], sim_targeted_factories[20][20];
vector<event> events, sim_events, temp_events;
vector<quad> poss_moves, sim_poss_moves;
vector<quad> moves_to_print;
vector<scoring> poss_moves_with_scores;
vector<int> N, W, move_taken;
set<int> known_bomb_id;

void find_possible_moves()
{
    int x;
    par p;
    poss_moves.pb(quad(par(4, -1), par(-1, -1)));
    for(int i=0; i<n; i++)
    {
        if(factory_owner[i] == 1)
        {
            if(factory[i].fi >= 10 && factory[i].se < 3) poss_moves.pb(quad(par(3, i), par(-1, -1)));
            if(bombs > 0)
            {
                for(int j=0; j<graph[i].size(); j++)
                {
                    p = graph[i][j];
                    if(factory_owner[p.fi] != -1) continue;
                    poss_moves.pb(quad(par(2, p.se), par(i, p.fi)));
                }
            }
            if(factory[i].fi > 0)
            {
                x = 1;
                while(x*x <= factory[i].fi) x++;            // Finding sqrt(factory[i].fi) 
                x--;
                for(int j=0; j<graph[i].size(); j++)
                {
                    p = graph[i][j];
                    // Encoding distance in negative first element of move.
                    for(int k=1; k<factory[i].fi; k+=x) poss_moves.pb(quad(par(-p.se, k), par(i, p.fi)));
                    poss_moves.pb(quad(par(-p.se, factory[i].fi), par(i, p.fi)));  
                }
            }
        }
    }

    return;
}

void make_move(int move_id)
{
    quad q = poss_moves[move_id];
    if(q.fi.fi < 0)
    {
        events.pb(event(-q.fi.fi, quad(par(1, q.fi.se), par(q.se.fi, q.se.se))));
        factory[q.se.fi].fi -= q.fi.se;
        targeted_factories[q.se.fi][q.se.se] = 1;
    }
    else if(q.fi.fi == 2)
    {
        events.pb(event(q.fi.se, quad(par(1, -1), par(q.se.fi, q.se.se))));
        bombed_now[q.se.se] = 1;
    }
    else if(q.fi.fi == 3)
    {
        factory[q.fi.se].fi -= 10;
        factory[q.fi.se].se++;
    }
    return;
}

void add_move(int move_id)
{
    quad q = poss_moves[move_id];
    if(q.fi.fi < 0) targeted_factories[q.se.fi][q.se.se] = 1;
    else if(q.fi.fi == 2) bombed_now[q.se.se] = 1;
    return;
}

void find_sim_poss_moves(int player)
{
    int x;
    par p;
    sim_poss_moves.pb(quad(par(4, -1), par(-1, -1)));
    for(int i=0; i<n; i++)
    {
        if(sim_factory_owner[i] == player)
        {
            if(sim_factory[i].fi >= 10 && sim_factory[i].se < 3) sim_poss_moves.pb(quad(par(3, i), par(-1, -1)));
            if(sim_bombs[player+1] > 0)
            {
                for(int j=0; j<graph[i].size(); j++)
                {
                    p = graph[i][j];
                    if(sim_factory_owner[p.fi] != -1 * player) continue;
                    sim_poss_moves.pb(quad(par(2, p.se), par(i, p.fi)));
                }
            }
            if(sim_factory[i].fi > 0)
            {
                // Can't take all possibilities into account, as there are too may of them.
                x = 1;
                while(x*x <= sim_factory[i].fi) x++;            // Finding sqrt(factory[i].fi) 
                x--;
                for(int j=0; j<graph[i].size(); j++)
                {
                    p = graph[i][j];
                    // Encoding distance in negative first element of move.
                    for(int k=1; k<sim_factory[i].fi; k+=x) sim_poss_moves.pb(quad(par(-p.se, k), par(i, p.fi)));
                    sim_poss_moves.pb(quad(par(-p.se, sim_factory[i].fi), par(i, p.fi)));
                }
            }
        }
    }

    return;
}

void sim_make_move(int move_id, int player, int time)
{
    quad q = sim_poss_moves[move_id];
    if(q.fi.fi < 0)
    {
        sim_events.pb(event(-q.fi.fi, quad(par(player, q.fi.se), par(q.se.fi, q.se.se))));
        sim_factory[q.se.fi].fi -= q.fi.se;
        sim_targeted_factories[q.se.fi][q.se.se] = time;
    }
    else if(q.fi.fi == 2)
    {
        sim_bombs[player+1]--;
        sim_events.pb(event(q.fi.se, quad(par(player, -1), par(q.se.fi, q.se.se))));
        sim_bombed_now[q.se.se] = 1;
    }
    else if(q.fi.fi == 3)
    {
        sim_factory[q.fi.se].fi -= 10;
        sim_factory[q.fi.se].se++;
    }
    return;
}

bool is_sim_move_possible(int move_id, int player, int time)
{
    quad q = sim_poss_moves[move_id];
    if(q.fi.fi < 0)
    {
        if(sim_factory[q.se.fi].fi >= q.fi.se && sim_targeted_factories[q.se.fi][q.se.se] < time) return true;
        else return false;
    }
    else if(q.fi.fi == 2)
    {
        if(sim_bombs[player+1] > 0 && sim_bombed_now[q.se.se] == 0) return true;
        else return false;
    }
    else if(q.fi.fi == 3)
    {
        if(sim_factory[q.fi.se].fi >= 10 && sim_factory[q.fi.se].se < 3) return true;
        else return false;
    }
    return true;
}

bool is_move_possible(int move_id, int player)
{
    quad q = poss_moves[move_id];
    if(q.fi.fi < 0)
    {
        if(factory[q.se.fi].fi >= q.fi.se && targeted_factories[q.se.fi][q.se.se] == 0) return true;
        else return false;
    }
    else if(q.fi.fi == 2)
    {
        int player_bombs;
        if(player == 1) player_bombs = bombs;
        else player_bombs = opp_bombs;
        if(player_bombs > 0 && bombed_now[q.se.se] == 0) return true;
        else return false;
    }
    else if(q.fi.fi == 3)
    {
        if(factory[q.fi.se].fi >= 10 && factory[q.fi.se].se < 3) return true;
        else return false;
    }
    return true;
}

void produce()
{
    for(int i=0; i<n; i++)
    {
        if(sim_stopped[i] > 0) sim_stopped[i]--;
        else sim_factory[i].fi += sim_factory[i].se;
    }
    return;
}

void advance_all_events()
{
    int destination, damage;
    event e;
    quad q;
    temp_events.clear();
    // First we resolve battles...
    for(int i=0; i<sim_events.size(); i++)
    {
        e = sim_events[i];
        q = e.se;
        destination = q.se.se;
        if(q.fi.se == -1) continue;
        e.fi--;
        if(e.fi > 0) temp_events.pb(e);
        else
        {
            damage = q.fi.se;
            if(q.fi.fi == sim_factory_owner[destination]) sim_factory[destination].fi += damage;
            else
            {
                sim_factory[destination].fi -= damage;
                if(sim_factory[destination].fi < 0)
                {
                    sim_factory[destination].fi *= -1;
                    sim_factory_owner[destination] = q.fi.fi;
                }
            }
        }
    }

    // ... then bombs explode
    for(int i=0; i<sim_events.size(); i++)
    {
        e = sim_events[i];
        q = e.se;
        destination = q.se.se;
        if(q.fi.se != -1) continue;
        e.fi--;
        if(e.fi > 0) temp_events.pb(e);
        else
        {
            damage = min(sim_factory[destination].fi, max(10, sim_factory[destination].fi/2));
            sim_factory[destination].fi -= damage;
            sim_stopped[destination] = 5;
        }
    }

    sim_events.clear();
    for(int i=0; i<temp_events.size(); i++) sim_events.pb(temp_events[i]); 
    return;
}

void initialize_simulation()
{
    int id, move, move_id;
    event e;
    for(int i=0; i<n; i++)
    {
        sim_factory_owner[i] = factory_owner[i];
        sim_factory[i] = factory[i];
        sim_stopped[i] = stopped[i];
        sim_bombed_now[i] = bombed_now[i];
        for(int j=0; j<n; j++) sim_targeted_factories[i][j] = targeted_factories[i][j];
    }
    sim_bombs[0] = opp_bombs;
    sim_bombs[2] = bombs;
    sim_events.clear();
    for(int i=0; i<events.size(); i++) sim_events.pb(events[i]);
    sim_poss_moves.clear();
    for(int i=0; i<poss_moves.size(); i++) sim_poss_moves.pb(poss_moves[i]);
    return;
}

int simulate(int action_cnt)
{
    int id, move, move_id;
    event e;

    // for(int i=0; i<action_cnt; i++)
    // {
    //     move_id = curr_moves[i];
    //     sim_make_move(move_id, 1, 1);
    // }

    for(int i=2; i<=sim_depth; i++)
    {
        sim_poss_moves.clear();
        find_sim_poss_moves(2*(i%2)-1);
        if(sim_poss_moves.size() > 1)
        {
            for(int j=1; j<=moves_in_turn; j++)                          // Probably needs optimization to choose all moves at once.
            {
                move = rand() % sim_poss_moves.size();
                if(is_sim_move_possible(move, 2*(i%2)-1, i)) sim_make_move(move, 2*(i%2)-1, i);
                else j--;
            }
        }
        if(i%2 == 0)
        { 
            produce();
            advance_all_events();
        }
    }

    // Evaluating final position
    int score = 10000;                                                  // To make score non-negative
    score += sim_bombs[2] - sim_bombs[0];
    for(int i=0; i<n; i++)
    {
        if(sim_factory_owner[i] == 1) score += sim_factory[i].se * 10 + sim_factory[i].fi;
        else score -= sim_factory[i].se * 10 + sim_factory[i].fi;
    }
    for(int i=0; i<sim_events.size(); i++)
    {
        e = sim_events[i];
        if(e.se.fi.se != -1) score += e.se.fi.fi * e.se.fi.se;
    }
    return score;
}

void print_moves()
{
    int dist;
    quad q;
    if(moves_to_print.size() == 0)
    {
        cout<<"WAIT"<<endl;
        return;
    }

    for(int i=0; i<moves_to_print.size(); i++)
    {
        q = moves_to_print[i];
        if(i != 0) cout<<";";
        if(q.fi.fi < 0) cout<<"MOVE "<<q.se.fi<<" "<<q.se.se<<" "<<q.fi.se;
        else if(q.fi.fi == 2)
        {
            cout<<"BOMB "<<q.se.fi<<" "<<q.se.se;
            bombs--;
        }
        else if(q.fi.fi == 3) cout<<"INC "<<q.fi.se;
        else cout<<"WAIT";
    }
    cout<<endl;
    return;
}

void cerr_print()
{
    int dist;
    quad q;

    for(int i=0; i<moves_to_print.size(); i++)
    {
        q = moves_to_print[i];
        if(q.fi.fi < 0) cerr<<"MOVE "<<q.se.fi<<" "<<q.se.se<<" "<<q.fi.se<<"\t";
        else if(q.fi.fi == 2) cerr<<"BOMB "<<q.se.fi<<" "<<q.se.se<<"\t";
        else if(q.fi.fi == 3) cerr<<"INC "<<q.fi.se<<"\t";
        else cerr<<"WAIT \t";
        cerr<<endl;
    }
    return;
}

void cerr_print_moves()
{
    int dist, id;
    quad q;

    cerr<<poss_moves.size()<<endl;
    for(int i=0; i<poss_moves_with_scores.size(); i++)
    {
        id = poss_moves_with_scores[i].se;
        q = poss_moves[id];
        if(q.fi.fi < 0) cerr<<"MOVE "<<q.se.fi<<" "<<q.se.se<<" "<<q.fi.se<<"\t";
        else if(q.fi.fi == 2) cerr<<"BOMB "<<q.se.fi<<" "<<q.se.se<<"\t";
        else if(q.fi.fi == 3) cerr<<"INC "<<q.fi.se<<"\t";
        else cerr<<"WAIT \t";
        cerr<<poss_moves_with_scores[i].fi<<"\t"<<W[id]<<"/"<<N[id]<<"\n";
    }
    cerr<<endl;
    return;
}

int main()
{
    int f1, f2, dist, entity_cnt, timer = 0;
    int arg_1, arg_2, arg_3, arg_4, arg_5, id;
    double a, b, c, max_score;
    string type;
    par fact1, fact2;
    par p, p1, p2;
    quad q;
    event e;
    int action_cnt, cost, target_id, sending_id, damage, move_id, score, N0, W0, moves_made;

    srand(time(NULL));
    cin>>n>>m;
    for(int i=1; i<=m; i++)
    {
        cin>>f1>>f2>>dist; cin.ignore();
        graph[f1].pb(par(f2, dist));
        graph[f2].pb(par(f1, dist));
    }


    // Game loop
    while(true)
    {
        timer++;
        for(int i=0; i<n; i++)
        {
            factory[i] = par(-1, -1);
            factory_owner[i] = 0;
            stopped[i] = 0;
            bombed[i] = 0;
            bombed_now[i] = 0;
            for(int j=0; j<n; j++) targeted_factories[i][j] = 0;
        }
        events.clear();

        cin>>entity_cnt; cin.ignore();      // The number of entities (factories, troops and bombs).
        for(int i=0; i<entity_cnt; i++) 
        {
            cin>>id>>type>>arg_1>>arg_2>>arg_3>>arg_4>>arg_5; cin.ignore();
            if(type == "FACTORY")
            {
                factory[id] = par(arg_2, arg_3);
                factory_owner[id] = arg_1;
                stopped[id] = arg_4;
                if(arg_4 > 0) bombed[id] = 1;
            }
            else if(type == "TROOP")
            {
                events.pb(event(arg_5, quad(par(arg_1, arg_4), par(arg_2, arg_3))));
            }
            else if(type == "BOMB")
            {
                if(arg_1 == 1)
                {
                    events.pb(event(arg_4, quad(par(arg_1, -1), par(arg_2, arg_3))));
                    bombed[arg_3] = 1;
                }
                else
                {
                    // HEURISTIC : If opponent fired a new bomb, assume the target is our factory with the most cyborgs.
                    if(known_bomb_id.find(id) != known_bomb_id.end())
                    {
                        events.pb(event(arg_4, quad(par(arg_1, -1), par(arg_2, bomb_target[id]))));
                        bombed[bomb_target[id]] = 1;
                    }
                    else
                    {
                        damage = -1;
                        target_id = -1;
                        opp_bombs -= 1;
                        for(int j=0; j<graph[arg_2].size(); j++)
                        {
                            p = graph[arg_2][j];
                            if(factory_owner[p.fi] == 1 && factory[p.fi].fi > damage)
                            {
                                damage = factory[p.fi].fi;
                                target_id = p.fi;
                            }
                        }
                        if(damage > -1)
                        {
                            bomb_target[id] = target_id;
                            bombed[target_id] = 1;
                            events.pb(event(arg_4, quad(par(arg_1, -1), par(arg_2, bomb_target[id]))));
                        }
                        known_bomb_id.insert(id);
                    }
                }
            }
        }
        sort(events.begin(), events.end());

        moves_to_print.clear();
        poss_moves.clear();
        find_possible_moves();
        if(poss_moves.size() == 1)
        {
            print_moves();
            continue;
        }
        N.clear();
        W.clear();
        move_taken.clear();
        N.resize(poss_moves.size(), 0);
        W.resize(poss_moves.size(), 0);
        move_taken.resize(poss_moves.size(), 0);

        // Main loop of simulations
        for(int i=1; i<=num_sims; i++)
        {
            initialize_simulation();
            action_cnt = 0;
            for(int j=1; j<=moves_in_turn; j++)
            {
                for(int k=1; k<=10; k++)
                {
                    move_id = rand() % poss_moves.size();
                    if(move_taken[move_id] == 0 && is_sim_move_possible(move_id, 1, 1))
                    {
                        curr_moves[action_cnt] = move_id;
                        sim_make_move(move_id, 1, 1);
                        action_cnt++;
                        move_taken[move_id] = 1;
                        break;
                    }
                }
            }

            // if(i == 1)
            // {
            //     for(int j=0; j<action_cnt; j++) moves_to_print.pb(poss_moves[curr_moves[j]]);
            //     cerr_print();
            //     cerr<<"--------------------------------------"<<endl;
            //     moves_to_print.clear();
            // }

            score = simulate(action_cnt);
            for(int j=0; j<action_cnt; j++)
            {
                move_id = curr_moves[j];
                N[move_id]++;
                W[move_id] += score;
                move_taken[move_id] = 0;

                // q = poss_moves[move_id];
                // if(q.se.fi >= 0) targeted_factories[q.se.fi][q.se.se] = 0;
            }
        }

        poss_moves_with_scores.clear();
        for(int i=0; i<poss_moves.size(); i++)
        {
            if(N[i] == 0) continue;
            a = W[i];
            b = N[i];
            if(poss_moves[i].fi.fi < 0)
            {
                c = poss_moves[i].fi.se + 1;
                //b *= log10(c);
            }
            c = a / b;                 // To make them non-negative
            poss_moves_with_scores.pb(scoring(-c, i));
        }
        sort(poss_moves_with_scores.begin(), poss_moves_with_scores.end());
        
        max_score = -poss_moves_with_scores[0].fi;
        moves_made = 0;
        for(int i=0; i<poss_moves_with_scores.size(); i++)
        {
            if(moves_made == moves_in_turn) break;
            score = -poss_moves_with_scores[i].fi;
            move_id = poss_moves_with_scores[i].se;
            //if(score < max_score * 0.8 && score < max_score / 0.8) break;
            if(is_move_possible(move_id, 1))
            {
                make_move(move_id);
                moves_to_print.pb(poss_moves[move_id]);
                moves_made++;
            }
        }
        print_moves();
        //cerr_print_moves();
    }
}
