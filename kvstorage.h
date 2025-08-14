#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <thread>
#include <span>
#include <unordered_map>
#include <optional>
#include <algorithm>

template <typename Clock>
class KVStorage {
public:
    using TimePoint = typename Clock::time_point;

    explicit KVStorage(std::span<std::tuple<std::string, std::string, uint32_t>> entries, Clock& clock = Clock()) : clock_(clock) {
        map_.reserve(entries.size());
        for (const auto& [key, value, ttl] : entries) {
            set(key, value, ttl);
        }
    }

    ~KVStorage() = default;

    void set(std::string key, std::string value, uint32_t ttl) {
        TimePoint expiration_time;
        if (ttl == 0) {
            expiration_time = TimePoint::max();
        }
        else {
            expiration_time = clock_.now() + std::chrono::seconds(ttl);
        }
        map_[key] = { value, expiration_time };
    }

    bool remove(std::string_view key) {
        return map_.erase(std::string(key)) > 0;
    }

    std::optional<std::string> get(std::string_view key) const {
        auto it = map_.find(std::string(key));
        if (it == map_.end()) {
            return std::nullopt;
        }
        if (clock_.now() >= it->second.expiration_time) {
            return std::nullopt;
        }
        return it->second.value;
    }

    std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const {
        std::vector<std::pair<std::string, std::string>> result;
        if (count == 0) {
            return result;
        }

        std::vector<std::string> keys;
        keys.reserve(map_.size());
        for (const auto& pair : map_) {
            keys.push_back(pair.first);
        }
        std::sort(keys.begin(), keys.end());

        auto it = std::lower_bound(keys.begin(), keys.end(), key);
        while (it != keys.end() && result.size() < count) {
            const auto& cur_key = *it;
            const auto& entry = map_.at(cur_key);

            if (clock_.now() < entry.expiration_time) {
                result.push_back({ cur_key, entry.value });
            }
            ++it;
        }
        return result;
    }

    std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry() {
        for (auto it = map_.begin(); it != map_.end(); ++it) {
            if (clock_.now() >= it->second.expiration_time && it->second.expiration_time != TimePoint::max()) {
                auto result = std::make_pair(it->first, it->second.value);
                map_.erase(it);
                return result;
            }
        }
        return std::nullopt;
    }

private:
    struct ValueEntry {
        std::string value;
        TimePoint expiration_time;
    };

    Clock& clock_;
    std::unordered_map<std::string, ValueEntry> map_;
};