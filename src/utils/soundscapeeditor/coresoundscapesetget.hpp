//== CoreSoundScapeSetGet -> Written by Unusuario2, https://github.com/Unusuario2  =//
//
// Purpose: General Getters and Setters...
//
// License:
//        MIT License
//
//        Copyright (c) 2025 [un usuario], https://github.com/Unusuario2
//
//        Permission is hereby granted, free of charge, to any person obtaining a copy
//        of this software and associated documentation files (the "Software"), to deal
//        in the Software without restriction, including without limitation the rights
//        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//        copies of the Software, and to permit persons to whom the Software is
//        furnished to do so, subject to the following conditions:
//
//        The above copyright notice and this permission notice shall be included in all
//        copies or substantial portions of the Software.
//
//        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//        SOFTWARE.
//
// $NoKeywords: $
//==============================================================================//
#ifndef CORESOUNDSCAPESETGET_HPP
#define CORESOUNDSCAPESETGET_HPP

#ifdef _WIN32
#pragma once
#endif // _WIN32

#include <tier1/strtools.h>
#include <tier0/icommandline.h>
#include <tools_minidump.h>
#include <loadcmdline.h>
#include <cmdlib.h>
#include <filesystem_init.h>
#include <filesystem_tools.h>
#include <resourcecopy/cresourcecopy.hpp>
#include <KeyValues.h>
#include "coresoundscaperootkv.hpp"
#include "corefgdparser.hpp"


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSoundScapeGetSetKeyValues
{
private:
	std::vector<KeyValues>* m_pObjFgd = nullptr;

protected:
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	inline CSoundScapeGetSetKeyValues::CSoundScapeGetSetKeyValues(CSoundScapeFgdParser* ObjFgd)
	{
		m_pObjFgd = ObjFgd->GetSoundScapeBaseValues();
	}


	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	inline CSoundScapeGetSetKeyValues::~CSoundScapeGetSetKeyValues()
	{
	}


	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	inline virtual std::size_t GetKeyValueContainerCount() 
	{
		return m_pObjFgd->size();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Seters
	//-----------------------------------------------------------------------------
	inline virtual void SetKeyValueValue(KeyValues::types_t TypeKv, void* pData, std::size_t uiPosition)
	{
		switch (TypeKv)
		{
		case KeyValues::TYPE_NONE:
			break;
		case KeyValues::TYPE_STRING:
			m_pObjFgd->data()[uiPosition].SetString(m_pObjFgd->data()[uiPosition].GetName(), static_cast<const char*>(pData));
			break;
		case KeyValues::TYPE_INT:
			m_pObjFgd->data()[uiPosition].SetInt(m_pObjFgd->data()[uiPosition].GetName(), *static_cast<int*>(pData));
			break;
		case KeyValues::TYPE_FLOAT:
			m_pObjFgd->data()[uiPosition].SetFloat(m_pObjFgd->data()[uiPosition].GetName(), *static_cast<float*>(pData));
			break;
		case KeyValues::TYPE_PTR:
			m_pObjFgd->data()[uiPosition].SetPtr(m_pObjFgd->data()[uiPosition].GetName(), pData);
			break;
		case KeyValues::TYPE_WSTRING:
			m_pObjFgd->data()[uiPosition].SetWString(m_pObjFgd->data()[uiPosition].GetName(), static_cast<const wchar_t*>(pData));
			break;
		case KeyValues::TYPE_COLOR:
			m_pObjFgd->data()[uiPosition].SetColor(m_pObjFgd->data()[uiPosition].GetName(), *static_cast<Color*>(pData));
			break;
		case KeyValues::TYPE_UINT64:
			m_pObjFgd->data()[uiPosition].SetUint64(m_pObjFgd->data()[uiPosition].GetName(), *static_cast<uint64_t*>(pData));
			break;
		case KeyValues::TYPE_NUMTYPES:
			break;
		default:
			break;
		}
	}


	//-----------------------------------------------------------------------------
	// Purpose: Geters
	//-----------------------------------------------------------------------------
	inline virtual void* GetKeyValueValue(KeyValues::types_t TypeKv, void* pData, std::size_t uiPosition)
	{
		switch (TypeKv)
		{
		case KeyValues::TYPE_NONE:
			return nullptr;
		case KeyValues::TYPE_STRING:
			return (void*)(m_pObjFgd->data()[uiPosition].GetString(m_pObjFgd->data()[uiPosition].GetName()));
			break;
		case KeyValues::TYPE_INT:
			return (void*)m_pObjFgd->data()[uiPosition].GetInt(m_pObjFgd->data()[uiPosition].GetName());
			break;
		case KeyValues::TYPE_FLOAT:
			return (void*)m_pObjFgd->data()[uiPosition].GetFloat(m_pObjFgd->data()[uiPosition].GetName());
			break;
		case KeyValues::TYPE_PTR:
			return (void*)m_pObjFgd->data()[uiPosition].GetPtr(m_pObjFgd->data()[uiPosition].GetName(), pData);
			break;
		case KeyValues::TYPE_WSTRING:
			return (void*)m_pObjFgd->data()[uiPosition].GetWString(m_pObjFgd->data()[uiPosition].GetName());
			break;
		case KeyValues::TYPE_COLOR:
			return (void*)&m_pObjFgd->data()[uiPosition].GetColor(m_pObjFgd->data()[uiPosition].GetName());
			break;
		case KeyValues::TYPE_UINT64:
			return (void*)m_pObjFgd->data()[uiPosition].GetUint64(m_pObjFgd->data()[uiPosition].GetName());
			break;
		case KeyValues::TYPE_NUMTYPES:
			return nullptr;
		default:
			return nullptr;
		}
	}
};


#endif // CORESOUNDSCAPESETGET_HPP

