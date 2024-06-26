#pragma once

#include <DataType/ISQLType.h>

#include <optional>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
/**
  \brief Интерфейс SQL-типа данных "integer"
*/
//---
class ISQLTypeInteger : public ISQLType {
public:
  /// Деструктор
  virtual ~ISQLTypeInteger() override = default;

public:
  /// Получить тип данных
  virtual SQLDataType GetType() const override final {
    return SQLDataType::Integer;
  }

public:
  /// Получить значение
  /// \return Значение, содержащееся в переменной, если она непустая,
  ///         иначе \c std::nullopt.
  virtual std::optional<int> GetValue() const = 0;
  /// Установить значение
  virtual void SetValue(int value) = 0;
};

/// Тип указателя на ISQLTypeInteger
using ISQLTypeIntegerPtr = std::shared_ptr<ISQLTypeInteger>;
