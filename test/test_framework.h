/**
 * @file test_framework.h
 * @author Willy Firmware
 * @brief Framework de Testes Unitários para Willy ESP32-S3
 * @version 1.0
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <Arduino.h>
#include <functional>
#include <string>
#include <vector>

// Estrutura de resultado de teste
struct TestResult {
  std::string name;
  bool passed;
  std::string message;
  uint32_t duration;
  std::string details;
};

// Estrutura de teste
struct TestCase {
  std::string name;
  std::function<void()> test_func;
  std::function<void()> setup_func = nullptr;
  std::function<void()> teardown_func = nullptr;
};

// Classe de gerenciamento de testes
class TestFramework {
public:
  TestFramework();
  ~TestFramework();

  // Gerenciamento de testes
  void add_test(const std::string &name, std::function<void()> test_func);
  void add_test_with_setup(const std::string &name,
                           std::function<void()> test_func,
                           std::function<void()> setup_func,
                           std::function<void()> teardown_func);

  // Execução de testes
  void run_all_tests();
  void run_test(const std::string &test_name);
  void run_tests_matching(const std::string &pattern);

  // Resultados
  std::vector<TestResult> get_results() const;
  uint32_t get_total_tests() const;
  uint32_t get_passed_tests() const;
  uint32_t get_failed_tests() const;
  double get_success_rate() const;

  // Utilitários
  void print_results();
  void print_summary();
  void save_results_to_file(const std::string &filename);

  // Macros de asserção
  static void assert_true(bool condition, const std::string &message = "");
  static void assert_false(bool condition, const std::string &message = "");
  static void assert_equal(int expected, int actual,
                           const std::string &message = "");
  static void assert_equal(uint32_t expected, uint32_t actual,
                           const std::string &message = "");
  static void assert_equal(float expected, float actual,
                           float tolerance = 0.001,
                           const std::string &message = "");
  static void assert_equal(const std::string &expected,
                           const std::string &actual,
                           const std::string &message = "");
  static void assert_not_equal(int expected, int actual,
                               const std::string &message = "");
  static void assert_not_null(void *ptr, const std::string &message = "");
  static void assert_null(void *ptr, const std::string &message = "");
  static void assert_greater(int value, int min_value,
                             const std::string &message = "");
  static void assert_less(int value, int max_value,
                          const std::string &message = "");
  static void assert_within_range(int value, int min_value, int max_value,
                                  const std::string &message = "");

  // Testes de performance
  static void measure_performance(const std::string &operation,
                                  std::function<void()> func,
                                  uint32_t iterations = 1000);

  // Testes de memória
  static uint32_t get_free_heap();
  static uint32_t get_minimum_free_heap();
  static void assert_memory_usage(uint32_t expected_free,
                                  const std::string &message = "");

  // Testes de tempo
  static void assert_time_less(uint32_t max_time_ms, std::function<void()> func,
                               const std::string &message = "");
  static void assert_time_within(uint32_t min_time_ms, uint32_t max_time_ms,
                                 std::function<void()> func,
                                 const std::string &message = "");

private:
  std::vector<TestCase> _tests;
  std::vector<TestResult> _results;
  uint32_t _total_tests;
  uint32_t _passed_tests;
  uint32_t _failed_tests;

  // Métodos privados
  void run_test_case(const TestCase &test_case);
  bool matches_pattern(const std::string &name, const std::string &pattern);
  std::string get_timestamp();
  void print_test_result(const TestResult &result);
};

// Instância global
extern TestFramework testFramework;

// Macros convenientes
#define TEST(name)                                                             \
  void test_##name();                                                          \
  void test_##name()

#define TEST_F(name, class_name)                                               \
  void test_##name##_##class_name();                                           \
  void test_##name##_##class_name()

#define SETUP() void setup_##test_name()
#define TEARDOWN() void teardown_##test_name()

#define RUN_TEST(name) testFramework.add_test(#name, test_##name)
#define RUN_TEST_F(name, class_name)                                           \
  testFramework.add_test_with_setup(#name, test_##name##_##class_name,         \
                                    setup_##test_name##_##class_name,          \
                                    teardown_##test_name##_##class_name)

#define ASSERT_TRUE(condition) testFramework.assert_true(condition, #condition)
#define ASSERT_FALSE(condition)                                                \
  testFramework.assert_false(condition, #condition)
#define ASSERT_EQUAL(expected, actual)                                         \
  testFramework.assert_equal(expected, actual, #expected " == " #actual)
#define ASSERT_FLOAT_EQUAL(expected, actual, tolerance)                        \
  testFramework.assert_equal(expected, actual, tolerance,                      \
                             #expected " == " #actual)
#define ASSERT_STRING_EQUAL(expected, actual)                                  \
  testFramework.assert_equal(expected, actual, #expected " == " #actual)
#define ASSERT_NOT_EQUAL(expected, actual)                                     \
  testFramework.assert_not_equal(expected, actual, #expected " != " #actual)
#define ASSERT_NOT_NULL(ptr)                                                   \
  testFramework.assert_not_null(ptr, #ptr " should not be NULL")
#define ASSERT_NULL(ptr) testFramework.assert_null(ptr, #ptr " should be NULL")
#define ASSERT_GREATER(value, min)                                             \
  testFramework.assert_greater(value, min, #value " > " #min)
#define ASSERT_LESS(value, max)                                                \
  testFramework.assert_less(value, max, #value " < " #max)
#define ASSERT_WITHIN_RANGE(value, min, max)                                   \
  testFramework.assert_within_range(value, min, max,                           \
                                    #value " within [" #min ", " #max "]")

#define MEASURE_PERF(operation, func, iterations)                              \
  testFramework.measure_performance(operation, func, iterations)

#define ASSERT_MEMORY(expected_free)                                           \
  testFramework.assert_memory_usage(expected_free, "Memory usage check")

#define ASSERT_TIME_LESS(max_time, func)                                       \
  testFramework.assert_time_less(                                              \
      max_time, func, #func " should take less than " #max_time "ms")
#define ASSERT_TIME_WITHIN(min_time, max_time, func)                           \
  testFramework.assert_time_within(min_time, max_time, func,                   \
                                   #func " should take between " #min_time     \
                                         " and " #max_time "ms")

#endif // TEST_FRAMEWORK_H
