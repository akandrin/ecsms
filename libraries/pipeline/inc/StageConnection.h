#pragma once

class StageConnection {
public:
  virtual ~StageConnection() = default;

  virtual void shutdown() = 0;
};
