#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <utility>

namespace algo {

// Tiem kiem tuyen tinh, O(n): gia tri dung khi du lieu chua sap xep.
template <class T, class Key, class KeyFn>
int linearSearch(const std::vector<T>& a, const Key& target, KeyFn keyFn) {
    for (std::size_t i = 0; i < a.size(); ++i)
        if (keyFn(a[i]) == target) return static_cast<int>(i);
    return -1;
}

// Tiem kiem nhi phan, O(log n), YEU CAU mang tang dan theo khoa.
template <class T, class Key, class KeyFn>
int binarySearch(const std::vector<T>& sorted, const Key& target, KeyFn keyFn) {
    std::size_t l = 0, r = sorted.size();
    while (l < r) {
        const std::size_t m = l + (r - l) / 2;
        const auto key = keyFn(sorted[m]);
        if (key < target) l = m + 1;
        else r = m;
    }
    if (l < sorted.size() && keyFn(sorted[l]) == target) return static_cast<int>(l);
    return -1;
}

// Merge sort on dinh: O(n log n), O(n) bo nho phu.
template <class T, class Compare>
void mergeSort(std::vector<T>& a, Compare comp) {
    if (a.size() < 2) return;
    std::vector<T> tmp(a.size());
    for (std::size_t width = 1; width < a.size();) {
        for (std::size_t l = 0; l < a.size(); l += 2 * width) {
            const std::size_t m = std::min(l + width, a.size());
            const std::size_t r = std::min(l + 2 * width, a.size());
            std::size_t i = l, j = m, k = l;
            while (i < m && j < r) {
                if (!comp(a[j], a[i])) tmp[k++] = a[i++];
                else tmp[k++] = a[j++];
            }
            while (i < m) tmp[k++] = a[i++];
            while (j < r) tmp[k++] = a[j++];
        }
        a.swap(tmp);
        if (width > a.size() / 2) break; // tran so width*2
        width *= 2;
    }
}

struct KnapItem { int id; int cost; int score; };
struct KnapResult { int bestScore; int totalCost; std::vector<int> ids; };

// Quy hoach dong 0/1 Knapsack, O(n*budget) time, O(n*budget) trace bits.
// cost/budget cung don vi, score >= 0, moi mon chi chon toi da 1 lan.
inline KnapResult knapsack01(const std::vector<KnapItem>& items, int budget) {
    if (budget < 0 || budget > 100000)
        throw std::invalid_argument("Budget phai nam trong [0, 100000]");
    if (items.size() > 5000)
        throw std::invalid_argument("Qua nhieu mon de truy vet nghiem");
    std::vector<int> dp(static_cast<std::size_t>(budget) + 1, 0);
    std::vector<std::vector<std::uint8_t>> take(items.size(),
                                      std::vector<std::uint8_t>(static_cast<std::size_t>(budget) + 1, 0));
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (items[i].cost <= 0 || items[i].score < 0)
            throw std::invalid_argument("Item cost > 0 va score >= 0");
        if (items[i].cost > budget) continue;
        for (int b = budget; b >= items[i].cost; --b) {
            int proposal = dp[b - items[i].cost] + items[i].score;
            if (proposal > dp[b]) {
                dp[b] = proposal;
                take[i][b] = 1;
            }
        }
    }
    int b = budget;
    std::vector<int> picked;
    int total = 0;
    for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
        if (take[i][b]) {
            picked.push_back(items[i].id);
            b -= items[i].cost;
            total += items[i].cost;
        }
    }
    std::reverse(picked.begin(), picked.end());
    return {dp[budget], total, std::move(picked)};
}
}
