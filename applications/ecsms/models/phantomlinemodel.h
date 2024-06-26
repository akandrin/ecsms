#pragma once

#include "../widgets/connectnodewidget.h"
#include "imodel.h"

#include <memory>
#include <optional>

class PhantomLineModel : public IModel {
public:
  PhantomLineModel() = default;

  PhantomLineModel &operator=(const PhantomLineModel &other);

  std::optional<QPoint> GetBegin() const;
  std::optional<NodeId> GetBeginNode() const;
  void SetBegin(std::optional<NodeId> node, std::optional<QPoint> begin);
  std::optional<QPoint> GetEnd() const;
  void SetEnd(QPoint end);

  void Clear();

private:
  std::optional<NodeId> _begin_node;
  std::optional<QPoint> _begin, _end;
};
