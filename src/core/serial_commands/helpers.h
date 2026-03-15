#ifndef __SERIAL_HELPERS_H__
#define __SERIAL_HELPERS_H__
#ifndef LITE_VERSION
#include <precompiler_flags.h>

#include <Arduino.h>
#include <PSRamFS.h>

bool _setupPsramFs();
char *_readFileFromSerial(size_t fileSizeChar = SAFE_STACK_BUFFER_SIZE);

// Função de sanitização global para entradas
bool sanitizeInput(const String &input, size_t maxLength = 1024);
String sanitizeFilename(const String &filename);

#endif
#endif
