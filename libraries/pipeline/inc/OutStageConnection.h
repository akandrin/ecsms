#pragma once

#include "StageConnection.h"
#include "StageTask.h"

template <typename T>
class OutStageConnection : public virtual StageConnection {
 public:
  virtual std::shared_ptr<StageTask<T>> getProducerTask() = 0;

  virtual void releaseProducerTask(std::shared_ptr<T> taskData,
                                   size_t newTaskId,
                                   bool produced) = 0;
};
