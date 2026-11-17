#pragma once
#include <string>
#include <vector>

struct Dish { int id; std::string name; long long price; int popularity; bool available; };
struct Table { int id; std::string name; bool occupied; };
struct OrderItem { int dishId; std::string name; long long unitPrice; int qty; };
struct Order { int id; int tableId; bool open; std::string startedAt; std::vector<OrderItem> items; };
struct Receipt { int id; int orderId; int tableId; long long total; std::string paidAt; };

class Restaurant {
public:
    explicit Restaurant(std::string dataDir = "data");
    void run();
private:
    std::string dataDir_;
    std::vector<Dish> dishes_;
    std::vector<Table> tables_;
    std::vector<Order> orders_;
    std::vector<Receipt> receipts_;

    void load();
    void seed();
    void save() const;
    void showTables() const;
    void addTable();
    void showDishes(bool sortByPrice = false) const;
    void addDish();
    void updateDish();
    void searchDish() const;
    void openOrder();
    void addItem();
    void showOrder() const;
    void removeItem();
    void checkout();
    void report() const;
    void suggestCombo() const;
    Dish* findDish(int id);
    const Dish* findDish(int id) const;
    Table* findTable(int id);
    const Table* findTable(int id) const;
    Order* openOrderForTable(int tableId);
    const Order* openOrderForTable(int tableId) const;
};
