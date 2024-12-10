#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include "nlohmann/json.hpp"

using std::string;
using std::unique_ptr;
using std::make_unique;
using std::vector;
using std::array;

using board = vector<vector<int>>;
using katanuki = vector<vector<bool>>;
using array_action = vector<int>;
using json = nlohmann::json;

enum direction
{
    up,
    down,
    left,
    right
};

struct action
{
    int katanuki;
    int y = 0;
    int x = 0;
    direction direct;
};

class kyougi_app
{
public:
    kyougi_app();
    kyougi_app(const string&);
    kyougi_app(const json&);
    
    void set_start(const board&);
    void set_goal(const board&);
    void reset_board();
    
    void set_game(const json&); 
    void set_game(const string&); 
    
    void generate_general_katanuki();
    void add_katanuki(const katanuki&);
    
    //void step(const action&);
    void step(const action&);
    void reverse_step(const action&);

    inline void shift_step(const action&);

    void write_board() const;
    void write_katanuki() const;
    
    katanuki get_katanuki(const int&);
    board get_board();
    board get_goal();

    int cnt;
    
    vector<katanuki> katanuki_;
    board board_;
    bool rot;
private:
    board start_;
    board goal_;
};
