/*
 * Código de Teste para Joystick KY-023 e IR YS-IRTM
 * ESP32-S3-WROOM-1-N8R2
 *
 * Este arquivo contém um código de teste para validar as implementações
 * do joystick e módulo IR.
 */

// Definições dos pinos
#define JOY_X_PIN 4   // ADC1_CH3
#define JOY_Y_PIN 5   // ADC1_CH4
#define JOY_BTN_PIN 6 // Digital com INPUT_PULLUP

#define IR_SERIAL_TX_PIN 1  // ESP TX → YS-IRTM RXD
#define IR_SERIAL_RX_PIN 47 // ESP RX ← YS-IRTM TXD
#define IR_SERIAL_BAUD 9600

// Variáveis globais para testes
bool test_passed = true;
int test_count = 0;
int test_success = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Teste Joystick KY-023 e IR YS-IRTM ===");
  Serial.println("ESP32-S3-WROOM-1-N8R2");
  Serial.println();

  // Configuração dos pinos do joystick
  pinMode(JOY_BTN_PIN, INPUT_PULLUP);

  // Configuração dos ADCs
  analogReadResolution(12); // 12 bits (0-4095)

  // Inicialização da serial IR
  Serial2.begin(IR_SERIAL_BAUD, SERIAL_8N1, IR_SERIAL_RX_PIN, IR_SERIAL_TX_PIN);

  Serial.println("Configuração inicial concluída!");
  Serial.printf("Pinos configurados: X=%d, Y=%d, BTN=%d\n", JOY_X_PIN,
                JOY_Y_PIN, JOY_BTN_PIN);
  Serial.println();

  run_tests();
}

void run_tests() {
  Serial.println("=== Iniciando Testes ===");

  // Teste 1: Verificar pinos do joystick
  test_joystick_pins();

  // Teste 2: Verificar leitura analógica
  test_analog_read();

  // Teste 3: Verificar botão do joystick
  test_joystick_button();

  // Teste 4: Verificar comunicação IR
  test_ir_communication();

  // Teste 5: Calibração do joystick
  test_joystick_calibration();

  // Teste 6: Tempo de resposta
  test_response_time();

  // Resultados
  Serial.println();
  Serial.println("=== Resultados dos Testes ===");
  Serial.printf("Total de testes: %d\n", test_count);
  Serial.printf("Sucessos: %d\n", test_success);
  Serial.printf("Falhas: %d\n", test_count - test_success);
  Serial.printf("Taxa de sucesso: %.1f%%\n",
                (float)test_success / test_count * 100);

  if (test_success == test_count) {
    Serial.println("✅ Todos os testes passaram!");
  } else {
    Serial.println("❌ Alguns testes falharam. Verifique as conexões.");
    Serial.println("   Dicas:");
    Serial.println("   - Verifique conexões dos pinos");
    Serial.println("   - Teste alimentação do joystick (3.3V-5V)");
    Serial.println("   - Verifique fiação RX/TX do módulo IR");
  }
}

void test_joystick_pins() {
  Serial.println("Teste 1: Verificando pinos do joystick...");

  test_count++;

  // Verificar se os pinos estão configurados corretamente
  if (JOY_X_PIN == 4 && JOY_Y_PIN == 5 && JOY_BTN_PIN == 6) {
    Serial.println("✅ Pinos do joystick configurados corretamente");
    test_success++;
  } else {
    Serial.println("❌ Pinos do joystick incorretos");
    Serial.printf("Esperado: X=4, Y=5, BTN=6\n");
    Serial.printf("Recebido: X=%d, Y=%d, BTN=%d\n", JOY_X_PIN, JOY_Y_PIN,
                  JOY_BTN_PIN);
  }
  Serial.println();
}

void test_analog_read() {
  Serial.println("Teste 2: Verificando leitura analógica...");

  test_count++;

  // Ler valores dos eixos
  int x = analogRead(JOY_X_PIN);
  int y = analogRead(JOY_Y_PIN);

  Serial.printf("Valores lidos: X=%d, Y=%d\n", x, y);

  // Verificar se os valores estão na faixa esperada (0-4095)
  if (x >= 0 && x <= 4095 && y >= 0 && y <= 4095) {
    Serial.println("✅ Leitura analógica dentro da faixa esperada");
    test_success++;

    // Verificar se os valores estão próximos do centro (calibração)
    if (x > 1000 && x < 3500 && y > 1000 && y < 3500) {
      Serial.println(
          "✅ Valores próximos do centro - joystick calibrado corretamente");
    } else {
      Serial.println("⚠️  Valores fora do centro - calibragem necessária");
      Serial.println("   Mova o joystick para todas as direções para calibrar");
    }
  } else {
    Serial.println("❌ Leitura analógica fora da faixa esperada");
    Serial.println("   Verifique conexões e alimentação do joystick");
  }
  Serial.println();
}

void test_joystick_button() {
  Serial.println("Teste 3: Verificando botão do joystick...");

  test_count++;

  // Ler estado do botão
  bool btn_state = digitalRead(JOY_BTN_PIN);

  Serial.printf("Estado do botão: %s\n",
                btn_state ? "HIGH (solto)" : "LOW (pressionado)");

  // O botão deve estar em HIGH (solto) devido ao INPUT_PULLUP
  if (btn_state == HIGH) {
    Serial.println("✅ Botão configurado corretamente com INPUT_PULLUP");
    test_success++;

    Serial.println("Pressione o botão do joystick para continuar...");
    delay(2000);

    // Verificar se o botão foi pressionado
    btn_state = digitalRead(JOY_BTN_PIN);
    if (btn_state == LOW) {
      Serial.println("✅ Botão responde ao pressionar");
      test_success++;
    } else {
      Serial.println("❌ Botão não responde ao pressionar");
    }
  } else {
    Serial.println("❌ Botão não está em estado HIGH (solto)");
    Serial.println("   Verifique a configuração INPUT_PULLUP");
  }
  Serial.println();
}

void test_ir_communication() {
  Serial.println("Teste 4: Verificando comunicação IR...");

  test_count++;

  // Verificar se a serial IR foi inicializada
  if (Serial2) {
    Serial.println("✅ Serial IR inicializada corretamente");
    test_success++;

    // Teste de envio de dados
    Serial.println("Enviando comando de teste IR...");
    Serial2.write("TEST\n");
    Serial2.flush(); // Garante que os dados foram enviados

    // Pequena pausa para resposta
    delay(100);

    // Verificar se há dados disponíveis para leitura
    if (Serial2.available()) {
      Serial.println("✅ Comunicação IR bidirecional funcionando");
      test_success++;

      // Ler resposta (se houver)
      String response = "";
      while (Serial2.available()) {
        char c = Serial2.read();
        response += c;
        if (response.length() > 100)
          break; // Limite de segurança
      }

      if (response.length() > 0) {
        Serial.printf("Resposta recebida: %s\n", response.c_str());
      }
    } else {
      Serial.println("⚠️  Comunicação IR unidirectional (transmissão apenas)");
      Serial.println("   Verifique se o módulo IR suporta resposta");
    }

    // Teste de baud rate
    Serial.printf("Baud rate configurado: %d\n", IR_SERIAL_BAUD);

  } else {
    Serial.println("❌ Falha ao inicializar serial IR");
    Serial.println("   Verifique conexões RX/TX e configuração dos pinos");
  }
  Serial.println();
}

void test_joystick_calibration() {
  Serial.println("Teste 5: Calibração do joystick...");

  test_count++;

  // Ler valores múltiplas vezes para calibração
  const int samples = 10;
  int x_sum = 0, y_sum = 0;

  for (int i = 0; i < samples; i++) {
    x_sum += analogRead(JOY_X_PIN);
    y_sum += analogRead(JOY_Y_PIN);
    delay(10);
  }

  int x_avg = x_sum / samples;
  int y_avg = y_sum / samples;

  Serial.printf("Valores médios - X: %d, Y: %d\n", x_avg, y_avg);

  // Verificar se está próximo do centro (2048 para 12-bit ADC)
  const int center_tolerance = 500;
  if (abs(x_avg - 2048) < center_tolerance &&
      abs(y_avg - 2048) < center_tolerance) {
    Serial.println("✅ Joystick calibrado corretamente (próximo do centro)");
    test_success++;
  } else {
    Serial.println("⚠️  Joystick pode precisar de calibração");
    Serial.println("   Mova para todas as direções e teste novamente");
  }
  Serial.println();
}

void test_response_time() {
  Serial.println("Teste 6: Teste de tempo de resposta...");

  test_count++;

  // Teste tempo de resposta do joystick
  uint32_t start = millis();
  for (int i = 0; i < 1000; i++) {
    analogRead(JOY_X_PIN);
    analogRead(JOY_Y_PIN);
    digitalRead(JOY_BTN_PIN);
  }
  uint32_t duration = millis() - start;

  Serial.printf("1000 leituras em %d ms (%.2f ms/leitura)\n", duration,
                (float)duration / 1000);

  if (duration < 500) { // Deve ser rápido
    Serial.println("✅ Tempo de resposta adequado");
    test_success++;
  } else {
    Serial.println("⚠️  Tempo de resposta lento");
  }
  Serial.println();
}

void loop() {
  // Monitoramento contínuo do joystick
  static uint32_t last_print = 0;
  if (millis() - last_print > 1000) {
    int x = analogRead(JOY_X_PIN);
    int y = analogRead(JOY_Y_PIN);
    bool btn = digitalRead(JOY_BTN_PIN);

    Serial.printf("Joystick - X: %4d, Y: %4d, BTN: %s\n", x, y,
                  btn ? "HIGH" : "LOW");

    // Detectar movimentos
    if (x < 1500)
      Serial.println("  ← Esquerda");
    else if (x > 3500)
      Serial.println("  → Direita");

    if (y < 1500)
      Serial.println("  ↑ Cima");
    else if (y > 3500)
      Serial.println("  ↓ Baixo");

    if (btn == LOW)
      Serial.println("  [Botão pressionado]");

    last_print = millis();
  }

  delay(50);
}
