#include "cli.h"
#include "badusb_commands.h"
#include "benchmark_commands.h"
#include "core/sd_functions.h"
#include "crypto_commands.h"
#include "dynamic_config_commands.h"
#include "gpio_commands.h"
#include "interpreter_commands.h"
#include "ir_commands.h"
#include "plugin_commands.h"
#include "power_commands.h"
#include "rf_commands.h"
#include "screen_commands.h"
#include "settings_commands.h"
#include "sound_commands.h"
#include "storage_commands.h"
#include "util_commands.h"
#include "wifi_commands.h"
#include <globals.h>

void cliErrorCallback(cmd_error *e) {
  CommandError cmdError(e); // Create wrapper object

  // Usar AdvancedLogger para logging estruturado
  AdvancedLogger &logger = AdvancedLogger::getInstance();
  logger.error(LogModule::SYSTEM, "CLI command error: %s",
               cmdError.toString().c_str());

  serialDevice->print("ERROR: ");
  serialDevice->println(cmdError.toString());

  if (cmdError.hasCommand()) {
    serialDevice->print("Did you mean \"");
    serialDevice->print(cmdError.getCommand().toString());
    serialDevice->println("\"?");
  } else {
    serialDevice->println("Type 'help' for a list of available commands.");
  }
}

SerialCli::SerialCli() { setup(); }

void SerialCli::setup() {
  _cli.setOnError(cliErrorCallback);

  createBenchmarkCommands(&_cli);
  createCryptoCommands(&_cli);
  createDynamicConfigCommands(&_cli);
  createGpioCommands(&_cli);
  createIrCommands(&_cli);
  createPluginCommands(&_cli);
  createPowerCommands(&_cli);
  createRfCommands(&_cli);
  createSettingsCommands(&_cli);
  createStorageCommands(&_cli);
  createUtilCommands(&_cli);
  createWifiCommands(&_cli);

#ifdef USB_as_HID
  createBadUsbCommands(&_cli);
#endif
#ifndef LITE_VERSION
  createInterpreterCommands(&_cli);
#endif
#ifdef HAS_SCREEN
  createScreenCommands(&_cli);
#endif
#if defined(HAS_NS4168_SPKR) || defined(BUZZ_PIN)
  createSoundCommands(&_cli);
#endif
}

// Implementação da validação de entrada segura
bool SerialCli::validateInput(const String &input) {
  // Verificar comprimento máximo para evitar buffer overflow
  const size_t MAX_INPUT_LENGTH = 1024;
  if (input.length() > MAX_INPUT_LENGTH) {
    logger.error(LogModule::SYSTEM, "Input too long: %d characters (max: %d)",
                 input.length(), MAX_INPUT_LENGTH);
    return false;
  }

  // Verificar caracteres permitidos (alfanuméricos, espaços, hífen, underscore,
  // ponto)
  for (size_t i = 0; i < input.length(); ++i) {
    char c = input[i];
    if (!isalnum(c) && c != ' ' && c != '-' && c != '_' && c != '.' &&
        c != '/' && c != '"' && c != '\r' && c != '\n') {
      logger.warning(LogModule::SYSTEM,
                     "Invalid character in input: '%c' (ASCII: %d)", c, (int)c);
      return false;
    }
  }

  // Verificar se não contém sequências perigosas
  if (input.indexOf("..") != -1 || input.indexOf("//") != -1) {
    logger.warning(LogModule::SYSTEM,
                   "Potentially dangerous path sequence detected");
    return false;
  }

  return true;
}
