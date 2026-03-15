#include "core/type_convertion.h"
#include <unity.h>

// Testes para funções de conversões de dados em type_convertion.cpp

void test_hexStrToBinStr(void) {
  // Teste de conversão hex string para bin string
  String result = hexStrToBinStr("41");
  TEST_ASSERT_EQUAL_STRING("01000001", result.c_str());

  result = hexStrToBinStr("FF");
  TEST_ASSERT_EQUAL_STRING("11111111", result.c_str());

  result = hexStrToBinStr("00");
  TEST_ASSERT_EQUAL_STRING("00000000", result.c_str());

  result = hexStrToBinStr("A5");
  TEST_ASSERT_EQUAL_STRING("10100101", result.c_str());

  // Testes adicionais para validação
  result = hexStrToBinStr("0F");
  TEST_ASSERT_EQUAL_STRING("00001111", result.c_str());

  result = hexStrToBinStr("F0");
  TEST_ASSERT_EQUAL_STRING("11110000", result.c_str());

  result = hexStrToBinStr("55");
  TEST_ASSERT_EQUAL_STRING("01010101", result.c_str());

  result = hexStrToBinStr("AA");
  TEST_ASSERT_EQUAL_STRING("10101010", result.c_str());

  // Teste com letras minúsculas
  result = hexStrToBinStr("ab");
  TEST_ASSERT_EQUAL_STRING("10101011", result.c_str());
}

void test_decimalToHexString(void) {
  // Teste de conversão decimal para hex string
  char output[17]; // Buffer suficiente

  decimalToHexString(255, output);
  TEST_ASSERT_EQUAL_STRING("FF", output);

  decimalToHexString(0, output);
  TEST_ASSERT_EQUAL_STRING("0", output);

  decimalToHexString(4096, output);
  TEST_ASSERT_EQUAL_STRING("1000", output);

  decimalToHexString(65535, output);
  TEST_ASSERT_EQUAL_STRING("FFFF", output);
}

void test_hexCharToDecimal(void) {
  // Teste de conversão char hex para decimal
  TEST_ASSERT_EQUAL_UINT8(0, hexCharToDecimal('0'));
  TEST_ASSERT_EQUAL_UINT8(9, hexCharToDecimal('9'));
  TEST_ASSERT_EQUAL_UINT8(10, hexCharToDecimal('A'));
  TEST_ASSERT_EQUAL_UINT8(15, hexCharToDecimal('F'));
  TEST_ASSERT_EQUAL_UINT8(10, hexCharToDecimal('a'));
  TEST_ASSERT_EQUAL_UINT8(15, hexCharToDecimal('f'));
}

void test_hexStringToDecimal(void) {
  // Teste de conversão hex string para decimal
  TEST_ASSERT_EQUAL_UINT32(255, hexStringToDecimal("FF"));
  TEST_ASSERT_EQUAL_UINT32(0, hexStringToDecimal("0"));
  TEST_ASSERT_EQUAL_UINT32(4096, hexStringToDecimal("1000"));
  TEST_ASSERT_EQUAL_UINT32(65535, hexStringToDecimal("FFFF"));
  TEST_ASSERT_EQUAL_UINT32(3735928559UL, hexStringToDecimal("DEADBEEF"));
}

void test_dec2binWzerofill(void) {
  // Teste de conversão decimal para binário com preenchimento
  char *result = dec2binWzerofill(5, 8);
  TEST_ASSERT_EQUAL_STRING("00000101", result);
  free(result);

  result = dec2binWzerofill(255, 8);
  TEST_ASSERT_EQUAL_STRING("11111111", result);
  free(result);

  result = dec2binWzerofill(1, 4);
  TEST_ASSERT_EQUAL_STRING("0001", result);
  free(result);
}

void test_hexToStr(void) {
  // Teste de conversão hex array para string
  uint8_t data1[] = {0x41, 0x42, 0x43};
  String result = hexToStr(data1, 3);
  TEST_ASSERT_EQUAL_STRING("41 42 43", result.c_str());

  uint8_t data2[] = {0xFF, 0x00};
  result = hexToStr(data2, 2, '-');
  TEST_ASSERT_EQUAL_STRING("FF-00", result.c_str());

  uint8_t data3[] = {0x0A};
  result = hexToStr(data3, 1);
  TEST_ASSERT_EQUAL_STRING("0A", result.c_str());
}

// Testes adicionais para validação de entrada e tratamento de erros
void test_hexStrToBinStr_edge_cases(void) {
  // Teste com entrada vazia
  String result = hexStrToBinStr("");
  TEST_ASSERT_TRUE(result.length() == 0);

  // Teste com caracteres inválidos (deve ignorar ou tratar)
  result = hexStrToBinStr("XX");
  // Comportamento depende da implementação - verificar se não crash

  // Teste performance
  uint32_t start = millis();
  for (int i = 0; i < 1000; i++) {
    hexStrToBinStr("FF");
  }
  uint32_t duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 100); // Deve ser rápido
}

void test_decimalToHexString_edge_cases(void) {
  char output[17];

  // Teste com 0
  decimalToHexString(0, output);
  TEST_ASSERT_EQUAL_STRING("0", output);

  // Teste com valores grandes
  decimalToHexString(4294967295UL, output); // UINT32_MAX
  TEST_ASSERT_EQUAL_STRING("FFFFFFFF", output);

  // Teste performance
  uint32_t start = millis();
  for (int i = 0; i < 1000; i++) {
    decimalToHexString(i, output);
  }
  uint32_t duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 100);
}

void test_hexCharToDecimal_edge_cases(void) {
  // Teste com caracteres válidos
  TEST_ASSERT_EQUAL_UINT8(0, hexCharToDecimal('0'));
  TEST_ASSERT_EQUAL_UINT8(9, hexCharToDecimal('9'));
  TEST_ASSERT_EQUAL_UINT8(10, hexCharToDecimal('A'));
  TEST_ASSERT_EQUAL_UINT8(15, hexCharToDecimal('F'));
  TEST_ASSERT_EQUAL_UINT8(10, hexCharToDecimal('a'));
  TEST_ASSERT_EQUAL_UINT8(15, hexCharToDecimal('f'));

  // Teste com caracteres inválidos (comportamento indefinido, mas não deve
  // crash) hexCharToDecimal('G'); // Pode causar comportamento indefinido
}

void test_hexStringToDecimal_edge_cases(void) {
  // Teste com strings vazias
  TEST_ASSERT_EQUAL_UINT32(0, hexStringToDecimal(""));

  // Teste com strings grandes
  TEST_ASSERT_EQUAL_UINT32(3735928559UL, hexStringToDecimal("DEADBEEF"));

  // Teste com letras minúsculas
  TEST_ASSERT_EQUAL_UINT32(255, hexStringToDecimal("ff"));

  // Teste performance
  uint32_t start = millis();
  for (int i = 0; i < 1000; i++) {
    hexStringToDecimal("FF");
  }
  uint32_t duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 100);
}

void test_dec2binWzerofill_edge_cases(void) {
  // Teste com diferentes larguras
  char *result = dec2binWzerofill(5, 4);
  TEST_ASSERT_EQUAL_STRING("0101", result);
  free(result);

  result = dec2binWzerofill(255, 16);
  TEST_ASSERT_EQUAL_STRING("0000000011111111", result);
  free(result);

  // Teste com valor 0
  result = dec2binWzerofill(0, 8);
  TEST_ASSERT_EQUAL_STRING("00000000", result);
  free(result);

  // Teste performance
  uint32_t start = millis();
  for (int i = 0; i < 100; i++) {
    char *res = dec2binWzerofill(i % 256, 8);
    free(res);
  }
  uint32_t duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 50);
}

void test_hexToStr_edge_cases(void) {
  // Teste com array vazio
  uint8_t empty[] = {};
  String result = hexToStr(empty, 0);
  TEST_ASSERT_EQUAL_STRING("", result.c_str());

  // Teste com separador customizado
  uint8_t data[] = {0xAA, 0xBB};
  result = hexToStr(data, 2, '-');
  TEST_ASSERT_EQUAL_STRING("AA-BB", result.c_str());

  // Teste com um byte
  uint8_t single[] = {0xFF};
  result = hexToStr(single, 1);
  TEST_ASSERT_EQUAL_STRING("FF", result.c_str());

  // Teste performance
  uint8_t large_data[100];
  for (int i = 0; i < 100; i++)
    large_data[i] = i;

  uint32_t start = millis();
  for (int i = 0; i < 100; i++) {
    hexToStr(large_data, 100);
  }
  uint32_t duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 200);
}

// Função setup do Unity
void setUp(void) {
  // Configuração antes de cada teste
}

// Função teardown do Unity
void tearDown(void) {
  // Limpeza após cada teste
}

// Função principal de execução dos testes
void setup() {
  UNITY_BEGIN();

  RUN_TEST(test_hexStrToBinStr);
  RUN_TEST(test_decimalToHexString);
  RUN_TEST(test_hexCharToDecimal);
  RUN_TEST(test_hexStringToDecimal);
  RUN_TEST(test_dec2binWzerofill);
  RUN_TEST(test_hexToStr);

  // Novos testes de edge cases
  RUN_TEST(test_hexStrToBinStr_edge_cases);
  RUN_TEST(test_decimalToHexString_edge_cases);
  RUN_TEST(test_hexCharToDecimal_edge_cases);
  RUN_TEST(test_hexStringToDecimal_edge_cases);
  RUN_TEST(test_dec2binWzerofill_edge_cases);
  RUN_TEST(test_hexToStr_edge_cases);

  UNITY_END();
}

void loop() {
  // Loop vazio para testes unitários
}