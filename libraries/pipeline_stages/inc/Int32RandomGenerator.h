#pragma once

#include "ProducerStage.h"

class Int32RandomGenerator : public ProducerStage<int32_t> {
public:
  using connectionT = OutStageConnection<int32_t>;

  Int32RandomGenerator(std::shared_ptr<connectionT> outConnection);

  void produce(std::shared_ptr<int32_t> outData) override;

public:
  static constexpr auto stageName = "Int32RandomGenerator";
  static constexpr auto stageType = PipelineStageType::producer;
  using consumptionT = void;
  using productionT = int32_t;
};
