#pragma once

#include "imodel.h"

#include <QPoint>
#include <QRect>
#include <optional>

class PhantomRectangleModel : public IModel {
public:
  PhantomRectangleModel() = default;
  std::optional<QPoint> GetP1() const;
  void SetP1(QPoint p1);
  std::optional<QPoint> GetP2() const;
  void SetP2(std::optional<QPoint> p2);
  bool ContainsRect(QRect r) const;

private:
  std::optional<QPoint> _p1, _p2;
};