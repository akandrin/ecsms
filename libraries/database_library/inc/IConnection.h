#pragma once

#include <IExecuteResult.h>
#include <IFile.h>
#include <DataType/SQLDataType.h>

#include <memory>
#include <variant>

//------------------------------------------------------------------------------
/**
  ������ ����������
*/
//---
enum class ConnectionStatus
{
	Ok,     ///< ���������� ������� �����������
	Bad,    ///< ������ ��������� ����������
	Unknown ///< ����������� ������
};

/// ��� ��������� ��� ���������� ������ Execute
using ExecuteArgType = std::variant<std::string, std::vector<char>>;

//------------------------------------------------------------------------------
/**
  ��������� ����������.
  ��� ������������� ������ � �� ������������� ��� ������� ������ ���������
  ���� ������ ���������� (� ��������� ������ ������ ����� ������������� � �������).
*/
//---
class IConnection
{
public:
	/// ����������
	virtual ~IConnection() = default;

public:
	/// ������� �� ����������
	virtual bool IsValid() const = 0;
	/// �������� ������ ����������
	virtual ConnectionStatus GetStatus() const = 0;

	/// ��������� ������. ������ ����� ��������� ��������� SQL ������, ����� ��� ����� ��������� � ������ ����� ����������
	/// (������ ���� ������� BEGIN/COMMIT �� �������� ���� � ������, ����� ��������� ��� �� ��������� ����������)
	virtual IExecuteResultPtr Execute(const std::string & query) = 0;

	/// ������� ����������
	virtual IExecuteResultStatusPtr BeginTransaction() = 0;
	/// ������� ���������� � ����������� ���������
	virtual IExecuteResultStatusPtr CommitTransaction() = 0;
	/// �������� ���������� (��� ���������� ���������)
	virtual IExecuteResultStatusPtr RollbackTransaction() = 0;

public:
	/// ������� ��������� ����.
	/// ���������������: ����������� ������ � ��������� ������ �������� ������ � ������ ����������.
	/// ������������� ����� � �������� ����� ����������� � ����� ����������.
	virtual IFilePtr CreateRemoteFile() = 0;

protected:
	// todo: IConnection::Execute ���������� � ��������� �������
	// ������ ��� �� ������ ������ �� ������������ � ������� �� ������������,
	// ��� ��� �������� ��� ���� � protected-������.

	/// ��������� ������ � �����������. ������ �� ����� ��������� ����� ����� SQL-�������.
	/// \param query ������ � ��������. ��������� � ������� ������������ ������ ������� � ����������� ������� ���������,
	///              ������� � �������. ������ �������: "INSERT INTO table VALUES($1, $2);".
	/// \param args ������ ����������, ������ �� ������� ����� ���� ���� ��������� �������, ���� �������� ����.
	/// \param resultFormat ������, � ������� ����� ����������� ��������� (�������� ��� ���������)
	/// \param types ������ ����� ����������.
	///              ���� ������� ������ ������, �� ���� ���������� ����� ��������� �������������.
	///              ����� ����� ����������� ������� �������� ��������� � ��������� �����.
	///              � �������� ���� ��������� ����� ���� ������ ����������� ��� (SQLDataType::Unknown) - � ����
	///              ������ ��� ����� ��������� ����� �������� �������������.
	virtual IExecuteResultPtr Execute(const std::string & singleCommand,
		const std::vector<ExecuteArgType> & args,
		const ResultFormat resultFormat = ResultFormat::Text,
		const std::vector<SQLDataType> & types = {}) = 0;
};

/// ��������� �� IConnection
using IConnectionPtr = std::shared_ptr<IConnection>;
