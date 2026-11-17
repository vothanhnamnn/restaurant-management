#include "restaurant.hpp"
#include "algorithms.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace {
std::string inputLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) throw std::runtime_error("Dong nhap da ket thuc");
    return s;
}
long long readInteger(const std::string& prompt, long long low, long long high) {
    while (true) {
        auto s = inputLine(prompt);
        std::istringstream ss(s);
        long long v;
        char leftover;
        if ((ss >> v) && !(ss >> leftover) && v >= low && v <= high) return v;
        std::cout << "  ! Vui long nhap so trong [" << low << ", " << high << "]\n";
    }
}
std::string now() {
    std::time_t t = std::time(nullptr);
    std::tm v{};
#ifdef _WIN32
    localtime_s(&v, &t);
#else
    localtime_r(&t, &v);
#endif
    std::ostringstream os;
    os << std::put_time(&v, "%Y-%m-%d_%H:%M:%S");
    return os.str();
}
template <class T, class F>
int nextId(const std::vector<T>& xs, F getId) {
    int result = 1;
    for (auto& x: xs) result = std::max(result, getId(x) + 1);
    return result;
}
std::string cleanName(std::string x) {
    for (char& c : x) if (c == '\t' || c == '\r' || c == '\n') c = ' ';
    if (x.size() > 80) x.resize(80);
    return x;
}
}

Restaurant::Restaurant(std::string dataDir) : dataDir_(std::move(dataDir)) { load(); }

void Restaurant::seed() {
    tables_.clear(); dishes_.clear();
    for (int i=1;i<=8;++i) tables_.push_back({i, "Ban " + std::to_string(i), false});
    dishes_ = {
        {101,"Pho bo",45000,9,true}, {102,"Bun cha",50000,9,true},
        {103,"Com rang",40000,7,true}, {104,"Ga nuong",99000,10,true},
        {105,"Nem ran",35000,8,true}, {106,"Canh chua",55000,7,true},
        {107,"Rau xao",30000,6,true}, {108,"Ca kho",85000,9,true},
        {109,"Bo luc lac",120000,10,true}, {110,"Che",25000,5,true},
        {111,"Tra da",10000,4,true}, {112,"Nuoc cam",30000,6,true}
    };
}

void Restaurant::load() {
    std::filesystem::create_directories(dataDir_);
    auto path = [&](const char* n){ return std::filesystem::path(dataDir_) / n; };
    bool newDb = !std::filesystem::exists(path("dishes.db"));
    if (newDb) { seed(); save(); return; }
    {
        std::ifstream f(path("dishes.db"));
        Dish d{}; int a;
        while (f >> d.id >> std::quoted(d.name) >> d.price >> d.popularity >> a) {
            d.available = a != 0; dishes_.push_back(d);
        }
    }
    {
        std::ifstream f(path("tables.db"));
        Table t{}; int x;
        while (f >> t.id >> std::quoted(t.name) >> x) {
            t.occupied = x != 0; tables_.push_back(t);
        }
    }
    {
        std::ifstream f(path("orders.db"));
        int id, tid, open, count;
        std::string at;
        while (f >> id >> tid >> open >> std::quoted(at) >> count) {
            if (count < 0 || count > 10000) break;
            Order o{id,tid,open != 0,at,{}};
            bool valid = true;
            for (int i=0;i<count;++i) {
                OrderItem it{};
                if (!(f >> it.dishId >> std::quoted(it.name) >> it.unitPrice >> it.qty)) {valid=false;break;}
                o.items.push_back(it);
            }
            if (!valid) break;
            orders_.push_back(o);
        }
    }
    {
        std::ifstream f(path("receipts.db"));
        Receipt r{};
        while (f >> r.id >> r.orderId >> r.tableId >> r.total >> std::quoted(r.paidAt)) receipts_.push_back(r);
    }
    // Tinh lai occupied tu cac don mo, thay vi tin vao co stored state.
    for (auto& t:tables_) t.occupied = openOrderForTable(t.id) != nullptr;
}

void Restaurant::save() const {
    auto path = [&](const char* n){ return std::filesystem::path(dataDir_) / n; };
    std::filesystem::create_directories(dataDir_);
    std::ofstream d(path("dishes.db"),std::ios::trunc), t(path("tables.db"),std::ios::trunc);
    std::ofstream o(path("orders.db"),std::ios::trunc), r(path("receipts.db"),std::ios::trunc);
    if (!d || !t || !o || !r) throw std::runtime_error("Khong the mo file luu tru trong " + dataDir_);
    for (const auto& x:dishes_) d<<x.id<<' '<<std::quoted(x.name)<<' '<<x.price<<' '<<x.popularity<<' '<<x.available<<'\n';
    for (const auto& x:tables_) t<<x.id<<' '<<std::quoted(x.name)<<' '<<x.occupied<<'\n';
    for (const auto& x:orders_) {
        o<<x.id<<' '<<x.tableId<<' '<<x.open<<' '<<std::quoted(x.startedAt)<<' '<<x.items.size()<<'\n';
        for(const auto& it:x.items) o<<it.dishId<<' '<<std::quoted(it.name)<<' '<<it.unitPrice<<' '<<it.qty<<'\n';
    }
    for (const auto& x:receipts_) r<<x.id<<' '<<x.orderId<<' '<<x.tableId<<' '<<x.total<<' '<<std::quoted(x.paidAt)<<'\n';
}

Dish* Restaurant::findDish(int id) {
    auto it = std::find_if(dishes_.begin(),dishes_.end(),[&](const Dish& d){return d.id==id;});
    return it==dishes_.end()?nullptr:&*it;
}
const Dish* Restaurant::findDish(int id) const {
    auto it = std::find_if(dishes_.begin(),dishes_.end(),[&](const Dish& d){return d.id==id;});
    return it==dishes_.end()?nullptr:&*it;
}
Table* Restaurant::findTable(int id) {
    auto it=std::find_if(tables_.begin(),tables_.end(),[&](const Table& x){return x.id==id;});
    return it==tables_.end()?nullptr:&*it;
}
const Table* Restaurant::findTable(int id) const {
    auto it=std::find_if(tables_.begin(),tables_.end(),[&](const Table& x){return x.id==id;});
    return it==tables_.end()?nullptr:&*it;
}
Order* Restaurant::openOrderForTable(int tableId) {
    auto it=std::find_if(orders_.begin(),orders_.end(),[&](const Order& o){return o.tableId==tableId && o.open;});
    return it==orders_.end()?nullptr:&*it;
}
const Order* Restaurant::openOrderForTable(int tableId) const {
    auto it=std::find_if(orders_.begin(),orders_.end(),[&](const Order& o){return o.tableId==tableId && o.open;});
    return it==orders_.end()?nullptr:&*it;
}
void Restaurant::showTables() const {
    std::cout << "\n ID   TEN BAN                TRANG THAI\n";
    for(const auto& t:tables_) std::cout << std::setw(3)<<t.id<<"   "<<std::left<<std::setw(22)<<t.name
        <<(t.occupied?"DANG PHUC VU":"TRONG")<<std::right<<'\n';
}
void Restaurant::addTable() {
    auto name=cleanName(inputLine("Ten ban: "));
    if (name.empty()) {std::cout<<"Ten ban rong!\n";return;}
    int id=nextId(tables_,[](const Table& t){return t.id;});
    tables_.push_back({id,name,false}); save();
    std::cout<<"Da them ban #"<<id<<"\n";
}
void Restaurant::showDishes(bool sortByPrice) const {
    auto a=dishes_;
    if(sortByPrice) {
        algo::mergeSort(a,[](const Dish& x,const Dish& y){return x.price == y.price ? x.id < y.id : x.price < y.price;});
    } else {
        algo::mergeSort(a,[](const Dish& x,const Dish& y){return x.id < y.id;});
    }
    std::cout<<"\n ID    MON AN                    GIA(VND)    DIEM   MO BAN\n";
    for (const auto& d:a) {
        std::cout<<std::setw(4)<<d.id<<"   "<<std::left<<std::setw(26)<<d.name<<std::right
                 <<std::setw(10)<<d.price<<"   "<<std::setw(3)<<d.popularity<<"    "
                 <<(d.available?"CO":"KHONG")<<'\n';
    }
}
void Restaurant::addDish() {
    auto name=cleanName(inputLine("Ten mon: "));
    if (name.empty()) {std::cout<<"Ten mon rong!\n";return;}
    long long price=readInteger("Gia VND (1000..10000000): ",1000,10000000);
    int score=static_cast<int>(readInteger("Diem ua thich (1..10): ",1,10));
    int id=nextId(dishes_,[](const Dish& d){return d.id;});
    dishes_.push_back({id,name,price,score,true}); save();
    std::cout<<"Da them mon #"<<id<<"\n";
}
void Restaurant::updateDish() {
    int id=static_cast<int>(readInteger("Ma mon: ",1,100000000));
    Dish* d=findDish(id);
    if(!d){std::cout<<"Khong tim thay!\n";return;}
    std::cout<<"1. Sua gia  2. Sua diem  3. Bat/tat kinh doanh\n";
    int k=static_cast<int>(readInteger("Chon: ",1,3));
    if(k==1) d->price=readInteger("Gia moi: ",1000,10000000);
    if(k==2) d->popularity=static_cast<int>(readInteger("Diem moi: ",1,10));
    if(k==3) d->available=!d->available;
    save();std::cout<<"Da cap nhat.\n";
}
void Restaurant::searchDish() const {
    int id=static_cast<int>(readInteger("Nhap ma mon can tim: ",1,100000000));
    auto a=dishes_;
    algo::mergeSort(a,[](const Dish& x,const Dish& y){return x.id < y.id;});
    int i=algo::binarySearch(a,id,[](const Dish& d){return d.id;});
    if(i<0) std::cout<<"Khong co mon ma "<<id<<"\n";
    else std::cout<<"Tim thay: "<<a[i].name<<" - "<<a[i].price<<" VND\n";
}
void Restaurant::openOrder() {
    int id=static_cast<int>(readInteger("Ma ban: ",1,100000000));
    Table* t=findTable(id);
    if(!t){std::cout<<"Ban khong ton tai!\n";return;}
    if(openOrderForTable(id)){std::cout<<"Ban dang co don mo!\n";return;}
    int orderId=nextId(orders_,[](const Order& o){return o.id;});
    orders_.push_back({orderId,id,true,now(),{}});
    t->occupied=true;save();
    std::cout<<"Da tao don #"<<orderId<<" cho "<<t->name<<"\n";
}
void Restaurant::addItem() {
    int tableId=static_cast<int>(readInteger("Ma ban: ",1,100000000));
    Order* order=openOrderForTable(tableId);
    if(!order){std::cout<<"Ban chua co don mo!\n";return;}
    int dishId=static_cast<int>(readInteger("Ma mon: ",1,100000000));
    const Dish* d=findDish(dishId);
    if(!d || !d->available){std::cout<<"Mon khong ton tai/da ngung ban.\n";return;}
    int qty=static_cast<int>(readInteger("So luong (1..100): ",1,100));
    auto it=std::find_if(order->items.begin(),order->items.end(),[&](const OrderItem& x){return x.dishId==dishId && x.unitPrice==d->price;});
    if(it==order->items.end()) order->items.push_back({d->id,d->name,d->price,qty});
    else {
        if(it->qty + qty > 10000) {std::cout<<"Qua so luong cho phep.\n";return;}
        it->qty+=qty;
    }
    save(); std::cout<<"Da them mon vao don.\n";
}
void Restaurant::showOrder() const {
    int id=static_cast<int>(readInteger("Ma ban: ",1,100000000));
    const Order* order=openOrderForTable(id);
    if(!order){std::cout<<"Ban chua co don mo.\n";return;}
    std::cout<<"\nDON #"<<order->id<<" - BAN "<<id<<" - "<<order->startedAt<<'\n';
    long long total=0;
    for(const auto& i:order->items){
        long long amount=i.unitPrice*i.qty;
        std::cout<<"  ["<<i.dishId<<"] "<<i.name<<" x "<<i.qty<<" @ "<<i.unitPrice<<" = "<<amount<<" VND\n";
        total+=amount;
    }
    std::cout<<"TONG: "<<total<<" VND\n";
}
void Restaurant::removeItem() {
    int id=static_cast<int>(readInteger("Ma ban: ",1,100000000));
    Order* order=openOrderForTable(id);
    if(!order){std::cout<<"Khong co don mo.\n";return;}
    int dishId=static_cast<int>(readInteger("Ma mon muon xoa khoi don: ",1,100000000));
    auto count=order->items.size();
    order->items.erase(std::remove_if(order->items.begin(),order->items.end(),
                    [&](const OrderItem& it){return it.dishId==dishId;}),order->items.end());
    if(count==order->items.size()) {std::cout<<"Don khong co mon nay.\n";return;}
    save();std::cout<<"Da xoa mon khoi don.\n";
}
void Restaurant::checkout() {
    int id=static_cast<int>(readInteger("Ma ban can thanh toan: ",1,100000000));
    Order* order=openOrderForTable(id);
    if(!order){std::cout<<"Khong co don mo.\n";return;}
    if(order->items.empty()){std::cout<<"Don rong; khong the thanh toan.\n";return;}
    long long total=0;
    for(const auto& it:order->items) total+=it.unitPrice*it.qty;
    std::cout<<"Tong can thu: "<<total<<" VND\n";
    int ok=static_cast<int>(readInteger("Xac nhan (1=thu tien, 0=huy): ",0,1));
    if(!ok)return;
    int rid=nextId(receipts_,[](const Receipt& r){return r.id;});
    receipts_.push_back({rid,order->id,id,total,now()});
    order->open=false;
    if(Table* t=findTable(id)) t->occupied=false;
    save();
    std::cout<<"\n============== HOA DON #"<<rid<<" ==============\n";
    std::cout<<"Ban "<<id<<", thoi gian: "<<receipts_.back().paidAt<<"\n";
    for(const auto& it:order->items) std::cout<<it.name<<" x "<<it.qty<<" = "<<it.unitPrice*it.qty<<"\n";
    std::cout<<"THANH TOAN: "<<total<<" VND\n==========================================\n";
}
void Restaurant::report() const {
    long long revenue=0;
    for(auto& r:receipts_) revenue+=r.total;
    int active=0;
    for(auto& t:tables_) active+=t.occupied;
    std::cout<<"\n===== BAO CAO =====\n"
             <<"So ban: "<<tables_.size()<<" | Ban dang phuc vu: "<<active<<'\n'
             <<"Mon trong menu: "<<dishes_.size()<<'\n'
             <<"So hoa don da thanh toan: "<<receipts_.size()<<'\n'
             <<"Tong doanh thu da thu: "<<revenue<<" VND\n";
}
void Restaurant::suggestCombo() const {
    int budgetK=static_cast<int>(readInteger("Ngan sach (nghin VND, 10..2000): ",10,2000));
    std::vector<algo::KnapItem> items;
    for(const auto& d:dishes_) if(d.available && d.price>0 && d.price<=static_cast<long long>(budgetK)*1000)
        items.push_back({d.id,static_cast<int>((d.price+999)/1000),d.popularity});
    auto result=algo::knapsack01(items,budgetK);
    if(result.ids.empty()){std::cout<<"Khong co mon phu hop.\n";return;}
    std::cout<<"\nGOI Y COMBO (moi mon toi da 1 lan)\n";
    long long totalPrice=0;
    for(int id:result.ids) {
        const auto* d=findDish(id);
        if(d){std::cout<<" - "<<d->name<<" : "<<d->price<<" VND | diem "<<d->popularity<<'\n';totalPrice+=d->price;}
    }
    std::cout<<"Tong tien: "<<totalPrice<<" VND | Tong diem ua thich: "<<result.bestScore<<'\n';
    std::cout<<"Luu y: diem ua thich la du lieu minh hoa; bai toan 0/1 khong chon lap mon.\n";
}
void Restaurant::run() {
    while(true) {
        std::cout<<"\n========== QUAN LY NHA HANG - CONSOLE =========="
                 <<"\n 1. Danh sach ban             2. Them ban"
                 <<"\n 3. Thuc don (theo ID)        4. Them mon"
                 <<"\n 5. Sua/An mon                6. Tim mon (Binary Search)"
                 <<"\n 7. Sap xep mon theo gia (Merge Sort)"
                 <<"\n 8. Mo don cho ban            9. Them mon vao don"
                 <<"\n10. Xem don                  11. Xoa mon khoi don"
                 <<"\n12. Thanh toan               13. Bao cao doanh thu"
                 <<"\n14. Goi y combo (0/1 Knapsack)"
                 <<"\n 0. Luu va thoat"
                 <<"\n=================================================\n";
        int x=static_cast<int>(readInteger("Chon: ",0,14));
        switch(x) {
            case 0: save();std::cout<<"Da luu. Tam biet!\n";return;
            case 1:showTables();break;
            case 2:addTable();break;
            case 3:showDishes();break;
            case 4:addDish();break;
            case 5:updateDish();break;
            case 6:searchDish();break;
            case 7:showDishes(true);break;
            case 8:openOrder();break;
            case 9:addItem();break;
            case 10:showOrder();break;
            case 11:removeItem();break;
            case 12:checkout();break;
            case 13:report();break;
            case 14:suggestCombo();break;
        }
    }
}
