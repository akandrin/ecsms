#pragma once

#include <IExecuteResult.h>
#include <IConnection.h>
#include <DataType/ISQLType.h>
#include <DataType/ISQLTypeText.h>

#include <memory>
#include <vector>
#include <map>

class IExecutorEAVNamingRules;

//------------------------------------------------------------------------------
/**
  ��������� ����������� �������� EAV.
  �� ������������� ������� �������, ������� ����� ��������� ��������� ��� ������
  � ����� ������, �������������� ������ EAV.

  ������� ���������� ������ ��� ���� ������ ������������ ����������� IExecutorEAVNamingRules.

  ����� SetRegisteredEntities ����� ������� ������� �� ��������� ��������, ���� �������� � ����
  ���� createTables = true.
  ��������� ������ �� ������� �������, � ���������� �������������, ���������� ���� �������.
  ��� ���������� ����� ������ ������� ����� �������� ������.

  ��� ������ ������ �� ��������� � �� ��������� ����������.
  ����� �������, ����� � ���������� ���� ������� ����������, ������� ���� ��� ��������� �������
  ������, ����� ���� ������������� (��� ��������) ����������, �������� (��� �� ��������) �����
  ��������� ���������.

  (���� ���������� ��� �� ��������� ����������, �� ������ ����� ������ ����� �������� � ������
  ����� ��� ���������� ���������� (������� �� ���������� ����������� ������))
*/
//---
class IExecutorEAV
{
public:
	/// ��� ��� �������� �������� (����������� �������� ��������).
	using EntityName = std::string;
	/// ��� ��� �������������� �������� (�������� ������������� ���������� �������� � � ������� ���������).
	using EntityId = int;
	/// ��� ��� �������� ��������.
	using AttrName = ISQLTypeTextPtr;
	/// ��� �������� ��������
	using ValueType = ISQLTypePtr;

	/// ��������� ��� �������� ����: ������� � ��� ��������
	struct AttrValue
	{
		AttrName attrName; ///< �������� ��������
		ValueType value; ///< �������� ��������
	};

	/// ������ � ������� EAV
	using EAVRegisterEntries = std::map<
		EntityName, // �������� ��������
		std::vector<SQLDataType>>; // ���� ���������, ������� ��� ����� ������������

public:
	/// ����������
	virtual ~IExecutorEAV() = default;

public:
	/// ����������� EAV-��������� ��� ��������� ������ � �������
	/// \param entries ��������� � �������������� "�������� ��������" - "���� ���������, ������� ��� ����������"
	/// \param createTables ��������� �� �������� ������� ������� �� ������������������ ���������
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr SetRegisteredEntities(const EAVRegisterEntries & entries,
		bool createTables) = 0;
	/// �������� ������������������ ��������
	/// \return ��������� � �������������� "�������� ��������" - "���� ���������, ������� ��� ����������"
	virtual const EAVRegisterEntries & GetRegisteredEntities() const = 0;

public:
	/// �������� ������, ������������ ������� ���������� ������
	/// \return ������, ������������ ������� ���������� ������
	virtual const IExecutorEAVNamingRules & GetNamingRules() const = 0;

public: // ������ ��� �������� ����� ��������� � ������ ��� ������������ ���������
	/// ������� ����� �������� ������� ����
	/// \param entityName �������� ��������
	/// \param[out] result ������������� ����� ��������
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr CreateNewEntity(const EntityName & entityName, EntityId & result) = 0;
	/// �������� ��� �������������� �������� ������� ����
	/// \param entityName �������� ��������
	/// \param[out] result ������ ��������������� ��������.
	///                    � ������ ������ ������ ����������� ����� ������� ����������.
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr GetEntityIds(const EntityName & entityName,
		std::vector<EntityId> & result) = 0;
	/// �������� ��� ������������ ��������� ���������� ����, ������� ���������� ������ ��������
	/// \param entityName �������� ��������
	/// \param sqlDataType ��� ��������
	/// \param result[out] ������ ������������ ���������.
	///                    � ������ ������ ������ ����������� ����� ������� ����������.
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr GetAttributeNames(const EntityName & entityName,
		SQLDataType sqlDataType, std::vector<AttrName> & result) = 0;
	/// ����� ��������, � ������� ���� ��� �� ��������� ��� �������-��������
	/// \param entityName �������� ��������
	/// \param attrValues ������ ��� �������-��������.
	///                   � �������� �������� ����� ���� �������� ������ ����������.
	///                   ����� ���������� ����� ����� ��������� ��������, � ������� �������� �� ������.
	/// \param[out] result ������ ��������������� ��������, � ������� ���� ��� �� ��������� ��� �������-��������
	///                     ������ ����������� ����� ������� ����������.
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr FindEntitiesByAttrValues(const EntityName & entityName,
		const std::vector<AttrValue> & attrValues, std::vector<EntityId> & result) = 0;

public: // ������ ��� �������/���������� ������
	/// �������� �������� ��� �������� ��������
	/// \param entityName �������� ��������
	/// \param entityId ������������ ��������
	/// \param attrName ������������ ��������
	/// \param value �������� ��������. ���� �������� �������� ������,
	///              �� �������� �� ����� ��������� � ������� ��������.
	///              (����� EAV � ���, ��� �� �� ������ null)
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr Insert(const EntityName & entityName, EntityId entityId,
		const AttrName & attrName, const ValueType & value) = 0;
	/// �������� �������� ��� �������� ��������
	/// \param entityName �������� ��������
	/// \param entityId ������������ ��������
	/// \param attrName ������������ ��������
	/// \param value �������� ��������. ���� �������� �������� ������, �� �������� ��� �������� ��������
	///              ����� ������� �� ������� ��������.
	///              (����� EAV � ���, ��� �� �� ������ null)
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr Update(const EntityName & entityName, EntityId entityId,
		const AttrName & attrName, const ValueType & value) = 0;
	/// �������� �������� ��� �������� �������� ��� ��������, ���� ������ �������� ��� �� ����
	/// \param entityName �������� ��������
	/// \param entityId ������������ ��������
	/// \param attrName ������������ ��������
	/// \param value �������� ��������. ���� �������� �������� ������,
	///              �� �������� �� ����� ��������� � ������� �������� (���� ��� �� ����),
	///              ��� ����� ������� �� ������� �������� (���� ��� ����).
	///              (����� EAV � ���, ��� �� �� ������ null)
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr InsertOrUpdate(const EntityName & entityName, EntityId entityId,
		const AttrName & attrName, const ValueType & value) = 0;

public: // ������ ��� ��������� ������
	/// �������� �������� �������� ��������.
	/// ���� � ������ �������� � ������� ��������� ���� ������� ������� ���� � ������ ���������,
	/// ������ � ������� �������� ��� ������� �������������� �������� � ������� ��������
	/// ��� ��������, �� ����� �� ������ ������, � ������ ������ ����������.
	/// \param entityName �������� ��������
	/// \param entityId ������������ ��������
	/// \param attrName ������������ ��������
	/// \param[in,out] value ��������� ������ ����������, �� ���� ������� ����� ��������� ��� ��������,
	///                      � � ������� ��������� ���������. ���� ����� �������� �������� ����������,
	///                      �� �������� ������.
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr GetValue(const EntityName & entityName, EntityId entityId,
		const AttrName & attrName, ValueType value) = 0;
	/// �������� �������� ���� ��������� ��������.
	/// ��� ����������� ����� ��������� ����� ���������� GetRegisteredEntities.
	/// ����� �������, ����� �� ������ ���������� � �������������������� ����� ���������, ���� ��� ����.
	/// \param entityName �������� ��������
	/// \param entityId ������������� ��������
	/// \param[out] attrValues ������ ��� �������-��������, ��������������� � ������ ���������.
	///                        ����� ���������� ����� �� ���� �������-��������, � ������� �������� �� ������:
	///                        ����� �������� ����� ������������ ������ SQL-����������.
	/// \return ������ ���������� ��������
	virtual IExecuteResultStatusPtr GetAttributeValues(const EntityName & entityName,
		EntityId entityId, std::vector<AttrValue> & attrValues) = 0;
};

/// ��� ��������� �� ����������� EAV-��������
using IExecutorEAVPtr = std::shared_ptr<IExecutorEAV>;
