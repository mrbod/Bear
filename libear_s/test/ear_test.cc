#include "gtest/gtest.h"

#include "string_functions.h"
#include "environment.h"

namespace {

    TEST(get_array_end_test, NotCrashOnNull) {
        const char **input = nullptr;

        EXPECT_EQ(nullptr, get_array_end(input));
    }

    TEST(get_array_end_test, NotCrashOnEmpty) {
        const char *input[] = { nullptr };

        EXPECT_EQ(&input[0], get_array_end(input));
    }

    TEST(get_array_end_test, FindsEnd) {
        const char *input0 = "this";
        const char *input1 = "that";
        const char *input[] = { input0, input1, 0 };

        EXPECT_EQ(&input[2], get_array_end(input));
    }

    TEST(get_array_end_test, NotCrashOnNEmptyString) {
        const char input[] = "";

        EXPECT_EQ(&input[0], get_array_end(input));
    }

    TEST(get_array_end_test, FindStringEnd) {
        const char input[] = "this";
        const size_t input_size = sizeof(input);

        EXPECT_EQ(&input[input_size - 1], get_array_end(input));
    }

    TEST(get_array_length_test, NotCrashOnNull) {
        const char **input = nullptr;

        EXPECT_EQ(0, get_array_length(input));
    }

    TEST(get_array_length_test, NotCrashOnEmpty) {
        const char *input[] = { nullptr };

        EXPECT_EQ(0, get_array_length(input));
    }

    TEST(get_array_length_test, FindsEnd) {
        const char *input0 = "this";
        const char *input1 = "that";
        const char *input[] = { input0, input1, 0 };

        EXPECT_EQ(2, get_array_length(input));
    }

    TEST(get_array_length_test, NotCrashOnNEmptyString) {
        const char input[] = "";

        EXPECT_EQ(0, get_array_length(input));
    }

    TEST(get_array_length_test, FindStringEnd) {
        const char input[] = "this";
        const size_t input_size = sizeof(input) / sizeof(const char);

        EXPECT_EQ(input_size - 1, get_array_length(input));
    }

    constexpr static char key[] = "this";
    constexpr static size_t key_size = sizeof(key) / sizeof(char);

    TEST(get_env_value_test, FindIfItIsThere) {
        const char *input[] = { "key1=value1", "this=that", "key2=value2", nullptr };
        const size_t input_size = sizeof(input) / sizeof(const char*);
        const char **begin = input;
        const char **end = begin + input_size;

        const char *key_begin = key;
        const char *key_end = key_begin + 4;

        EXPECT_STREQ("that", get_env_value(begin, end, key_begin, key_end));
    }

    TEST(get_env_value_test, DontFindIfItIsNotThere) {
        const char *input[] = { "these=those", nullptr };
        const size_t input_size = sizeof(input) / sizeof(const char*);
        const char **begin = input;
        const char **end = begin + input_size;

        const char *key_begin = key;
        const char *key_end = key_begin + 4;

        EXPECT_STREQ(nullptr, get_env_value(begin, end, key_begin, key_end));
    }

    TEST(get_env_value_test, DontFindLongerKeys) {
        const char *input[] = { "thisisit=that", nullptr };
        const size_t input_size = sizeof(input) / sizeof(const char*);
        const char **begin = input;
        const char **end = begin + input_size;

        const char *key_begin = key;
        const char *key_end = key_begin + 4;

        EXPECT_STREQ(nullptr, get_env_value(begin, end, key_begin, key_end));
    }

    constexpr static size_t buffer_size = 256;

    TEST(capture_env_value_test, ReturnFalseIfNotThere) {
        const char *input[] = { "thisisit=that", nullptr };
        char buffer[buffer_size];

        EXPECT_EQ(false, capture_env_value(input, key, buffer, buffer_size));
    }

    TEST(capture_env_value_test, ReturnTrueIfIsThere) {
        const char *input[] = { "this=that", nullptr };
        char buffer[buffer_size];

        EXPECT_EQ(true, capture_env_value(input, key, buffer, buffer_size));
    }

    TEST(capture_env_value_test, CopyContent) {
        const char *input[] = { "this=that", nullptr };
        char buffer[buffer_size];

        capture_env_value(input, key, buffer, buffer_size);

        EXPECT_STREQ("that", buffer);
    }

}
