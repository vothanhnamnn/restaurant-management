#include "restaurant.hpp"
#include <exception>
#include <iostream>
int main(int argc,char** argv) {
    try {
        Restaurant restaurant(argc>1?argv[1]:"data");
        restaurant.run();
    } catch(const std::exception& e) {
        std::cerr<<"Loi: "<<e.what()<<'\n';
        return 1;
    }
    return 0;
}
