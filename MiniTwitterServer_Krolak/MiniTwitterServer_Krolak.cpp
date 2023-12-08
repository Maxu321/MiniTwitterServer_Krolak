// MiniTwitterServer_Krolak.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>
#include <memory>
#include <string>
#include <grpcpp\grpcpp.h>
#include "minitwitter.pb.h"
#include "minitwitter.grpc.pb.h"

using namespace minitwitter;


class MiniTwitterImpl final : public MiniTwitter::Service {
    std::vector<minitwitter::Tweet> db;
    std::vector<std::string> users;

public:
    grpc::Status PostTweet(::grpc::ServerContext* context, const PostTweetRequest* request, PostTweetResponse* response) override {
        
        if (db.size() < 100) {//maximum of 100 tweets in databse
            db.push_back(request->tweet());
        }
        else {
            db.erase(db.begin());
            db.push_back(request->tweet());
        }

        std::cout << "User: " << request->tweet().user() <<" posted " <<"\""<< request->tweet().text()<<"\"\n";

        response->set_success(1);
        std::cout << "Wyslalem: " << response->success() << '\n';
        return grpc::Status::OK;
    }
    
    grpc::Status GetTweets(grpc::ServerContext* context, const GetTweetsRequest* request, GetTweetsResponse* response) override {

        std::cout << "User " << request->user() << " requests " << request->n() <<" tweets;" << '\n';
        std::cout << "In database there are " << db.size() << " tweets."<< '\n';

        if (!db.empty()) {
            auto it = db.end()-1;
            if (request->n() <= db.size()) {
                for (int i=0;i < request->n();i++) {
                    *(response->add_tweets())=*it;
                    it--;
                }
            }
            else {
                for (auto current = db.begin();current != db.end();current++) {
                    *(response->add_tweets()) = *current;
                }
            }
        }
        return grpc::Status::OK;
    }

    grpc::Status LogIn(grpc::ServerContext* context, const LogInRequest* request, LogInResponse* response) override {
        if (std::find(users.begin(), users.end(), request->user()) == users.end())
        {
            if (db.size() < 100) {//maximum of 100 logged in users
                users.push_back(request->user());
                std::cout << "New user " << request->user() << " logged in.\n";
                response->set_success(1);
                return grpc::Status::OK;
            }
            else {//user not loged in because 100 user already logged in
                response->set_success(2);
                return grpc::Status::OK;
            }
        }
        else {//user not logged in because nickname already exists
            response->set_success(3);
            return grpc::Status::OK;
        }

        
        return grpc::Status::CANCELLED;
    }

    grpc::Status LogOut(grpc::ServerContext* context, const LogOutRequest* request, LogOutResponse* response) override {

        if (std::find(users.begin(), users.end(), request->user()) != users.end())
        {
            std::find(users.begin(), users.end(), request->user())->erase();
            std::cout << "User " << request->user() << " logged out.\n";
            response->set_success(1);
            return grpc::Status::OK;
        }
        else {// user not logged out because he wasn't logged in
            response->set_success(2);
        }

        return grpc::Status::CANCELLED;
    }
};

void RunServer() {
    MiniTwitterImpl service;
    std::string server_address("0.0.0.0:50051");
    

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main()
{
    RunServer();
}

