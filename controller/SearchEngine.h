#pragma once
#include "model/LibraryDatabase.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

class SearchEngine {
public:
    using Results = std::vector<const LibraryItem*>;
    enum class Field { Title, Author, Type, Year };
    enum class Algorithm { Linear, Binary, Recursive };
    enum class SortAlgorithm { Insertion, Merge };

    static Results catalogue(const LibraryDatabase& db) {
        Results out;
        for (const auto& item : db.items()) out.push_back(item.get());
        return out;
    }
    static Results search(const Results& input, const std::string& query, Field field, Algorithm algorithm) {
        if (algorithm == Algorithm::Recursive) return recursiveSearch(input, query, field, 0);
        if (algorithm == Algorithm::Binary) {
            if (field == Field::Type || field == Field::Year) return linearSearch(input, query, field);
            Results sorted = input;
            insertionSort(sorted, field);
            return binarySearch(sorted, query, field);
        }
        return linearSearch(input, query, field);
    }
    static void sort(Results& values, Field field, SortAlgorithm algorithm) {
        if (algorithm == SortAlgorithm::Insertion) insertionSort(values, field);
        else if (values.size() > 1) mergeSort(values, field, 0, values.size());
    }
private:
    static std::string lower(std::string value) {
        for (char& ch : value) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        return value;
    }
    static std::string key(const LibraryItem* item, Field field) {
        switch (field) {
            case Field::Title: return lower(item->title());
            case Field::Author: return lower(item->author());
            case Field::Type: return lower(item->type());
            case Field::Year: return std::to_string(item->year());
        }
        return {};
    }
    static bool less(const LibraryItem* a, const LibraryItem* b, Field field) {
        if (field == Field::Year && a->year() != b->year()) return a->year() < b->year();
        const auto ak = key(a, field), bk = key(b, field);
        return ak == bk ? a->id() < b->id() : ak < bk;
    }
    static bool contains(const LibraryItem* item, const std::string& query, Field field) {
        return key(item, field).find(lower(query)) != std::string::npos;
    }
    static Results linearSearch(const Results& input, const std::string& query, Field field) {
        Results found;
        for (const auto* item : input) if (contains(item, query, field)) found.push_back(item);
        return found;
    }
    static Results recursiveSearch(const Results& input, const std::string& query, Field field, std::size_t index) {
        if (index == input.size()) return {};
        Results found = recursiveSearch(input, query, field, index + 1);
        if (contains(input[index], query, field)) found.insert(found.begin(), input[index]);
        return found;
    }
    static Results binarySearch(const Results& sorted, const std::string& query, Field field) {
        // Lower-bound search for the first prefix match; then scan matching neighbours.
        const std::string needle = lower(query);
        std::size_t lo = 0, hi = sorted.size();
        while (lo < hi) {
            const std::size_t mid = lo + (hi - lo) / 2;
            if (key(sorted[mid], field) < needle) lo = mid + 1;
            else hi = mid;
        }
        Results found;
        for (std::size_t i = lo; i < sorted.size(); ++i) {
            const auto value = key(sorted[i], field);
            if (value.compare(0, needle.size(), needle) != 0) break;
            found.push_back(sorted[i]);
        }
        return found;
    }
    static void insertionSort(Results& values, Field field) {
        for (std::size_t i = 1; i < values.size(); ++i) {
            const auto* current = values[i];
            std::size_t j = i;
            while (j > 0 && less(current, values[j - 1], field)) {
                values[j] = values[j - 1]; --j;
            }
            values[j] = current;
        }
    }
    static void mergeSort(Results& values, Field field, std::size_t begin, std::size_t end) {
        if (end - begin <= 1) return;
        const auto middle = begin + (end - begin) / 2;
        mergeSort(values, field, begin, middle);
        mergeSort(values, field, middle, end);
        Results merged;
        std::size_t i = begin, j = middle;
        while (i < middle && j < end) {
            if (!less(values[j], values[i], field)) merged.push_back(values[i++]);
            else merged.push_back(values[j++]);
        }
        while (i < middle) merged.push_back(values[i++]);
        while (j < end) merged.push_back(values[j++]);
        for (std::size_t k = 0; k < merged.size(); ++k) values[begin + k] = merged[k];
    }
};
