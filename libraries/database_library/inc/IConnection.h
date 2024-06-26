#pragma once

#include <DataType/SQLDataType.h>
#include <IExecuteResult.h>
#include <IFile.h>

#include <memory>
#include <variant>

//------------------------------------------------------------------------------
/**
  \brief Статус соединения
*/
//---
enum class ConnectionStatus {
  Ok,     ///< Соединение успешно установлено
  Bad,    ///< Ошибка установки соединения
  Unknown ///< Неизвестный статус
};

/// Тип аргумента для выполнения метода Execute
using ExecuteArgType = std::variant<std::string, std::vector<char>>;

//------------------------------------------------------------------------------
/**
  \brief Интерфейс соединения.

  При многопоточной работе с БД рекомендуется для каждого потока создавать
  свой объект соединения (в противном случае потоки будут выстраиваться в
  очередь).
*/
//---
class IConnection {
public:
  /// Деструктор
  virtual ~IConnection() = default;

public:
  /// Проверить, валидно ли соединение
  /// \return \c true, если соединение валидно, иначе \c false.
  virtual bool IsValid() const = 0;
  /// Получить статус соединения
  virtual ConnectionStatus GetStatus() const = 0;

  /// Выполнить запрос.
  /// Запрос может содержать несколько SQL команд, тогда они автоматически будут
  /// выполнены в рамках одной транзакции, кроме случаев, когда команды для
  /// разбиения на транзакции не присутствуют явно (посредством добавления
  /// команд BEGIN/COMMIT в текст запроса query или посредством вызова методом
  /// BeginTransaction/CommitTransaction).
  /// \param query Строка, содержащая SQL запрос.
  /// \return Результат выполнения запроса
  virtual IExecuteResultPtr Execute(const std::string &query) = 0;

  /// Открыть транзакцию
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr BeginTransaction() = 0;
  /// Закрыть транзакцию с применением изменений
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr CommitTransaction() = 0;
  /// Отменить транзакцию (без применения изменений)
  /// \return Статус выполнения операции
  virtual IExecuteResultStatusPtr RollbackTransaction() = 0;

public:
  /// Создать большой бинарный объект.
  /// Метод может вызываться как внутри транзакции, так и не в ней.
  /// При вызове внутри транзакции действуют обычные правила транзакции:
  /// если транзакция не была завершена фиксацией или была завершена отменой, то
  /// файл не создатся.
  /// \warning Последующая работа с большим бинарным объектом возможна только в
  ///          рамках транзакции.
  /// \return Интерфейс для взаимодействия с большим бинарным объектом.
  virtual IFilePtr CreateRemoteFile() = 0;
  /// Удалить большой бинарный объект.
  /// Метод может вызываться как в рамках транзакции, так и не в них.
  /// При вызове внутри транзакции действуют обычные правила транзакции:
  /// если транзакция не была завершена фиксацией или была завершена отменой, то
  /// файл не удалится.
  /// \param filename Идентификатор большого бинарного объекта, который
  ///                 требуется удалить.
  /// \return \c true, если удалось ли удалить файл, иначе \c false.
  virtual bool DeleteRemoteFile(const std::string &filename) = 0;
  /// Получить большой бинарный объект.
  /// \param filename Идентификатор большого бинарного объекта на сервере.
  /// \warning Последующая работа с большим бинарным объектом возможна только в
  ///          рамках транзакции.
  /// \return \c nullptr в случае, если передан идентификатор, который не
  ///         соответствует правилам именования, или если соединение
  ///         невалидно. В противном случае функция возвращает ненулевой
  ///         указатель. При этом полученный файл может не существовать - это
  ///         проверяется на этапе его открытия методом IFile::Open.
  virtual IFilePtr GetRemoteFile(const std::string &filename) = 0;

protected:
  /// \anchor IConnection_Execute_Binary
  /// Выполнить запрос с аргументами. Запрос не может содержать более одной
  /// SQL-команды.
  /// \todo На данный момент данная перегрузка не доступна для внешнего
  ///       пользователя интерфейса.
  /// \param singleCommand Строка в запросом. Аргументы в запросе обозначаются
  ///                      знаком доллара с последующим номером аргумента,
  ///                      начиная с единицы.
  ///                      Строка не может содержать более одной SQL-команды.
  ///                      Пример запроса:
  ///                      "INSERT INTO table VALUES($1, $2);".
  /// \param args Массив аргументов, каждый из которых может быть либо текстовой
  ///             строкой, либо массивом байт.
  /// \param resultFormat Формат, в котором будет представлен результат
  ///                     (бинарный или текстовый)
  /// \param types Массив типов аргументов.
  ///              Если передан пустой массив, то типы аргументов будут
  ///              вычислены автоматически. Иначе будет предпринята попытка
  ///              привести аргументы к требуемым типам. В качестве типа
  ///              аргумента может быть указан неизвестный тип
  ///              (SQLDataType::Unknown) - в этом случае тип этого аргумента
  ///              будет вычислен автоматически.
  /// \return Результат выполнения запроса.
  virtual IExecuteResultPtr
  Execute(const std::string &singleCommand,
          const std::vector<ExecuteArgType> &args,
          const ResultFormat resultFormat = ResultFormat::Text,
          const std::vector<SQLDataType> &types = {}) = 0;
};

/// Указатель на IConnection
using IConnectionPtr = std::shared_ptr<IConnection>;
