// MiniTwitter_Krolak.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>
#include <memory>
#include <string>
#include <grpcpp\grpcpp.h>
#include "minitwitter.pb.h"
#include "minitwitter.grpc.pb.h"
#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>


using namespace minitwitter;
using namespace grpc;




int main()
{
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    auto stub = MiniTwitter::NewStub(channel);
    std::string choice;
    GetTweetsRequest gtrq;
    GetTweetsResponse gtrs;
    Tweet tweet;
    std::string post;
    PostTweetResponse ptrs;
    PostTweetRequest ptrq;
    LogInResponse lirs;
    LogInRequest lirq;
    LogOutResponse lors;
    LogOutRequest lorq;
    ClientContext contextLogIn,contextLogOut;
    Status status;
    int n;
    
    std::string name;
    std::cout << "Choose your nickname(letters only!): " << "\n";
    std::cin >> name;

    while (bool contains_non_alpha
        = std::find_if(name.begin(), name.end(),
            std::not1(std::ptr_fun((int(*)(int))std::isalpha))) != name.end()) {
        std::cout << "Name contains not-letter charcters, try again: " << "\n";
        std::cin >> name;
    }

    lirq.set_user(name);
    
    status = stub->LogIn(&contextLogIn,lirq,&lirs);

    int j = 0;
    while (lirs.success() != 1 || !status.ok()) {
        if (j > 40) {
            std::cout << "Problem with server. Closing down.\n";
            return 0;
        }
        j++;
        ClientContext contextLogIn;
        std::cout << "Log in failed.\n";
        if (lirs.success() == 3) {
            std::cout << "Username is taken. Try different one:\n";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin >> name;
            while (bool contains_non_alpha
                = std::find_if(name.begin(), name.end(),
                    std::not1(std::ptr_fun((int(*)(int))std::isalpha))) != name.end()) {
                std::cout << "Name contains not-letter charcters, try again: " << "\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin >> name;
            }
            
        }
        else if (lirs.success() == 2) {
            std::cout << "Too many users logged in. Try later\n";
            return 0;
        }
        else {
            std::cout << "Problem with server. Tring to connect.\n";
        }

        lirq.set_user(name);
        status = stub->LogIn(&contextLogIn, lirq, &lirs);
    }

    std::cout << "You logged in as " << name << "\n";

    while (1) {
        std::cout << "Choose action: " << "\n";
        std::cout << "1. Post a tweet. " << "\n";
        std::cout << "2. Show chosen number of tweets " << "\n";
        std::cout << "3. Exit " << "\n";

        std::cin >> choice;
      
        while (choice != "1" && choice != "2" && choice != "3") {
            std::cout << "No such action. Try again: " << "\n";
            std::cin >> choice;
        }

        if (choice == "1") {
            ClientContext context;
            

            std::cout << "Type your message: " << "\n";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::getline(std::cin, post);
            while (post.size() < 1 && post.size() > 40) {
                if (post.size() > 40) {
                    std::cout << "Message is to big. Try again. " << "\n";
                }
                else {
                    std::cout << "Something went wrong. Try again. " << "\n";
                }
                std::cout << "Type your message: " << "\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::getline(std::cin, post);
            }

                tweet.set_text(post);
                tweet.set_user(name);
                *(ptrq.mutable_tweet()) = tweet;

                std::cout << "You sent tweet: " << ptrq.tweet().text() << "\n ";

                ptrs.set_success(0);
                auto success = stub->PostTweet(&context, ptrq, &ptrs);
                if (!success.ok() || ptrs.success() == 0) {
                    std::cout << "Something went wrong. Error code:  " << success.error_code() << ".\n";
                }
        }
        else if(choice == "2") {
            ClientContext context;
            std::cout << "How many tweets you want to display: " << "\n";
            std::cin >> n;
            while (n < 1 || n >100) {
                std::cout << "It was not a number betwwen 1 and 100. Try again: " << "\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin >> n;
            }

            gtrq.set_n(n);
            auto success = stub->GetTweets(&context, gtrq, &gtrs);

            for (int i = gtrs.tweets().size() - 1;i > -1;i--) {
                std::cout << gtrs.tweets(i).user() << ":" << gtrs.tweets(i).text() << "\n";
            }
        } else {
            break;
        }
    }

    lorq.set_user(name);
    status = stub->LogOut(&contextLogOut, lorq, &lors);
    int k = 0;
    while (lors.success() != 1 || !status.ok()) {
        ClientContext contextLogOut;
        k++;
        std::cout << "Logging out failed.\n";
        if (lors.success() == 2) {
            std::cout << "No such user.\n";
        }
        else {
            std::cout << "Problem with server. Tring again.\n";
        }

        status = stub->LogOut(&contextLogOut, lorq, &lors);
        if (k > 40) {
            std::cout << "Problem with server. Closing down.\n";
            return 0;
        }
    }
    

    return 0;
}

