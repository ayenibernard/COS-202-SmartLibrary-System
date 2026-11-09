#pragma once
#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

// Fixed-size cache of the most frequently accessed catalogue IDs.
// IDs (rather than pointers) remain safe when the catalogue changes.
class FrequentItemCache {
public:
    static constexpr std::size_t Capacity = 5;
    void recordAccess(const std::string& id) {
        ++counts_[id];
        rebuild();
    }
    void remove(const std::string& id) {
        counts_.erase(id);
        rebuild();
    }
    void clear() { counts_.clear(); top_.fill(""); size_ = 0; }
    std::size_t size() const { return size_; }
    std::string at(std::size_t index) const { return index < size_ ? top_[index] : ""; }
    std::size_t accessCount(const std::string& id) const {
        auto it = counts_.find(id);
        return it == counts_.end() ? 0 : it->second;
    }
private:
    void rebuild() {
        std::vector<std::pair<std::string,std::size_t>> ranked(counts_.begin(), counts_.end());
        std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
            return a.second != b.second ? a.second > b.second : a.first < b.first;
        });
        top_.fill("");
        size_ = std::min(Capacity, ranked.size());
        for (std::size_t i = 0; i < size_; ++i) top_[i] = ranked[i].first;
    }
    std::unordered_map<std::string,std::size_t> counts_;
    std::array<std::string, Capacity> top_{};
    std::size_t size_ = 0;
};
