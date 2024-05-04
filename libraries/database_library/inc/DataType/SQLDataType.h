#pragma once

//------------------------------------------------------------------------------
/**
	��� ������ � ���� ������
*/
//---
enum class SQLDataType : int
{
	Integer, ///< ����� �����
	Text, ///< ����� ������������ �����
	ByteArray, ///< ������ ���� ������������ �����

	// ^ ����� ���� ��������� ���� ^
	Unknown, ///< ����������� ��� ������ (���� ��������� ��� ������������ ��������� ����� ������)
	Invalid, ///< ���������� ��� ������ (������ ���� ������ �� ����������)
	Count ///< ���������� ��������� � ������������
};