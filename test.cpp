#include <gtest/gtest.h>
#include "kvstorage.h"
#include <chrono>
#include <string>
#include <vector>

struct FakeClock {
    using time_point = std::chrono::steady_clock::time_point;
    FakeClock() : now_(std::chrono::steady_clock::now()) {}
    time_point now() const { return now_; }
    void advanceSeconds(int sec) { now_ += std::chrono::seconds(sec); }
private:
    time_point now_;
};

TEST(KVStorageTest, SetAndGet) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {
        {"key1", "val1", 11},
        {"key2", "val2", 20}
    };

    FakeClock clock;
    KVStorage<FakeClock> storage(data, clock);

    auto val1 = storage.get("key1");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(val1.value(), "val1");

    auto val2 = storage.get("key2");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(val2.value(), "val2");
}

TEST(KVStorageTest, Expiration) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {
        {"temp", "data", 5}
    };

    FakeClock clock;
    KVStorage<FakeClock> storage(data, clock);

    EXPECT_TRUE(storage.get("temp").has_value());

    clock.advanceSeconds(6);
    auto val = storage.get("temp");
    EXPECT_FALSE(val.has_value());
}

TEST(KVStorageTest, UpdateExistingKey) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {
        {"key1", "old_value", 10}
    };

    FakeClock clock;
    KVStorage<FakeClock> storage(data, clock);

    EXPECT_EQ(storage.get("key1").value(), "old_value");

    storage.set("key1", "new_value", 10);
    EXPECT_EQ(storage.get("key1").value(), "new_value");
}

TEST(KVStorageTest, InsertNewKey) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data;

    FakeClock clock;
    KVStorage<FakeClock> storage(data, clock);

    storage.set("newkey", "newdata", 15);
    EXPECT_TRUE(storage.get("newkey").has_value());
    EXPECT_EQ(storage.get("newkey").value(), "newdata");
}

TEST(KVStorageTest, MissingKey) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {
        {"key1", "value1", 10}
    };

    FakeClock clock;
    KVStorage<FakeClock> storage(data, clock);

    EXPECT_FALSE(storage.get("not_found").has_value());
}

TEST(KVStorageTest, MultipleExpirationTimes) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {
        {"fast", "short_life", 3},
        {"slow", "long_life", 10}
    };

    FakeClock clock;
    KVStorage<FakeClock> storage(data, clock);

    EXPECT_TRUE(storage.get("fast").has_value());
    EXPECT_TRUE(storage.get("slow").has_value());

    clock.advanceSeconds(4);
    EXPECT_FALSE(storage.get("fast").has_value());
    EXPECT_TRUE(storage.get("slow").has_value());

    clock.advanceSeconds(7);
    EXPECT_FALSE(storage.get("slow").has_value());
}