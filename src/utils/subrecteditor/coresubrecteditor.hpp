//= CoreSubRectEditor -> Written by Unusuario2, https://github.com/Unusuario2  =//
//
// Purpose: Use this .cpp file as a template for your custom util in the source engine.
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
#include <tier1/strtools.h>
#include <tier0/icommandline.h>
#include <tools_minidump.h>
#include <loadcmdline.h>
#include <cmdlib.h>
#include <filesystem_init.h>
#include <filesystem_tools.h>
#include <resourcecopy/cresourcecopy.hpp>
#include <KeyValues.h>
#include <utlbuffer.h>
#include <vector>


// Small unit test
	/*
	CCoreSubRectEditor* aa = new CCoreSubRectEditor("test");

	int pos = aa->AddChildRectangle();
	aa->SetMinKvValues(0, pos);
	aa->SetMaxKvValues(256, pos);

	pos = aa->AddChildRectangle();
	aa->SetMinKvValues(0, pos);
	aa->SetMaxKvValues(256, pos);
	aa->RemoveChildRectangle(pos);
	aa->WriteRectFile();
	*/


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
using KvContainer = std::vector<KeyValues*>;


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CCoreSubRectEditor
{
private:
	KeyValues*		m_pRootKv				= nullptr;
	KvContainer*	m_pChildKvRentangle		= nullptr; // yeah i am not using KeyValues[][] :)
	char			m_szFileName[MAX_PATH];


private:
	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	const int GetRectKvValues(const std::size_t uiPos, const char* pKeyName) const { m_pChildKvRentangle->at(uiPos)->GetInt(pKeyName); }
	void SetRectKvValues(const uint16_t uiValue, const std::size_t uiPos, const char* pKeyName) const { m_pChildKvRentangle->at(uiPos)->SetInt(pKeyName, uiValue); }


public:
	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	inline CCoreSubRectEditor::CCoreSubRectEditor(const char* pFileName)
	{
		V_sprintf_safe(m_szFileName, "%s.rect", pFileName);
		m_pRootKv = new KeyValues("Rectangles");
		m_pChildKvRentangle = new KvContainer();
	}


	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	inline CCoreSubRectEditor::~CCoreSubRectEditor()
	{
		for (int i = 0; i < m_pChildKvRentangle->size(); ++i)
			m_pChildKvRentangle->at(i)->deleteThis();
		delete m_pChildKvRentangle;
		delete[] m_szFileName;
		m_pRootKv->deleteThis();
	}


	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	inline std::size_t AddChildRectangle() const
	{
		KeyValues* pChildKv = new KeyValues("rectangle");
		pChildKv->CreateKey("min");
		pChildKv->CreateKey("max");
		
		// Set the default values.
		pChildKv->SetInt("min", 0);
		pChildKv->SetInt("max", 0);
		m_pChildKvRentangle->push_back(pChildKv);

		// Add the last child as a subkey
		m_pRootKv->AddSubKey(m_pChildKvRentangle->back());
		return static_cast<std::size_t>(m_pChildKvRentangle->size() - 1);
	}


	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	inline void RemoveChildRectangle(const std::size_t uiPos) { m_pRootKv->RemoveSubKey(m_pChildKvRentangle->at(uiPos)); m_pChildKvRentangle->at(uiPos)->deleteThis(); }


	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	inline const std::size_t SizeOfChildKvList() const { return m_pChildKvRentangle->size(); }
	inline const std::size_t EndOfChildKvList() const { return static_cast<std::size_t>(m_pChildKvRentangle->size() - 1); }


	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	inline const bool WriteRectFile() const { return m_pRootKv->SaveToFile(g_pFileSystem, m_szFileName, "MOD"); }


	//-----------------------------------------------------------------------------
	// Purpose: Seters...
	//-----------------------------------------------------------------------------
	inline void SetMinKvValues(const uint16_t uiValue, const std::size_t uiPos) const { this->SetRectKvValues(uiValue, uiPos, "min"); }
	inline void SetMaxKvValues(const uint16_t uiValue, const std::size_t uiPos) const { this->SetRectKvValues(uiValue, uiPos, "max"); }


	//-----------------------------------------------------------------------------
	// Purpose: Geters...
	//-----------------------------------------------------------------------------
	inline const int GetMinKvValues(const std::size_t uiPos) const { return this->GetRectKvValues(uiPos, "min"); }
	inline const int GetMaxKvValues(const std::size_t uiPos) const { return this->GetRectKvValues(uiPos, "max"); }
};