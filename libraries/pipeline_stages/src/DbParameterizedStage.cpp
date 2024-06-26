#include "DbParameterizedStage.h"

#include <IDatabaseManager.h>

DbParameterizedStage::DbParameterizedStage()
    : m_keys({"login", "password", "ip", "port", "db", "entityName",
              "attributeName"}),
      m_keysToObviousParamName(
          {{"login", L"Имя пользователя базы данных"},
           {"password", L"Пароль пользователя базы данных"},
           {"ip", L"IP-адрес базы данных"},
           {"port", L"Порт базы данных"},
           {"db", L"Название базы данных"},
           {"entityName", L"Название сохраняемой сущности"},
           {"attributeName", L"Название атрибута сущности"}}) {
  // todo : Сейчас заполняю здесь, чтобы каждый раз не вводить из GUI
  // Потом можно удалить
  m_keyValues["login"] = "postgres";
  m_keyValues["password"] = "password";
  m_keyValues["ip"] = "localhost";
  m_keyValues["port"] = "5432";
  m_keyValues["db"] = "testdb";
  m_keyValues["entityName"] = "results";
  m_keyValues["attributeName"] = "data";
}

std::vector<IParameterized::PatameterValue>
DbParameterizedStage::GetPatameterValues() const {
  std::vector<PatameterValue> res;
  for (auto&& key : m_keys) {
    res.push_back({key, m_keyValues[key]});
  }
  return res;
}

bool DbParameterizedStage::SetParameterValue(const std::string& paramName,
                                             const std::string& paramValue) {
  if (std::find(m_keys.begin(), m_keys.end(), paramName) == m_keys.end())
    // Такого ключа нет
    return false;

  m_keyValues[paramName] = paramValue;
  return true;
}

std::optional<std::wstring> DbParameterizedStage::GetObviousParamName(
    const std::string& paramName) const {
  if (auto iter = m_keysToObviousParamName.find(paramName);
      iter != m_keysToObviousParamName.end())
    return iter->second;
  return std::nullopt;
}

void DbParameterizedStage::ApplyParameterValues() noexcept(false) {
  if (m_isApliedParams) {
    m_isApliedParams = false;
    m_connection.reset();
    m_executorEAV.reset();
    m_file.reset();
  }

  auto connectionInfo = GetConnectionInfo();
  auto entityName = GetEntityName();
  auto attributeName = GetAttributeName();

  auto&& dbManager = GetDatabaseManager();
  m_connection = dbManager.GetConnection(connectionInfo);
  m_executorEAV = dbManager.GetExecutorEAV(m_connection);
  if (!m_connection || !m_connection->IsValid() || !m_executorEAV)
    throw std::runtime_error("Bad connectionInfo: \"" + connectionInfo + "\"");
  auto converter = dbManager.GetSQLTypeConverter();

  IExecutorEAV::EAVRegisterEntries entries({{entityName, {SQLDataType::RemoteFileId}}});
  if (auto status = m_executorEAV->SetRegisteredEntities(entries, true);
      status->HasError())
    throw std::runtime_error(status->GetErrorMessage());

  ResetFile();

  m_isApliedParams = true;
}

bool DbParameterizedStage::IsFullyParameterized() const {
  return m_isApliedParams;
}

std::wstring DbParameterizedStage::GetHelpString() const {
  /*
  {"login", L"Имя пользователя базы данных"},
           {"password", L"Пароль пользователя базы данных"},
           {"ip", L"IP-адрес базы данных"},
           {"port", L"Порт базы данных"},
           {"db", L"Название базы данных"},
           {"entityName", L"Название сохраняемой сущности"},
           {"attributeName", L"Название атрибута сущности"}}
  */
  return L"Имя пользователя базы данных, пароль пользователя базы данных, "
         L"IP-адрес базы данных, порт базы данных и название базы данных - "
         L"это стандартные параметры для подключения к базе данных, "
         L"которые пользователь вводит, например, чтобы подключиться к БД "
         L"с помощью утилиты psql.\n"
         L"Под сущностью понимается произвольный объект, который может иметь "
         L"атрибуты.\n"
         L"Например, сущность \"пользователь\" может иметь атрибуты \"имя\" и "
         L"\"адрес\".\n"
         L"Сущность \"результат\" может иметь атрибут \"данные\" "
         L"(результата).\n"
         L"Идентификатор сущности - это идентификатор конкретного объекта "
         L"(например, конкретного пользователя или конкретного результата)";
}

std::string DbParameterizedStage::GetConnectionInfo() const {
  return "postgresql://" + m_keyValues["login"] + ":" +
         m_keyValues["password"] + "@" + m_keyValues["ip"] + ":" +
         m_keyValues["port"] + "/" + m_keyValues["db"];
}

std::string DbParameterizedStage::GetEntityName() const {
  return m_keyValues["entityName"];
}

std::string DbParameterizedStage::GetAttributeName() const {
  return m_keyValues["attributeName"];
}
