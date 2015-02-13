/**
 * @file ShaderSource.cpp
 *
 * OpenGL Template
 * Copyright 2015 Eetu 'Devenec' Oinasmaa
 *
 * OpenGL Template is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <ShaderSource.h>

using namespace std;

// Public

// Static

bool ShaderSource::load(const string& filepath, string& source)
{
	ifstream fileStream(filepath, ios::binary);

	if(fileStream.good())
	{
		fileStream.seekg(0u, ios::end);
		source.reserve(static_cast<unsigned int>(fileStream.tellg()));
		fileStream.seekg(0u, ios::beg);
		source.assign(istreambuf_iterator<char>(fileStream), istreambuf_iterator<char>());

		if(fileStream.good())
			return true;
	}

	return false;
}
