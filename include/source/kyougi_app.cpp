#include "../header/kyougi_app.h"
#include <vector>
#include <string>
#include <iostream>
#include <ranges>

namespace util
{
    template <class Tt>
    string vector2_to_string(Tt t)
    {
        string data;

        for (size_t i = 0; i < t.size(); i++)
        {
            for (size_t j = 0; j < t[i].size(); j++)
            {
                switch (t[i][j])
                {
                case 0:
                    data += "\x1b[41m";
                    break;
                case 1:
                    data += "\x1b[42m";
                    break;
                case 2:
                    data += "\x1b[43m";
                    break;
                case 3:
                    data += "\x1b[44m";
                    break;
                case 4:
                    data += "\x1b[45m";
                    break;
                default:
                    data += "\x1b[0m";
                    break;
                }
                data += '0' + t[i][j];
                data += ",";
            }
            data += "\x1b[0m\x1b[49m\n";
        }
        data += "\x1b[0m\x1b[49m";

        return data;
    }

    template <class Type>
    vector<vector<Type>> json_to_vector2(const json& input)
    {
        vector<vector<Type>> start;
        for (const auto& row : input)
        {
            vector<Type> row_values;
            for (const char c : row.get<string>())
            {
                row_values.push_back(c - '0');
            }
            start.push_back(row_values);
        }
        return start;
    };
}

kyougi_app::kyougi_app()
{
    generate_general_katanuki();
}

kyougi_app::kyougi_app(const json& game_json)
{
    generate_general_katanuki();
    set_game(game_json);
}

kyougi_app::kyougi_app(const string& game_string)
{
    generate_general_katanuki();
    set_game(json::parse(game_string));
}

void kyougi_app::generate_general_katanuki()
{
    katanuki_.clear();

    const vector<int> patterns = {2, 4, 8, 16, 32, 64, 128, 256};

    {
        const katanuki pattern = {{true}};
        add_katanuki(pattern);
    }

    for (const auto& size : patterns)
    {
        {
            katanuki pattern(size, vector<bool>(size, true));
            add_katanuki(pattern);
        }
        {
            katanuki pattern(size, vector<bool>(size, true));
            for (int i = 0; i < size / 2; ++i)
            {
                for (auto line : pattern[2 * i + 1]) line = false;
            }
            add_katanuki(pattern);
        }
        {
            katanuki pattern(size, vector<bool>(size, true));
            for (int i = 0; i < size / 2; ++i)
            {
                for (int j = 0; j < size; ++j) pattern[j][2 * i + 1] = false;
            }
            add_katanuki(pattern);
        }
    }
}

void kyougi_app::reset_board()
{
    cnt=0;
    board_ = start_;
}

void kyougi_app::add_katanuki(const katanuki& pattern)
{
    this->katanuki_.push_back(pattern);
}

void kyougi_app::set_start(const board& board)
{
    this->start_ = board;
    this->board_ = board;
    cnt=0;
}

void kyougi_app::set_goal(const board& goal)
{
    this->goal_ = goal;
}

void kyougi_app::step(const action& act)
{
    //特殊パターンの処理
    if(!rot&&(act.direct==left||act.direct==right)&&(act.katanuki<=23)&&(act.katanuki!=6)&&(act.katanuki!=9)&&(act.katanuki!=12)&&(act.katanuki!=15)&&(act.katanuki!=18)&&(act.katanuki!=21)){
        shift_step(act);
        return;
    }

    //変更禁止
    const katanuki& pattern = get_katanuki(act.katanuki);

    //空欄定数
    constexpr int blank = 4;

    //変数にサイズをあらかじめ代入
    const int board_y = board_.size();//board_yをboard.size()で置き換えてはいけない(戒め)
    const int board_x = board_[0].size();//board_xをboard[0].size()で置き換えてはいけない(戒め)

    //オフセット
    const int offset_y = act.y;
    const int offset_x = act.x;

    //型抜きの範囲
    const int pattern_y = pattern.size();
    const int pattern_x = pattern[0].size();

    // スタート地点の計算
    const int start_y = std::max(0, offset_y);
    const int start_x = std::max(0, offset_x);

    // 終点の計算
    const int end_y = std::min(board_y, offset_y + pattern_y);
    const int end_x = std::min(board_x, offset_x + pattern_x);

    //抜き出したピースのバッファ
    board piece_buffer(board_.size(), vector<int>(board_[0].size(), blank));

    //型抜き処理
    for (int y = 0; y < pattern.size(); ++y)
        for (int x = 0; x < pattern[0].size(); ++x)
            if ((y + act.y >= 0) && (x + act.x >= 0) && (y + act.y < board_.size()) && (x + act.x < board_[0].size()))
                if (pattern[y][x])
                {
                    //バッファに抜き出したピースを保存
                    piece_buffer[y + act.y][x + act.x] = board_[y + act.y][x + act.x];

                    //抜き出したピースを空白で置き換え
                    board_[y + act.y][x + act.x] = blank;
                }

    //寄せ処理
    if (act.direct == up)
    {
        for (int y = start_y; y < end_y; ++y)
            for (int x = start_x; x < end_x; ++x)
                while (board_[y][x] == blank)
                {
                    //ピースを寄せる
                    for (int i = 0; i < board_.size() - y - 1; ++i) board_[y + i][x] = board_[y + i + 1][x];
                    //ブランクにバッファに保存したピースを適用
                    board_[board_.size() - 1][x] = piece_buffer[y][x];
                    //バッファをピースに合わせて移動させる
                    for (int i = 0; i < board_.size() - y - 1; ++i) piece_buffer[y + i][x] = piece_buffer[y + i + 1][x];
                }
    }
    else if (act.direct == down)
    {
        for (int y = end_y - 1; y >= start_y ; --y)
            for (int x = start_x; x < end_x; ++x)
                while (board_[y][x] == blank)
                {
                    //ピースを寄せる
                    for (int i = 0; i < y; ++i) board_[y - i][x] = board_[y - i - 1][x];
                    //ブランクにバッファに保存したピースを適用
                    board_[0][x] = piece_buffer[y][x];
                    //バッファをピースに合わせて移動させる
                    for (int i = 0; i < y; ++i) piece_buffer[y - i][x] = piece_buffer[y - i - 1][x];
                }
    }
    else if (act.direct == left)
    {
        for (int x = start_x ; x < end_x ; ++x)
            for (int y = start_y; y < end_y ; ++y)
                while (board_[y][x] == blank)
                {
                    //ピースを寄せる
                    for (int i = 0; i < board_[0].size() - x - 1; ++i) board_[y][x + i] = board_[y][x + i + 1];
                    //ブランクにバッファに保存したピースを適用
                    board_[y][board_[0].size() - 1] = piece_buffer[y][x];
                    //バッファをピースに合わせて移動させる
                    for (int i = 0; i < board_[0].size() - x - 1; ++i) piece_buffer[y][x + i] = piece_buffer[y][x +
                        i + 1];
                }
    }
    else if (act.direct == right)
    {
        for (int x = end_x - 1; x >= start_x ; --x)
            for (int y = start_y ; y < end_y; ++y)
                while (board_[y][x] == blank)
                {
                    //ピースを寄せる
                    for (int i = 0; i < x; ++i) board_[y][x - i] = board_[y][x - i - 1];
                    //ブランクにバッファに保存したピースを適用
                    board_[y][0] = piece_buffer[y][x];
                    //バッファをピースに合わせて移動させる
                    for (int i = 0; i < x; ++i) piece_buffer[y][x - i] = piece_buffer[y][x - i - 1];
                }
    }
}

inline void kyougi_app::shift_step(const action& act){

    //変数にサイズをあらかじめ代入
    const int board_x = board_[0].size();//board_xをboard[0].size()で置き換えてはいけない(戒め)
    const int board_y = board_.size();//board_yをboard.size()で置き換えてはいけない(戒め)

    //オフセット
    const int offset_y = act.y;
    const int offset_x = act.x;

    //型抜きの範囲
    const int size = static_cast<int>(katanuki_[act.katanuki].size()); //正方形

    // スタート地点の計算
    const int start_y = std::max(0, offset_y);
    const int start_x = std::max(0, offset_x);

    // 終点の計算
    const int end_y = std::min(board_y, offset_y + size);

    if(act.katanuki==0){

        int N = std::min(board_x, offset_x + size) - start_x;
        if(N<=0) return;

        if (act.direct == right)
            std::rotate(board_[act.y].begin(), board_[act.y].begin() + start_x , board_[act.y].begin() + start_x + 1);
        else
            std::rotate(board_[act.y].begin() + start_x , board_[act.y].begin() + start_x + 1, board_[act.y].end());

    }
    if(act.katanuki==3){//動作可能

        int N =  std::min(board_x,offset_x + 1) - start_x;
        if(N<=0) { return; }

        for (int y = start_y; y < end_y; ++y){
            if (act.direct == right)
                std::rotate(board_[y].begin(), board_[y].begin() + start_x, board_[y].begin() + start_x + N);
            else
                std::rotate(board_[y].begin() + start_x , board_[y].begin() + start_x + N, board_[y].end());
        }
    }
    else if(act.katanuki%3==1){
        int N = std::min(board_x, offset_x + size) - start_x;
        if(N<=0) return;

        for (int y = start_y; y < end_y; ++y){
            if (act.direct == right)
                std::rotate(board_[y].begin(), board_[y].begin() + start_x, board_[y].begin() + start_x + N);
            else
                std::rotate(board_[y].begin() + start_x , board_[y].begin() + start_x + N, board_[y].end());
        }
    }
    else if(act.katanuki%3==2){
        int N = std::min(board_x, offset_x + size) - start_x;
        if(N<=0) return;

        for (int y = start_y; y < end_y; y+=2){
            if (act.direct == right)
                std::rotate(board_[y].begin(), board_[y].begin() + start_x, board_[y].begin() + start_x + N);
            else
                std::rotate(board_[y].begin() + start_x , board_[y].begin() + start_x + N, board_[y].end());
        }
    }
}

void kyougi_app::reverse_step(const action& act)
{
    //変更禁止
    const katanuki& pattern = get_katanuki(act.katanuki);

    //空欄定数
    constexpr int blank = 4;

    //抜き出したピースのバッファ
    board piece_buffer(board_.size(), vector<int>(board_[0].size(), blank));

    //寄せ処理
    if (act.direct == up)
    {
        //ボードを下から参照
        for (int x = 0; x < pattern[0].size(); ++x)
        {
            int i = 0;
            for (int y = pattern.size() - 1; y >= 0 ; --y)
                if ((y + act.y >= 0) && (x + act.x >= 0) && (y + act.y < board_.size()) && (x + act.x < board_[0].size()))
                    if (pattern[y][x])
                    {
                        //抜いたピースをバッファの型抜きの場所に移動
                        piece_buffer[y + act.y][x + act.x] = board_[board_.size() - (1+i)][x + act.x];
                        //抜き出したピースを空白で置き換え
                        board_[board_.size() - (1+i)][x + act.x] = blank;
                        i++;
                    }
        }

        //ピースの逆移動
        for (int x = 0; x < board_[0].size(); ++x)
            for (int y = board_.size() - 1; y >= 0; --y) //Y座標を下から参照
                if(piece_buffer[y][x]==blank&&board_[y][x]==blank)
                {
                    int i = board_.size()-y;
                    while(board_[board_.size() - (1+i)][x]==blank) i++; //最後尾のピースを探索
                    board_[y][x] = board_[board_.size() - (1+i)][x]; //ピースを移動
                    board_[board_.size() - (1+i)][x] = blank;
                }
    }
    if (act.direct == down)
    {
        //ボードを下から参照
        for (int x = 0; x < pattern[0].size(); ++x)
        {
            int i = 0;
            for (int y = 0; y < pattern.size(); ++y)
                if ((y + act.y >= 0) && (x + act.x >= 0) && (y + act.y < board_.size()) && (x + act.x < board_[0].size()))
                    if (pattern[y][x])
                    {
                        //抜いたピースをバッファの型抜きの場所に移動
                        piece_buffer[y + act.y][x + act.x] = board_[i][x + act.x];
                        //抜き出したピースを空白で置き換え
                        board_[i][x + act.x] = blank;
                        i++;
                    }
        }

        //ピースの逆移動
        for (int x = 0; x < board_[0].size(); ++x)
            for (int y = 0; y < board_.size(); ++y)
                if(piece_buffer[y][x]==blank&&board_[y][x]==blank)
                {
                    int i = y;
                    while(board_[i][x]==blank) i++; //最後尾のピースを探索
                    board_[y][x] = board_[i][x]; //ピースを移動
                    board_[i][x] = blank;
                }
    }


    if (act.direct == left) {
        // ボードを右から参照
        for (int y = 0; y < pattern.size(); ++y) {
            int i = 0;
            for (int x = pattern[0].size() - 1; x >= 0; --x) {
                if ((y + act.y >= 0) && (x + act.x >= 0) && (y + act.y < board_.size()) && (x + act.x < board_[0].size())) {
                    if (pattern[y][x]) {
                        // 抜いたピースをバッファの型抜きの場所に移動
                        piece_buffer[y + act.y][x + act.x] = board_[y + act.y][board_[0].size() - (1 + i)];
                        // 抜き出したピースを空白で置き換え
                        board_[y + act.y][board_[0].size() - (1 + i)] = blank;
                        i++;
                    }
                }
            }
        }

        // ピースの逆移動
        for (int y = 0; y < board_.size(); ++y) {
            for (int x = board_[0].size() - 1; x >= 0; --x) { // X座標を右から参照
                if (piece_buffer[y][x] == blank && board_[y][x] == blank) {
                    int i = board_[0].size() - x;
                    while (board_[y][board_[0].size() - (1 + i)] == blank) i++; // 最後尾のピースを探索
                    board_[y][x] = board_[y][board_[0].size() - (1 + i)]; // ピースを移動
                    board_[y][board_[0].size() - (1 + i)] = blank;
                }
            }
        }
    }

    if (act.direct == right) {
        // ボードを左から参照
        for (int y = 0; y < pattern.size(); ++y) {
            int i = 0;
            for (int x = 0; x < pattern[0].size(); ++x) {
                if ((y + act.y >= 0) && (x + act.x >= 0) && (y + act.y < board_.size()) && (x + act.x < board_[0].size())) {
                    if (pattern[y][x]) {
                        // 抜いたピースをバッファの型抜きの場所に移動
                        piece_buffer[y + act.y][x + act.x] = board_[y + act.y][i];
                        // 抜き出したピースを空白で置き換え
                        board_[y + act.y][i] = blank;
                        i++;
                    }
                }
            }
        }

        // ピースの逆移動
        for (int y = 0; y < board_.size(); ++y) {
            for (int x = 0; x < board_[0].size(); ++x) {
                if (piece_buffer[y][x] == blank && board_[y][x] == blank) {
                    int i = x;
                    while (board_[y][i] == blank) i++; // 最後尾のピースを探索
                    board_[y][x] = board_[y][i]; // ピースを移動
                    board_[y][i] = blank;
                }
            }
        }
    }


    //抜き出したピースをバッファから戻す
    for (int y = 0; y < piece_buffer.size(); ++y)
        for (int x = 0; x < piece_buffer[0].size(); ++x)
            if(piece_buffer[y][x]!=blank)
                board_[y][x] = piece_buffer[y][x];


    //エラー防止
    if(true)
    {
        for (int y = 0; y < board_.size(); ++y)
            for (int x = 0; x < board_[0].size(); ++x)
                if(board_[y][x]==blank)
                {
                    std::cout << "AA\n";
                    throw "未処理のブランクが存在します";
                }
    }
}

void kyougi_app::set_game(const json& game)
{
    set_start(util::json_to_vector2<int>(game["board"]["start"]));
    set_goal(util::json_to_vector2<int>(game["board"]["goal"]));
    for (int i = 0; i < game["general"]["n"]; ++i)
        add_katanuki(util::json_to_vector2<bool>(game["general"]["patterns"][i]["cells"]));
}

void kyougi_app::set_game(const string& game_string)
{
    set_game(json::parse(game_string));
}

void kyougi_app::write_board() const
{
    std::cout << util::vector2_to_string(board_);
}

void kyougi_app::write_katanuki() const
{
    std::cout << "value:" << katanuki_.size() << "\n";

    int i = 0;
    for (const auto& pattern : katanuki_)
    {
        std::cout << "---<number:" << i << ">---\n";
        std::cout << util::vector2_to_string(pattern);
        i++;
    }
}

katanuki kyougi_app::get_katanuki(const int& number)
{
    return katanuki_[number];
}

board kyougi_app::get_board()
{
    return this->board_;
}

board kyougi_app::get_goal()
{
    return this->goal_;
}