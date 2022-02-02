// #undef _GLIBCXX_DEBUG                // disable run-time bound checking, etc
// #pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

// #pragma GCC target("bmi,bmi2,lzcnt,popcnt")                      // bit manipulation
// #pragma GCC target("movbe")                                      // byte swap
// #pragma GCC target("aes,pclmul,rdrnd")                           // encryption
// #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include <iostream>
#include <vector>
#include <queue>
#include <variant>
#include <random>
#include <optional>
#include <algorithm>
#include <chrono>
#include <map>

using namespace std;
using namespace chrono;

auto seed = 1337;
std::mt19937 gen{ seed };

struct edge_t {
    int to, dist;
    edge_t(int fi, int se) : to(fi), dist(se) { /* empty */ }
};

vector<vector<edge_t>> graph;

enum player_t : int8_t {
    agent = 1,
    nobody = 0,
    enemy = -1
};

string player_to_string(const player_t &player) {
    switch (player) {
    case agent:
        return "AGENT";
    case enemy:
        return "ENEMY";
    default:
        return "NOBODY";
    }
}

player_t int_to_player(const int &p) {
    switch (p) {
        case 0:
            return nobody;
        case -1:
            return enemy;
        case 1:
            return agent;
        default:
            throw runtime_error("Invalid int for player_t");
    }
}

struct factory_t {
    int cyborgs, normal_prod;
    player_t owner;
    int until_normal = 0;

    int production() const {
        return until_normal > 0 ? 0 : normal_prod;
    }
};

int get_distance(int origin_factory, int target_factory) {
    // @TODO: Speed
    return find_if(
        begin(graph[origin_factory]),
        end(graph[origin_factory]),
        [&target_factory](const edge_t& e) {
            return e.to == target_factory;
        }
        // @Note: may err if no match is found...
    )->dist;
}

struct troop_t {
    int origin_factory, target_factory, size, distance;
    player_t owner;

    troop_t(player_t owner, int of, int tf, int size) :
        origin_factory(of), target_factory(tf), size(size),
        distance(get_distance(origin_factory, target_factory)),
        owner(owner) { /* empty */
    }
};

struct bomb_t {
    int origin_factory, target_factory, distance;
    player_t owner;

    bomb_t(player_t owner, int of, int tf) :
        origin_factory(of), target_factory(tf),
        distance(get_distance(origin_factory, target_factory)),
        owner(owner) { /* empty */
    }
};

struct state_t {
    vector<factory_t> factories;
    vector<troop_t> troops;
    vector<bomb_t> bombs;

    int agent_bombs_left, enemy_bombs_left;

    state_t() : agent_bombs_left(0), enemy_bombs_left(0) {
        /* empty */
    }
};

void print_state(const state_t &state);

namespace action {
    struct move_t {
        int src, dest, count;

        static optional<move_t> random(const state_t& state) {
            // Weights for random distribution used to pick src and dest factories.
            // Vector `src_weights` consist of 0s and 1s for enemy's and agent's factories, respectively - agent can move cyborgs only from their factory.
            // Vector `dest_weights` consists of 2s and 1s for enemy's and agent's factories, respectively - attacking is preffered to merging troops.
            vector<int> src_weights(state.factories.size());
            vector<int> dest_weights(state.factories.size());

            bool src_available = false;

            for (size_t i = 0; i < state.factories.size(); i++) {
                const factory_t factory = state.factories[i];

                if (factory.owner == agent && factory.cyborgs > 0) {
                    src_weights[i] = 1;
                    dest_weights[i] = 1;
                    src_available = true;
                }
                else {
                    dest_weights[i] = 2;
                }
            }

            // print_state(state);
            // cerr << "Src weights:\n";
            // for (const int w : src_weights) {
            //     cerr << w << " ";
            // }
            // cerr << "\nDest weights:\n";
            // for (const int w : dest_weights) {
            //     cerr << w << " ";
            // }
            // cerr << "\n";

            if (!src_available) {
                return nullopt;
            }

            std::discrete_distribution<> src_d(begin(src_weights), end(src_weights));
            std::discrete_distribution<> dest_d(begin(dest_weights), end(dest_weights));

            move_t action;
            action.src = src_d(gen);
            do { // Don't send to self
                action.dest = dest_d(gen);
            } while (action.dest == action.src);

            // cerr << "Will move from " << action.src << " to " << action.dest << "\n";
            // cerr << "Generating random count in range [" << 1 << ", " << state.factories[action.src].cyborgs << "]\n";

            uniform_int_distribution<> cyborgs_d(1, state.factories[action.src].cyborgs);

            action.count = cyborgs_d(gen);

            // cerr << "Chose " << action.count << "\n";

            return { action };
        }
    };

    struct bomb_t {
        int src, dest;

        bomb_t(int src, int dest) : src(src), dest(dest) { /* empty */ }

        static optional<bomb_t> random(const state_t& state) {
            if (state.agent_bombs_left == 0) {
                return nullopt;
            }

            // Weights for random distribution used to pick src and dest factories.
            // Vector `src_weights` consist of 0s and 1s for enemy's and agent's factories, respectively - agent can move cyborgs only from their factory.
            // Vector `dest_weights` consists of 1s and 0s for enemy's and agent's factories, respectively - we want to bomb enemy's factory.
            vector<int> src_weights(state.factories.size());
            vector<int> dest_weights(state.factories.size());

            bool src_available = false;
            bool dest_available = false;

            for (size_t i = 0; i < state.factories.size(); i++) {
                const factory_t factory = state.factories[i];

                if (factory.owner == agent) {
                    src_weights[i] = 1;
                    src_available = true;
                } else {
                    dest_weights[i] = 1;
                    dest_available = true;
                }
            }

            if (!src_available || !dest_available) {
                return nullopt;
            }

            std::discrete_distribution<> src_d(begin(src_weights), end(src_weights));
            std::discrete_distribution<> dest_d(begin(dest_weights), end(dest_weights));

            bomb_t action(src_d(gen), dest_d(gen));
            return { action };
        }
    };

    struct inc_t {
        int factory_id;

        inc_t(int id) : factory_id(id) { /* empty */ }

        static optional<inc_t> random(const state_t& state) {
            // Weights for random distribution used to pick src and dest factories.
            // Vector `src_weights` consist of 0s and 1s for enemy's and agent's factories, respectively - agent can move cyborgs only from their factory.
            vector<int> weights(state.factories.size());

            bool available = false;

            for (size_t i = 0; i < state.factories.size(); i++) {
                const factory_t factory = state.factories[i];

                if (factory.owner == agent && factory.cyborgs >= 10 && factory.normal_prod < 3) {
                    weights[i] = 1;
                    available = true;
                }
            }

            if (!available) {
                return nullopt;
            }

            std::discrete_distribution<> d(begin(weights), end(weights));

            inc_t action(d(gen));
            return { action };
        }
    };

    struct wait_t {};

    struct msg_t {
        string text;

        msg_t(string t) : text(t) { /* empty */ }
    };

    typedef std::variant<move_t, bomb_t, inc_t, wait_t, msg_t> action_t;

    void print_action(ostream &out, const action_t &action);

    static action_t random(const state_t& state) {
        // Random action only takes from troop moves, bombs and increments
        // unless no action of these types is possible.
        std::discrete_distribution<> d({ 3, 1, 1 });

        // cerr << "Random action generation...\n";
        // print_state(state);

        bool cant_move = false;
        bool cant_bomb = state.agent_bombs_left == 0;
        bool cant_inc = false;

        while (true) {
            if (cant_move && cant_bomb && cant_inc) {
                return { msg_t("No action possible") };
            }

            auto action_no = d(gen);

            if (action_no == 0) { // move
                // cerr << "Random move action generation...\n";
                if (auto ret = move_t::random(state); ret.has_value()) {
                    // print_state(state);
                    // print_action(cerr, { ret.value() }); cerr << "\n";
                    return { ret.value() };
                }
                else {
                    cant_move = true;
                    continue;
                }
            }
            else if (action_no == 1) { // bomb
                // cerr << "Random bomb action generation...\n";
                if (auto ret = bomb_t::random(state); ret.has_value()) {
                    return { ret.value() };
                }
                else {
                    cant_bomb = true;
                    continue;
                }
            }
            else if (action_no == 2) { // inc
                // cerr << "Random inc action generation...\n";
                if (auto ret = inc_t::random(state); ret.has_value()) {
                    return { ret.value() };
                }
                else {
                    cant_inc = true;
                    continue;
                }
            }
            else {
                throw runtime_error(
                    "While generating random action, distribution returned "
                    "unexpected value"
                );
            }
        }
    }
};

using action_t = action::action_t;

void perform_action(state_t& state, const player_t& player, const action::action_t& action);

struct gamemove_t {
    vector<action::action_t> actions;

    static gamemove_t random(const state_t& init_state, int alpha = 3) {
        state_t state = init_state;

        poisson_distribution<> d(alpha);
        auto action_count = d(gen);

        gamemove_t ret;
        ret.actions.reserve(action_count);
        for (int i = 0; i < action_count; i++) {
            const auto action = action::random(state);
            if (auto mmsg = get_if<action::msg_t>(&action); mmsg && mmsg->text == "No action possible") {
                break;
            }

            ret.actions.push_back(action);
            perform_action(state, agent, ret.actions.back());
        }

        return ret;
    }
};

void print_gamemove(ostream &out, const gamemove_t& gamemove);

void perform_gamemove(state_t& state, const player_t& player, const gamemove_t& move) {
    // Advance troops and bomb
    for_each(begin(state.troops), end(state.troops), [](troop_t& troop) {
        troop.distance--;
        });

    for_each(begin(state.bombs), end(state.bombs), [](bomb_t& bomb) {
        bomb.distance--;
        });

    for_each(begin(state.factories), end(state.factories), [](factory_t& factory) {
        if (factory.until_normal > 0)
            factory.until_normal--;
        });

    // @Note: Referee doesn't execute actions in order, but instead will
    // execute all move actions first, then bombs and finally inc's.
    // I don't know how much it matters.
    for (auto action : move.actions) {
        perform_action(state, player, action);
    }

    for_each(begin(state.factories), end(state.factories), [](factory_t& factory) {
        if (factory.owner != nobody)
            factory.cyborgs += factory.production();
        });

    vector<int> agent_arriving_cyborgs(state.factories.size()),
        enemy_arriving_cyborgs(state.factories.size());

    for_each(begin(state.troops), end(state.troops), [&](troop_t& troop) {
        if (troop.distance > 0)
            return;

        switch (troop.owner) {
        case agent:
            agent_arriving_cyborgs[troop.target_factory] += troop.size;
            break;
        case enemy:
            enemy_arriving_cyborgs[troop.target_factory] += troop.size;
            break;
        default:
            throw runtime_error("Nobody cannot send troops!");
            break;
        }
        });

    for (size_t i = 0; i < state.factories.size(); i++) {
        factory_t& factory = state.factories[i];

        // Fight between arriving troops
        player_t winner = nobody;
        int units = min(agent_arriving_cyborgs[i], enemy_arriving_cyborgs[i]);
        int arriving_units;
        if (agent_arriving_cyborgs[i] > units) {
            winner = agent;
            arriving_units = agent_arriving_cyborgs[i] - units;
        }
        else if (enemy_arriving_cyborgs[i] > units) {
            winner = enemy;
            arriving_units = enemy_arriving_cyborgs[i] - units;
        }

        if (winner == nobody)
            continue;

        if (factory.owner == winner) {
            // Winner reinforces their factory
            factory.cyborgs += arriving_units;
            continue;
        }
        else {
            // Winner fights to take over the factory
            if (arriving_units > factory.cyborgs) {
                factory.owner = winner;
                factory.cyborgs = arriving_units - factory.cyborgs;
            }
            else {
                factory.cyborgs -= arriving_units;
            }
        }
    }

    remove_if(begin(state.bombs), end(state.bombs), [&](const bomb_t& bomb) {
        if (bomb.distance > 0 || bomb.target_factory == -1)
            return false; // don't remove

        factory_t& factory = state.factories[bomb.target_factory];
        factory.cyborgs = max(0, min(10, factory.cyborgs / 2));
        factory.until_normal = 5;

        return true; // remove
        });

    remove_if(begin(state.troops), end(state.troops), [](const troop_t& troop) {
        return troop.distance <= 0;
        });

    // @Note: Not checking game's end conditions
}

float agent_score(const state_t& state) {
    int agent_prod = 0, enemy_prod = 0;
    int agent_factoried_cyborgs = 0, enemy_factoried_cyborgs = 0;
    int agent_trooped_cyborgs = 0, enemy_trooped_cyborgs = 0;

    for_each(begin(state.factories), end(state.factories), [&](const factory_t& factory) {
        switch (factory.owner) {
        case agent:
            agent_factoried_cyborgs += factory.cyborgs;
            agent_prod += factory.normal_prod;
            break;
        case enemy:
            enemy_factoried_cyborgs += factory.cyborgs;
            enemy_prod += factory.normal_prod;
            break;
        default:
            break;
        }
    });

    for_each(begin(state.troops), end(state.troops), [&](const troop_t& troop) {
        switch (troop.owner) {
        case agent:
            agent_trooped_cyborgs += troop.size;
            break;
        case enemy:
            enemy_trooped_cyborgs += troop.size;
            break;
        default:
            break;
        }
    });

    return float(agent_prod * agent_prod + agent_factoried_cyborgs + agent_trooped_cyborgs * 0.5)
        / float(agent_prod * agent_prod + enemy_factoried_cyborgs + agent_trooped_cyborgs * 0.5);
}

template<size_t sz>
class Chromosome {
    deque<gamemove_t> moves;
    state_t init_state, end_state;
    float score;

    Chromosome(state_t init_state, deque<gamemove_t> m, state_t end_state, float s)
        : moves(m), init_state(init_state), end_state(end_state), score(s) { /* empty */
    }

public:

    Chromosome(deque<gamemove_t> m, state_t init_state)
        : moves(m), score(-1.0f), init_state(init_state) {
        this->compute_score();
    }

    static Chromosome random(const state_t& init_state) {
        state_t state = init_state;
        deque<gamemove_t> moves;

        for (size_t i = 0; i < sz; i++) {
            moves.push_back(gamemove_t::random(state));
            // print_gamemove(cerr, moves.back());
            perform_gamemove(state, agent, moves.back());
        }

        float score = agent_score(state);

        return Chromosome(init_state, moves, state, score);
    }

    deque<gamemove_t> get_moves() const {
        return moves;
    }

    float get_score() const {
        return score;
    }

    state_t get_init_state() const {
        return init_state;
    }

    state_t get_end_state() const {
        return end_state;
    }

    void roll() {
        perform_gamemove(init_state, agent, moves.front());
        moves.pop_front();
        add_gamemove();
    }

private:

    void add_gamemove() {
        moves.push_back(gamemove_t::random(end_state));
        perform_gamemove(end_state, agent, moves.back());
        score = agent_score(end_state);
    }

    void compute_score() {
        state_t state = init_state;

        for (const auto& move : moves) {
            perform_gamemove(state, agent, move);
        }

        end_state = state;
        score = agent_score(end_state);
    }
};

template<size_t sz>
deque<gamemove_t> crossover(const Chromosome<sz>& chrom1, const Chromosome<sz>& chrom2) {
    // @Note: This crossover mechanism most likely won't preserve
    // the validity of the gamemoves and ones from the second chromosome
    // will probably not be valid after the gamemoves from the first chromosome
    // are executed.
    const auto& moves1 = chrom1.get_moves();
    const auto& moves2 = chrom2.get_moves();

    size_t cross_pt = uniform_int_distribution<>(1, sz - 2)(gen);
    deque<gamemove_t> moves;
    for (size_t i = 0; i < cross_pt; i++) {
        moves.push_back(moves1[i]);
    }
    for (size_t i = cross_pt; i < sz; i++) {
        moves.push_back(moves2[i]);
    }
    return moves;
}

template<size_t sz>
Chromosome<sz> mutate(const state_t& init_state, deque<gamemove_t> moves) {
    // @TODO: When we generate random actions/moves we are very much driven
    // by state and validity of actions in said state. However, this mutation
    // most likely won't preserve the validity.
    // And alternative approach would be to select mutation point at random
    // but mutate it only slightly and hope for the best
    // or subsequently try to fix all following moves.
    const state_t state = init_state; // @TODO: Do something with this

    size_t mut_pt = uniform_int_distribution<>(0, sz - 1)(gen);
    moves[mut_pt] = gamemove_t::random(state);
    return Chromosome<sz>(moves, init_state);
}

template<size_t sz>
Chromosome<sz> mutate_crossover(const state_t& init_state, const Chromosome<sz>& chrom1, const Chromosome<sz>& chrom2) {
    // @TODO: Pass proper state
    return mutate<sz>(init_state, crossover<sz>(chrom1, chrom2));
}

template<size_t sz>
vector<Chromosome<sz>> selection(vector<Chromosome<sz>> chroms) {
    // float GRADED_RETAIN_PERCENT = 0.3; // elitism
    int ELITISM_SIZE = 1;
    float NONGRADED_RETAIN_PERCENT = 0.2;

    sort(begin(chroms), end(chroms), [](const Chromosome<sz>& lhs, const Chromosome<sz>& rhs) {
        return lhs.get_score() < rhs.get_score();
    });

    // int threshold = (int)(chroms.size() * (1 - GRADED_RETAIN_PERCENT));
    int threshold = chroms.size() - ELITISM_SIZE;

    vector<Chromosome<sz>> results;
    copy(begin(chroms) + threshold, end(chroms), back_inserter(results));
    sample(begin(chroms), begin(chroms) + threshold, back_inserter(results), int(NONGRADED_RETAIN_PERCENT * threshold), gen);

    return results;
}

void perform_action(state_t& state, const player_t& player, const action::action_t& action) {
    if (auto mmove = get_if<action::move_t>(&action)) {
        int of = mmove->src;

        // @TODO: Wrap in debugging flag
        if (state.factories[of].owner != player) {
            throw runtime_error(
                "Move action performed from factory not owned by the player"
            );
        }
        else if (state.factories[of].cyborgs < mmove->count) {
            throw runtime_error(
                "Move action requires more cyborgs than available"
            );
        }

        state.factories[of].cyborgs -= mmove->count;
        state.troops.push_back(troop_t(player, of, mmove->dest, mmove->count));
    }
    else if (auto mbomb = get_if<action::bomb_t>(&action)) {
        int of = mbomb->src;

        // @TODO: Wrap in debugging flag
        if (state.factories[of].owner != player) {
            throw runtime_error(
                "Bomb action performed from factory not owned by the player"
            );
        }
        else if (player == agent && state.agent_bombs_left == 0) {
            throw runtime_error(
                "Bomb action requires bomb available but agent has none left"
            );
        }
        else if (player == enemy && state.enemy_bombs_left == 0) {
            throw runtime_error(
                "Bomb action requires bomb available but agent has none left"
            );
        }

        if (player == agent) {
            state.agent_bombs_left--;
        }
        else {
            state.enemy_bombs_left--;
        }

        state.bombs.push_back(bomb_t(player, of, mbomb->dest));
    }
    else if (auto minc = get_if<action::inc_t>(&action)) {
        int of = minc->factory_id;

        // @TODO: Wrap in debugging flag
        if (state.factories[of].owner != player) {
            throw runtime_error(
                "Inc action performed from factory not owned by the player"
            );
        }
        else if (state.factories[of].cyborgs < 10) {
            throw runtime_error(
                "Inc action requires more cyborgs than available"
            );
        }
        else if (state.factories[of].normal_prod >= 3) {
            throw runtime_error(
                "Inc action requires normal production of factory to be less than 3"
            );
        }

        state.factories[of].normal_prod++;
        state.factories[of].cyborgs -= 10;
    }
    else {
        // MSG and WAIT don't change game state
        return;
    }
}

state_t read_state_input() {
    cerr << "Reading input...\n";
    state_t read_state;

    int entity_cnt;
    cin >> entity_cnt;
    cerr << "Will read " << entity_cnt << " entities\n";
    for (int i = 0; i < entity_cnt; i++) {
        int id, ignored;
        string typ;
        cin >> id >> typ;
        if (typ == "FACTORY") {
            // cerr << "Read a factory!\n";
            int owner, cyborgs, prod, tt_norm_prod;
            cin >> owner >> cyborgs >> prod >> tt_norm_prod >> ignored;

            read_state.factories.push_back(factory_t());
            read_state.factories.back().owner = int_to_player(owner);
            read_state.factories.back().cyborgs = cyborgs;
            read_state.factories.back().normal_prod = prod;
            read_state.factories.back().until_normal = tt_norm_prod;
        }
        else if (typ == "TROOP") {
            // cerr << "Read a troop!\n";
            int owner, of, tf, sz, dist;
            cin >> owner >> of >> tf >> sz >> dist;

            read_state.troops.push_back(troop_t(int_to_player(owner), of, tf, sz));
            read_state.troops.back().distance = dist;
        }
        else if (typ == "BOMB") {
            // cerr << "Read a bomb!\n";
            int owner, of, tf, dist;
            cin >> owner >> of >> tf >> dist >> ignored;

            read_state.bombs.push_back(bomb_t(int_to_player(owner), of, tf));
            read_state.bombs.back().distance = dist;
        }
        else {
            // Unexpected value
            cerr << "Unexpected value for typ: '" << typ << "'!\n";
        }
    }

    cerr << "Read input\n";
    return read_state;
}

template<typename T>
T choice(const vector<T>& options) {
    std::uniform_int_distribution<int> unif_dist(0, options.size() - 1);
    return options[unif_dist(gen)];
}

void action::print_action(ostream &out, const action_t &action) {
    if (auto mmove = get_if<action::move_t>(&action)) {
        out << "MOVE " << mmove->src << " " << mmove->dest << " " << mmove->count;
    } else if (auto mbomb = get_if<action::bomb_t>(&action)) {
        out << "BOMB " << mbomb->src << " " << mbomb->dest;
    } else if (auto minc = get_if<action::inc_t>(&action)) {
        out << "INC " << minc->factory_id;
    } else if (auto mmsg = get_if<action::msg_t>(&action)) {
        out << "MSG " << mmsg->text;
    } else if (holds_alternative<action::wait_t>(action)) {
        out << "WAIT";
    }
}

void print_gamemove(ostream &out, const gamemove_t &gamemove) {
    bool first = true;
    cerr << "About to print the game move!\n";

    if (gamemove.actions.size() == 0) {
        action::print_action(out, { action::msg_t("No move possible") });
        out << "\n";
        return;
    }

    for_each(begin(gamemove.actions), end(gamemove.actions), [&](const action_t& action) {
        if (first) {
            first = false;
        } else {
            out << "; ";
        }

        print_action(out, action);
    });
    out << "\n";
    cerr << "Done\n";
}

void print_state(const state_t& state) {
    cerr << "-----BEGIN STATE-----\n";

    cerr << "Factories:\n";
    for_each(begin(state.factories), end(state.factories), [](const factory_t& f) {
        string owner = player_to_string(f.owner);
        cerr << owner << " " << f.cyborgs << " " << f.production();
        if (f.until_normal > 0) {
            cerr << " " << f.normal_prod << " (" << f.until_normal << ")";
        }
        cerr << "\n";
        });

    cerr << "Troops:\n";
    for_each(begin(state.troops), end(state.troops), [](const troop_t& t) {
        cerr << player_to_string(t.owner) << ": " << t.origin_factory << " --[" << t.size << "]> " << t.target_factory << " in " << t.distance << "\n";
    });

    cerr << "Bombs:\n";
    for_each(begin(state.bombs), end(state.bombs), [](const bomb_t& b) {
        cerr << player_to_string(b.owner) << ": " << b.origin_factory << " --> " << b.target_factory << " in " << b.distance << "\n";
    });

    cerr << "Bombs left: " << state.agent_bombs_left << ", " << state.enemy_bombs_left << "\n";

    cerr << "------END STATE------\n";
}

// @TODO: Find good values
const int CHROM_SZ = 5;
const int POP_SZ = 50;

state_t rhea() {
    // Initialization
    const state_t read_state = read_state_input();

    const state_t state = read_state;

    // Chromosome<CHROM_SZ>::random(read_state);
    // cerr << "Done\n";

    vector<Chromosome<CHROM_SZ>> population;
    for (int i = 0; i < POP_SZ; i++) {
        population.push_back(Chromosome<CHROM_SZ>::random(read_state));
    }

    // Game loop
    state_t local_state = state;
    for (int timer = 0; true /* @TODO: End game condition */; timer++) {
        cerr << "loop\n";
        // print_state(local_state);
        float max_score = numeric_limits<float>::min();
        float scores = 0.0;

        chrono::nanoseconds run_time = 0ns;
        for (int generation = 0; run_time < 30ms; generation++) {
            cerr << "Generation " << generation << "\n";
            auto start = high_resolution_clock::now();
            // Chromosome scoring
            for (const auto& chrom : population) {
                scores += chrom.get_score();
                max_score = max(max_score, chrom.get_score());
            }

            // Selection
            const auto selected = selection(population);
            auto new_population = selected;

            // Mutation, crossover
            while (new_population.size() < population.size()) {
                new_population.push_back(Chromosome<CHROM_SZ>::random(local_state));
                // auto p1 = choice(selected);
                // auto p2 = choice(selected);
                // if (p1.get_score() > p2.get_score()) {
                //     new_population.push_back(p1);
                // } else {
                //     new_population.push_back(p2);
                // }
                // // new_population.push_back(mutate_crossover(local_state, p1, p2));
            }

            // New population
            population = new_population;

            run_time += high_resolution_clock::now() - start;
        }

        // Select best chromosome
        auto best = *max_element(begin(population), end(population), [](const auto& lhs, const auto& rhs) {
            return lhs.get_score() < rhs.get_score();
        });

        if (population.size() == 0 || best.get_moves().size() == 0) {
            cerr << "Something is empty!\n";
        }

        // Print out it's first gamemove
        print_gamemove(cout, best.get_moves().front());

        local_state = read_state_input();
    }

    cerr << "Returning...\n";

    return state;
}

int main() {
    // cerr << "hello world\n";

    int n, m;
    cin >> n >> m;
    graph.resize(n);

    // cerr << "there are " << n << " factories with " << m << " links\n";

    for (int i = 1; i <= m; i++) {
        // cerr << "link " << i << "\n";
        int f1, f2, dist;
        cin >> f1 >> f2 >> dist; cin.ignore();
        graph[f1].push_back(edge_t(f2, dist));
        graph[f2].push_back(edge_t(f1, dist));
        // cerr << "] " << f1 << " " << f2 << " " << dist << "\n";
    }

    // cerr << "before rhea\n";

    rhea();
}