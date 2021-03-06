/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012-2014 Icinga Development Team (http://www.icinga.org)    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#include "base/string.hpp"
#include "base/value.hpp"
#include "base/primitivetype.hpp"
#include <boost/algorithm/string/trim.hpp>
#include <ostream>

using namespace icinga;

REGISTER_PRIMITIVE_TYPE(String);

const String::SizeType String::NPos = std::string::npos;

String::String(void)
	: m_Data()
{ }

String::String(const char *data)
	: m_Data(data)
{ }

String::String(const std::string& data)
	: m_Data(data)
{ }

String::String(String::SizeType n, char c)
	: m_Data(n, c)
{ }

String::String(const String& other)
	: m_Data(other.m_Data)
{ }

String& String::operator=(const String& rhs)
{
	m_Data = rhs.m_Data;
	return *this;
}

String& String::operator=(const std::string& rhs)
{
	m_Data = rhs;
	return *this;
}

String& String::operator=(const char *rhs)
{
	m_Data = rhs;
	return *this;
}

const char& String::operator[](String::SizeType pos) const
{
	return m_Data[pos];
}

char& String::operator[](String::SizeType pos)
{
	return m_Data[pos];
}

String& String::operator+=(const String& rhs)
{
	m_Data += rhs.m_Data;
	return *this;
}

String& String::operator+=(const char *rhs)
{
	m_Data += rhs;
	return *this;
}

String& String::operator+=(const Value& rhs)
{
	m_Data += static_cast<String>(rhs);
	return *this;
}

String& String::operator+=(char rhs)
{
	m_Data += rhs;
	return *this;
}

bool String::IsEmpty(void) const
{
	return m_Data.empty();
}

bool String::operator<(const String& rhs) const
{
	return m_Data < rhs.m_Data;
}

String::operator const std::string&(void) const
{
	return m_Data;
}

const char *String::CStr(void) const
{
	return m_Data.c_str();
}

void String::Clear(void)
{
	m_Data.clear();
}

String::SizeType String::GetLength(void) const
{
	return m_Data.size();
}

std::string& String::GetData(void)
{
	return m_Data;
}

const std::string& String::GetData(void) const
{
	return m_Data;
}

String::SizeType String::Find(const String& str, String::SizeType pos) const
{
	return m_Data.find(str, pos);
}

String::SizeType String::RFind(const String& str, String::SizeType pos) const
{
	return m_Data.rfind(str, pos);
}

String::SizeType String::FindFirstOf(const char *s, String::SizeType pos) const
{
	return m_Data.find_first_of(s, pos);
}

String::SizeType String::FindFirstOf(char ch, String::SizeType pos) const
{
	return m_Data.find_first_of(ch, pos);
}

String::SizeType String::FindFirstNotOf(const char *s, String::SizeType pos) const
{
	return m_Data.find_first_not_of(s, pos);
}

String::SizeType String::FindFirstNotOf(char ch, String::SizeType pos) const
{
	return m_Data.find_first_not_of(ch, pos);
}

String String::SubStr(String::SizeType first, String::SizeType len) const
{
	return m_Data.substr(first, len);
}

void String::Replace(String::SizeType first, String::SizeType second, const String& str)
{
	m_Data.replace(first, second, str);
}

void String::Trim(void)
{
	boost::algorithm::trim(m_Data);
}

bool String::Contains(const String& str) const
{
	return (m_Data.find(str) != std::string::npos);
}

void String::swap(String& str)
{
	m_Data.swap(str.m_Data);
}

String::Iterator String::erase(String::Iterator first, String::Iterator last)
{
	return m_Data.erase(first, last);
}

String::Iterator String::Begin(void)
{
	return m_Data.begin();
}

String::ConstIterator String::Begin(void) const
{
	return m_Data.begin();
}

String::Iterator String::End(void)
{
	return m_Data.end();
}

String::ConstIterator String::End(void) const
{
	return m_Data.end();
}

std::ostream& icinga::operator<<(std::ostream& stream, const String& str)
{
	stream << static_cast<std::string>(str);
	return stream;
}

std::istream& icinga::operator>>(std::istream& stream, String& str)
{
	std::string tstr;
	stream >> tstr;
	str = tstr;
	return stream;
}

String icinga::operator+(const String& lhs, const String& rhs)
{
	return static_cast<std::string>(lhs) + static_cast<std::string>(rhs);
}

String icinga::operator+(const String& lhs, const char *rhs)
{
	return static_cast<std::string>(lhs) + rhs;
}

String icinga::operator+(const char *lhs, const String& rhs)
{
	return lhs + static_cast<std::string>(rhs);
}

bool icinga::operator==(const String& lhs, const String& rhs)
{
	return static_cast<std::string>(lhs) == static_cast<std::string>(rhs);
}

bool icinga::operator==(const String& lhs, const char *rhs)
{
	return static_cast<std::string>(lhs) == rhs;
}

bool icinga::operator==(const char *lhs, const String& rhs)
{
	return lhs == static_cast<std::string>(rhs);
}

bool icinga::operator<(const String& lhs, const char *rhs)
{
	return static_cast<std::string>(lhs) < rhs;
}

bool icinga::operator<(const char *lhs, const String& rhs)
{
	return lhs < static_cast<std::string>(rhs);
}

bool icinga::operator>(const String& lhs, const String& rhs)
{
	return static_cast<std::string>(lhs) > static_cast<std::string>(rhs);
}

bool icinga::operator>(const String& lhs, const char *rhs)
{
	return static_cast<std::string>(lhs) > rhs;
}

bool icinga::operator>(const char *lhs, const String& rhs)
{
	return lhs > static_cast<std::string>(rhs);
}

bool icinga::operator<=(const String& lhs, const String& rhs)
{
	return static_cast<std::string>(lhs) <= static_cast<std::string>(rhs);
}

bool icinga::operator<=(const String& lhs, const char *rhs)
{
	return static_cast<std::string>(lhs) <= rhs;
}

bool icinga::operator<=(const char *lhs, const String& rhs)
{
	return lhs <= static_cast<std::string>(rhs);
}

bool icinga::operator>=(const String& lhs, const String& rhs)
{
	return static_cast<std::string>(lhs) >= static_cast<std::string>(rhs);
}

bool icinga::operator>=(const String& lhs, const char *rhs)
{
	return static_cast<std::string>(lhs) >= rhs;
}

bool icinga::operator>=(const char *lhs, const String& rhs)
{
	return lhs >= static_cast<std::string>(rhs);
}

bool icinga::operator!=(const String& lhs, const String& rhs)
{
	return static_cast<std::string>(lhs) != static_cast<std::string>(rhs);
}

bool icinga::operator!=(const String& lhs, const char *rhs)
{
	return static_cast<std::string>(lhs) != rhs;
}

bool icinga::operator!=(const char *lhs, const String& rhs)
{
	return lhs != static_cast<std::string>(rhs);
}

String::Iterator icinga::range_begin(String& x)
{
	return x.Begin();
}

String::ConstIterator icinga::range_begin(const String& x)
{
	return x.Begin();
}

String::Iterator icinga::range_end(String& x)
{
	return x.End();
}

String::ConstIterator icinga::range_end(const String& x)
{
	return x.End();
}
