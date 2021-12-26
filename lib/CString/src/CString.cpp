/************************************************************************************
 * 
 * Name    : mrCString
 * File    : CString.h
 * Author  : Mark Reds <marco@markreds.it>
 * Date    : November 13, 2020
 * Version : 1.0.0
 * Notes   : Lightweight printable string class utility for Arduino.
 *           Based on Mikal Hart's PString.
 *
 * Copyright (C) 2020 Marco Rossi (aka Mark Reds).  All right reserved.
 * 
 * This file is part of mrCString.
 * 
 * mrCString is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * mrCString is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with mrCString. If not, see <http://www.gnu.org/licenses/>.
 * 
 ************************************************************************************/

#include "CString.h"

CString::CString(char *buf) :
	m_buf(buf),
	m_size(strlen(buf))
{
	m_cur = m_buf + m_size;
}

CString::CString(char *buf, size_t size) :
	m_buf(buf),
	m_size(size)
{
	begin();
}

CString::~CString() { }

template<class T> CString::CString(char *buf, size_t size, T arg) :
	m_buf(buf),
	m_size(size)
{
	begin();
	print(arg);
}

template<class T> CString::CString(char *buf, size_t size, T arg, int modifier) :
	m_buf(buf),
	m_size(size)
{
	begin();
	print(arg, modifier);
}

bool CString::operator == (const char *str) {
	return m_size > 0 && !strcmp(m_buf, str);
}

void CString::begin() {
	m_cur = m_buf;
	if (m_size > 0) {
		m_buf[0] = '\0';
	}
}

#if defined(ARDUINO) && ARDUINO >= 100
size_t CString::write(uint8_t b) {
#else
void CString::write(uint8_t b) {
#endif
	if (m_cur + 1 < m_buf + m_size) {
		*m_cur++ = (char)b;
		*m_cur = '\0';
		#if defined(ARDUINO) && ARDUINO >= 100
		return 1;
		#endif
	}

	#if defined(ARDUINO) && ARDUINO >= 100
	return 0;
	#endif
}

int CString::format(char const *str, ...) {
	va_list argptr;
	va_start(argptr, str);
	int ret = vsnprintf(m_cur, m_size - (m_cur - m_buf), str, argptr);
	if (m_size) {
		while (*m_cur) {
			++m_cur;
		}
	}
	return ret;
}

char* CString::c_str(void) {
	return m_buf;
}

bool CString::contains(const char *str) {
	if (!m_buf || !str) {
		return false;
	}
	return (strstr(m_buf, str) != NULL);
}

bool CString::startsWith(const char *str) {
	if (!m_buf || !str) {
		return false;
	}
	return (strncmp(m_buf, str, strlen(str)) == 0);
}

bool CString::endsWith(const char *str) {
	if (!m_buf || !str) {
		return false;
	}
	size_t len = strlen(str);
	if (len > m_size) {
		return false;
	}
	return (strncmp(m_buf + m_size - len, str, len) == 0);
}
