#pragma once

#include <DataType/ISQLType.h>
#include <DataType/ISQLTypeText.h>
#include <IConnection.h>
#include <IExecuteResult.h>

#include <map>
#include <memory>
#include <vector>

class IExecutorEAVNamingRules;

//------------------------------------------------------------------------------
/**
  Интерфейс исполнителя запросов EAV.
  Он предоставляет базовые запросы, которые могут оказаться полезными при работе
  с базой данных, представленной схемой EAV.

  Правила именования таблиц для базы данных определяются интерфейсом
  IExecutorEAVNamingRules.

  Метод SetRegisteredEntities может создать таблицы по указанным правилам, если
  передать в него флаг createTables = true. Остальные методы не создают таблицы,
  а пользуются существующими, созданными этим методом. При отсутствии таких
  таблиц команды будут выдавать ошибку.

  Все методы класса не открывают и не закрывают транзакции.
  Таким образом, можно в вызывающем коде открыть транзакцию, вызвать один или
  несколько методов класса, после чего зафиксировать (или отменить) транзакцию,
  применив (или не применив) разом несколько изменений.

  (Если вызывающий код не открывает транзакцию, то каждый метод класса будет
  работать в рамках одной или нескольких транзакций (зависит от реализации
  конкретного метода))
*/
//---
class IExecutorEAV {
public:
  /// Тип для названия сущности (осмысленное название сущности).
  using EntityName = std::string;
  /// Тип для идентификатора сущности (числовой идентификатор конкретной
  /// сущности в её таблице сущностей).
  using EntityId = int;
  /// Тип для названия атрибута.
  using AttrName = ISQLTypeTextPtr;
  /// Тип значения атрибута
  using ValueType = ISQLTypePtr;

  /// Структура для хранения пары: атрибут и его значение
  struct AttrValue {
    AttrName attrName; ///< Название атрибута
    ValueType value;   ///< Значение атрибута
  };

  /// Запись в реестре EAV
  using EAVRegisterEntries =
      std::map<EntityName, // Название сущности
               std::vector<SQLDataType>>; // Типы атрибутов, которые она будет
                                          // использовать

public:
  /// Деструктор
  virtual ~IExecutorEAV() = default;

public:
  /// Регистрация EAV-сущностей для дальнешей работы с классом
  /// \param entries Контейнер с соответствиями "название сущности" - "типы
  /// атрибутов, которые она использует" \param createTables Требуется ли
  /// пытаться создать таблицы по зарегистрированным сущностям \return Статус
  /// выполнения операции
  virtual IExecuteResultStatusPtr
  SetRegisteredEntities(const EAVRegisterEntries &entries,
                        bool createTables) = 0;
  /// Получить зарегистрированные сущности
  /// \return Контейнер с соответствиями "название сущности" - "типы атрибутов,
  /// которые она использует"
  virtual const EAVRegisterEntries &GetRegisteredEntities() const = 0;

public:
  /// Получить объект, определяющий правила именования таблиц
  /// \return Объект, определяющий правила именования таблиц
  virtual const IExecutorEAVNamingRules &GetNamingRules() const = 0;

public
    : // Методы для создания новых сущностей и поиска уже существующих сущностей
  /// Создать новую сущность данного вида
  /// \param entityName Название сущности
  /// \param[out] result Идентификатор новой сущности
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr CreateNewEntity(const EntityName &entityName,
                                                  EntityId &result) = 0;
  /// Получить все идентификаторы сущности данного вида
  /// \param entityName Название сущности
  /// \param[out] result Массив идентификаторов сущности.
  ///                    В случае успеха массив прочищается перед записью
  ///                    результата.
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr
  GetEntityIds(const EntityName &entityName, std::vector<EntityId> &result) = 0;
  /// Получить все наименования атрибутов указанного типа, которые использует
  /// данная сущность \param entityName Название сущности \param sqlDataType Тип
  /// атрибута \param result[out] Массив наименований атрибутов.
  ///                    В случае успеха массив прочищается перед записью
  ///                    результата.
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr
  GetAttributeNames(const EntityName &entityName, SQLDataType sqlDataType,
                    std::vector<AttrName> &result) = 0;
  /// Найти сущности, у которых есть все из указанных пар атрибут-значение
  /// \param entityName Название сущности
  /// \param attrValues Массив пар атрибут-значение.
  ///                   В качестве значения может быть передана пустая
  ///                   переменная. Тогда произведен поиск таких атрибутов
  ///                   сущности, у которых значение не задано.
  /// \param[out] result Массив идентификаторов сущности, у которых есть все из
  /// указанных пар атрибут-значение
  ///                     Массив прочищается перед записью результата.
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr
  FindEntitiesByAttrValues(const EntityName &entityName,
                           const std::vector<AttrValue> &attrValues,
                           std::vector<EntityId> &result) = 0;

public: // Методы для вставки/обновления данных
  /// Вставить значение для атрибута сущности в таблицу значений,
  /// а также вставить название атрибутов в таблицу атрибутов, если его там ещё
  /// не было. \param entityName Название сущности \param entityId Идентфикатор
  /// сущности. При передаче несуществующего идентификатора
  ///                 вернется ошибка.
  /// \param attrName Наименование атрибута. При передаче названия
  /// несуществующего атрибута
  ///                 будет добавлен новый атрибут с таким названием.
  /// \param value Значение атрибута. Это обязательно должна быть непустая
  /// (ISQLType::IsEmpty)
  ///              и ненулевая переменная, в противном случае запрос не будет
  ///              выполнен и вернется ошибка. (Смысл EAV в том, что мы не
  ///              храним null, так что бессмысленно вставлять пустые значения,
  ///              символизирующие null)
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr Insert(const EntityName &entityName,
                                         EntityId entityId,
                                         const AttrName &attrName,
                                         const ValueType &value) = 0;
  /// Обновить значение для атрибута сущности в таблице значений.
  /// \param entityName Название сущности
  /// \param entityId Идентфикатор сущности. При передаче несуществующего
  /// идентификатора
  ///                 вернется ошибка.
  /// \param attrName Наименование атрибута. При передаче несуществующего
  /// названия
  ///                 вернется ошибка.
  /// \param value Значение атрибута. Это обязательно должна быть ненулевая
  /// переменная,
  ///              иначе запрос не будет выполнен и вернется ошибка.
  ///              Если значение атрибута пустое (ISQLType::IsEmpty),
  ///              то значение атрибута сущности будет удалено из таблицы
  ///              значений (если оно там есть). (Смысл EAV в том, что мы не
  ///              храним null, так что попытка занулить значение приводит к его
  ///              удалению)
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr Update(const EntityName &entityName,
                                         EntityId entityId,
                                         const AttrName &attrName,
                                         const ValueType &value) = 0;
  /// Обновить значение для атрибута сущности в таблице значений
  /// или вставить значение атрибута в таблицу значений, если такого значения
  /// ещё не было. А также вставить название атрибута в таблицу атрибутов, если
  /// его там ещё не было. \param entityName Название сущности \param entityId
  /// Идентфикатор сущности. При передаче несуществующего идентификатора
  ///                 вернется ошибка.
  /// \param attrName Наименование атрибута. При передаче названия
  /// несуществующего атрибута
  ///                 будет добавлен новый атрибут с таким названием.
  /// \param value Значение атрибута. Это обязательно должна быть ненулевая
  /// переменная,
  ///              иначе запрос не будет выполнен и вернется ошибка.
  ///              Если значение атрибута пустое (ISQLType::IsEmpty),
  ///              то значение не будет вставлено в таблицу значений (если его
  ///              не было), или будет удалено из таблицы значений (если оно
  ///              было). (Смысл EAV в том, что мы не храним null) Вне
  ///              зависимости от того, пришла пустая или непустая переменная
  ///              value, атрибут будет добавлен в таблицу атрибутов (если его
  ///              там ещё не было).
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr InsertOrUpdate(const EntityName &entityName,
                                                 EntityId entityId,
                                                 const AttrName &attrName,
                                                 const ValueType &value) = 0;

public: // Методы для получения данных
  /// Получить значение атрибута сущности.
  /// Если у данной сущности в таблице атрибутов есть атрибут данного типа с
  /// данным названием, однако в таблице значений для данного идентификатора
  /// сущности и данного атрибута нет значения, то метод НЕ вернет ошибку, а
  /// вернет пустую переменную. \param entityName Название сущности \param
  /// entityId Идентфикатор сущности \param attrName Наименование атрибута
  /// \param[in,out] value Ненулевая пустая переменная, по типу которой будет
  /// определен тип атрибута,
  ///                      и в которую запишется результат. Если будет передана
  ///                      непустая переменная, то вернется ошибка.
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr GetValue(const EntityName &entityName,
                                           EntityId entityId,
                                           const AttrName &attrName,
                                           ValueType value) = 0;
  /// Получить значения всех атрибутов сущности.
  /// Для определения типов атрибутов метод использует GetRegisteredEntities.
  /// Таким образом, метод не вернет информацию о незарегистрированных типах
  /// атрибутов, если они есть. \param entityName Название сущности \param
  /// entityId Идентификатор сущности \param[out] attrValuesByType Мапа, в
  /// которой ключем является тип атрибута, а значением -
  ///             массив структур, содержащих название атрибута и его значение.
  ///             Метод возвращает также те пары атрибут-значение, в которых
  ///             значение не задано: тогда значение будет представлено пустой
  ///             SQL-переменной. В случае успеха метод очищает вектор перед
  ///             вставкой.
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr GetAttributeValues(
      const EntityName &entityName, EntityId entityId,
      std::map<SQLDataType, std::vector<AttrValue>> &attrValuesByType) = 0;
};

/// Тип указателя на исполнитель EAV-запросов
using IExecutorEAVPtr = std::shared_ptr<IExecutorEAV>;
