//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=====================================================================================//
#ifndef COLLISIONMODEL_H
#define COLLISIONMODEL_H

#ifdef _WIN32
#pragma once
#endif // _WIN32


void Cmd_CollisionText( void );
int DoCollisionModel( bool separateJoints );

// execute after simplification, before writing
void CollisionModel_Build( void );
// execute during writing
void CollisionModel_Write( long checkSum );

void CollisionModel_ExpandBBox( Vector &mins, Vector &maxs );


#endif // COLLISIONMODEL_H
