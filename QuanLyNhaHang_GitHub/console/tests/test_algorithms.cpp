#include "algorithms.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

int main() {
    {std::vector<int> xs;
     assert(algo::binarySearch(xs,3,[](int x){return x;})==-1);
     assert(algo::linearSearch(xs,3,[](int x){return x;})==-1);}
    {std::vector<int> xs{1,2,2,4,7,8};
     assert(algo::binarySearch(xs,2,[](int x){return x;})==1);
     assert(algo::binarySearch(xs,5,[](int x){return x;})==-1);
     assert(algo::binarySearch(xs,0,[](int x){return x;})==-1);}
    std::mt19937 rng(2026);
    for(int n:{0,1,2,5,10,100,1000}) {
        std::vector<int> a(n);
        for(auto& v:a)v=static_cast<int>(rng()%100);
        auto b=a;
        algo::mergeSort(a,[](int x,int y){return x<y;});
        std::stable_sort(b.begin(),b.end());
        assert(a==b);
    }
    {std::vector<algo::KnapItem> items{{1,3,4},{2,4,5},{3,5,7}};
     auto r=algo::knapsack01(items,7);
     assert(r.bestScore==9);
     assert(r.totalCost==7);
     assert((r.ids==std::vector<int>{1,2}));}
    {std::vector<algo::KnapItem> items{{1,7,9},{2,4,5}};
     auto r=algo::knapsack01(items,2);
     assert(r.bestScore==0 && r.ids.empty());}
    // Doi chieu nghiem knapsack voi brute force tren du lieu nho.
    for(int test=0;test<100;++test) {
        int n=1+static_cast<int>(rng()%12);
        int B=5+static_cast<int>(rng()%50);
        std::vector<algo::KnapItem> items;
        for(int i=0;i<n;++i)items.push_back({i+1,1+static_cast<int>(rng()%20),1+static_cast<int>(rng()%15)});
        int brute=0;
        for(int mask=0;mask<(1<<n);++mask){
            int cost=0,score=0;
            for(int i=0;i<n;++i)if(mask&(1<<i)){cost+=items[i].cost;score+=items[i].score;}
            if(cost<=B)brute=std::max(brute,score);
        }
        auto r=algo::knapsack01(items,B);
        assert(r.bestScore==brute);
        int cost=0,score=0;
        for(int id:r.ids){auto& it=items[id-1];cost+=it.cost;score+=it.score;}
        assert(score==r.bestScore && cost==r.totalCost && cost<=B);
    }
    std::cout<<"ALL ALGORITHM TESTS PASSED (including 100 random exhaustive checks)\n";
}
