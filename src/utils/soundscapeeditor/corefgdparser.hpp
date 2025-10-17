//=== CoreFGDPaser -> Writen by Unusuario2, https://github.com/Unusuario2  ===//
//
// Purpose:
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
#ifndef COREFGDPARSER_HPP
#define COREFGDPARSER_HPP

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
#include <fgdlib/fgdlib.h>
#include <vector>


/*
KeyValues* kv = m_pRoot->FindKey("dsp");

switch (kv->GetDataType())
{
case KeyValues::TYPE_STRING:
    Msg("String value: %s\n", kv->GetString());
    break;
case KeyValues::TYPE_STRING:
    Msg("Int value: %d\n", kv->GetInt());
    break;
case KeyValues::TYPE_STRING:
    Msg("Float value: %f\n", kv->GetFloat());
    break;
default:
    Msg("Unknown or uninitialized type\n");
}
*/


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSoundScapeFgdParser
{
private:
    std::vector<KeyValues>* m_pSoundScapeBase       = nullptr;
    std::vector<KeyValues>* m_pSoundScapeRules      = nullptr;
    std::vector<KeyValues>* m_pSoundScapeCommonKv   = nullptr;
    const char*             m_szFgdPath             = nullptr;
	enum FgdClassType
    {
        SoundScapeBaseClass = 0,
        SoundScapeCommonKVClass = 0,
        SoundScapeRuleClass = 0,
    };


private:
    //-----------------------------------------------------------------------------
    // Purpose: 
    //-----------------------------------------------------------------------------
    inline bool ParseSoundScapeGeneric(const char* pFgdPath, FgdClassType ClassType)
    {
    
    }


    //-----------------------------------------------------------------------------
    // Purpose: 
    //-----------------------------------------------------------------------------
    inline const bool ParseSoundScapeBaseValues() { return ParseSoundScapeGeneric(m_szFgdPath, FgdClassType::SoundScapeBaseClass); }
    inline const bool ParseSoundScapeCommonValues() { return ParseSoundScapeGeneric(m_szFgdPath, FgdClassType::SoundScapeCommonKVClass); }
    inline const bool ParseSoundScapeRuleValues() { return ParseSoundScapeGeneric(m_szFgdPath, FgdClassType::SoundScapeRuleClass); }


protected:
    //-----------------------------------------------------------------------------
    // Purpose: 
    //-----------------------------------------------------------------------------
    inline CSoundScapeFgdParser::CSoundScapeFgdParser(const char* pFgdPath) 
    { 
        m_pSoundScapeBase       = new std::vector<KeyValues>; 
        m_pSoundScapeRules      = new std::vector<KeyValues>;
        m_pSoundScapeCommonKv   = new std::vector<KeyValues>;
        m_szFgdPath             = V_strdup(pFgdPath); 

        if(!ParseSoundScapeBaseValues() || !ParseSoundScapeCommonValues() || !ParseSoundScapeRuleValues())
        {
            // TODO!!!
        }
    }


    //-----------------------------------------------------------------------------
    // Purpose: 
    //-----------------------------------------------------------------------------
    inline CSoundScapeFgdParser::~CSoundScapeFgdParser() 
    { 
        delete m_pSoundScapeBase;
        delete m_pSoundScapeRules;
        delete m_pSoundScapeCommonKv;
        delete m_szFgdPath; 
    }
    
public:
    //-----------------------------------------------------------------------------
    // Purpose: 
    //-----------------------------------------------------------------------------
    inline std::vector<KeyValues>* GetSoundScapeBaseValues() { return m_pSoundScapeBase; }
    inline std::vector<KeyValues>* GetSoundScapeCommonValues() { return m_pSoundScapeCommonKv; }
    inline std::vector<KeyValues>* GetSoundScapeRulesValues() { return m_pSoundScapeRules; }
};


#endif // COREFGDPARSER_HPP

