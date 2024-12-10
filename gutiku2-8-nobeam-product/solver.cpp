#include"solver.hpp"

#ifdef LOCAL
#  include "../include/header/debug_print.hpp"
#  define debug(...) debug_print::multi_print(#__VA_ARGS__, __VA_ARGS__)
#else
#  define debug(...) (static_cast<void>(0))
#endif

void Solver::init(){
    bitp.resize(9);
    bitp[0]=0;
    bitp[1]=1;
    bitp[2]=4;
    bitp[3]=7;
    bitp[4]=10;
    bitp[5]=13;
    bitp[6]=16;
    bitp[7]=19;
    bitp[8]=22;
}

void Solver::set_problem(const string& game_string){
    json game_json=json::parse(game_string);
    S_string=game_string;

    my_app.set_game(game_json);

    S=my_app.board_;
    G=my_app.get_goal();
    H=my_app.board_.size();
    W=my_app.board_[0].size();

    Rl=W-(int)W/6.6;
    randomBitsInit();
}

string Solver::get_answer(){
    json Ans;
    int steps=(solve1_actions.size()+solve2_actions.size()+solve3_actions.size());
    debug(steps);
    Ans["n"]=steps;
    for(int i=0;i<solve1_actions.size();i++){
        json query;
        query["p"]=solve1_actions[i].katanuki;
        query["x"]=solve1_actions[i].x;
        query["y"]=solve1_actions[i].y;
        query["s"]=solve1_actions[i].direct;
        Ans["ops"].push_back(query);
    }
    for(int i=0;i<solve2_actions.size();i++){
        json query;
        query["p"]=solve2_actions[i].katanuki;
        query["x"]=solve2_actions[i].x;
        query["y"]=solve2_actions[i].y;
        query["s"]=solve2_actions[i].direct;
        Ans["ops"].push_back(query);
    }
    for(int i=0;i<solve3_actions.size();i++){
        json query;
        query["p"]=solve3_actions[i].katanuki;
        query["x"]=solve3_actions[i].x;
        query["y"]=solve3_actions[i].y;
        query["s"]=solve3_actions[i].direct;
        Ans["ops"].push_back(query);
    }
    answer_string=Ans.dump();
    return answer_string;
}

void Solver::solve(){
    auto [board1,actions1]=solve1();
    auto [board2,actions2]=solve2();
    auto [board3,actions3]=solve3();
    solve1_actions=actions1;
    solve2_actions=actions2;
    solve3_actions=actions3;
    // if(move_check()){
    //     return;
    // }else{
    //     exit(1);
    // }
}


bool Solver::move_check(){
    kyougi_app my_app2;
    my_app2.set_game(S_string);
    //debug(solve1_actions.size());
    for(auto act:solve1_actions){
        my_app2.step(act);
    }
    for(auto act:solve2_actions){
        my_app2.step(act);
    }
    for(auto act:solve3_actions){
        my_app2.step(act);
    }
    return my_app2.board_==G;
}

bool Solver::State1::apply_move(const int& H,const int& W,kyougi_app& my_app,const int& Rl,const board& Rg,const int sp,const vector<vector<vector<unsigned long long>>>& randomBits,const vector<int>& bitp){
    my_app.board_=now_board;
    int l=0,r=H;
    int kat_num=6;
    int dx=0;

    vector<int> use_katanuki(kat_num+1,0);

    std::unordered_map<unsigned long long,std::set<int>> board_map;
    std::priority_queue<action_data1> Q;

    std::priority_queue<line_action1,vector<line_action1>,std::greater<line_action1>> kat_dist;

    while(1){
        int now_turn=get_turn();
        //debug(now_turn);
        for(int i=l;i<r;i++){
            // 時間を管理
            time[i]=now_turn;
            // Right
            if(Rp[i]<Rl){
                for(int j=dx;j<W-Rp[i];j++){
                    // 使う型抜きは0-6
                    for(int k=0;k<=kat_num;k++){
                        const katanuki& kat=my_app.katanuki_[k];
                        // 動かしても周りに影響が出ないか確認
                        bool ng=0;
                        for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                            if(W-Rp[p]<j+(int)kat[p-i].size())ng=1;
                        }
                        if(ng)continue; // 定型型抜きならbreakでok

                        // vector<vector<int>> now_want(kat.size());
                        unsigned long long hash=0;

                        for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                            int cp=0;
                            for(int q=j;q<std::min(j+(int)kat[p-i].size(),W);q++){
                                if(kat[p-i][q-j]){
                                    if(Rp[p]+cp>=Rl){
                                        ng=1;
                                        goto out;
                                    }
                                    hash^=randomBits[p-i][q-j][my_app.board_[p][q]];
                                    // now_want[p-i].push_back(my_app.board_[p][q]);
                                    cp++;
                                }else{
                                    // now_want[p-i].push_back(-1);
                                }
                            }
                        }
                        out:;
                        if(!ng){
                            board_map[hash].insert(i*W+j);
                        }
                    }
                }
            }
        }
        // debug(2);

        for(int i=0;i<H;i++){
            for(int k=0;k<=kat_num;k++){
                bool ng=0;
                katanuki& kat=my_app.katanuki_[k];
                // vector<vector<int>> want(kat.size());
                int cnt=0;
                unsigned long long hash=0;
                for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                    int cp=0;
                    for(int q=0;q<(int)kat[p-i].size();q++){ // qは0に注意
                        if(kat[p-i][q]){
                            if(Rp[p]+cp>=Rl){
                                ng=1;
                                goto kat_out;
                            }
                            hash^=randomBits[p-i][q][Rg[p][Rp[p]+cp]];
                            // want[p-i].push_back(Rg[p][Rp[p]+cp]);
                            cp++;
                        }else{
                            // want[p-i].push_back(-1);
                        }
                    }
                    cnt+=cp;
                }
                kat_out:;
                if(ng){
                    continue;
                }
                if(board_map.count(hash)){
                    // debug(hash);
                    const std::set<int>& top_vec=board_map[hash];
                    // debug(top_vec.size());
                    auto itr=top_vec.lower_bound(i*W);
                    if(itr!=top_vec.end()&&*itr<(i+1)*W){
                        int top=*itr;
                        int tf=top/W; int ts=top%W;
                        if(tf==i){
                            // 次のピースは何ピース持ってこられる？
                            Q.push({2*cnt,-Rp[i],{k,i,ts,left},now_turn,i,-1});
                        }
                    }
                        
                    for(const int& top:top_vec){
                        int tf=top/W; int ts=top%W;
                        for(int p=0;p<H;p++){
                            if(W-Rp[p]<ts+(int)kat[0].size()){
                                ng=1;
                            }
                        }
                        if(ng)continue;
                        // int dist=0;
                        Q.push({cnt,-Rp[i],{k,i,ts,left},now_turn,tf,(int)kat[0].size()});
                        break;
                    }
                }
            }
        }

        {

            action nex={-1,-1,-1,down};
            action_data1 d;
            int nex_score=-1;
            int c=0;
            vector<action_data1> skip;
            while(!Q.empty()){ // emptyのときはcは無視される
                nex_score=Q.top().score; // ピース数
                d=Q.top();
                Q.pop();
                bool ok=1;
                // TODO: 他行から持ってくる場合の範囲を設定すること
                if(d.siz==-1){
                    for(int p=std::max(d.act.y-(int)my_app.katanuki_[d.act.katanuki].size()+1,0);p<std::min(d.act.y+(int)my_app.katanuki_[d.act.katanuki].size(),H);p++){
                        if(d.ct<time[p]){
                            ok=0;
                        }
                    }
                }else{
                    if(d.ct<now_turn){
                        ok=0;
                    }
                }
                
                if(ok){
                    if(c==sp){
                        nex=d.act;
                        break;
                    }else{
                        c++;
                        skip.push_back(d);
                    }
                }
            }

            for(const auto& d:skip){
                Q.push(d);
            }

            if(nex.katanuki!=-1){
                if(nex.y!=d.line){
                    if(nex.y>d.line){
                        int nex_y=H-(nex.y-d.line);
                        int nex_x=nex.x+d.siz-256;
                        // debug(1,d.score,nex_y,nex_x);
                        my_app.step((action){22,nex_y,nex_x,down});
                        move_history.push_back((action){22,nex_y,nex_x,down});
                    }else{
                        int nex_y=(d.line-nex.y)-256;
                        int nex_x=nex.x+d.siz-256;
                        // debug(2,d.score,nex_y,nex_x);
                        my_app.step((action){22,nex_y,nex_x,up});
                        move_history.push_back((action){22,nex_y,nex_x,up});
                    }

                    // debug(nex.y,nex.x);

                    l=0,r=H;
                    dx=0;
                    Q=decltype(Q)();
                    board_map.clear();
                }else{
                    // FIXME: 8と決めつけるのは危険なので後で直す
                    l=std::max(nex.y-8,0),r=std::min(nex.y+(int)my_app.katanuki_[nex.katanuki].size(),H);
                    dx=std::max(nex.x-8,0);
                    for(int i=l;i<r;i++){
                        // Right
                        if(Rp[i]<Rl){
                            for(int j=dx;j<W-Rp[i];j++){
                                // 使う型抜きは0-6
                                for(int k=0;k<=kat_num;k++){
                                    katanuki& kat=my_app.katanuki_[k];
                                    // 動かしても周りに影響が出ないか確認
                                    bool ng=0;
                                    for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                                        if(W-Rp[p]<j+(int)kat[p-i].size())ng=1;
                                    }
                                    if(ng)continue; // 定型型抜きならbreakでok

                                    // vector<vector<int>> now_want(kat.size());
                                    unsigned long long hash=0;

                                    for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                                        int cp=0;
                                        for(int q=j;q<std::min(j+(int)kat[p-i].size(),W);q++){
                                            if(kat[p-i][q-j]){
                                                if(Rp[p]+cp>=Rl){
                                                    ng=1;
                                                    goto out2;
                                                }
                                                // now_want[p-i].push_back(my_app.board_[p][q]);
                                                hash^=randomBits[p-i][q-j][my_app.board_[p][q]];
                                                cp++;
                                            }else{
                                                // now_want[p-i].push_back(-1);
                                            }
                                        }
                                    }
                                    out2:;
                                    if(!ng){
                                        // debug(i,j);
                                        // 違う型、同じボードを消す可能性に注意
                                        if(board_map[hash].count(i*W+j)){
                                            board_map[hash].erase(i*W+j);
                                            if(board_map[hash].empty()){
                                                board_map.erase(hash);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                move_history.push_back(nex);
                my_app.step(nex);

                use_katanuki[nex.katanuki]++;

                const katanuki& nex_kat=my_app.katanuki_[nex.katanuki];

                for(int p=nex.y;p<std::min(nex.y+(int)nex_kat.size(),H);p++){
                    for(int q=nex.x;q<std::min(nex.x+(int)nex_kat[p-nex.y].size(),W);q++){
                        if(nex_kat[p-nex.y][q-nex.x]){
                            Rp[p]++;
                            plus++;
                        }
                    }
                }
            }else{
                int nex_y=0;
                while(nex_y!=H&&Rp[nex_y]>=Rl)nex_y++;

                // つまりゴール
                if(nex_y==H){
                    now_board=my_app.board_;
                    return 1;
                }
                
                // より揃って居ない方を選ぶ
                for(int l=0;l<H;l++){
                    // if(nex_y==l)continue;
                    for(int m=0;m<W-Rp[l];m++){
                        if(my_app.board_[l][m]==Rg[nex_y][Rp[nex_y]]){;
                            if(m<W-Rl)kat_dist.push({__builtin_popcount(abs(nex_y-l)),abs(nex_y-l),l,m});
                            else kat_dist.push({__builtin_popcount(abs(nex_y-l))+1,abs(nex_y-l),l,m});
                        }
                    }
                }

                // distのサイズが0ならばゴールに辿り着くことはできない
                if(kat_dist.empty()){
                    std::cerr<<"dist is empty"<<std::endl;
                    exit(1);
                }

                line_action1 nex; 
                for(int c=0;(c<=sp&&!kat_dist.empty());c++){
                    nex=kat_dist.top();
                    kat_dist.pop();
                }
                
                if(W-Rl<=nex.x){
                    my_app.step((action){0,nex.y,nex.x,right});
                    move_history.push_back((action){0,nex.y,nex.x,right});
                    nex.x=0;
                }

                for(int k=8;0<=k;k--){
                    if((1<<k)&abs(nex_y-nex.y)){
                        if(nex_y>nex.y){
                            my_app.step((action){bitp[k],nex.y+1,nex.x-(1<<k)+1,down});
                            move_history.push_back((action){bitp[k],nex.y+1,nex.x-(1<<k)+1,down});
                            nex.y+=1<<k;
                        }else{
                            my_app.step((action){bitp[k],nex.y-(1<<k),nex.x-(1<<k)+1,up});
                            move_history.push_back((action)(action){bitp[k],nex.y-(1<<k),nex.x-(1<<k)+1,up});
                            nex.y-=1<<k;
                        }
                    }
                }
                if(nex_y!=nex.y){
                    std::cerr<<"error"<<std::endl;  
                    exit(1);
                }
                
                Rp[nex_y]++;
                plus++;
                my_app.step((action){0,nex_y,nex.x,left});
                move_history.push_back((action){0,nex_y,nex.x,left});

                kat_dist=decltype(kat_dist)();

                l=0,r=H;
                dx=0;
                Q=decltype(Q)();
                board_map.clear();
            }
            int queue_size=Q.size();
            if(queue_size>1000000){
                // たまにQクリアする
                l=0,r=H;
                dx=0;
                Q=decltype(Q)();
                board_map.clear();
            }
            if(now_turn%50==0)debug(1,now_turn);
        }
    }
    return 0;
}


bool Solver::State2::apply_move(const int& H,const int& W,kyougi_app& my_app,const int& Ll,const board& Lg,const int sp,const vector<vector<vector<unsigned long long>>>& randomBits,const vector<int>& bitp){
    my_app.board_=now_board;
    int l=0,r=Ll;
    int kat_num=6;
    int dy=0;

    std::unordered_map<unsigned long long,std::set<int>> board_map;
    std::priority_queue<action_data2> Q;

    while(1){
        // debug(0);
        int now_turn=(int)move_history.size()+1;
        for(int j=l;j<r;j++){
            // 時間を管理
            time[j]=now_turn;
            // Right
            if(Lp[j]<H-1){
                for(int i=dy;i<H-Lp[j];i++){
                    // 使う型抜きは0-6
                    for(int k=0;k<=kat_num;k++){
                        katanuki& kat=my_app.katanuki_[k];
                        // 動かしても周りに影響が出ないか確認
                        bool ng=0;
                        if(j+(int)kat[0].size()>Ll)continue; // =は付かない
                        for(int q=j;q<(j+(int)kat[0].size());q++){ // FIXME: 0で決めつけるのは良くない
                            if(H-Lp[q]<i+(int)kat.size())ng=1;
                        }
                        if(ng)continue;

                        // vector<vector<int>> now_want(kat[0].size());
                        unsigned long long hash=0;

                        for(int q=j;q<(j+(int)kat[0].size());q++){
                            int cp=0;
                            for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                                if(kat[p-i][q-j]){
                                    if(Lp[q]+cp>=H-1){
                                        ng=1;
                                        goto out;
                                    }
                                    hash^=randomBits[p-i][q-j][my_app.board_[p][q]];
                                    // now_want[p-i].push_back(my_app.board_[p][q]);
                                    cp++;
                                }else{
                                    // now_want[p-i].push_back(-1);
                                }
                            }
                        }
                        out:;
                        if(!ng){
                            board_map[hash].insert(j*H+i);
                        }
                    }
                }
            }
        }
        // debug(1);

        for(int j=0;j<Ll;j++){
            for(int k=0;k<=kat_num;k++){
                bool ng=0;
                katanuki& kat=my_app.katanuki_[k];
                // vector<vector<int>> want(kat[0].size());
                if(j+(int)kat[0].size()>Ll)continue;
                
                int cnt=0;
                unsigned long long hash=0;
                for(int q=j;q<std::min(j+(int)kat[0].size(),Ll);q++){
                    int cp=0;
                    for(int p=0;p<(int)kat.size();p++){ // pは0に注意
                        if(kat[p][q-j]){
                            if(Lp[q]+cp>=H-1){
                                ng=1;
                                goto kat_out;
                            }
                            hash^=randomBits[p][q-j][Lg[Lp[q]+cp][q]];
                            // want[p].push_back(Lg[Lp[q]+cp][q]);
                            cp++;
                        }else{
                            // want[p].push_back(-1);
                        }
                    }
                    cnt+=cp;
                }
                kat_out:;
                if(ng){
                    continue;
                }
                if(board_map.count(hash)){
                    const std::set<int>& top_vec=board_map[hash];

                    auto itr=top_vec.lower_bound(j*H);
                    if(itr!=top_vec.end()&&*itr<(j+1)*H){
                        int top=*itr;
                        int tf=top/H; int ts=top%H;
                        if(tf==j){
                            // 次のピースは何ピース持ってこられる？
                            Q.push({cnt,-Lp[j],{k,ts,j,up},now_turn,j,-1,-1,-1});
                        }
                    }
                    for(const int& top:top_vec){
                        int tf=top/H; int ts=top%H;
                        if(j<=tf)continue;
                        for(int q=0;q<Ll;q++){
                            if(H-Lp[q]<ts+(int)kat.size()){
                                ng=1;
                            }
                        }
                        if(ng)continue;
                        for(int kk=0,q=1;kk<9;kk++,q*=2){
                            if(abs(j-tf)==q){
                                if((int)kat.size()<=my_app.katanuki_[bitp[kk]].size()&&tf+(int)kat[0].size()+my_app.katanuki_[bitp[kk]][0].size()<=Ll){
                                    Q.push({cnt/2,-Lp[j],{k,ts,j,up},now_turn,tf,kk,(int)kat.size(),(int)kat[0].size()});
                                    goto kat_ok;
                                }
                            }
                        }
                    }
                    kat_ok:;
                }
            }
        }
        // debug(2);

        {

            action nex={-1,-1,-1,right};
            action_data2 d;
            int nex_score=-1;
            int c=0;
            vector<action_data2> skip;
            while(!Q.empty()){
                nex_score=Q.top().score; // ピース数
                d=Q.top();
                Q.pop();
                bool ok=1;
                if(d.siz==-1){
                    for(int p=std::max(d.act.x-(int)my_app.katanuki_[d.act.katanuki][0].size()+1,0);p<std::min(d.act.x+(int)my_app.katanuki_[d.act.katanuki][0].size(),Ll);p++){
                        if(d.ct<time[p]){
                            ok=0;
                        }
                    }
                }else{
                    if(d.ct<now_turn){
                        ok=0;
                    }
                }

                if(ok){
                    if(c==sp){
                        nex=d.act;
                        break;
                    }else{
                        c++;
                        skip.push_back(d);
                    }
                }
            }

            for(auto& d:skip){
                Q.push(d);
            }
            
            if(nex.katanuki!=-1){
                // TODO: 256が使えないのでsolve1の前のやつにする必要がある
                if(nex.x!=d.line){
                    // 確実にtrue
                    if(nex.x>d.line){
                        int katanuki=bitp[d.siz];
                        int nex_y=nex.y+d.height-(int)my_app.katanuki_[katanuki].size();
                        int nex_x=d.line+d.width;
                        my_app.step((action){katanuki,nex_y,nex_x,right});
                        move_history.push_back((action){katanuki,nex_y,nex_x,right});
                    }

                    l=0,r=Ll;
                    dy=0;
                    Q=decltype(Q)();
                    board_map.clear();
                }else{
                    l=std::max(nex.x-8,0),r=std::min(nex.x+(int)my_app.katanuki_[nex.katanuki][0].size(),Ll);
                    dy=std::max(nex.y-4,0);
                    for(int j=l;j<r;j++){
                        // Right
                        if(Lp[j]<H-1){
                            for(int i=dy;i<H-Lp[j];i++){
                                // 使う型抜きは0-6
                                for(int k=0;k<=kat_num;k++){
                                    katanuki& kat=my_app.katanuki_[k];
                                    // 動かしても周りに影響が出ないか確認
                                    bool ng=0;
                                    if(j+(int)kat[0].size()>Ll)continue; // =は付かない
                                    for(int q=j;q<(j+(int)kat[0].size());q++){ // FIXME: 0で決めつけるのは良くない
                                        if(H-Lp[q]<i+(int)kat.size())ng=1;
                                    }
                                    if(ng)continue;

                                    // vector<vector<int>> now_want(kat[0].size());
                                    unsigned long long hash=0;

                                    for(int q=j;q<(j+(int)kat[0].size());q++){
                                        int cp=0;
                                        for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                                            if(kat[p-i][q-j]){
                                                if(Lp[q]+cp>=H-1){
                                                    ng=1;
                                                    goto out2;
                                                }
                                                hash^=randomBits[p-i][q-j][my_app.board_[p][q]];
                                                // now_want[p-i].push_back(my_app.board_[p][q]);
                                                cp++;
                                            }else{
                                                // now_want[p-i].push_back(-1);
                                            }
                                        }
                                    }
                                    out2:;
                                    if(!ng){
                                        if(board_map[hash].count(j*H+i)){
                                            board_map[hash].erase(j*H+i);
                                            if(board_map[hash].empty()){
                                                board_map.erase(hash);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                my_app.step(nex);
                move_history.push_back(nex);

                katanuki& nex_kat=my_app.katanuki_[nex.katanuki];

                for(int q=nex.x;q<(nex.x+(int)nex_kat[0].size());q++){
                    for(int p=nex.y;p<std::min(nex.y+(int)nex_kat.size(),H);p++){
                        if(nex_kat[p-nex.y][q-nex.x]){
                            Lp[q]++;
                            plus++;
                        }
                    }
                }
            }else{
                std::priority_queue<line_action2,vector<line_action2>,std::greater<line_action2>> kat_dist;
                int nex_x=0;
                while(nex_x!=Ll&&Lp[nex_x]>=H-1)nex_x++;

                // つまりゴール
                if(nex_x==Ll){
                    now_board=my_app.board_;
                    return 1;
                }

                // より揃っていない方を選ぶ
                for(int m=0;m<Ll;m++){
                    // if(nex_x==m)continue;
                    for(int l=0;l<H-Lp[m];l++){
                        if(my_app.board_[l][m]==Lg[Lp[nex_x]][nex_x]){
                            if(l==0&&m<nex_x)kat_dist.push({__builtin_popcount(abs(nex_x-m)),abs(nex_x-m),l,m});
                            else if(l==0||m<nex_x)kat_dist.push({__builtin_popcount(abs(nex_x-m))+1,abs(nex_x-m),l,m});
                            else kat_dist.push({__builtin_popcount(abs(nex_x-m))+2,abs(nex_x-m),l,m});
                        }
                    }
                }

                // distのサイズが0ならばゴールに辿り着くことはできない
                if(kat_dist.empty()){
                    std::cerr<<"dist is empty"<<std::endl;
                    exit(1);
                }

                line_action2 nex;
                for(int c=0;(c<=sp&&!kat_dist.empty());c++){
                    nex=kat_dist.top();
                    kat_dist.pop();
                }

                if(0!=nex.y){
                    my_app.step((action){0,nex.y,nex.x,down});
                    move_history.push_back((action){0,nex.y,nex.x,down});
                    nex.y=0;
                }

                if(nex.x>nex_x){
                    my_app.step((action){0,nex.y,nex.x,right});
                    move_history.push_back((action){0,nex.y,nex.x,right});
                    nex.x=0;
                }

                for(int k=8;0<=k;k--){
                    if((1<<k)&abs(nex_x-nex.x)){
                        if(nex_x>nex.x){
                            my_app.step((action){bitp[k],nex.y-(1<<k)+1,nex.x+1,right});
                            move_history.push_back((action){bitp[k],nex.y-(1<<k)+1,nex.x+1,right});
                            nex.x+=1<<k;
                        }
                        // 片方のみで良い
                    }
                }
                if(nex_x!=nex.x){
                    std::cerr<<"error"<<std::endl;  
                    exit(1);
                }
                
                Lp[nex_x]++;
                plus++;
                my_app.step((action){0,nex.y,nex_x,up});
                move_history.push_back((action){0,nex.y,nex_x,up});

                l=0,r=Ll;
                dy=0;
                Q=decltype(Q)();
                board_map.clear();
            }
            int queue_size=Q.size();
            if(queue_size>1000000){
                // たまにQクリアする
                l=0,r=Ll;
                dy=0;
                Q=decltype(Q)();
                board_map.clear();
            }
            // debug(Lp);
            // debug(2,now_turn,l,r,board_map.size());
            if(now_turn%50==0)debug(2,now_turn);
        }
    }
    return 0;
}

void Solver::randomBitsInit(){
    randomBits.resize(H,vector<vector<unsigned long long>>(W,vector<unsigned long long>(4)));
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;

    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            for(int k = 0; k < 4; ++k){
                randomBits[i][j][k] = dis(gen);
            }
        }
    }
}

std::pair<board,vector<action>> Solver::solve1(){

    board Rg(H,vector<int>(Rl));
    for(int i=0;i<H;i++){
        for(int j=0;j<Rl;j++){
            Rg[i][j]=G[i][W-Rl+j];
        }
    }

    vector<int> time(H,-1);
    vector<int> Rp(H,0);

    for(int i=0;i<H;i++){
        for(int j=W-1;W-Rl<=j;j--){
            bool nice=1;
            for(int k=W-Rl;k<j+1;k++){
                if(my_app.board_[i][W-j+k-1]!=G[i][k]){
                    nice=0;
                    break;
                }
            }
            if(nice){
                Rp[i]=j+1-(W-Rl);
                break;
            }
        }
    }

    unsigned long long zobrist_hash=0;

    Node1 node(State1(H,W,my_app.board_,time,zobrist_hash,Rp),false);
    node.advance(H,W,my_app,Rl,Rg,0,randomBits,bitp);

    return {node.state.now_board,node.state.move_history};
}

std::pair<board,vector<action>> Solver::solve2(){

    const int Ll=W-Rl;
    board Lg(H-1,vector<int>(Ll));
    for(int i=0;i<H-1;i++){
        for(int j=0;j<Ll;j++){
            Lg[i][j]=G[i+1][j];
        }
    }


    vector<int> time(Ll,-1);
    vector<int> Lp(Ll,0);

    // for(int i=0;i<Ll;i++){
    //     for(int j=H-1;0<=j;j--){
    //         bool nice=1;
    //         for(int k=0;k<j+1;k++){
    //             if(my_app.board_[i][W-j+k-1]!=G[i][k]){
    //                 nice=0;
    //                 break;
    //             }
    //         }
    //         if(nice){
    //             Lp[i]=j;
    //             break;
    //         }
    //     }
    // }
    
    unsigned long long zobrist_hash=0;

    Node2 node(State2(H,W,my_app.board_,time,zobrist_hash,Lp),false);
    node.advance(H,W,my_app,Ll,Lg,0,randomBits,bitp);

    return {node.state.now_board,node.state.move_history};
}

std::pair<board,vector<action>> Solver::solve3(){
    vector<action> solve_act;
    int Ll=W-Rl;

    // H=0,W=line-1までの並び替え
    vector<bool> pp(Ll,0);
    int pj=Ll-1;
    for(int j=Ll-1;0<=j;j--){
        if(my_app.board_[0][j]==G[0][pj]){
            pp[j]=1;
            pj--;
        }
    }

    for(int k=pj;k>=0;k--){
        for(int j=0;j<Ll;j++){
            if(!pp[j]&&my_app.board_[0][j]==G[0][k]){
                my_app.step((action){0,0,j,right});
                solve_act.push_back((action){0,0,j,right});
                for(int i=j-1;0<=i;i--){
                    pp[i+1]=pp[i];
                }
                
                pp[0]=1;
                break;
            }
        }
    }

    return {my_app.board_,solve_act};
}