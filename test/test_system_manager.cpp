#include "core/IModule.h"
#include "core/SystemManager.h"
#include <unity.h>

// Mock simples de módulo para testes
class MockModule : public IModule {
public:
  MockModule(const std::string &name)
      : moduleName(name), initialized(false), processed(false) {}

  void initialize() override { initialized = true; }

  void process() override { processed = true; }

  void shutdown() override {
    initialized = false;
    processed = false;
  }

  std::string getName() const override { return moduleName; }

  bool isInitialized() const { return initialized; }

  bool isProcessed() const { return processed; }

private:
  std::string moduleName;
  bool initialized;
  bool processed;
};

// Testes para o componente MVC SystemManager

void test_systemManager_singleton(void) {
  // Teste do padrão singleton
  SystemManager &instance1 = SystemManager::getInstance();
  SystemManager &instance2 = SystemManager::getInstance();

  // Verifica que são a mesma instância
  TEST_ASSERT_EQUAL_PTR(&instance1, &instance2);
}

void test_module_registration(void) {
  // Teste de registro de módulos
  SystemManager &manager = SystemManager::getInstance();

  // Cria módulos mock
  auto module1 = std::make_unique<MockModule>("TestModule1");
  auto module2 = std::make_unique<MockModule>("TestModule2");

  // Registra módulos
  manager.registerModule(std::move(module1));
  manager.registerModule(std::move(module2));

  // Verifica se os módulos foram registrados
  auto registeredModules = manager.getRegisteredModules();
  TEST_ASSERT_EQUAL_SIZE(2, registeredModules.size());

  // Verifica nomes dos módulos
  bool foundModule1 = false;
  bool foundModule2 = false;
  for (const auto &module : registeredModules) {
    if (module->getName() == "TestModule1")
      foundModule1 = true;
    if (module->getName() == "TestModule2")
      foundModule2 = true;
  }
  TEST_ASSERT_TRUE(foundModule1);
  TEST_ASSERT_TRUE(foundModule2);
}

void test_module_initialization(void) {
  // Teste de inicialização de módulos
  SystemManager &manager = SystemManager::getInstance();

  // Cria e registra módulo
  auto module = std::make_unique<MockModule>("InitTestModule");
  MockModule *rawPtr = module.get();
  manager.registerModule(std::move(module));

  // Inicializa todos os módulos
  manager.initializeAllModules();

  // Verifica se o módulo foi inicializado
  TEST_ASSERT_TRUE(rawPtr->isInitialized());
}

void test_module_processing(void) {
  // Teste de processamento de módulos
  SystemManager &manager = SystemManager::getInstance();

  // Cria e registra módulo
  auto module = std::make_unique<MockModule>("ProcessTestModule");
  MockModule *rawPtr = module.get();
  manager.registerModule(std::move(module));

  // Inicializa e processa
  manager.initializeAllModules();
  manager.processAllModules();

  // Verifica se o módulo foi processado
  TEST_ASSERT_TRUE(rawPtr->isProcessed());
}

void test_module_retrieval_by_name(void) {
  // Teste de recuperação de módulo por nome
  SystemManager &manager = SystemManager::getInstance();

  // Cria e registra módulo
  auto module = std::make_unique<MockModule>("RetrieveTestModule");
  manager.registerModule(std::move(module));

  // Tenta recuperar o módulo
  IModule *retrieved = manager.getModule("RetrieveTestModule");
  TEST_ASSERT_NOT_NULL(retrieved);
  TEST_ASSERT_EQUAL_STRING("RetrieveTestModule", retrieved->getName().c_str());

  // Tenta recuperar módulo inexistente
  IModule *notFound = manager.getModule("NonExistentModule");
  TEST_ASSERT_NULL(notFound);
}

void test_module_count(void) {
  // Teste de contagem de módulos
  SystemManager &manager = SystemManager::getInstance();

  // Limpa módulos existentes (se houver)
  // Nota: Em um teste real, pode ser necessário resetar o singleton

  // Adiciona alguns módulos
  manager.registerModule(std::make_unique<MockModule>("CountModule1"));
  manager.registerModule(std::make_unique<MockModule>("CountModule2"));
  manager.registerModule(std::make_unique<MockModule>("CountModule3"));

  // Verifica contagem
  TEST_ASSERT_EQUAL_SIZE(3, manager.getRegisteredModules().size());
}

void test_module_operations_edge_cases(void) {
  // Teste de operações de módulo com casos extremos
  SystemManager &manager = SystemManager::getInstance();

  // Teste com módulo nulo (não deve crash)
  // manager.registerModule(nullptr); // Comentado pois pode causar crash

  // Teste com nomes vazios
  manager.registerModule(std::make_unique<MockModule>(""));
  auto modules = manager.getRegisteredModules();
  TEST_ASSERT_TRUE(modules.size() > 0);

  // Teste busca por nome vazio
  IModule *found = manager.getModule("");
  TEST_ASSERT_NOT_NULL(found);

  // Teste busca por nome inexistente
  found = manager.getModule("NonExistentModule12345");
  TEST_ASSERT_NULL(found);
}

void test_module_lifecycle(void) {
  // Teste do ciclo de vida completo dos módulos
  SystemManager &manager = SystemManager::getInstance();

  // Cria módulo
  auto module = std::make_unique<MockModule>("LifecycleModule");
  MockModule *rawPtr = module.get();
  manager.registerModule(std::move(module));

  // Inicializa
  manager.initializeAllModules();
  TEST_ASSERT_TRUE(rawPtr->isInitialized());

  // Processa
  manager.processAllModules();
  TEST_ASSERT_TRUE(rawPtr->isProcessed());

  // Shutdown (se houver método)
  // Nota: SystemManager pode não ter shutdownAllModules
}

void test_systemManager_performance(void) {
  // Teste de performance do SystemManager
  SystemManager &manager = SystemManager::getInstance();

  // Teste performance de registro
  uint32_t start = millis();
  for (int i = 0; i < 100; i++) {
    char name[20];
    sprintf(name, "PerfModule%d", i);
    manager.registerModule(std::make_unique<MockModule>(name));
  }
  uint32_t duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 500); // Deve ser razoável

  // Teste performance de busca
  start = millis();
  for (int i = 0; i < 1000; i++) {
    char name[20];
    sprintf(name, "PerfModule%d", i % 100);
    manager.getModule(name);
  }
  duration = millis() - start;
  TEST_ASSERT_TRUE(duration < 200);
}

void test_module_initialization_order(void) {
  // Teste de ordem de inicialização dos módulos
  SystemManager &manager = SystemManager::getInstance();

  // Registra módulos em ordem específica
  auto module1 = std::make_unique<MockModule>("Module1");
  auto module2 = std::make_unique<MockModule>("Module2");
  auto module3 = std::make_unique<MockModule>("Module3");

  MockModule *ptr1 = module1.get();
  MockModule *ptr2 = module2.get();
  MockModule *ptr3 = module3.get();

  manager.registerModule(std::move(module1));
  manager.registerModule(std::move(module2));
  manager.registerModule(std::move(module3));

  // Inicializa todos
  manager.initializeAllModules();

  // Verifica que todos foram inicializados
  TEST_ASSERT_TRUE(ptr1->isInitialized());
  TEST_ASSERT_TRUE(ptr2->isInitialized());
  TEST_ASSERT_TRUE(ptr3->isInitialized());
}

// Função setup do Unity
void setUp(void) {
  // Nota: Como SystemManager é singleton, pode haver estado residual entre
  // testes Em um ambiente de teste real, seria melhor ter um método de reset
}

// Função teardown do Unity
void tearDown(void) {
  // Limpeza após cada teste
}

// Função principal de execução dos testes
void setup() {
  UNITY_BEGIN();

  RUN_TEST(test_systemManager_singleton);
  RUN_TEST(test_module_registration);
  RUN_TEST(test_module_initialization);
  RUN_TEST(test_module_processing);
  RUN_TEST(test_module_retrieval_by_name);
  RUN_TEST(test_module_count);
  RUN_TEST(test_module_operations_edge_cases);
  RUN_TEST(test_module_lifecycle);
  RUN_TEST(test_systemManager_performance);
  RUN_TEST(test_module_initialization_order);

  UNITY_END();
}

void loop() {
  // Loop vazio para testes unitários
}