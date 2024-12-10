#include "solver.hpp"
#include "../include/header/kyougi_app.h"

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

string input_path;
string output_path;

class Solver_Interface {
public:
    Solver_Interface(string host,const unsigned short receive_port,const unsigned short send_port,std::unique_ptr<ISolver> solver) :
        host_(std::move(host)),
        receive_port_(receive_port),
        send_port_(send_port),
        receive_buffer_(),
        solver_(std::move(solver))
        {
            solver_->init();
        }

    void start() {
        receive_problem(); //受信ループ 問題を受信したら終了

        std::cout << "start solving\n";
        solver_->solve();//解答 解き終わったら終了
        std::cout << "finish solving\n";

        save_answer(solver_->get_answer());
    }

    void receive_problem() {
        std::string problem;
        std::ifstream f(input_path);
        string tmp;
        while (f >> tmp)
            problem += tmp;
        f.close();
        solver_->set_problem(problem);
    }

    void send_answer(const std::string& answer) {
    }

    static void save_answer(const std::string& answer){
        std::ofstream out_stream(output_path);
        out_stream << answer;
        out_stream.close();
    };

private:
    const int max_packet_size_ = 1024;
    // boost::asio::io_context io_context_;

    string host_;
    const unsigned short send_port_;
    const unsigned short receive_port_;
    
    std::array<char, 1024> receive_buffer_;
    std::unique_ptr<ISolver> solver_;
};

int main(int argc, char *argv[]){ //host receive_port send_port
    input_path = argv[1];
    output_path = argv[2];

    Solver_Interface receiver("",0,0,(std::move(std::make_unique<Solver>())));
    receiver.start();
    return 0;
}