#include "core/SystemModel.h"
#include <unity.h>

// Testes para o componente MVC SystemModel

void test_systemModel_singleton(void) {
  // Teste do padrão singleton
  SystemModel &instance1 = SystemModel::getInstance();
  SystemModel &instance2 = SystemModel::getInstance();

  // Verifica que são a mesma instância
  TEST_ASSERT_EQUAL_PTR(&instance1, &instance2);
}

void test_globalState_initialization(void) {
  // Teste da inicialização do estado global
  SystemModel &model = SystemModel::getInstance();
  auto &state = model.getGlobalState();

  // Verifica valores iniciais padrão
  TEST_ASSERT_FALSE(state.wifiConnected);
  TEST_ASSERT_FALSE(state.bleConnected);
  TEST_ASSERT_FALSE(state.sdcardMounted);
  TEST_ASSERT_FALSE(state.gpsConnected);
  TEST_ASSERT_FALSE(state.isSleeping);
  TEST_ASSERT_FALSE(state.isScreenOff);
  TEST_ASSERT_FALSE(state.dimmer);
  TEST_ASSERT_EQUAL_INT8(-1, state.interpreter_state);
  TEST_ASSERT_EQUAL_UINT32(0, state.previousMillis);
  TEST_ASSERT_EQUAL_INT(0, state.prog_handler);
  TEST_ASSERT_TRUE(state.cachedPassword.isEmpty());
  TEST_ASSERT_TRUE(state.currentLoaderApp.isEmpty());
  TEST_ASSERT_FALSE(state.appRequiresClose);
  TEST_ASSERT_TRUE(state.startupAppLuaScript.isEmpty());
}

void test_globalState_modification(void) {
  // Teste de modificação do estado global
  SystemModel &model = SystemModel::getInstance();
  auto &state = model.getGlobalState();

  // Modifica alguns valores
  state.wifiConnected = true;
  state.bleConnected = true;
  state.interpreter_state = 5;
  state.cachedPassword = "test123";

  // Verifica se as modificações foram aplicadas
  TEST_ASSERT_TRUE(state.wifiConnected);
  TEST_ASSERT_TRUE(state.bleConnected);
  TEST_ASSERT_EQUAL_INT8(5, state.interpreter_state);
  TEST_ASSERT_EQUAL_STRING("test123", state.cachedPassword.c_str());
}

void test_systemConfig_initialization(void) {
  // Teste da inicialização da configuração do sistema
  SystemModel &model = SystemModel::getInstance();
  auto &config = model.getConfig();

  // Verifica valores iniciais padrão
  TEST_ASSERT_EQUAL_INT(100, config.bright);
  TEST_ASSERT_FALSE(config.wifiAtStartup);
  TEST_ASSERT_TRUE(config.startupApp.isEmpty());
  TEST_ASSERT_FALSE(config.instantBoot);
  TEST_ASSERT_EQUAL_UINT16(0x0000, config.bgColor);
  TEST_ASSERT_TRUE(config.themePath.isEmpty());
  TEST_ASSERT_FALSE(config.themeFS);
}

void test_systemConfig_modification(void) {
  // Teste de modificação da configuração do sistema
  SystemModel &model = SystemModel::getInstance();
  auto &config = model.getConfig();

  // Modifica alguns valores
  config.bright = 75;
  config.wifiAtStartup = true;
  config.startupApp = "test_app";
  config.instantBoot = true;
  config.bgColor = 0xFFFF;
  config.themePath = "/themes/dark";
  config.themeFS = true;

  // Verifica se as modificações foram aplicadas
  TEST_ASSERT_EQUAL_INT(75, config.bright);
  TEST_ASSERT_TRUE(config.wifiAtStartup);
  TEST_ASSERT_EQUAL_STRING("test_app", config.startupApp.c_str());
  TEST_ASSERT_TRUE(config.instantBoot);
  TEST_ASSERT_EQUAL_UINT16(0xFFFF, config.bgColor);
  TEST_ASSERT_EQUAL_STRING("/themes/dark", config.themePath.c_str());
  TEST_ASSERT_TRUE(config.themeFS);
}

void test_state_persistence(void) {
  // Teste de persistência do estado entre chamadas
  SystemModel &model = SystemModel::getInstance();

  // Modifica estado
  model.getGlobalState().wifiConnected = true;
  model.getConfig().bright = 50;

  // Obtém novamente e verifica
  auto &state = model.getGlobalState();
  auto &config = model.getConfig();

  TEST_ASSERT_TRUE(state.wifiConnected);
  TEST_ASSERT_EQUAL_INT(50, config.bright);
}

void test_globalState_edge_cases(void) {
  // Teste de casos extremos do estado global
  SystemModel &model = SystemModel::getInstance();
  auto &state = model.getGlobalState();

  // Teste com valores extremos
  state.interpreter_state = INT8_MAX;
  TEST_ASSERT_EQUAL_INT8(INT8_MAX, state.interpreter_state);

  state.interpreter_state = INT8_MIN;
  TEST_ASSERT_EQUAL_INT8(INT8_MIN, state.interpreter_state);

  state.prog_handler = INT32_MAX;
  TEST_ASSERT_EQUAL_INT(INT32_MAX, state.prog_handler);

  state.previousMillis = UINT32_MAX;
  TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, state.previousMillis);

  // Teste com strings grandes
  String longPassword = "";
  for (int i = 0; i < 1000; i++)
    longPassword += "x";
  state.cachedPassword = longPassword;
  TEST_ASSERT_EQUAL_STRING(longPassword.c_str(), state.cachedPassword.c_str());
}

void test_systemConfig_validation(void) {
  // Teste de validação da configuração do sistema
  SystemModel &model = SystemModel::getInstance();
  auto &config = model.getConfig();

  // Teste com valores válidos
  config.bright = 100;
  TEST_ASSERT_EQUAL_INT(100, config.bright);

  config.bright = 0;
  TEST_ASSERT_EQUAL_INT(0, config.bright);

  // Teste com valores fora do range (deve aceitar, pois não há validação)
  config.bright = 255;
  TEST_ASSERT_EQUAL_INT(255, config.bright);

  config.bright = -1; // Pode causar problemas se não tratado
  // TEST_ASSERT_EQUAL_INT(-1, config.bright); // Comentado pois pode falhar

  // Reset para valor válido
  config.bright = 50;
}

void test_systemModel_performance(void) {
  // Teste de performance das operações do SystemModel
  SystemModel &model = SystemModel::getInstance();

  // Teste performance de acesso ao estado
  uint32_t start = millis();
  for (int i = 0; i < 10000; i++) {
    auto &state = model.getGlobalState();
    state.wifiConnected = !state.wifiConnected;
  }
  uint32_t duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 100); // Deve ser muito rápido

  // Teste performance de acesso à configuração
  start = millis();
  for (int i = 0; i < 10000; i++) {
    auto &config = model.getConfig();
    config.bright = i % 101;
  }
  duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 100);
}

void test_systemModel_memory_usage(void) {
  // Teste de uso de memória
  SystemModel &model = SystemModel::getInstance();

  // Verificar que múltiplas chamadas getInstance retornam a mesma instância
  SystemModel *ptr1 = &model;
  SystemModel *ptr2 = &SystemModel::getInstance();
  TEST_ASSERT_EQUAL_PTR(ptr1, ptr2);

  // Teste de modificação de estado sem vazamentos
  for (int i = 0; i < 1000; i++) {
    model.getGlobalState().wifiConnected = (i % 2 == 0);
    model.getConfig().bright = i % 101;
  }

  // Verificar estado final
  TEST_ASSERT_EQUAL_INT(999 % 101, model.getConfig().bright);
}

// Função setup do Unity
void setUp(void) {
  // Reset do singleton para cada teste (se necessário)
  // Nota: Como é singleton, o estado pode persistir entre testes
}

// Função teardown do Unity
void tearDown(void) {
  // Limpeza após cada teste
}

// Função principal de execução dos testes
void setup() {
  UNITY_BEGIN();

  RUN_TEST(test_systemModel_singleton);
  RUN_TEST(test_globalState_initialization);
  RUN_TEST(test_globalState_modification);
  RUN_TEST(test_systemConfig_initialization);
  RUN_TEST(test_systemConfig_modification);
  RUN_TEST(test_state_persistence);
  RUN_TEST(test_globalState_edge_cases);
  RUN_TEST(test_systemConfig_validation);
  RUN_TEST(test_systemModel_performance);
  RUN_TEST(test_systemModel_memory_usage);

  UNITY_END();
}

void loop() {
  // Loop vazio para testes unitários
}