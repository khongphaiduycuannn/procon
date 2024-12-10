#include"solver.hpp"

#ifdef LOCAL
#  include "../include/header/debug_print.hpp"
#  define debug(...) debug_print::multi_print(#__VA_ARGS__, __VA_ARGS__)
#else
#  define debug(...) (static_cast<void>(0))
#endif

const int KAT_NUM = 6;
const float LEFT = 7.0;

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
    // debug(game_string);
    json game_json=json::parse(game_string);
    // debug(game_json.dump(4));
    S_string=game_string;

    my_app.set_game(game_json);

    S=my_app.board_;
    G=my_app.get_goal();
    H=my_app.board_.size();
    W=my_app.board_[0].size();

    Rl=W-(int)W/LEFT;
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

int Solver::solve(){
    auto [board1,actions1]=solve1();
    auto [board2,actions2]=solve2();
    auto [board3,actions3]=solve3();
    solve1_actions=actions1;
    solve2_actions=actions2;
    solve3_actions=actions3;

    // if(move_check()){
    //     return 0;
    // }else{
    //     exit(1);
    // }
    return 0;
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
    const int up_dy=0;
    int l=up_dy,r=H;
    const int kat_num=KAT_NUM;
    const int left_dx=0;
    int dx=left_dx;

    int bb=3;

    vector<int> use_katanuki(kat_num+1,0);

    int rp=0;

    // vector<vector<int>> num(H,vector<int>(4));
    // for(int i=0;i<H;i++){
    //     for(int j=0;j<W;j++){
    //         num[i][now_board[i][j]]++;
    //     }
    // }
    std::unordered_map<unsigned long long,std::set<int>> board_map;

    vector<vector<int>> Hp(H,vector<int>(bb,0));

    std::priority_queue<line_action1,vector<line_action1>,std::greater<line_action1>> kat_dist;

    while(1){
        std::priority_queue<action_data1> Q;
        vector<action_data1> P;
        int now_turn=(int)move_history.size()+1;

        // if(now_turn%100==0){
        //     // ここ詳しく考える
        //     int mrp=*std::max_element(Rp.begin(),Rp.end());
        //     action nex1={23,static_cast<int>(rnd(10)-5),W-mrp-256,up}; // 23 or 24?
        //     action nex2={24,static_cast<int>(rnd(10)-5),W-mrp-256,right}; // 23 or 
        //     if(static_cast<int>(rnd(2))==0){
        //         my_app.step(nex1);
        //         move_history.push_back(nex1);
        //     }else{
        //         my_app.step(nex2);
        //         move_history.push_back(nex2);
        //     }
        //     l=0,r=H;
        //     dx=0;
        //     board_map.clear();
        // }


        for(int i=l;i<r;i++){
            // 時間を管理
            if(i>=0)time[i]=now_turn;
            // Right
            if(i<0||Rp[(i+rp)%H]<Rl){
                for(int j=dx;(i<0&&j<W)||(i>=0&&j<W-Rp[(i+rp)%H]);j++){
                    // 使う型抜きは0-6
                    for(int k=0;k<=kat_num;k++){
                        const katanuki& kat=my_app.katanuki_[k];
                        // 動かしても周りに影響が出ないか確認
                        bool ng=0;
                        int start_p = std::max(i, 0);
                        int end_p = std::min(i + (int)kat.size(), H);
                        for(int p = start_p; p < end_p; p++){
                            if((i < 0) || (W - Rp[(p + rp) % H] < j + (int)kat[p - i].size())) ng = 1;
                        }
                        if(ng)continue; // 定型型抜きならbreakでok

                        // vector<vector<int>> now_want(kat.size());
                        unsigned long long hash=0;

                        for(int p=std::max(0,i);p<std::min(i+(int)kat.size(),H);p++){
                            int cp=0;
                            for(int q=std::max(0,j);q<std::min(j+(int)kat[p-i].size(),W);q++){
                                if(kat[p-i][q-j]){
                                    if(Rp[(p+rp)%H]+cp>=Rl){
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
                            board_map[hash].insert((i+H)*(W+W)+j+W); // 2*H*Wのボードで考える
                        }
                    }
                }
            }
        }
        // debug(2);

        vector<vector<int>> Hpp(H);

        for(int i=up_dy;i<H;i++){
            for(int k=0;k<=kat_num;k++){
                bool ng=0;
                const katanuki& kat=my_app.katanuki_[k];
                // vector<vector<int>> want(kat.size());
                bool di=0;
                if(i<0||i+kat.size()>H){
                    di=1;
                }
                int cnt=0;
                unsigned long long hash=0;
                for(int p=std::max(i,0);p<std::min(i+(int)kat.size(),H);p++){
                    int cp=0;
                    for(int q=0;q<(int)kat[p-i].size();q++){ // 負の値はその型抜きを別途追加で対応？
                        if(kat[p-i][q]){
                            if(Rp[(p+rp)%H]+cp>=Rl){
                                ng=1;
                                goto kat_out;
                            }
                            hash^=randomBits[p-i][q][Rg[(p+rp)%H][Rp[(p+rp)%H]+cp]];
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
                if(cnt>0&&board_map.count(hash)){
                    // debug(hash);
                    const std::set<int>& top_vec=board_map[hash];
                    // debug(top_vec.size());
                    auto itr=top_vec.lower_bound((i+H)*(W+W));
                    if(itr!=top_vec.end()&&*itr<(i+H+1)*(W+W)){ //(i+H)*(W+W)+j+W
                        int top=*itr;
                        int tf=top/(W+W)-H; int ts=top%(W+W)-W;
                        if(tf==i){
                            // 次のピースは何ピース持ってこられる？
                            P.push_back({2*cnt,-Rp[(i+rp)%H],{k,i,ts,left},now_turn,i,-1,0});
                            Hpp[0].push_back(cnt);
                        }
                    }

                    if(di)continue;
                    for(const int& top:top_vec){
                        int tf=top/(W+W)-H; int ts=top%(W+W)-W;
                        for(int p=0;p<H;p++){
                            if(W-Rp[(p+rp)%H]<ts+(int)kat[0].size()){
                                ng=1;
                            }
                        }
                        if(ng)continue;
                        // int dist=0;
                        P.push_back({cnt,-Rp[(i+rp)%H],{k,i,ts,left},now_turn,tf,(int)kat[0].size(),0});
                        Hpp[(tf-i+H)%H].push_back(cnt);
                        break;
                    }
                }
            }
        }

        
        for(int i=0;i<H;i++){
            for(int j=0;j<bb;j++){
                Hp[i][j]=0; // FIXME: これじゃむり
            }
            std::sort(Hpp[i].rbegin(),Hpp[i].rend());
            for(int j=0;j<std::min(bb,(int)Hpp[i].size());j++){
                Hp[i][j]=Hpp[i][j];
            }
        }

        for(auto& p:P){
            for(int m=0;m<bb;m++){
                p.dd=p.dd+Hp[(p.line-p.act.y+H)%H][m];
            }
            if((p.line-p.act.y+H)%H==0)p.dd+=p.dd/bb;
            //debug(p.dd);
            Q.push(p);
        }
        // debug(Hp);
        // exit(0);
        //exit(0);

        // debug(3);
        // int now_turn=(int)move_history.size()+1;
        {
            // int l=0,r=H;
            // TODO:差分更新は後で考える
            // if(last_pos!=-1){
            //     l=std::max(last_pos-last_siz,0);
            //     r=std::min(last_pos+last_siz+1,H);
            // }

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
                    // for(int p=0;p<H;p++){
                    //     if(d.ct<time[p]){
                    //         ok=0;
                    //     }
                    // }
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
            
            // if(nex.katanuki==0){
            //     if(rnd(3)>=1){
            //         nex.katanuki=-1;
            //     }
            // }

            if(nex.katanuki!=-1){
                int z=0;
                if(nex.y!=d.line){
                    // むずい
                    int mrp=*std::max_element(Rp.begin(),Rp.end());
                    if(nex.y>d.line){
                        // 場所？
                        int nex_y=(nex.y-d.line)-256;
                        int nex_x=W-mrp;
                        // debug(1,d.score,nex_y,nex_x);
                        my_app.step((action){22,nex_y,nex_x,up});
                        move_history.push_back((action){22,nex_y,nex_x,up});
                        z=(nex.y-d.line);
                    }else{
                        int nex_y=H-(d.line-nex.y);
                        int nex_x=W-mrp;
                        // debug(2,d.score,nex_y,nex_x);
                        my_app.step((action){22,nex_y,nex_x,down});
                        move_history.push_back((action){22,nex_y,nex_x,down});
                        z=-(d.line-nex.y);
                    }

                    // debug(nex.y,nex.x);

                    l=up_dy,r=H;
                    dx=left_dx;
                    board_map.clear();
                }else{
                    // l=up_dy,r=H;
                    // dx=left_dx;
                    // Q=decltype(Q)();
                    // board_map.clear();
                    // FIXME: 8と決めつけるのは危険なので後で直す
                    l=std::max(nex.y-8,0),r=std::min(nex.y+(int)my_app.katanuki_[nex.katanuki].size(),H);
                    dx=std::max(nex.x-8,0);
                    for(int i=l;i<r;i++){
                        // Right
                        if(Rp[(i+rp)%H]<Rl){
                            for(int j=dx;j<W-Rp[(i+rp)%H];j++){
                                // 使う型抜きは0-6
                                for(int k=0;k<=kat_num;k++){
                                    katanuki& kat=my_app.katanuki_[k];
                                    // 動かしても周りに影響が出ないか確認
                                    bool ng=0;
                                    for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                                        if(W-Rp[(p+rp)%H]<j+(int)kat[p-i].size())ng=1;
                                    }
                                    if(ng)continue; // 定型型抜きならbreakでok

                                    // vector<vector<int>> now_want(kat.size());
                                    unsigned long long hash=0;

                                    for(int p=i;p<std::min(i+(int)kat.size(),H);p++){
                                        int cp=0;
                                        for(int q=j;q<std::min(j+(int)kat[p-i].size(),W);q++){
                                            if(kat[p-i][q-j]){
                                                if(Rp[(p+rp)%H]+cp>=Rl){
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
                                        if(board_map[hash].count((i+H)*(W+W)+j+W)){
                                            board_map[hash].erase((i+H)*(W+W)+j+W);
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

                nex.y=d.line;
                move_history.push_back(nex);
                my_app.step(nex);

                use_katanuki[nex.katanuki]++;
                // debug(d.score,nex.y,nex.x,nex.katanuki);

                const katanuki& nex_kat=my_app.katanuki_[nex.katanuki];

                rp=(rp+z+H)%H;
                for(int p=std::max(nex.y,0);p<std::min(nex.y+(int)nex_kat.size(),H);p++){
                    for(int q=std::max(nex.x,0);q<std::min(nex.x+(int)nex_kat[p-nex.y].size(),W);q++){
                        if(nex_kat[p-nex.y][q-nex.x]){
                            Rp[(p+rp)%H]++;
                            plus++;
                            if(Rp[(p+rp)%H]>Rl){
                                std::cerr<<"error"<<std::endl;
                                debug(Rp,rp,z);
                                exit(1);
                            }
                        }
                    }
                }
            }else{
                int nex_y=0;
                while(nex_y!=H&&Rp[(nex_y+rp)%H]>=Rl)nex_y++;

                // つまりゴール
                if(nex_y==H){
                    if(rp!=0){
                        my_app.step((action){22,H-rp,0,down});
                        move_history.push_back((action){22,H-rp,0,down});
                    }
                    now_board=my_app.board_;
                    // debug(use_katanuki);
                    return 1;
                }
                
                // より揃って居ない方を選ぶ
                for(int l=0;l<H;l++){
                    // if(nex_y==l)continue;
                    for(int m=0;m<W-Rp[(l+rp)%H];m++){
                        if(my_app.board_[l][m]==Rg[(nex_y+rp)%H][Rp[(nex_y+rp)%H]]){;
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
                
                Rp[(nex_y+rp)%H]++;
                plus++;
                my_app.step((action){0,nex_y,nex.x,left});
                move_history.push_back((action){0,nex_y,nex.x,left});

                kat_dist=decltype(kat_dist)();

                l=up_dy,r=H;
                dx=left_dx;
                board_map.clear();
            }
            // debug(Rp);
            int now=0;
            for(int i=0;i<H;i++){
                now+=Rp[i];
            }
            for(int i=0;i<H;i++){
                for(int j=0;j<Rp[(i+rp)%H];j++){
                    if(Rg[(i+rp)%H][j]!=my_app.board_[i][W-Rp[(i+rp)%H]+j]){
                        std::cerr<<"error"<<std::endl;
                        debug(1,now_turn,now,rp,d.act.katanuki,d.score,d.dd,now/(double)now_turn);
                        exit(1);
                    }
                }
            }
            // debug(1,now_turn,now,rp,d.act.katanuki,d.score,d.dd,now/(double)now_turn);
            if(now_turn%50==0)debug(1,now_turn,now);
        }
    }
    now_board=my_app.board_;
    return 0;
}


bool Solver::State2::apply_move(const int& H,const int& W,kyougi_app& my_app,const int& Ll,const board& Lg,const int sp,const vector<vector<vector<unsigned long long>>>& randomBits,const vector<int>& bitp){
    my_app.board_=now_board;
    int l=0,r=Ll;
    int kat_num=KAT_NUM;
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
