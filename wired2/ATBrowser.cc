/* 
 * 3dyne Legacy Tools GPL Source Code
 * 
 * Copyright (C) 1996-2012 Matthias C. Berger & Simon Berger.
 * 
 * This file is part of the 3dyne Legacy Tools GPL Source Code ("3dyne Legacy
 * Tools Source Code").
 *   
 * 3dyne Legacy Tools Source Code is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * 3dyne Legacy Tools Source Code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * 3dyne Legacy Tools Source Code.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * In addition, the 3dyne Legacy Tools Source Code is also subject to certain
 * additional terms. You should have received a copy of these additional terms
 * immediately following the terms and conditions of the GNU General Public
 * License which accompanied the 3dyne Legacy Tools Source Code.
 * 
 * Contributors:
 *     Matthias C. Berger (mcb77@gmx.de) - initial API and implementation
 *     Simon Berger (simberger@gmail.com) - initial API and implementation
 */



// ATBrowser.cc

#include <stdio.h>

#include "archetype.h"
#include "Wired.hh"
#include "ATBrowser.hh"
#include "Customize.hh"

ATBrowser	*atbrowser_i;

/*
  ========================================
  class: ATBrowser

  aka the archetype editor
  ========================================
*/

ATBrowser::ATBrowser( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	arche_t		*a;
	kvpair_t	*p;

	atbrowser_i = this;

	qcb_attemplates = new QComboBox( this );
	qlb_atcurrent = new QListBox( this );

	ql_type = new QLabel( "Type:", this );
	qle_type = new QLineEdit( this );
	ql_key = new QLabel( "Key:", this );
	qle_key = new QLineEdit( this );
	ql_value = new QLabel( "Value:", this );
	qle_value = new QLineEdit( this );

	qpb_apply = new QPushButton( "Apply", this );
	qpb_add = new QPushButton( "Add", this );
	qpb_delete = new QPushButton( "Delete", this );

	ql_linkkey = new QLabel( "Link key:", this );
	qle_linkkey = new QLineEdit( this );
	ql_linkvalue = new QLabel( "Link value:", this );
	qle_linkvalue = new QLineEdit( this );

	//
	// connect
	//

	connect( qcb_attemplates, SIGNAL( activated( int ) ), this, SLOT( templateClickedSlot( int ) ) ); 
	connect( qlb_atcurrent, SIGNAL( selected( int ) ), this, SLOT( listClickedSlot( int ) ) );
	connect( qpb_apply, SIGNAL( clicked() ), this, SLOT( applySlot() ) );
	connect( qpb_delete, SIGNAL( clicked() ), this, SLOT( deleteSlot() ) );

	this->initTemplates();

#if 1
	for ( a = templates; a ; a=a->next )		
	{
		p = AT_GetPair( a, "type" );
		if ( !p )
		{
			printf( "ATBrowser::ATBrowser: template with no type. ignore.\n" );
			continue;
		}

		qcb_attemplates->insertItem( p->value );
	}
	currenttemplate = templates;
#endif

//	this->setCurrent( AT_NewArche() );
	this->setCurrent( NULL );

	// init selection
	singlearche = NULL;
	archenum = 0;
}


ATBrowser::~ATBrowser()
{
	delete qcb_attemplates;
	delete qlb_atcurrent;
}

void ATBrowser::setArche( arche_t *arche )
{
	// clear old current arche

	if ( currentarche->pairs )
		AT_FreePairList( currentarche->pairs );

	currentarche->pairs = AT_NewPairsFromList( arche->pairs );
	updateCurrent();
}


void ATBrowser::getArche( arche_t *arche )
{
	if ( arche->pairs )
		AT_FreePairList( arche->pairs );

	arche->pairs = AT_NewPairsFromList( currentarche->pairs );
}

arche_t * ATBrowser::getCurrentArche( void )
{
	return currentarche;
}

//
// Events ...
//

/*
  ====================
  initTemplates

  build template menu from *c_att[]
  customization strings
  ====================
*/

void ATBrowser::initTemplates()
{
	int		i;
	kvpair_t	*p;
	arche_t *a = 0;

	currentarche = NULL;
	memset( &currentpair, 0, sizeof( kvpair_t ) );
	this->templates = NULL;

	for( i = 0; c_att[i]; )
	{
		if ( c_att[i][0] == '_' )
		{
			// new template
			printf( "add template: %s\n", &c_att[i][1] );
//			qcb_attemplates->insertItem( &c_att[i][1] );
			a = AT_NewArche();
			a->next = templates;
			templates = a;
			i++;
			continue;
		}
		
		//
		// get type, key, value
		//
		p = AT_NewPair( c_att[i], c_att[i+1], c_att[i+2] );
		i += 3;
		AT_AddPair( a, p );
	}
	
#if 0

	// light
	a = AT_NewArche();
	p = AT_NewPair( "STRING", "type", "light" );
	AT_AddPair( a, p );
	p = AT_NewPair( "VEC3D", "origin", "0 0 0" );
	AT_AddPair( a, p );
	p = AT_NewPair( "FLOAT", "value", "100.0" );
	AT_AddPair( a, p );
	p = AT_NewPair( "VEC3D", "color", "1 1 1" );
	AT_AddPair( a, p );
     
	a->next = templates;
	templates = a;

	// player
	a = AT_NewArche();
	p = AT_NewPair( "STRING", "type", "player_start" );
	AT_AddPair( a, p );
	p = AT_NewPair( "VEC3D", "origin", "0 0 0" );
	AT_AddPair( a, p );

	a->next = templates;
	templates = a;

	// spotlight
	a = AT_NewArche();
	p = AT_NewPair( "STRING", "type", "spotlight" );
	AT_AddPair( a, p );
	p = AT_NewPair( "VEC3D", "origin", "0 0 0" );
	AT_AddPair( a, p );
	p = AT_NewPair( "VEC3D", "spotvec", "0 -1 0" );
	AT_AddPair( a, p );
	p = AT_NewPair( "FLOAT", "value", "100.0" );
	AT_AddPair( a, p );
	p = AT_NewPair( "FLOAT", "angle", "45.0" );
	AT_AddPair( a, p );
	p = AT_NewPair( "FLOAT", "falloff", "0.8" );
	AT_AddPair( a, p );
	p = AT_NewPair( "VEC3D", "color", "1 1 1" );
	AT_AddPair( a, p );

	a->next = templates;
	templates = a;
#endif
}

void ATBrowser::setCurrent( arche_t *arche )
{
	currentarche = arche;
	
	updateCurrent();
}

void ATBrowser::updateCurrent( void )
{
	char			text[256];
	kvpair_t		*p;
		
	if ( !currentarche )
		return;

	qlb_atcurrent->clear();

	for ( p = currentarche->pairs; p ; p=p->next )
	{
		sprintf( text, "( %s ) \"%s\": \"%s\"", p->type, p->key, p->value );
		qlb_atcurrent->insertItem( text );
	}
}

void ATBrowser::setCurrentPair( kvpair_t *pair )
{
	qle_type->setText( pair->type );	
	qle_key->setText( pair->key );	
	qle_value->setText( pair->value );	
	
	memcpy( &currentpair, pair, sizeof( kvpair_t ));

}

void ATBrowser::applyCurrentPair( void )
{
	const char	*text;

	if ( !currentarche )
	{
		wired_i->printComment( "no archetype selected. can't apply key." );
		return;
	}

	text = qle_type->text();
	strcpy( currentpair.type, text );
	text = qle_key->text();
	strcpy( currentpair.key, text );
	text = qle_value->text();
	strcpy( currentpair.value, text );

	AT_SetPair( currentarche, currentpair.type, currentpair.key, currentpair.value );

	// update list box
	updateCurrent();
}

void ATBrowser::deleteCurrentPair( void )
{
	const char	*text;

	if ( !currentarche )
	{
		wired_i->printComment( "no archetype selected" );
		return;
	}

	text = qle_key->text();
	if ( text[0]==0 )
	{
		wired_i->printComment( "no current pair" );
		return;
	}

	AT_RemovePair( currentarche, (char*) text );

	// update list box
	updateCurrent();
}


void ATBrowser::changeCurrentValueVec3d( vec3d_t v )
{
	AT_CastVec3dToValue( currentpair.value, v );	
	AT_SetPair( currentarche, currentpair.type, currentpair.key, currentpair.value );
	updateCurrent();
}

/*
  ==============================
  getLinkFromCurrent

  ==============================
*/
void ATBrowser::getLinkFromCurrent( void )
{
	kvpair_t		*type;
	kvpair_t		*name;

	printf( "ATBrowser::getLinkFromCurrent\n" );

	if ( !currentarche )
	{
		wired_i->printComment( "no archetype selected" );
		return;
	}

	type = AT_GetPair( currentarche, "type" );
	name = AT_GetPair( currentarche, "name" );

	if ( !type )
	{
		wired_i->printComment( "no type found in archetype" );
		qle_linkkey->clear();
		qle_linkvalue->clear();
		return;
	}

	if ( !name )
	{
		wired_i->printComment( "no name found in archetype" );
		qle_linkkey->clear();
		qle_linkvalue->clear();
		return;
	}
	
	qle_linkkey->setText( type->value );
	qle_linkvalue->setText( name->value );
}

/*
  ==============================
  setLinkOfCurrent

  ==============================
*/
void ATBrowser::setLinkOfCurrent( void )
{
	const char	*linkkey, *linkvalue;

	printf( "ATBrowser::setLinkOfCurrent\n" );

	linkkey = qle_linkkey->text();
	linkvalue = qle_linkvalue->text();

	if ( linkkey[0]==0 || linkvalue[0]==0 )
	{
		wired_i->printComment( "current link is invalide" );
		return;
	}

	if ( !currentarche )
	{
		wired_i->printComment( "no archetype selected" );
		return;
	}

	AT_SetPair( currentarche, "clsref", (char*) linkkey, (char*) linkvalue );
	updateCurrent();
}


//
// private slots
//

void ATBrowser::templateClickedSlot( int id )
{
	int		i;
	arche_t		*a;

	printf( "ATBrowser::templateClickedSlot: id = %d\n", id );

	for( a = templates, i = 0; a && i!=id; a=a->next, i++ )
	{ /* do nothing */ }

	if ( !a )
	{
		printf( "ATBrowser::templateClickedSlot: clicked template not found ?\n" );
		return;
	}

	currenttemplate = a;

#if 0
	// delete all pair in archetype
	AT_FreePairList( currentarche->pairs );

	// copy pairs from template
	currentarche->pairs = AT_NewPairsFromList( a->pairs );
	
	// update list box
	setCurrent( currentarche );
#endif
}

void ATBrowser::applySlot()
{
	applyCurrentPair();
}

void ATBrowser::deleteSlot()
{
	deleteCurrentPair();
}

void ATBrowser::listClickedSlot( int id )
{
	int		i;
	kvpair_t	*p;

	printf( "ATBrowser::listClickedSlot: id = %d\n", id );
	
	if ( !currentarche )
	{
		printf( "ATBrowser::listClickedSlot: currentarche = NULL ?\n" );
		return;
	}

	// find pair with indexnum id
	for( p = currentarche->pairs, i = 0; p && i!=id; p=p->next, i++ )
	{ /* do nothing */ }
	
	if ( !p )
	{
		printf( "ATBrowser::listClickedSlot: clicked pair not found ?\n" );
		return;
	}

	// found pair
	setCurrentPair( p );
}

//
// events
//

void ATBrowser::resizeEvent( QResizeEvent * )
{
	printf( "ATBrowser::resizeEvent\n" );

	qcb_attemplates->setGeometry( 0, 0, 150, 25 );
	qlb_atcurrent->setGeometry( 0, 25, 300, 200 );
	ql_type->setGeometry( 0, 225, 150, 25 );
	qle_type->setGeometry( 150, 225, 150, 25 );
	ql_key->setGeometry( 0, 250, 150, 25 );
	qle_key->setGeometry( 150, 250, 150, 25 );
	ql_value->setGeometry( 0, 275, 150, 25 );
	qle_value->setGeometry( 150, 275, 150, 25 );

	qpb_apply->setGeometry( 0, 300, 80, 25 );
	qpb_add->setGeometry( 80, 300, 80, 25 );
	qpb_delete->setGeometry( 160, 300, 80, 25 );

	ql_linkkey->setGeometry( 0, 350, 150, 25 );
	qle_linkkey->setGeometry( 150, 350, 150, 25 );
	ql_linkvalue->setGeometry( 0, 375, 150, 25 );
	qle_linkvalue->setGeometry( 150, 375, 150, 25 );
}

void ATBrowser::enterEvent( QEvent * )
{
	printf( "ATBrowser::enterEvent\n" );
//	wired_i->disableAccel();
}

void ATBrowser::leaveEvent( QEvent * )
{
	printf( "ATBrowser::leaveEvent\n" );
//	wired_i->enableAccel();
}


//
// public 
//

arche_t* ATBrowser::createArche( vec3d_t v )
{
	arche_t		*a;
	char		text[256];

	// fixme: what's happend with currentarche ?

	a = AT_NewArche();

	// copy pairs from template
	a->pairs = AT_NewPairsFromList( currenttemplate->pairs );

	// set origin
	AT_CastVec3dToValue( text, v );
	AT_SetPair( a, "VEC3D", "origin", text );

	//
	// add to wwm
	// 
//	a->status = OS_NORMAL;
//	wwm_i->addArche( a, true );
	setCurrent( a );

	return a;
}

void ATBrowser::copyArche( void )
{

}

void ATBrowser::deleteArche( void )
{

}

//
// public slots
//

void ATBrowser::xzSelectSlot( Vec2 v )
{
	int		i, j;
	arche_t		*a;
	kvpair_t	*pair;

	vec3d_t		pos;

	int		prenum;
	arche_t		*pre[100];
	int		flag[100];

	printf( "ATBrowser::xzSelectSlot\n" );

	prenum = 0;
	for ( a = wwm_i->getFirstArche(); a ; a=a->next )
	{
		if ( !a->visible )
			continue;

		pair = AT_GetPair( a, "origin" );
		if ( !pair )
		{
			printf( "ATBrowser::xzSelectSlot: can't find key \"origin\"\n" );
			continue;
		}

		AT_CastValueToVec3d( pos, pair->value );
		Vec3dPrint( pos );

		if ( v[0] > pos[0]-32 && v[0] < pos[0]+32 &&
		     v[1] > pos[2]-32 && v[1] < pos[2]+32 )
		{

			flag[prenum] = 0;
			pre[prenum] = a;
			prenum++;
		}
	}
	printf( " prenum = %d\n", prenum );

	if ( !prenum )
	{
		printf( " no selection.\n" );
		
		if ( singlearche )
			if ( singlearche->status == OS_SINGLESELECT )
			{
				singlearche->status = -OS_NORMAL;
				wired_i->updateViews();
			}
		return;
	}

	if ( prenum != archenum )
	{
		printf( " new selection1.\n" );
		for ( i = 0; i < prenum; i++ )
			arches[i] = pre[i];
		archenum = prenum;
		arche = 0;
	}
	else
	{
		for ( i = 0; i < archenum; i++ )
		{
			for ( j = 0; j < archenum; j++ )
			{
				if ( pre[i] == arches[j] )
					flag[i] = 1;
			}
		}

		for( i = 0; i < archenum; i++ )
			if ( !flag[i] )
				break;

		if ( i == archenum )
		{
			printf( " old selection.\n" );
			arche++;
			if ( arche == archenum )
				arche = 0;
		}
		else
		{
			printf( " new selection2.\n" );
			for ( i = 0; i < archenum; i++ )
				arches[i] = pre[i];
			arche = 0;
		}
	}

	if ( singlearche )
	{
		singlearche->status = -abs(laststatus);
	}

	singlearche = arches[arche];
	laststatus = singlearche->status;
	singlearche->status = -OS_SINGLESELECT;

	this->setCurrent( singlearche );

	wired_i->updateViews();
}

void ATBrowser::pressSlot( Vec3 v )
{
	printf( "ATBrowser::pressSlot\n" );
	
	from[0] = v[0];
	from[1] = v[1];
	from[2] = v[2];
	
	xzview_i->screen2Back();	
	yview_i->screen2Back();
}

void ATBrowser::dragSlot( Vec3 v )
{
	vec3d_t		to;

	printf( "ATBrowser::dragSlot\n" );

	to[0] = v[0];
	to[1] = v[1];
	to[2] = v[2];
	
	Vec3dSub( delta, to, from );
	if ( delta[0]!=0 || delta[1]!=0 || delta[2]!=0 )
	{
		xzview_i->back2Screen();
		yview_i->back2Screen();	

		moveArches();
		wired_i->updateViews();
		Vec3dCopy( from, to );
	}
}

void ATBrowser::releaseSlot( Vec3 v )
{
	vec3d_t		to;

	printf( "ATBrowser::releaseSlot\n" );
	
	//Vec3dCopy( to, v );
	v.get(to);

	Vec3dSub( delta, to, from );
	moveArches();

	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();	
}

void ATBrowser::yPressSlot( Vec2 /*v*/ )
{

}

void ATBrowser::yDragSlot( Vec2 /*v*/ )
{

}

void ATBrowser::yReleaseSlot( Vec2 /*v*/ )
{

}


void ATBrowser::moveArches()
{
	arche_t		*a;
	vec3d_t		v;
	kvpair_t	*pair;	

	for ( a = wwm_i->getFirstArche(); a ; a=a->next )
	{
		if ( !(a->select&SELECT_BLUE) )
			continue;

		printf( "move Arche.\n" );

		pair = AT_GetPair( a, "origin" );
		if ( !pair )
		{
			printf( "ATBrowser::moveArches can't find key \"origin\"\n" );
			return;
		}	
		AT_CastValueToVec3d( v, pair->value );

		Vec3dAdd( v, delta, v );

		AT_CastVec3dToValue( pair->value, v );
		a->select|=SELECT_UPDATE;
	}
}
