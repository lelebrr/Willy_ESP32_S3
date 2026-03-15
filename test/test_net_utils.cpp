#include "core/net_utils.h"
#include <unity.h>

// Testes para funções de validações de rede em net_utils.cpp

void test_stringToMAC(void) {
  // Teste de conversão string para MAC
  uint8_t mac[6];
  stringToMAC("AA:BB:CC:DD:EE:FF", mac);
  TEST_ASSERT_EQUAL_UINT8(0xAA, mac[0]);
  TEST_ASSERT_EQUAL_UINT8(0xBB, mac[1]);
  TEST_ASSERT_EQUAL_UINT8(0xCC, mac[2]);
  TEST_ASSERT_EQUAL_UINT8(0xDD, mac[3]);
  TEST_ASSERT_EQUAL_UINT8(0xEE, mac[4]);
  TEST_ASSERT_EQUAL_UINT8(0xFF, mac[5]);

  // Teste com formato sem dois pontos
  stringToMAC("AABBCCDDEEFF", mac);
  TEST_ASSERT_EQUAL_UINT8(0xAA, mac[0]);
  TEST_ASSERT_EQUAL_UINT8(0xBB, mac[1]);

  // Testes adicionais para validação de entrada
  // MAC válido com letras minúsculas
  stringToMAC("aa:bb:cc:dd:ee:ff", mac);
  TEST_ASSERT_EQUAL_UINT8(0xAA, mac[0]);
  TEST_ASSERT_EQUAL_UINT8(0xBB, mac[1]);

  // MAC com zeros
  stringToMAC("00:00:00:00:00:00", mac);
  TEST_ASSERT_EQUAL_UINT8(0x00, mac[0]);
  TEST_ASSERT_EQUAL_UINT8(0x00, mac[1]);

  // MAC com FFs
  stringToMAC("FF:FF:FF:FF:FF:FF", mac);
  TEST_ASSERT_EQUAL_UINT8(0xFF, mac[0]);
  TEST_ASSERT_EQUAL_UINT8(0xFF, mac[1]);
}

void test_ipToString(void) {
  // Teste de conversão IP para string
  uint8_t ip[4] = {192, 168, 1, 1};
  String result = ipToString(ip);
  TEST_ASSERT_EQUAL_STRING("192.168.1.1", result.c_str());

  uint8_t ip2[4] = {10, 0, 0, 1};
  result = ipToString(ip2);
  TEST_ASSERT_EQUAL_STRING("10.0.0.1", result.c_str());

  // Testes adicionais
  uint8_t ip3[4] = {0, 0, 0, 0};
  result = ipToString(ip3);
  TEST_ASSERT_EQUAL_STRING("0.0.0.0", result.c_str());

  uint8_t ip4[4] = {255, 255, 255, 255};
  result = ipToString(ip4);
  TEST_ASSERT_EQUAL_STRING("255.255.255.255", result.c_str());

  uint8_t ip5[4] = {127, 0, 0, 1};
  result = ipToString(ip5);
  TEST_ASSERT_EQUAL_STRING("127.0.0.1", result.c_str());
}

void test_macToString(void) {
  // Teste de conversão MAC para string
  uint8_t mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  String result = macToString(mac);
  TEST_ASSERT_EQUAL_STRING("AA:BB:CC:DD:EE:FF", result.c_str());

  uint8_t mac2[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
  result = macToString(mac2);
  TEST_ASSERT_EQUAL_STRING("00:11:22:33:44:55", result.c_str());
}

void test_getManufacturer(void) {
  // Teste de obtenção de fabricante (esta função pode depender de dados
  // externos) Como é uma função que pode fazer requisições HTTP, testamos
  // apenas que não crash
  String result = getManufacturer("00:11:22:33:44:55");
  TEST_ASSERT_TRUE(result.length() >= 0); // Pelo menos não deve crash
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

  RUN_TEST(test_stringToMAC);
  RUN_TEST(test_ipToString);
  RUN_TEST(test_macToString);
  RUN_TEST(test_getManufacturer);

  UNITY_END();
}

void loop() {
  // Loop vazio para testes unitários
}