//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=====================================================================================//
#include <stdlib.h>
#include <stdio.h>
#include "HardwareVertexCache.h"
#include "filesystem_tools.h"


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHardwareVertexCache::CHardwareVertexCache()
{
	m_Fifo = NULL;
	m_Size = 0;
	Flush();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHardwareVertexCache::Init(int size)
{
	m_Size = size;
	m_Fifo = new int[size];
	Flush();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHardwareVertexCache::Flush( void )
{
	m_HeadIndex = 0;
	m_NumEntries = 0;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHardwareVertexCache::IsPresent(int index)
{
	int i;
	//	Msg( "testing if %d is present\n", index );
	for (i = 0; i < m_NumEntries; i++)
	{
		if (m_Fifo[(m_HeadIndex + i) % m_Size] == index)
		{
			//			Msg( "yes!\n" );
			return true;
		}
	}
	//	Msg( "no!\n" );
	//	Print();
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHardwareVertexCache::Insert(int index)
{
	//	Msg( "Inserting: %d\n", index );
	m_Fifo[(m_HeadIndex + m_NumEntries) % m_Size] = index;
	if (m_NumEntries == m_Size)
	{
		m_HeadIndex = (m_HeadIndex + 1) % m_Size;
	}
	else
	{
		m_NumEntries++;
	}
	//	Print();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHardwareVertexCache::Print(void)
{
	int i;
	for (i = 0; i < m_NumEntries; i++)
	{
		Msg("fifo entry %d: %d\n", i, (int)m_Fifo[(m_HeadIndex + i) % m_Size]);
	}
}

