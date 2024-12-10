#include<bits/stdc++.h>

void json_dump(int H,int W,board S,board G,vector<action> solve1_actions,vector<action> solve2_actions,vector<action> solve3_actions){
    {
        std::ofstream ofs;
        ofs.open("json/input.json");
        json P;
        P["startsAt"]=0;
        P["board"]["width"]=W;
        P["board"]["height"]=H;
        for(int i=0;i<H;i++){
            string s="";
            string g="";
            for(int j=0;j<W;j++){
                s+=S[i][j]+'0';
                g+=G[i][j]+'0';
            }
            P["board"]["start"].push_back(s);
            P["board"]["goal"].push_back(g);
        }
        P["general"]["n"]=0;
        ofs<<P.dump(4)<<std::endl;
        ofs.close();
    }
    {
        std::ofstream ofs;
        ofs.open("json/output.json");
        json A;
        A["n"]=solve1_actions.size()+solve2_actions.size()+solve3_actions.size();
        for(int i=0;i<solve1_actions.size();i++){
            json query;
            query["p"]=solve1_actions[i].katanuki;
            query["x"]=solve1_actions[i].x;
            query["y"]=solve1_actions[i].y;
            query["s"]=solve1_actions[i].direct;
            A["ops"].push_back(query);
        }
        for(int i=0;i<solve2_actions.size();i++){
            json query;
            query["p"]=solve2_actions[i].katanuki;
            query["x"]=solve2_actions[i].x;
            query["y"]=solve2_actions[i].y;
            query["s"]=solve2_actions[i].direct;
            A["ops"].push_back(query);
        }
        for(int i=0;i<solve3_actions.size();i++){
            json query;
            query["p"]=solve3_actions[i].katanuki;
            query["x"]=solve3_actions[i].x;
            query["y"]=solve3_actions[i].y;
            query["s"]=solve3_actions[i].direct;
            A["ops"].push_back(query);
        }
        ofs<<A.dump(4)<<endl;
        ofs.close();
    }
}