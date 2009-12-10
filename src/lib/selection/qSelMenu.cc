/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its terms.
 */


#include <anatomist/selection/qSelMenu.h>
#include <aims/qtcompat/qpopupmenu.h>
#include <qpixmap.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/object/optionMatcher.h>
#include <anatomist/object/actions.h>
#include <anatomist/control/wControl.h>
#include <graph/tree/tree.h>
#include <iostream>


using namespace anatomist;
using namespace carto;
using namespace std;


QPixmap	*QSelectMenu::_defPixmap = 0;

// ---------------------------------

MenuCallback::~MenuCallback()
{
}


void MenuCallback::activated()
{
  _func( _clientdata );
}

// ----------------------------------

QAOptionMenuCallback::QAOptionMenuCallback
( void (*func)( const std::set<anatomist::AObject *> & ),
  const std::set<anatomist::AObject *> *obj )
  : MenuCallback( 0, (void *) obj ),
    _callback( carto::rc_ptr<anatomist::ObjectMenuCallback>
               ( new ObjectMenuCallbackFunc( func ) ) )
{
}


QAOptionMenuCallback::~QAOptionMenuCallback()
{
}


void QAOptionMenuCallback::activated()
{
  _callback->doit( *(const std::set<anatomist::AObject *> *) _clientdata );
}

// ----------------------------------

QSelectMenu::QSelectMenu() 
  : QPopupMenu( 0, "popupmenu" ), _win( 0 )
{
  if( !_defPixmap )
    {
      _defPixmap = new QPixmap( (Settings::globalPath() 
				 + "/icons/menu_default.xpm").c_str() );
    }
}


QSelectMenu::~QSelectMenu()
{
  eraseCallbacks();
}


void QSelectMenu::update( AWindow* win, const Tree* specific )
{
  _win = win;

  clear();

  insertItem( tr( "View / select objects" ), this, SLOT( viewSlot() ) );
  insertItem( tr( "Unselect" ), this, SLOT( unselectSlot() ) );
  insertItem( tr( "Select all" ), this, SLOT( selectAllSlot() ) );
  insertItem( tr( "Remove from windows of this group" ), this, 
	      SLOT( removeSlot() ) );
  insertItem( tr( "Remove from this window" ), this, 
	      SLOT( removeThisWinSlot() ) );

  eraseCallbacks();

  //	variable contents

  const map<unsigned, set<AObject *> > & mo 
    = SelectFactory::factory()->selected();
  map<unsigned, set<AObject *> >::const_iterator	im 
    = mo.find( win->Group() );
  set<AObject *>::const_iterator			io, fo;
  bool							hasgraph = false;

  if( im != SelectFactory::factory()->selected().end() )
  {
    const set<AObject *> & so = (*im).second;

    if( !so.empty() )
    {
      //	object manipulations (with Xt-compatible callbacks
      Tree		otr;

      _objects = so;	// remember calling objects
      OptionMatcher::commonOptions( _objects, otr );

      if( otr.size() != 0 )
      {
        otr.setSyntax( QT_TRANSLATE_NOOP( "QSelectMenu",
                                          "Objects manipulations" ) );
        addOptionMenus( this, &otr );
      }

      //	graph-specific stuff

      for( io=so.begin(), fo=so.end(); !hasgraph && io!=fo; ++io )
        if( (*io)->isMultiObject() )
          switch( ((MObject *) (*io))->MType() )	// type-specific stuff
          {
          case AObject::GRAPHOBJECT:
            hasgraph = true;
            // cout << "graphobject " << (*io)->name() << " selected\n";
            break;
          default:
            /* cout << "obj " << (*io)->name() << " type : "
                  << (*io)->type() << endl; */
            break;
          }

      if( hasgraph )
      {
        insertSeparator();
        insertItem( tr( "Select neighbours" ), this,
                    SLOT( neighboursSlot() ) );
        insertItem( tr( "Select nodes of attribute ..." ), this,
                    SLOT( selAttribSlot() ) );
      }
    }
  }

  if( specific && specific->size() > 0 )
  {
    insertSeparator();
    Tree::const_iterator	it, ft=specific->end();
    for( it=specific->begin(); it!=ft; ++it )
      addMenus( this, (Tree *) *it );
  }
}


void QSelectMenu::viewSlot()
{
  emit viewSignal( _win );
}


void QSelectMenu::unselectSlot()
{
  emit unselectSignal( _win );
}


void QSelectMenu::selectAllSlot()
{
  emit selectAllSignal( _win );
}


void QSelectMenu::removeSlot()
{
  emit removeSignal( _win );
}


void QSelectMenu::removeThisWinSlot()
{
  emit removeThisWinSignal( _win );
}


void QSelectMenu::neighboursSlot()
{
  emit neighboursSignal( _win );
}


void QSelectMenu::selAttribSlot()
{
  emit selAttribSignal( _win );
}


void QSelectMenu::eraseCallbacks()
{
  set<MenuCallback *>::iterator	ic, fc=_callbacks.end();

  for( ic=_callbacks.begin(); ic!=fc; ++ic )
    delete *ic;
  _callbacks.erase( _callbacks.begin(), _callbacks.end() );
}


void QSelectMenu::addMenus( QMenuData* menu, const Tree* tree )
{
  MenuCallback	*cbk;
  void		*clientdata;
  bool		def;

  if( tree->size() == 0 )	// leaf
    {
      void	(*func)( void * );
      if( tree->getProperty( "callback", func ) )
	{
	  clientdata = 0;
	  tree->getProperty( "client_data", clientdata );
	  cbk = new MenuCallback( func, clientdata );
	  _callbacks.insert( cbk );
	  if( tree->getProperty( "default", def ) && def )
	    menu->insertItem( *_defPixmap, tr( tree->getSyntax().c_str() ), 
			      cbk, SLOT( activated() ) );
	  else
	    menu->insertItem( tr( tree->getSyntax().c_str() ), cbk, 
			      SLOT( activated() ) );
	}
      return;
    }

  Tree::const_iterator	it, ft=tree->end();
  QPopupMenu		*pop = new QPopupMenu( this );
  Tree			*t2;

  menu->insertItem( tr( tree->getSyntax().c_str() ), pop );

  for( it=tree->begin(); it!=ft; ++it )
    {
      t2 = (Tree *) *it;
      addMenus( pop, t2 );
    }
}


void QSelectMenu::addOptionMenus( QMenuData* menu, const Tree* tree )
{
  QAOptionMenuCallback	*cbk = 0;

  if( tree->size() == 0 )	// leaf
    {
      void	(*func)( const set<AObject *> & ) = 0;
      rc_ptr<ObjectMenuCallback>  c;
      if( tree->getProperty( "objectmenucallback", c ) )
        cbk = new QAOptionMenuCallback( c, &_objects );
      else if( tree->getProperty( "callback", func ) )
        cbk = new QAOptionMenuCallback( func, &_objects );
      if( cbk )
      {
        _callbacks.insert( cbk );
        /* int id = */ menu->insertItem( tr( tree->getSyntax().c_str() ),
                          cbk, SLOT( activated() ) );
        /* can't set accel keys here because they are never removed.
           I cannot find in Qt where this is stored or how to clean them */
        /*
        if( tree->getSyntax() == "Save" )
          menu->setAccel( CTRL + Key_S, id );
        */
      }
      return;
    }

  Tree::const_iterator	it, ft=tree->end();
  QPopupMenu		*pop = new QPopupMenu( this );
  Tree			*t2;

  menu->insertItem( tr( tree->getSyntax().c_str() ), pop );

  for( it=tree->begin(); it!=ft; ++it )
    {
      t2 = (Tree *) *it;
      addOptionMenus( pop, t2 );
    }
}
