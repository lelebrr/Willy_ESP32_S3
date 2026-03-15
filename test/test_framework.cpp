/**
 * @file test_framework.cpp
 * @author Willy Firmware
 * @brief Implementação do Framework de Testes Unitários para Willy ESP32-S3
 * @version 1.0
 */

#include "test_framework.h"
#include <Arduino.h>
#include <algorithm>
#include <chrono>

// Instância global
TestFramework testFramework;

// Implementação da classe TestFramework

TestFramework::TestFramework()
    : _total_tests(0), _passed_tests(0), _failed_tests(0) {}

TestFramework::~TestFramework() {}

void TestFramework::add_test(const std::string &name,
                             std::function<void()> test_func) {
  _tests.push_back({name, test_func});
}

void TestFramework::add_test_with_setup(const std::string &name,
                                        std::function<void()> test_func,
                                        std::function<void()> setup_func,
                                        std::function<void()> teardown_func) {
  _tests.push_back({name, test_func, setup_func, teardown_func});
}

void TestFramework::run_all_tests() {
  _results.clear();
  _total_tests = _tests.size();
  _passed_tests = 0;
  _failed_tests = 0;

  Serial.println("\n=== Executando Testes Unitários ===");

  for (const auto &test_case : _tests) {
    run_test_case(test_case);
  }

  print_summary();
}

void TestFramework::run_test(const std::string &test_name) {
  auto it = std::find_if(
      _tests.begin(), _tests.end(),
      [&test_name](const TestCase &tc) { return tc.name == test_name; });

  if (it != _tests.end()) {
    _results.clear();
    run_test_case(*it);
    print_results();
  } else {
    Serial.printf("Teste '%s' não encontrado.\n", test_name.c_str());
  }
}

void TestFramework::run_tests_matching(const std::string &pattern) {
  _results.clear();
  _passed_tests = 0;
  _failed_tests = 0;

  Serial.printf("\n=== Executando Testes com Padrão: %s ===\n",
                pattern.c_str());

  for (const auto &test_case : _tests) {
    if (matches_pattern(test_case.name, pattern)) {
      run_test_case(test_case);
    }
  }

  print_summary();
}

std::vector<TestResult> TestFramework::get_results() const { return _results; }

uint32_t TestFramework::get_total_tests() const { return _total_tests; }

uint32_t TestFramework::get_passed_tests() const { return _passed_tests; }

uint32_t TestFramework::get_failed_tests() const { return _failed_tests; }

double TestFramework::get_success_rate() const {
  if (_total_tests == 0)
    return 0.0;
  return (static_cast<double>(_passed_tests) / _total_tests) * 100.0;
}

void TestFramework::print_results() {
  Serial.println("\n=== Resultados dos Testes ===");
  for (const auto &result : _results) {
    print_test_result(result);
  }
}

void TestFramework::print_summary() {
  Serial.println("\n=== Resumo dos Testes ===");
  Serial.printf("Total: %d\n", _total_tests);
  Serial.printf("Aprovados: %d\n", _passed_tests);
  Serial.printf("Reprovados: %d\n", _failed_tests);
  Serial.printf("Taxa de Sucesso: %.2f%%\n", get_success_rate());

  if (_failed_tests > 0) {
    Serial.println("\nTestes Reprovados:");
    for (const auto &result : _results) {
      if (!result.passed) {
        Serial.printf("  - %s: %s\n", result.name.c_str(),
                      result.message.c_str());
      }
    }
  }
}

void TestFramework::save_results_to_file(const std::string &filename) {
  // Implementação simplificada - em um sistema real, salvaria em arquivo
  Serial.printf("Salvando resultados em: %s\n", filename.c_str());
  // TODO: Implementar salvamento real em arquivo
}

// Implementação das asserções estáticas

void TestFramework::assert_true(bool condition, const std::string &message) {
  if (!condition) {
    throw std::runtime_error(
        message.empty() ? "Asserção falhou: condição deve ser verdadeira"
                        : message);
  }
}

void TestFramework::assert_false(bool condition, const std::string &message) {
  if (condition) {
    throw std::runtime_error(
        message.empty() ? "Asserção falhou: condição deve ser falsa" : message);
  }
}

void TestFramework::assert_equal(int expected, int actual,
                                 const std::string &message) {
  if (expected != actual) {
    std::string msg = message.empty() ? "Asserção falhou: esperado " +
                                            std::to_string(expected) +
                                            ", obtido " + std::to_string(actual)
                                      : message;
    throw std::runtime_error(msg);
  }
}

void TestFramework::assert_equal(uint32_t expected, uint32_t actual,
                                 const std::string &message) {
  if (expected != actual) {
    std::string msg = message.empty() ? "Asserção falhou: esperado " +
                                            std::to_string(expected) +
                                            ", obtido " + std::to_string(actual)
                                      : message;
    throw std::runtime_error(msg);
  }
}

void TestFramework::assert_equal(float expected, float actual, float tolerance,
                                 const std::string &message) {
  if (std::abs(expected - actual) > tolerance) {
    std::string msg =
        message.empty()
            ? "Asserção falhou: esperado " + std::to_string(expected) +
                  ", obtido " + std::to_string(actual) +
                  " (tolerância: " + std::to_string(tolerance) + ")"
            : message;
    throw std::runtime_error(msg);
  }
}

void TestFramework::assert_equal(const std::string &expected,
                                 const std::string &actual,
                                 const std::string &message) {
  if (expected != actual) {
    std::string msg = message.empty()
                          ? "Asserção falhou: esperado '" + expected +
                                "', obtido '" + actual + "'"
                          : message;
    throw std::runtime_error(msg);
  }
}

void TestFramework::assert_not_equal(int expected, int actual,
                                     const std::string &message) {
  if (expected == actual) {
    std::string msg = message.empty()
                          ? "Asserção falhou: valores não devem ser iguais: " +
                                std::to_string(expected)
                          : message;
    throw std::runtime_error(msg);
  }
}

void TestFramework::assert_not_null(void *ptr, const std::string &message) {
  if (ptr == nullptr) {
    throw std::runtime_error(message.empty()
                                 ? "Asserção falhou: ponteiro não deve ser nulo"
                                 : message);
  }
}

void TestFramework::assert_null(void *ptr, const std::string &message) {
  if (ptr != nullptr) {
    throw std::runtime_error(
        message.empty() ? "Asserção falhou: ponteiro deve ser nulo" : message);
  }
}

void TestFramework::assert_greater(int value, int min_value,
                                   const std::string &message) {
  if (value <= min_value) {
    std::string msg =
        message.empty() ? "Asserção falhou: " + std::to_string(value) +
                              " deve ser maior que " + std::to_string(min_value)
                        : message;
    throw std::runtime_error(msg);
  }
}

void TestFramework::assert_less(int value, int max_value,
                                const std::string &message) {
  if (value >= max_value) {
    std::string msg =
        message.empty() ? "Asserção falhou: " + std::to_string(value) +
                              " deve ser menor que " + std::to_string(max_value)
                        : message;
    throw std::runtime_error(msg);
  }
}

void TestFramework::assert_within_range(int value, int min_value, int max_value,
                                        const std::string &message) {
  if (value < min_value || value > max_value) {
    std::string msg =
        message.empty() ? "Asserção falhou: " + std::to_string(value) +
                              " deve estar entre " + std::to_string(min_value) +
                              " e " + std::to_string(max_value)
                        : message;
    throw std::runtime_error(msg);
  }
}

// Testes de performance
void TestFramework::measure_performance(const std::string &operation,
                                        std::function<void()> func,
                                        uint32_t iterations) {
  uint32_t start_time = millis();

  for (uint32_t i = 0; i < iterations; ++i) {
    func();
  }

  uint32_t end_time = millis();
  uint32_t total_time = end_time - start_time;

  Serial.printf("Performance [%s]: %d iterações em %d ms (%.2f ms/it)\n",
                operation.c_str(), iterations, total_time,
                static_cast<float>(total_time) / iterations);
}

// Testes de memória
uint32_t TestFramework::get_free_heap() { return ESP.getFreeHeap(); }

uint32_t TestFramework::get_minimum_free_heap() { return ESP.getMinFreeHeap(); }

void TestFramework::assert_memory_usage(uint32_t expected_free,
                                        const std::string &message) {
  uint32_t free_heap = get_free_heap();
  if (free_heap < expected_free) {
    std::string msg = message.empty()
                          ? "Asserção falhou: heap livre insuficiente: " +
                                std::to_string(free_heap) + " < " +
                                std::to_string(expected_free)
                          : message;
    throw std::runtime_error(msg);
  }
}

// Testes de tempo
void TestFramework::assert_time_less(uint32_t max_time_ms,
                                     std::function<void()> func,
                                     const std::string &message) {
  uint32_t start_time = millis();
  func();
  uint32_t end_time = millis();
  uint32_t duration = end_time - start_time;

  if (duration >= max_time_ms) {
    std::string msg =
        message.empty()
            ? "Asserção falhou: tempo excedido: " + std::to_string(duration) +
                  " ms >= " + std::to_string(max_time_ms) + " ms"
            : message;
    throw std::runtime_error(msg);
  }
}

void TestFramework::assert_time_within(uint32_t min_time_ms,
                                       uint32_t max_time_ms,
                                       std::function<void()> func,
                                       const std::string &message) {
  uint32_t start_time = millis();
  func();
  uint32_t end_time = millis();
  uint32_t duration = end_time - start_time;

  if (duration < min_time_ms || duration > max_time_ms) {
    std::string msg =
        message.empty() ? "Asserção falhou: tempo fora da faixa: " +
                              std::to_string(duration) + " ms não está entre " +
                              std::to_string(min_time_ms) + " e " +
                              std::to_string(max_time_ms) + " ms"
                        : message;
    throw std::runtime_error(msg);
  }
}

// Métodos privados

void TestFramework::run_test_case(const TestCase &test_case) {
  TestResult result;
  result.name = test_case.name;
  result.passed = false;
  uint32_t start_time = millis();

  try {
    // Executar setup se existir
    if (test_case.setup_func) {
      test_case.setup_func();
    }

    // Executar teste
    test_case.test_func();

    // Executar teardown se existir
    if (test_case.teardown_func) {
      test_case.teardown_func();
    }

    result.passed = true;
    result.message = "PASSOU";
    _passed_tests++;

  } catch (const std::exception &e) {
    result.message = std::string("FALHOU: ") + e.what();
    _failed_tests++;
  } catch (...) {
    result.message = "FALHOU: Exceção desconhecida";
    _failed_tests++;
  }

  uint32_t end_time = millis();
  result.duration = end_time - start_time;

  // Adicionar detalhes de performance
  result.details = "Duração: " + std::to_string(result.duration) +
                   " ms, Heap: " + std::to_string(get_free_heap()) + " bytes";

  _results.push_back(result);
}

bool TestFramework::matches_pattern(const std::string &name,
                                    const std::string &pattern) {
  // Implementação simples de correspondência de padrões (pode ser melhorada)
  return name.find(pattern) != std::string::npos;
}

std::string TestFramework::get_timestamp() {
  // Implementação simplificada
  uint32_t now = millis();
  return std::to_string(now);
}

void TestFramework::print_test_result(const TestResult &result) {
  Serial.printf("[%s] %s - %s (%s)\n", result.passed ? "PASS" : "FAIL",
                result.name.c_str(), result.message.c_str(),
                result.details.c_str());
}