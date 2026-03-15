#include "dynamic_config_commands.h"
#include "core/DynamicConfigManager.h"
#include <globals.h>

uint32_t reloadConfigCallback(cmd *c) {
  AdvancedLogger &logger = AdvancedLogger::getInstance();
  logger.info(LogModule::SYSTEM, "Reloading dynamic configuration");

  bool success = DynamicConfigManager::getInstance().reloadConfigCommand();
  if (success) {
    logger.info(LogModule::SYSTEM, "Configuration reloaded successfully");
  } else {
    logger.error(LogModule::SYSTEM, "Failed to reload configuration");
  }
  return success;
}

uint32_t showConfigStatusCallback(cmd *c) {
  auto &dcm = DynamicConfigManager::getInstance();

  serialDevice->println("=== Status da Configuração Dinâmica ===");
  serialDevice->printf("Configuração carregada: %s\n",
                       dcm.isConfigLoaded() ? "Sim" : "Não");

  if (dcm.isConfigLoaded()) {
    serialDevice->println("Configuração atual:");
    serializeJsonPretty(dcm.getCurrentConfig(), Serial);
    serialDevice->println("");
  } else {
    serialDevice->println("Nenhuma configuração dinâmica carregada");
  }

  return true;
}

uint32_t loadConfigFileCallback(cmd *c) {
  Command cmd(c);
  Argument filename_arg = cmd.getArgument("filename");
  String filename = filename_arg.getValue();
  filename.trim();

  if (filename.length() == 0) {
    serialDevice->println("Uso: load_config <filename>");
    serialDevice->println("Exemplo: load_config /config/system_config.json");
    return false;
  }

  auto &dcm = DynamicConfigManager::getInstance();
  if (dcm.loadConfigFromFile(filename)) {
    serialDevice->printf("Configuração carregada de: %s\n", filename.c_str());
    return true;
  } else {
    serialDevice->printf("Falha ao carregar configuração de: %s\n",
                         filename.c_str());
    return false;
  }
}

void createDynamicConfigCommands(SimpleCLI *cli) {
  Command reloadCmd = cli->addCommand("reload_config", reloadConfigCallback);
  reloadCmd.setDescription(
      "Recarrega a configuração dinâmica do arquivo atual");

  Command statusCmd =
      cli->addCommand("config_status", showConfigStatusCallback);
  statusCmd.setDescription("Mostra o status da configuração dinâmica atual");

  Command loadCmd = cli->addCommand("load_config", loadConfigFileCallback);
  loadCmd.addPosArg("filename", "");
  loadCmd.setDescription("Carrega configuração de um arquivo específico");
}