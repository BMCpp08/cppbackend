#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "../src/tv.h"

// Тестовый стенд "Телевизор по умолчанию"
class TVByDefault : public testing::Test {
protected:
    TV tv_;
};
TEST_F(TVByDefault, IsOff) {
    EXPECT_FALSE(tv_.IsTurnedOn());
}
TEST_F(TVByDefault, DoesntShowAChannelWhenItIsOff) {
    EXPECT_FALSE(tv_.GetChannel().has_value());
}
// Включите этот тест и доработайте класс TV, чтобы тест выполнился успешно
#if 0
TEST_F(TVByDefault, CantSelectAnyChannel) {
    EXPECT_THROW(tv_.SelectChannel(10), std::logic_error);
    EXPECT_EQ(tv_.GetChannel(), std::nullopt);
    tv_.TurnOn();
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(1));
}
#endif

// Тестовый стенд "Включенный телевизор"
class TurnedOnTV : public TVByDefault {
protected:
    void SetUp() override {
        tv_.TurnOn();
    }
};
TEST_F(TurnedOnTV, ShowsChannel1) {
    EXPECT_TRUE(tv_.IsTurnedOn());
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(1));
}
TEST_F(TurnedOnTV, AfterTurningOffTurnsOffAndDoesntShowAnyChannel) {
    tv_.TurnOff();
    EXPECT_FALSE(tv_.IsTurnedOn());
    // Сравнение с nullopt в GoogleTest выполняется так:
    EXPECT_EQ(tv_.GetChannel(), std::nullopt);
}
TEST_F(TurnedOnTV, CanSelectChannelFrom1To99) {
    EXPECT_NO_THROW(tv_.SelectChannel(5));
    auto current_channel = tv_.GetChannel();
    ASSERT_TRUE(current_channel.has_value());
    EXPECT_EQ(current_channel.value(), 5);
}

TEST_F(TurnedOnTV, CantSelectChannelGreater99) {
    EXPECT_THROW(tv_.SelectChannel(100), std::out_of_range);
    auto channel = tv_.GetChannel();
    ASSERT_TRUE(channel.has_value());
    EXPECT_EQ(channel.value(), 1);
}

TEST_F(TurnedOnTV, CantSelectChannelLess1) {
    EXPECT_THROW(tv_.SelectChannel(0), std::out_of_range);
    auto channel = tv_.GetChannel();
    ASSERT_TRUE(channel.has_value());
    EXPECT_EQ(channel.value(), 1);
}

TEST_F(TurnedOnTV, TestSelectPreviousChannelReturnsToInitial) {
    auto initial_channel_opt = tv_.GetChannel();
    ASSERT_TRUE(initial_channel_opt.has_value());
    int initial_channel = initial_channel_opt.value();

    int different_channel = initial_channel + 1;
    if (different_channel > 99) {
        different_channel = 99;
    }
    tv_.SelectChannel(different_channel);
    tv_.SelectLastViewedChannel();

    auto current_channel_opt = tv_.GetChannel();
    ASSERT_TRUE(current_channel_opt.has_value());
    EXPECT_EQ(current_channel_opt.value(), initial_channel);
}