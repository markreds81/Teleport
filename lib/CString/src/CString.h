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

#ifndef _CString_h_
#define _CString_h_

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include "pins_arduino.h"
#endif
#include "Print.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CString_LIBRARY_VERSION 3

class CString : public Print {
	private:
  		char *m_buf;
		char *m_cur;
		size_t m_size;
	public:
		#if defined(ARDUINO) && ARDUINO >= 100
		virtual size_t write(uint8_t);
		#else
		virtual void write(uint8_t);
	#endif

	CString(char *buf);
	CString(char *buf, size_t size);
	
	virtual ~CString();
	
	template<class T> CString(char *buf, size_t size, T arg);  
	template<class T> CString(char *buf, size_t size, T arg, int modifier);

	template<class T> inline CString & operator = (T arg) {
		begin();
		print(arg);
		return *this;
	}
	
	template<class T> inline CString & operator += (T arg) {
		print(arg);
		return *this;
	}
	
	inline operator const char *(void) { return m_buf; }

	bool operator == (const char *str);

	inline const size_t length() { return m_cur - m_buf; }
	inline const size_t capacity() { return m_size; };

	void begin();
	int format(char const *str, ...);
	char* c_str(void);
	bool contains(const char *str);
	bool startsWith(const char *str);
	bool endsWith(const char *str);
};

#endif