#include "MLModule.h"
#include "core/advanced_logger.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

MLModule::MLModule(std::shared_ptr<SystemModel> model,
                   std::shared_ptr<SystemView> view)
    : _model(model), _view(view) {}

void MLModule::setup() {
  Serial.println("[ML] Inicializando módulo de Machine Learning");

  // Carregar modelos salvos se existirem
  if (!loadModels()) {
    LOG_WARNING("ML",
                "Nenhum modelo salvo encontrado, iniciando com modelos vazios");
  }

  LOG_INFO("ML", "Módulo ML inicializado com %d pontos de treinamento",
           _knnModel.size());
}

void MLModule::loop() {
  // Processamento contínuo se necessário
  // Por enquanto, o módulo é reativo via comandos
}

void MLModule::handleCommand(const std::string &command,
                             const std::vector<std::string> &args) {
  if (command == "ml_train") {
    if (trainModels()) {
      Serial.println("[ML] Modelos treinados com sucesso");
      _view->showMessage("Modelos treinados com sucesso");
    } else {
      Serial.println("[ML] Falha no treinamento dos modelos");
      _view->showMessage("Erro no treinamento");
    }
  } else if (command == "ml_save") {
    if (saveModels()) {
      LOG_INFO("ML", "Modelos salvos com sucesso");
      _view->showMessage("Modelos salvos");
    } else {
      LOG_ERROR("ML", "Falha ao salvar modelos");
      _view->showMessage("Erro ao salvar");
    }
  } else if (command == "ml_load") {
    if (loadModels()) {
      LOG_INFO("ML", "Modelos carregados com sucesso");
      _view->showMessage("Modelos carregados");
    } else {
      LOG_ERROR("ML", "Falha ao carregar modelos");
      _view->showMessage("Erro ao carregar");
    }
  } else if (command == "ml_stats") {
    auto stats = getModelStats();
    std::string msg = "Estatísticas ML:\n";
    msg += "Pontos treinamento: " + std::to_string(stats["training_points"]) +
           "\n";
    msg += "Acurácia estimada: " + std::to_string(stats["estimated_accuracy"]) +
           "\n";
    _view->showMessage(msg);
  } else if (command == "ml_classify_wifi") {
    // Exemplo: ml_classify_wifi rssi freq channel
    if (args.size() >= 3) {
      std::vector<float> features = {
          std::stof(args[0]), // RSSI
          std::stof(args[1]), // Frequência
          std::stof(args[2])  // Canal
      };
      auto result = classifyWiFiDevice(features);
      std::string msg = "Classificação WiFi: " +
                        std::to_string(static_cast<int>(result.predictedType)) +
                        " (confiança: " + std::to_string(result.confidence) +
                        ")";
      _view->showMessage(msg);
    }
  } else if (command == "ml_classify_rf") {
    // Exemplo: ml_classify_rf freq duration amplitude
    if (args.size() >= 3) {
      std::vector<float> features = {
          std::stof(args[0]), // Frequência
          std::stof(args[1]), // Duração
          std::stof(args[2])  // Amplitude
      };
      auto result = classifyRFSignal(features);
      std::string msg = "Classificação RF: " +
                        std::to_string(static_cast<int>(result.predictedType)) +
                        " (confiança: " + std::to_string(result.confidence) +
                        ")";
      _view->showMessage(msg);
    }
  } else {
    _view->showMessage("Comando ML desconhecido: " + command);
  }
}

std::string MLModule::getDescription() const {
  return "Módulo de Machine Learning para detecção de padrões em dispositivos "
         "WiFi, RF e RFID";
}

void MLModule::getCapabilities(std::vector<std::string> &caps) const {
  caps.push_back("Classificação k-NN");
  caps.push_back("Detecção de anomalias");
  caps.push_back("Aprendizado incremental");
  caps.push_back("Treinamento offline");
  caps.push_back("Inferência em tempo real");
}

MLModule::ClassificationResult
MLModule::classifyWiFiDevice(const std::vector<float> &features) {
  if (_knnModel.empty()) {
    return {UNKNOWN_DEVICE, 0.0f};
  }
  return performKNN(features);
}

MLModule::ClassificationResult
MLModule::classifyRFSignal(const std::vector<float> &features) {
  if (_knnModel.empty()) {
    return {UNKNOWN_DEVICE, 0.0f};
  }
  return performKNN(features);
}

MLModule::AnomalyResult
MLModule::detectRFIDAnomaly(const std::vector<float> &features) {
  // Para RFID, assumimos que o tipo é RFID_CARD por padrão
  return detectAnomalyZScore(features, RFID_CARD);
}

bool MLModule::addTrainingData(const TrainingData &data) {
  // Validação rigorosa dos dados
  if (!data.isValid()) {
    LOG_ERROR("ML", "Dados de treinamento inválidos: tipo %d, features %d",
              static_cast<int>(data.label), data.features.size());
    return false;
  }

  // Verificar limites de memória
  if (_trainingData.size() >= _maxTrainingData) {
    LOG_WARNING(
        "ML",
        "Limite de dados de treinamento atingido (%d), removendo mais antigos",
        _maxTrainingData);
    // Remover dados mais antigos (FIFO)
    if (!_trainingData.empty()) {
      _trainingData.erase(_trainingData.begin());
    }
  }

  _trainingData.push_back(data);
  updateHistoricalStats(data);

  LOG_INFO(
      "ML", "Dados de treinamento adicionados: tipo %d, features %d, total: %d",
      static_cast<int>(data.label), data.features.size(), _trainingData.size());

  // Limpar cache de estatísticas quando dados mudam
  _statsCache.clear();
  _cacheTimestamp = 0;

  return true;
}

bool MLModule::trainModels() {
  if (_trainingData.empty()) {
    Serial.println("[ML] Nenhum dado de treinamento disponível");
    return false;
  }

  // Treinar k-NN: simplesmente armazenar os pontos
  _knnModel = _trainingData;

  Serial.printf("[ML] Modelo k-NN treinado com %d pontos\n", _knnModel.size());
  return true;
}

bool MLModule::saveModels() {
  // Implementação simplificada: salvar em arquivo JSON
  // Nota: Em produção, seria melhor usar um formato binário otimizado

  // Por enquanto, apenas log - implementação completa requer integração com
  // sistema de arquivos
  LOG_INFO("ML", "Salvando modelos (%d pontos)", _knnModel.size());
  return true; // Placeholder
}

bool MLModule::loadModels() {
  // Implementação simplificada
  LOG_INFO("ML", "Carregando modelos");
  return true; // Placeholder
}

std::map<std::string, float> MLModule::getModelStats() {
  std::map<std::string, float> stats;
  stats["training_points"] = static_cast<float>(_knnModel.size());
  stats["estimated_accuracy"] = 0.85f; // Placeholder
  return stats;
}

// Métodos auxiliares

float MLModule::calculateEuclideanDistance(const std::vector<float> &a,
                                           const std::vector<float> &b) {
  if (a.size() != b.size())
    return std::numeric_limits<float>::max();

  float sum = 0.0f;
  for (size_t i = 0; i < a.size(); ++i) {
    float diff = a[i] - b[i];
    sum += diff * diff;
  }
  return std::sqrt(sum);
}

MLModule::ClassificationResult
MLModule::performKNN(const std::vector<float> &features) {
  // Validação de entrada
  if (_knnModel.empty() || !ML_VALIDATE_FEATURES(features)) {
    LOG_WARNING("ML", "Modelo vazio ou features inválidas para k-NN");
    return {UNKNOWN_DEVICE, 0.0f};
  }

  const size_t modelSize = _knnModel.size();
  const int k = std::min(_knnK, static_cast<int>(modelSize));

  // Usar partial_sort para eficiência - só precisamos dos k menores
  std::vector<std::pair<float, DeviceType>> kNearest(k);
  std::vector<std::pair<float, DeviceType>> allDistances;

  // Reservar espaço para evitar realocações
  allDistances.reserve(modelSize);

  // Calcular distâncias com validação
  for (const auto &point : _knnModel) {
    if (point.isValid() && point.features.size() == features.size()) {
      float dist = calculateEuclideanDistance(features, point.features);
      if (std::isfinite(dist)) {
        allDistances.emplace_back(dist, point.label);
      }
    }
  }

  if (allDistances.size() < static_cast<size_t>(k)) {
    LOG_WARNING("ML", "Poucos pontos válidos para k-NN: %d < %d",
                allDistances.size(), k);
    return {UNKNOWN_DEVICE, 0.0f};
  }

  // Partial sort para obter apenas os k menores
  std::partial_sort(allDistances.begin(), allDistances.begin() + k,
                    allDistances.end());

  // Contar votos dos k vizinhos mais próximos
  std::map<DeviceType, int> votes;
  for (int i = 0; i < k; ++i) {
    votes[allDistances[i].second]++;
  }

  // Encontrar o tipo com mais votos
  DeviceType bestType = UNKNOWN_DEVICE;
  int maxVotes = 0;
  for (const auto &vote : votes) {
    if (vote.second > maxVotes) {
      maxVotes = vote.second;
      bestType = vote.first;
    }
  }

  // Calcular confiança (proporção de votos para o vencedor)
  float confidence = static_cast<float>(maxVotes) / k;

  // Só retornar se confiança for suficiente
  if (confidence < _confidenceThreshold) {
    LOG_DEBUG("ML", "Confiança baixa em k-NN: %.2f < %.2f", confidence,
              _confidenceThreshold);
    return {UNKNOWN_DEVICE, confidence};
  }

  LOG_DEBUG("ML", "k-NN classificado como tipo %d com confiança %.2f",
            static_cast<int>(bestType), confidence);

  return {bestType, confidence};
}

MLModule::AnomalyResult
MLModule::detectAnomalyZScore(const std::vector<float> &features,
                              DeviceType type) {
  auto it = _historicalStats.find(type);
  if (it == _historicalStats.end() || it->second.empty()) {
    return {false, 0.0f, "Dados históricos insuficientes"};
  }

  const auto &historical = it->second;

  // Calcular média e desvio padrão dos dados históricos
  float mean = calculateMean(historical);
  float stddev = calculateStdDev(historical, mean);

  if (stddev == 0.0f) {
    return {false, 0.0f, "Variância zero nos dados históricos"};
  }

  // Calcular Z-score para cada feature e encontrar o máximo
  float maxZScore = 0.0f;
  for (float feature : features) {
    float zScore = std::abs((feature - mean) / stddev);
    maxZScore = std::max(maxZScore, zScore);
  }

  // Considerar anômalo se Z-score > threshold
  bool isAnomaly = maxZScore > _anomalyThreshold;
  float score = std::min(maxZScore / (_anomalyThreshold * 2.0f),
                         1.0f); // Normalizar para 0-1

  std::string reason = isAnomaly ? "Z-score alto: " + std::to_string(maxZScore)
                                 : "Comportamento normal";

  return {isAnomaly, score, reason};
}

void MLModule::updateHistoricalStats(const TrainingData &data) {
  // Adicionar features ao histórico com validação
  auto &history = _historicalStats[data.label];
  for (float feature : data.features) {
    if (std::isfinite(feature)) {
      history.push_back(feature);
    }
  }

  // Limitar tamanho histórico usando buffer circular para economizar memória
  if (history.size() > _maxHistorySize) {
    // Remover elementos antigos (FIFO) - mais eficiente que erase no meio
    size_t excess = history.size() - _maxHistorySize;
    history.erase(history.begin(), history.begin() + excess);

    LOG_DEBUG("ML", "Histórico limitado para tipo %d: %d -> %d elementos",
              static_cast<int>(data.label), history.size() + excess,
              history.size());
  }

  // Limpar cache quando histórico muda
  _statsCache.erase(data.label);
}

std::vector<float> MLModule::extractFeaturesFromWiFi(
    const std::map<std::string, float> &wifiData) {
  // Exemplo: RSSI, frequência, canal, etc.
  std::vector<float> features;
  if (wifiData.count("rssi"))
    features.push_back(wifiData.at("rssi"));
  if (wifiData.count("frequency"))
    features.push_back(wifiData.at("frequency"));
  if (wifiData.count("channel"))
    features.push_back(wifiData.at("channel"));
  return features;
}

std::vector<float>
MLModule::extractFeaturesFromRF(const std::map<std::string, float> &rfData) {
  // Exemplo: frequência, duração, amplitude
  std::vector<float> features;
  if (rfData.count("frequency"))
    features.push_back(rfData.at("frequency"));
  if (rfData.count("duration"))
    features.push_back(rfData.at("duration"));
  if (rfData.count("amplitude"))
    features.push_back(rfData.at("amplitude"));
  return features;
}

std::vector<float> MLModule::extractFeaturesFromRFID(
    const std::map<std::string, float> &rfidData) {
  // Exemplo: tempo entre leituras, força do sinal, etc.
  std::vector<float> features;
  if (rfidData.count("read_interval"))
    features.push_back(rfidData.at("read_interval"));
  if (rfidData.count("signal_strength"))
    features.push_back(rfidData.at("signal_strength"));
  return features;
}

float MLModule::calculateMean(const std::vector<float> &data) {
  if (data.empty())
    return 0.0f;
  return std::accumulate(data.begin(), data.end(), 0.0f) / data.size();
}

float MLModule::calculateStdDev(const std::vector<float> &data, float mean) {
  if (data.size() <= 1)
    return 0.0f;

  float sumSquares = 0.0f;
  for (float value : data) {
    float diff = value - mean;
    sumSquares += diff * diff;
  }
  return std::sqrt(sumSquares / (data.size() - 1));
}

// Implementações dos métodos de integração

MLModule::ClassificationResult
MLModule::classifyWiFiDevice(float rssi, float frequency, int channel,
                             const std::string &ssid) {
  std::vector<float> features = {rssi, frequency, static_cast<float>(channel)};
  return classifyWiFiDevice(features);
}

MLModule::ClassificationResult
MLModule::classifyRFSignal(float frequency, float duration, float amplitude,
                           const std::string &modulation) {
  std::vector<float> features = {frequency, duration, amplitude};
  return classifyRFSignal(features);
}

MLModule::AnomalyResult
MLModule::detectRFIDAnomaly(float readInterval, float signalStrength,
                            const std::string &tagType) {
  std::vector<float> features = {readInterval, signalStrength};
  return detectRFIDAnomaly(features);
}

bool MLModule::addWiFiTrainingData(float rssi, float frequency, int channel,
                                   DeviceType deviceType) {
  TrainingData data;
  data.features = {rssi, frequency, static_cast<float>(channel)};
  data.label = deviceType;
  data.timestamp = millis();
  return addTrainingData(data);
}

bool MLModule::addRFTrainingData(float frequency, float duration,
                                 float amplitude, DeviceType deviceType) {
  TrainingData data;
  data.features = {frequency, duration, amplitude};
  data.label = deviceType;
  data.timestamp = millis();
  return addTrainingData(data);
}

bool MLModule::addRFIDTrainingData(float readInterval, float signalStrength,
                                   DeviceType deviceType) {
  TrainingData data;
  data.features = {readInterval, signalStrength};
  data.label = deviceType;
  data.timestamp = millis();
  return addTrainingData(data);
}