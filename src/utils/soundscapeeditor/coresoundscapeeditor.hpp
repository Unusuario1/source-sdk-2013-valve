//= CoreSoundScriptEditor -> Written by Unusuario2, https://github.com/Unusuario2  =//
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

/*
Example of a SoundScript:

"swamp.water.slow"
{
	"dsp" "1"
	"dsp_spatial" "20"
	"dsp_volume"  "1.0"
	"fadetime"  "1.0"
	"soundmixer" "outside_swap_mixer"
	"playlooping"
	{
		"volume"	"0.98"
		"pitch"		"110"
		"soundlevel"	"SNDLVL_85dB"

		"position"	"0"

		"wave"	"ambient/swamps/water_Lap_loop_st.wav"
	}

	"playrandom"
	{
		"time"		"1,4"
		"volume"	"0.4,1"
		"pitch"		"90,105"
		"soundlevel"	"SNDLVL_85dB"
		"origin"	"3424.676025, 381.604095, 152.927948"

		"rndwave"
		{
			"wave"	"ambient/wind/wind_med1.wav"
			"wave"	"ambient/wind/wind_hit1.wav"
		}
	}
}

*/

//-----------------------------------------------------------------------------
// Purpose: Creates the Base KV root 
//-----------------------------------------------------------------------------
class CSoundScapeBase
{
public:
	KeyValues* KvRoot = nullptr;

public:
	CSoundScapeBase::CSoundScapeBase(const char* pSoundScapeName)
	{
		KeyValues* KvRoot = new KeyValues(pSoundScapeName);
	}

	CSoundScapeBase::~CSoundScapeBase()
	{
		KvRoot->deleteThis();
	}

	KeyValues* GetKvRoot()
	{
		return KvRoot;
	}
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSoundScapeBase : private CSoundScapeBase
{
public:
	KeyValues* m_KvRoot = nullptr;

	KeyValues* m_Dsp = nullptr;
	KeyValues* m_DspSpatial = nullptr;
	KeyValues* m_DspVolume = nullptr;
	KeyValues* m_FadeTime = nullptr;
	KeyValues* m_SoundMixer = nullptr;

public:
	inline CSoundScapeBase::CSoundScapeBase()
	{
		m_Dsp			= new KeyValues("");
		m_DspSpatial	= new KeyValues("");
		m_DspVolume		= new KeyValues("");
		m_FadeTime		= new KeyValues("");
		m_SoundMixer	= new KeyValues("");

		m_KvRoot->AddSubKey(m_Dsp);
		m_KvRoot->AddSubKey(m_DspSpatial);
		m_KvRoot->AddSubKey(m_DspVolume);
		m_KvRoot->AddSubKey(m_FadeTime);
		m_KvRoot->AddSubKey(m_SoundMixer);
	}

	inline CSoundScapeBase::~CSoundScapeBase()
	{
		m_Dsp->deleteThis();
		m_DspSpatial->deleteThis();
		m_DspVolume->deleteThis();
		m_FadeTime->deleteThis();
		m_SoundMixer->deleteThis();
	}

	// Seters...
	inline void SetValueDsp(const bool bSet)			{ m_Dsp->SetBool("dsp", bSet); }
	inline void SetValueDspSpatial(const int iSet)		{ m_DspSpatial->SetInt("dsp_spatial", iSet); }
	inline void SetValueDspVolume(const float flSet)	{ m_DspVolume->SetFloat("dsp_volume", flSet); }
	inline void SetValueFadeTime(const float flSet)		{ m_FadeTime->SetFloat("fadetime", flSet); }
	inline void SetValueSoundMixer(const char* pString) { m_SoundMixer->SetString("soundmixer", pString); }

	// Geters...
	inline const bool GetValueDsp(const bool bSet) const			{ return m_Dsp->GetBool(); }
	inline const int GetValueDspSpatial(const int iSet) const		{ return m_DspSpatial->GetInt(); }
	inline const float GetValueDspVolume(const float flSet) const	{ return m_DspVolume->GetFloat(); }
	inline const float GetValueFadeTime(const float flSet) const	{ return m_FadeTime->GetFloat(); }
	inline const char* GetValueSoundMixer(const char* pString) const { return m_SoundMixer->GetString(); }
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CSoundScapeRule : private CSoundScapeBase
{
	
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CSoundScapeWritter final : public CSoundScapeBase
{
private:


public:


};