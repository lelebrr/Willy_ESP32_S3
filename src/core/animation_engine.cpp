#include "animation_engine.h"
#include "SystemView.h"
#include <driver/timer.h>
#include <esp_timer.h>


// Implementação do Singleton
AnimationEngine &AnimationEngine::getInstance() {
  static AnimationEngine instance;
  return instance;
}

// Timer ISR para animações
void IRAM_ATTR AnimationEngine::onTimer() {
  AnimationEngine &engine = AnimationEngine::getInstance();
  if (engine.running_ && !engine.paused_) {
    engine.update();
  }
}

bool AnimationEngine::init(TFT_eSPI *tft) {
  if (initialized_)
    return true;

  tft_ = tft;
  if (!tft_)
    return false;

  // Criar sprite para renderização otimizada
  sprite_ = new TFT_eSprite(tft_);
  sprite_->createSprite(tft_->width(), tft_->height());

  // Configurar timer de hardware para animações
  animationTimer_ = timerBegin(0, 80, true); // Timer 0, prescaler 80 (1MHz)
  timerAttachInterrupt(animationTimer_, &AnimationEngine::onTimer, true);
  timerAlarmWrite(animationTimer_, 1000000 / frameRate_,
                  true); // Alarme a cada frame
  timerAlarmEnable(animationTimer_);

  // Inicializar efeitos
  scanlines_ = new ScanlinesEffect(tft_);
  glows_ = new GlowEffect(tft_);
  matrixRain_ = new MatrixRainEffect(tft_);
  borderGlow_ = new BorderGlowEffect(tft_);
  glitch_ = new GlitchEffect(tft_);

  scanlines_->init();
  glows_->init();
  matrixRain_->init();
  borderGlow_->init();
  glitch_->init();

  initialized_ = true;
  return true;
}

void AnimationEngine::deinit() {
  if (!initialized_)
    return;

  stopAnimation();

  if (animationTimer_) {
    timerEnd(animationTimer_);
    animationTimer_ = nullptr;
  }

  delete scanlines_;
  delete glows_;
  delete matrixRain_;
  delete borderGlow_;
  delete glitch_;

  if (sprite_) {
    sprite_->deleteSprite();
    delete sprite_;
    sprite_ = nullptr;
  }

  initialized_ = false;
}

void AnimationEngine::startAnimation() {
  if (!initialized_)
    return;
  running_ = true;
  paused_ = false;
}

void AnimationEngine::stopAnimation() {
  running_ = false;
  paused_ = false;
}

void AnimationEngine::pauseAnimation() { paused_ = true; }

void AnimationEngine::resumeAnimation() { paused_ = false; }

void AnimationEngine::setFrameRate(uint8_t fps) {
  frameRate_ = fps;
  if (animationTimer_) {
    timerAlarmWrite(animationTimer_, 1000000 / fps, true);
  }
}

void AnimationEngine::enableEffect(bool scanlines, bool glows, bool matrixRain,
                                   bool borderGlow, bool glitch) {
  scanlinesEnabled_ = scanlines;
  glowsEnabled_ = glows;
  matrixRainEnabled_ = matrixRain;
  borderGlowEnabled_ = borderGlow;
  glitchEnabled_ = glitch;
}

void AnimationEngine::update() {
  uint32_t currentTime = millis();
  if (currentTime - lastFrameTime_ < (1000 / frameRate_))
    return;
  lastFrameTime_ = currentTime;

  // Atualizar efeitos ativos
  if (scanlinesEnabled_)
    scanlines_->update();
  if (glowsEnabled_)
    glows_->update();
  if (matrixRainEnabled_)
    matrixRain_->update();
  if (borderGlowEnabled_)
    borderGlow_->update();
  if (glitchEnabled_)
    glitch_->update();
}

void AnimationEngine::render() {
  if (!initialized_ || !sprite_)
    return;

  // Limpar sprite
  sprite_->fillSprite(DEEP_BLACK);

  // Renderizar efeitos na ordem correta
  if (matrixRainEnabled_)
    matrixRain_->render();
  if (glowsEnabled_)
    glows_->render();
  if (borderGlowEnabled_)
    borderGlow_->render();
  if (scanlinesEnabled_)
    scanlines_->render();
  if (glitchEnabled_)
    glitch_->render();

  // Push sprite para display
  sprite_->pushSprite(0, 0);
}

// ==================== SCANLINES EFFECT ====================

AnimationEngine::ScanlinesEffect::ScanlinesEffect(TFT_eSPI *tft) : tft_(tft) {}

AnimationEngine::ScanlinesEffect::~ScanlinesEffect() {}

void AnimationEngine::ScanlinesEffect::init() { currentY_ = 0; }

void AnimationEngine::ScanlinesEffect::update() {
  currentY_ += speed_;
  if (currentY_ >= tft_->height()) {
    currentY_ = -SCANLINE_HEIGHT;
  }
}

void AnimationEngine::ScanlinesEffect::render() {
  if (!tft_)
    return;

  // Desenhar scanline com opacidade
  tft_->drawFastHLine(0, currentY_, tft_->width(), color_);
  if (currentY_ + 1 < tft_->height()) {
    tft_->drawFastHLine(0, currentY_ + 1, tft_->width(),
                        tft_->alphaBlend(opacity_, color_, DEEP_BLACK));
  }
}

void AnimationEngine::ScanlinesEffect::setSpeed(uint8_t speed) {
  speed_ = speed;
}
void AnimationEngine::ScanlinesEffect::setColor(uint16_t color) {
  color_ = color;
}
void AnimationEngine::ScanlinesEffect::setOpacity(uint8_t opacity) {
  opacity_ = opacity;
}

// ==================== GLOW EFFECT ====================

AnimationEngine::GlowEffect::GlowEffect(TFT_eSPI *tft) : tft_(tft) {}

AnimationEngine::GlowEffect::~GlowEffect() {}

void AnimationEngine::GlowEffect::init() {
  glows_.clear();
  pulsePhase_ = 0;
}

void AnimationEngine::GlowEffect::update() {
  pulsePhase_ += pulseSpeed_;
  if (pulsePhase_ >= 255)
    pulsePhase_ = 0;
}

void AnimationEngine::GlowEffect::render() {
  if (!tft_ || glows_.empty())
    return;

  // Calcular intensidade pulsante
  float intensity = (sin(pulsePhase_ * 3.14159 / 128.0) + 1.0) * 0.5;

  for (const auto &glow : glows_) {
    // Desenhar glow com intensidade variável
    uint8_t alpha = glow.intensity * intensity;
    tft_->drawRect(glow.x - 1, glow.y - 1, glow.width + 2, glow.height + 2,
                   tft_->alphaBlend(alpha, glow.color, DEEP_BLACK));
  }
}

void AnimationEngine::GlowEffect::addGlow(int16_t x, int16_t y, uint16_t width,
                                          uint16_t height, uint16_t color) {
  GlowElement glow = {x, y, width, height, color, 128};
  glows_.push_back(glow);
}

void AnimationEngine::GlowEffect::clearGlows() { glows_.clear(); }

void AnimationEngine::GlowEffect::setPulseSpeed(uint8_t speed) {
  pulseSpeed_ = speed;
}

// ==================== MATRIX RAIN EFFECT ====================

AnimationEngine::MatrixRainEffect::MatrixRainEffect(TFT_eSPI *tft)
    : tft_(tft) {}

AnimationEngine::MatrixRainEffect::~MatrixRainEffect() {}

void AnimationEngine::MatrixRainEffect::init() {
  for (uint8_t i = 0; i < dropCount_; i++) {
    generateDrop(i);
  }
}

void AnimationEngine::MatrixRainEffect::update() {
  for (uint8_t i = 0; i < dropCount_; i++) {
    drops_[i].y += drops_[i].speed;
    if (drops_[i].y > tft_->height() + 20) {
      generateDrop(i);
    }
  }
}

void AnimationEngine::MatrixRainEffect::render() {
  if (!tft_)
    return;

  for (uint8_t i = 0; i < dropCount_; i++) {
    const auto &drop = drops_[i];

    // Renderizar caracteres da gota
    for (uint8_t j = 0; j < drop.length && j < 10; j++) {
      int16_t y = drop.y - j * 8;
      if (y >= 0 && y < tft_->height()) {
        uint8_t alpha = 255 - (j * 25); // Fade out
        tft_->drawChar(drop.x, y, drop.chars[j],
                       tft_->alphaBlend(alpha, color_, DEEP_BLACK),
                       tft_->alphaBlend(alpha, DEEP_BLACK, color_), 1);
      }
    }
  }
}

void AnimationEngine::MatrixRainEffect::setDropCount(uint8_t count) {
  dropCount_ = min(count, (uint8_t)MAX_MATRIX_DROPS);
}

void AnimationEngine::MatrixRainEffect::setSpeed(uint8_t speed) {
  baseSpeed_ = speed;
}
void AnimationEngine::MatrixRainEffect::setColor(uint16_t color) {
  color_ = color;
}

void AnimationEngine::MatrixRainEffect::generateDrop(uint8_t index) {
  drops_[index].x = random(tft_->width());
  drops_[index].y = random(-50, 0);
  drops_[index].length = random(5, 15);
  drops_[index].speed = baseSpeed_ + random(-1, 2);

  // Gerar caracteres aleatórios
  for (uint8_t i = 0; i < drops_[index].length && i < 10; i++) {
    drops_[index].chars[i] = getRandomChar();
  }
}

char AnimationEngine::MatrixRainEffect::getRandomChar() {
  const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()";
  return chars[random(sizeof(chars) - 1)];
}

// ==================== BORDER GLOW EFFECT ====================

AnimationEngine::BorderGlowEffect::BorderGlowEffect(TFT_eSPI *tft)
    : tft_(tft) {}

AnimationEngine::BorderGlowEffect::~BorderGlowEffect() {}

void AnimationEngine::BorderGlowEffect::init() { pulsePhase_ = 0; }

void AnimationEngine::BorderGlowEffect::update() {
  pulsePhase_ += pulseSpeed_;
  if (pulsePhase_ >= 255)
    pulsePhase_ = 0;
}

void AnimationEngine::BorderGlowEffect::render() {
  if (!tft_)
    return;

  // Calcular intensidade pulsante
  float intensity = (sin(pulsePhase_ * 3.14159 / 128.0) + 1.0) * 0.5;
  uint8_t alpha = 255 * intensity;

  uint16_t blendedColor = tft_->alphaBlend(alpha, color_, DEEP_BLACK);

  // Desenhar borda com glow
  for (uint8_t t = 0; t < thickness_; t++) {
    // Topo
    tft_->drawFastHLine(t, t, tft_->width() - 2 * t, blendedColor);
    // Baixo
    tft_->drawFastHLine(t, tft_->height() - 1 - t, tft_->width() - 2 * t,
                        blendedColor);
    // Esquerda
    tft_->drawFastVLine(t, t, tft_->height() - 2 * t, blendedColor);
    // Direita
    tft_->drawFastVLine(tft_->width() - 1 - t, t, tft_->height() - 2 * t,
                        blendedColor);
  }
}

void AnimationEngine::BorderGlowEffect::setColor(uint16_t color) {
  color_ = color;
}
void AnimationEngine::BorderGlowEffect::setThickness(uint8_t thickness) {
  thickness_ = thickness;
}
void AnimationEngine::BorderGlowEffect::setPulseSpeed(uint8_t speed) {
  pulseSpeed_ = speed;
}

// ==================== GLITCH EFFECT ====================

AnimationEngine::GlitchEffect::GlitchEffect(TFT_eSPI *tft) : tft_(tft) {}

AnimationEngine::GlitchEffect::~GlitchEffect() {}

void AnimationEngine::GlitchEffect::init() { active_ = false; }

void AnimationEngine::GlitchEffect::update() {
  if (active_ && millis() - startTime_ > duration_) {
    active_ = false;
  }
}

void AnimationEngine::GlitchEffect::render() {
  if (!active_ || !tft_)
    return;

  // Aplicar efeito baseado no tipo aleatório
  uint8_t effectType = random(3);
  switch (effectType) {
  case 0:
    applyRGBShift();
    break;
  case 1:
    applyPixelCorruption();
    break;
  case 2:
    applyLineDistortion();
    break;
  }
}

void AnimationEngine::GlitchEffect::triggerGlitch(uint16_t duration) {
  active_ = true;
  startTime_ = millis();
  duration_ = duration;
}

void AnimationEngine::GlitchEffect::setIntensity(uint8_t intensity) {
  intensity_ = intensity;
}

void AnimationEngine::GlitchEffect::applyRGBShift() {
  // Deslocamento RGB simples (simulado com linhas coloridas)
  for (uint8_t i = 0; i < intensity_ / 10; i++) {
    int16_t y = random(tft_->height());
    uint16_t color = random(0xFFFF);
    tft_->drawFastHLine(0, y, tft_->width(), color);
  }
}

void AnimationEngine::GlitchEffect::applyPixelCorruption() {
  // Corromper pixels aleatórios
  for (uint16_t i = 0; i < intensity_; i++) {
    int16_t x = random(tft_->width());
    int16_t y = random(tft_->height());
    uint16_t color = random(0xFFFF);
    tft_->drawPixel(x, y, color);
  }
}

void AnimationEngine::GlitchEffect::applyLineDistortion() {
  // Distortion de linhas horizontais
  for (uint8_t i = 0; i < intensity_ / 20; i++) {
    int16_t y = random(tft_->height());
    int16_t offset = random(-5, 6);
    if (y + offset >= 0 && y + offset < tft_->height()) {
      // Copiar linha com offset (simplificado)
      tft_->drawFastHLine(0, y + offset, tft_->width(), tft_->readPixel(0, y));
    }
  }
}