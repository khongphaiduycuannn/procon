// #pragma once

#include<bits/stdc++.h>
#include"../include/header/kyougi_app.h"

class ISolver{
public:
    virtual void init() = 0;
    
    virtual void set_problem(const string& game_string) = 0;
    virtual void solve() = 0;
    virtual string get_answer() = 0;
    virtual bool move_check() = 0;
    string answer_string;    
};

class Solver: public ISolver{
public:
    void init() override;
    void set_problem(const string& game_string) override;
    void solve() override;
    string get_answer() override;
    bool move_check() override;

    string answer_string;

private:

    kyougi_app my_app;
    int H,W;
    board S,G;
    string S_string;
    int Rl;
    vector<int> bitp;
    vector<vector<vector<unsigned long long>>> randomBits;
    vector<action> solve1_actions,solve2_actions,solve3_actions;

    void randomBitsInit();
    std::pair<board,vector<action>> solve1();
    std::pair<board,vector<action>> solve2();
    std::pair<board,vector<action>> solve3();

    struct action_data{
    public:
        int score;
        int dist;
        action act;
        int ct; // 時間
        int line;
        int siz;
    };

    struct line_action{
    public:
        int score;
        int dist;
        int y;
        int x;
    };

    class State{
    public:
        State(int h,int w,board b,vector<int> ti,unsigned long long zh):
            H(h),W(w),time(ti),zobrist_hash(zh),plus(0),now_board(b)
        {}

        int get_turn() const{
            return (int)move_history.size();
        }

        int H,W;
        board now_board;
        vector<int> time;
        vector<action> move_history;
        int plus;

    private:
        unsigned long long zobrist_hash;
    };
    
    
        struct action_data1:public action_data{
        public:
            bool operator <(const action_data1& d) const{
                if(score==d.score){
                    if(dist==d.dist)return act.y<d.act.y;
                    else return dist<d.dist; // おおさいほうが優先
                }else return score<d.score;
            }

            bool operator >(const action_data1& d) const{
                if(score==d.score){
                    if(dist==d.dist)return act.y>d.act.y;
                    else return dist>d.dist; // おおきいほうが優先
                }else return score>d.score;
            }
        };

        struct line_action1:public line_action{
        public:
            bool operator <(const line_action1& d) const{
                if(score==d.score){
                    if(dist==d.dist){
                        return y<d.y;
                    }else return dist<d.dist;
                }else return score<d.score;
            }

            bool operator >(const line_action1& d) const{
                if(score==d.score){
                    if(dist==d.dist){
                        return y>d.y;
                    }else return dist>d.dist;
                }else return score>d.score;
            }
        };

        class State1:public State{
        public:
            vector<int> Rp;

            State1(int h,int w,board b,vector<int> ti,unsigned long long zh,vector<int> rp):
                State(h,w,b,ti,zh),Rp(rp)
            {}

            // trueなら完成
            bool apply_move(const int& H,const int& W,kyougi_app& my_app,const int& Rl,const board& Rg,const int sp,const vector<vector<vector<unsigned long long>>>& randomBits,const vector<int>& bitp);
        };

        class Node1{
        public:
            State1 state;
            bool is_goal;

            Node1(State1 s,bool g):state(s),is_goal(g){}

            float get_score() const{
                return (float)state.plus/(float)state.get_turn();
            }
            void advance(const int& H,const int& W,kyougi_app& my_app,const int& Rl,const board& Rg,const int sp,const vector<vector<vector<unsigned long long>>>& randomBits,const vector<int>& bitp){
                is_goal=state.apply_move(H,W,my_app,Rl,Rg,sp,randomBits,bitp);
            }
            bool operator <(const Node1& n) const{
                return get_score()<n.get_score();
            }
        };
    
    struct action_data2:public action_data{
    public:
        int height;
        int width;

        action_data2(int s,int d,action a,int c,int l,int sz,int h,int w):
            action_data({s, d, a, c, l, sz}), height(h), width(w)
        {}

        action_data2(){}

        bool operator <(const action_data2& d) const{
            if(score==d.score){
                if(dist==d.dist)return act.x<d.act.x;
                else return dist<d.dist; // おおさいほうが優先
            }else return score<d.score;
        }

        bool operator >(const action_data2& d) const{
            if(score==d.score){
                if(dist==d.dist)return act.x>d.act.x;
                else return dist>d.dist; // おおきいほうが優先
            }else return score>d.score;
        }
    };

    struct line_action2:public line_action{
    public:
        bool operator <(const line_action2& d) const{
            if(score==d.score){
                if(dist==d.dist){
                    return x<d.x;
                }else return dist<d.dist;
            }else return score<d.score;
        }

        bool operator >(const line_action2& d) const{
            if(score==d.score){
                if(dist==d.dist){
                    return x>d.x;
                }else return dist>d.dist;
            }else return score>d.score;
        }
    };

    class State2:public State{
    public:
        vector<int> Lp;

        State2(int H,int W,board b,vector<int> ti,unsigned long long zh,vector<int> lp):
            State(H,W,b,ti,zh),Lp(lp)
        {}

        // trueなら完成
        bool apply_move(const int& H,const int& W,kyougi_app& my_app,const int& Ll,const board& Lg,const int sp,const vector<vector<vector<unsigned long long>>>& randomBits,const vector<int>& bitp);
    };

    class Node2{
    public:
        State2 state;
        bool is_goal;

        Node2(State2 s,bool g):state(s),is_goal(g){}

        float get_score() const{
            return (float)state.plus/(float)state.get_turn();
        }
        void advance(const int& H,const int& W,kyougi_app& my_app,const int& Rl,const board& Rg,int sp,const vector<vector<vector<unsigned long long>>>& randomBits,const vector<int>& bitp){
            is_goal=state.apply_move(H,W,my_app,Rl,Rg,sp,randomBits,bitp);
        }
        bool operator <(const Node2& n) const{
            return get_score()<n.get_score();
        }
    };
};