#pragma once

#include <IFile.h>
#include <PGConnection.h>

#include <libpq-fe.h>


//------------------------------------------------------------------------------
/**
  ����� ��� �������������� � ������� �������� �������� �� ������� PostgreSQL
*/
//---
class PGRemoteFile : public IFile
{
	std::weak_ptr<PGConnection> m_connection; ///< ����������
	Oid m_objId; ///< ������������� �������� ��������� �������
	std::optional<int> m_fd; ///< ���������� ��������� �������� ��������� �������

public:
	/// �����������
	PGRemoteFile(std::weak_ptr<PGConnection> connection, Oid objId);
	/// ����������
	virtual ~PGRemoteFile() override;

	/// ����������� �����������
	PGRemoteFile(const PGRemoteFile &) = delete;
	/// ����������� �����������
	PGRemoteFile(PGRemoteFile &) = delete;
	/// �������� ������������ ������������
	PGRemoteFile& operator=(const PGRemoteFile &) = delete;
	/// �������� ������������ ������������
	PGRemoteFile& operator=(PGRemoteFile &&) = delete;

public:
	/// �������� ��� ����� (��� �������� ��������� ������� ��� - ��� �������������)
	virtual std::string GetFileName() const override;
	/// ������� ����
	virtual bool Open(const std::vector<FileOpenMode> & openModes) override;
	/// ������� ����
	virtual bool Close() override;
	/// ���������� �������� �����
	virtual std::optional<std::vector<char>> ReadBytes(size_t count) override;
	/// ���������� �������� �����
	virtual bool WriteBytes(const std::vector<char> & data) override;
};
