#include "core/utils.h"
#include "test_framework.h"
#include <unity.h>

// Testes para funções de formatação e conversões em utils.cpp

// Testes usando o framework personalizado
void test_formatTimeDecimal_custom(void) {
  // Teste básico de formatação de tempo
  String result = formatTimeDecimal(1000); // 1 segundo
  ASSERT_GREATER(result.length(), 0);
  ASSERT_TRUE(result.indexOf("1.000") >= 0);

  // Testes adicionais para cobertura
  result = formatTimeDecimal(0);
  ASSERT_TRUE(result.indexOf("0.000") >= 0);

  result = formatTimeDecimal(1500);
  ASSERT_TRUE(result.indexOf("1.500") >= 0);

  result = formatTimeDecimal(999);
  ASSERT_TRUE(result.indexOf("0.999") >= 0);

  result = formatTimeDecimal(60000); // 1 minuto
  ASSERT_TRUE(result.indexOf("60.000") >= 0);
}

void test_formatBytes_custom(void) {
  // Teste de formatação de bytes com casos edge
  ASSERT_STRING_EQUAL("0 B", formatBytes(0).c_str());
  ASSERT_STRING_EQUAL("1.00 KB", formatBytes(1024).c_str());
  ASSERT_STRING_EQUAL("1.00 MB", formatBytes(1024 * 1024).c_str());
  ASSERT_STRING_EQUAL("1.00 GB", formatBytes(1024LL * 1024 * 1024).c_str());

  // Testes adicionais
  ASSERT_STRING_EQUAL("512 B", formatBytes(512).c_str());
  ASSERT_STRING_EQUAL("1.50 KB", formatBytes(1536).c_str());
  ASSERT_STRING_EQUAL("2.00 MB", formatBytes(2 * 1024 * 1024).c_str());
  ASSERT_STRING_EQUAL("1.00 TB",
                      formatBytes(1024LL * 1024 * 1024 * 1024).c_str());

  // Teste com valores grandes
  ASSERT_STRING_EQUAL("1024.00 TB",
                      formatBytes(1024LL * 1024 * 1024 * 1024 * 1024).c_str());
}

void test_repeatString_custom(void) {
  // Teste de repetição de string com validações
  ASSERT_STRING_EQUAL("AAA", repeatString(3, "A").c_str());
  ASSERT_STRING_EQUAL("", repeatString(0, "A").c_str());
  ASSERT_STRING_EQUAL("***", repeatString(3, "*").c_str());

  // Testes adicionais para cobertura
  ASSERT_STRING_EQUAL("HelloHelloHello", repeatString(3, "Hello").c_str());
  ASSERT_STRING_EQUAL(" ", repeatString(1, " ").c_str());
  ASSERT_STRING_EQUAL("", repeatString(0, "").c_str());
  ASSERT_STRING_EQUAL("123123", repeatString(2, "123").c_str());

  // Teste com números grandes (performance)
  MEASURE_PERF(
      "repeatString_large",
      []() {
        String result = repeatString(1000, "x");
        ASSERT_EQUAL(1000, result.length());
      },
      10);
}

void test_backToMenu_custom(void) {
  // Teste da função backToMenu com validação de estado
  returnToMenu = false;
  backToMenu();
  ASSERT_TRUE(returnToMenu);

  // Teste estado já true
  returnToMenu = true;
  backToMenu();
  ASSERT_TRUE(returnToMenu); // Deve permanecer true
}

void test_addOptionToMainMenu_custom(void) {
  // Teste da função addOptionToMainMenu
  returnToMenu = true; // Simula estado anterior
  addOptionToMainMenu();
  ASSERT_FALSE(returnToMenu);

  // Teste estado já false
  returnToMenu = false;
  addOptionToMainMenu();
  ASSERT_FALSE(returnToMenu); // Deve permanecer false
}

// Testes de validação de entrada e tratamento de erros
void test_formatTimeDecimal_edge_cases(void) {
  // Teste com valores extremos
  String result = formatTimeDecimal(0);
  ASSERT_TRUE(result.length() > 0);

  result = formatTimeDecimal(UINT32_MAX);
  ASSERT_TRUE(result.length() > 0);

  // Teste performance
  ASSERT_TIME_LESS(10, []() {
    for (int i = 0; i < 100; i++) {
      formatTimeDecimal(i * 1000);
    }
  });
}

void test_formatBytes_edge_cases(void) {
  // Teste com valores extremos
  ASSERT_TRUE(formatBytes(0).length() > 0);
  ASSERT_TRUE(formatBytes(UINT64_MAX).length() > 0);

  // Teste performance
  ASSERT_TIME_LESS(5, []() {
    for (uint64_t i = 1; i < 1000000; i *= 10) {
      formatBytes(i);
    }
  });
}

void test_repeatString_edge_cases(void) {
  // Teste com strings vazias
  ASSERT_STRING_EQUAL("", repeatString(0, "").c_str());
  ASSERT_STRING_EQUAL("", repeatString(10, "").c_str());

  // Teste com caracteres especiais
  ASSERT_STRING_EQUAL("\n\n\n", repeatString(3, "\n").c_str());
  ASSERT_STRING_EQUAL("\t\t", repeatString(2, "\t").c_str());

  // Teste com números grandes (mas não excessivos para evitar overflow)
  String result = repeatString(100, "x");
  ASSERT_EQUAL(100, result.length());
}

// Testes de Unity (mantidos para compatibilidade)
void test_formatTimeDecimal(void) {
  String result = formatTimeDecimal(1000);
  TEST_ASSERT_TRUE(result.length() > 0);
  TEST_ASSERT_TRUE(result.indexOf("1.000") >= 0);
}

void test_formatBytes(void) {
  TEST_ASSERT_EQUAL_STRING("0 B", formatBytes(0).c_str());
  TEST_ASSERT_EQUAL_STRING("1.00 KB", formatBytes(1024).c_str());
  TEST_ASSERT_EQUAL_STRING("1.00 MB", formatBytes(1024 * 1024).c_str());
  TEST_ASSERT_EQUAL_STRING("1.00 GB",
                           formatBytes(1024LL * 1024 * 1024).c_str());
}

void test_repeatString(void) {
  TEST_ASSERT_EQUAL_STRING("AAA", repeatString(3, "A").c_str());
  TEST_ASSERT_EQUAL_STRING("", repeatString(0, "A").c_str());
  TEST_ASSERT_EQUAL_STRING("***", repeatString(3, "*").c_str());
}

void test_backToMenu(void) {
  returnToMenu = false;
  backToMenu();
  TEST_ASSERT_TRUE(returnToMenu);
}

void test_addOptionToMainMenu(void) {
  returnToMenu = true;
  addOptionToMainMenu();
  TEST_ASSERT_FALSE(returnToMenu);
}

// Setup e teardown do Unity
void setUp(void) {
  // Reset de estado global antes de cada teste
  returnToMenu = false;
}

void tearDown(void) {
  // Limpeza após cada teste
}

// Setup principal
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Testes Utils - Framework Customizado ===");

  // Registrar testes no framework customizado
  testFramework.add_test("formatTimeDecimal_custom",
                         test_formatTimeDecimal_custom);
  testFramework.add_test("formatBytes_custom", test_formatBytes_custom);
  testFramework.add_test("repeatString_custom", test_repeatString_custom);
  testFramework.add_test("backToMenu_custom", test_backToMenu_custom);
  testFramework.add_test("addOptionToMainMenu_custom",
                         test_addOptionToMainMenu_custom);
  testFramework.add_test("formatTimeDecimal_edge_cases",
                         test_formatTimeDecimal_edge_cases);
  testFramework.add_test("formatBytes_edge_cases", test_formatBytes_edge_cases);
  testFramework.add_test("repeatString_edge_cases",
                         test_repeatString_edge_cases);

  // Executar testes customizados
  testFramework.run_all_tests();

  Serial.println("\n=== Testes Utils - Framework Unity ===");

  // Executar testes Unity
  UNITY_BEGIN();
  RUN_TEST(test_formatTimeDecimal);
  RUN_TEST(test_formatBytes);
  RUN_TEST(test_repeatString);
  RUN_TEST(test_backToMenu);
  RUN_TEST(test_addOptionToMainMenu);
  UNITY_END();
}

void loop() {
  // Loop vazio para testes unitários
}