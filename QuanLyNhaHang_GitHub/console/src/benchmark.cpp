#include "algorithms.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using Clock=std::chrono::steady_clock;
template <class F> double measureMs(F&& f) {
    auto start=Clock::now(); f(); auto end=Clock::now();
    return std::chrono::duration<double,std::milli>(end-start).count();
}
int main(int argc,char** argv) {
    const std::uint32_t seed=20261117;
    std::mt19937 rng(seed);
    std::cout<<"algorithm,n,parameter,elapsed_ms,notes\n"<<std::fixed<<std::setprecision(4);
    volatile long long guard=0;
    for(int n:{1000,10000,100000,300000}) {
        std::vector<int> values(n); std::iota(values.begin(),values.end(),1);
        std::shuffle(values.begin(),values.end(),rng);
        auto sorted=values;
        double prep=measureMs([&](){algo::mergeSort(sorted,[](int a,int b){return a<b;});});
        std::uniform_int_distribution<int> key(1,n*2);
        std::vector<int> queries(10000);
        for(auto& q:queries)q=key(rng);
        double linear=measureMs([&](){for(int q:queries)guard+=algo::linearSearch(values,q,[](int x){return x;})>=0;});
        double binary=measureMs([&](){for(int q:queries)guard+=algo::binarySearch(sorted,q,[](int x){return x;})>=0;});
        std::cout<<"linear_search,"<<n<<",10000_queries,"<<linear<<",unsorted\n";
        std::cout<<"binary_search,"<<n<<",10000_queries,"<<binary<<",pre_sorted\n";
        std::cout<<"merge_sort_id_preprocess,"<<n<<",sort_once,"<<prep<<",not_in_binary_search_time\n";
    }
    for(int n:{1000,10000,100000,300000}) {
        std::uniform_int_distribution<int> price(1000,1000000);
        std::vector<int> original(n);
        for(auto& x:original)x=price(rng);
        auto a=original,b=original;
        double merge=measureMs([&](){algo::mergeSort(a,[](int x,int y){return x<y;});});
        double stl=measureMs([&](){std::sort(b.begin(),b.end());});
        if(a!=b){std::cerr<<"Sorting mismatch\n";return 2;}
        std::cout<<"merge_sort,"<<n<<",random_prices,"<<merge<<",O_nlogn\n";
        std::cout<<"std_sort_reference,"<<n<<",random_prices,"<<stl<<",baseline\n";
    }
    // Workload kieu don vi nghin VND; B <= 5000 de trace khong qua nang.
    for(const auto& [n,b]:std::vector<std::pair<int,int>>{{50,500},{100,1000},{200,2000},{400,4000}}) {
        std::uniform_int_distribution<int> cost(5,180),score(1,10);
        std::vector<algo::KnapItem> items;
        for(int i=0;i<n;++i) items.push_back({i+1,cost(rng),score(rng)});
        algo::KnapResult result{};
        double elapsed=measureMs([&](){result=algo::knapsack01(items,b);});
        guard+=result.bestScore;
        std::cout<<"knapsack_01,"<<n<<",budget_"<<b<<","<<elapsed<<",O_nB\n";
    }
    std::cerr<<"seed="<<seed<<"; compiler=build_machine; checksum="<<guard<<"\n";
    return 0;
}
