/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */


#include <anatomist/control/qObjTree.h>
#include <anatomist/browser/qObjBrowserWid.h>
#include <anatomist/browser/attDescr.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <qpixmap.h>
#include <anatomist/object/Object.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/graph/pythonAObject.h>
#include <anatomist/control/graphParams.h>
#include <aims/listview/qpython.h>
#include <stdio.h>

#if QT_VERSION >= 0x040000
using namespace Qt;
#endif
using namespace anatomist;
using namespace aims::gui;
using namespace carto;
using namespace std;


QObjectBrowserWidget::ObjectHelperSet	QObjectBrowserWidget::objectHelpers;


QObjectBrowserWidget::QObjectBrowserWidget( QWidget* parent, const char* name )
  : QAListView( parent, name ), _recursive( false )
{
  // clearWFlags( WNorthWestGravity | WRepaintNoErase );
  setMultiSelection( true );
  setSelectionMode( Extended );
  setRootIsDecorated( true );
  setAllColumnsShowFocus( true );

  objectHelpers[ AObject::GRAPH ] = describeGraph;
  objectHelpers[ Hierarchy::classType() ] = describeHierarchy;
  connect( this, SIGNAL( selectionChanged() ), 
           SLOT( unselectInvisibleItems() ) );
}


QObjectBrowserWidget::~QObjectBrowserWidget()
{
}


void QObjectBrowserWidget::registerObject( AObject* object, 
					   bool /*temporaryObject*/ )
{
  /* cout << "QObjectBrowserWidget::registerObject( AObject* object, bool ) "
  << object->name() << endl; */
  map<Q3ListViewItem *, AObject *>::const_iterator	ia, fa=_aobjects.end();
  unsigned const	regColumn = 5;

  AObject::ParentList parents = object->parents();
  AObject::ParentList::iterator ip, ip2, ep = parents.end();
  for( ip=parents.begin(); ip!=ep; )
  {
    if( (*ip)->type() == AObject::GRAPH )
      ++ip;
    else
    {
      ip2 = ip;
      ++ip;
      parents.erase( ip2 );
    }
  }

  for( ia=_aobjects.begin(); ia!=fa; ++ia )
    if( (*ia).second == object )
    {
      (*ia).first->setText( regColumn, "*" );
      return;		// already listed
    }
    else if( ia->second->type() == AObject::GRAPH
             && parents.find( static_cast<MObject *>( ia->second ) ) != ep )
      return; // already a parent graph displayed here

  //	mettre un truc plus rapide...
  map<Q3ListViewItem *, GenericObject *>::const_iterator 
    ig, fg=_gobjects.end();
  shared_ptr<AObject>	ao;

  for( ig=_gobjects.begin(); ig!=fg; ++ig )
    if( (*ig).second->getProperty( "ana_object", ao ) && ao.get() == object )
    {
      (*ig).first->setText( regColumn, "*" );
      return;	// indirectly in list (generally graph object)
    }

  Q3ListViewItem	*item = insertObject( object );
  item->setText( regColumn, "*" );
}


void QObjectBrowserWidget::unregisterObject( AObject* object, 
					     bool /*temporaryObject*/ )
{
  map<Q3ListViewItem *, AObject *>::iterator	ia, fa = _aobjects.end();
  const unsigned	regColumn = 5;

  for( ia=_aobjects.begin(); ia!=fa; ++ia )
    if( (*ia).second == object )
      {
	removeObject( (*ia).first, (*ia).second );
	break;
      }

  map<Q3ListViewItem *, GenericObject *>::iterator 
    ig, fg = _gobjects.end();
  shared_ptr<AObject>	ao;

  for( ig=_gobjects.begin(); ig!=fg; ++ig )
    if( (*ig).second->getProperty( "ana_object", ao ) && ao.get() == object )
      (*ig).first->setText( regColumn, 0 );
}


void QObjectBrowserWidget::updateObject( AObject* obj )
{
  map<Q3ListViewItem *, AObject *>::iterator	io, fo=_aobjects.end();
  Q3ListViewItem				*item;

  for( io=_aobjects.begin(); io!=fo; ++io )
    if( (*io).second == obj )
      {
	item = (*io).first;
	decorateItem( item, obj );
	if( item->childCount() > 0 )	// don't update objects without descr.
          describeAObject( obj, item );
      }
}


void QObjectBrowserWidget::updateObject( GenericObject* )
{
}


void QObjectBrowserWidget::registerObject( GenericObject* object )
{
  shared_ptr<AObject> ao;
  /* if( object->getProperty( "ana_object", ao ) )
  {
    registerObject( ao.get() );
    return;
  } */

  map<Q3ListViewItem *, GenericObject *>::const_iterator 
    ig, fg=_gobjects.end();

  for( ig=_gobjects.begin(); ig!=fg; ++ig )
    if( (*ig).second == object )
      return;		// already listed


  AttDescr	*attd = AttDescr::descr();
  Q3ListViewItem	*item;
  string 		name = attd->objectName( object ), lab1, lab2;
  char			id[20];
  float			sz;
  Edge			*ed = dynamic_cast<Edge *>( object );

  if( ed )
    {
      Edge::const_iterator	iv = ed->begin();
      string			str;

      (*iv)->getProperty( "label", lab1 );
      if( (*iv)->getProperty( "name", str ) )
	name = str + " - ";
      ++iv;
      if( (*iv)->getProperty( "name", str ) )
	name += str;
      (*iv)->getProperty( "label", lab2 );
    }

  SyntaxedInterface	*si = object->getInterface<SyntaxedInterface>();
  string		snt;
  if( si && si->hasSyntax() )
    snt = si->getSyntax();
  else
    snt = object->type();
  item = new Q3ListViewItem( this, name.c_str(), snt.c_str() );
  if( object->getProperty( "size", sz ) )
    {
      sprintf( id, "%7.1f", sz );
      item->setText( 2, id );
    }
  if( object->getProperty( "label", name ) )
    item->setText( 3, name.c_str() );
  if( lab1.size() > 0 )
    item->setText( 3, lab1.c_str() );
  if( lab2.size() > 0 )
    item->setText( 4, lab2.c_str() );

  attd->describeAttributes( this, item, object );

  _itemTypes[ item ] = GOBJECT;
  _gobjects[ item ] = object;
}


void QObjectBrowserWidget::unregisterObject( GenericObject* object )
{
  map<Q3ListViewItem *, GenericObject *>::iterator  ig, fg = _gobjects.end();

  for( ig=_gobjects.begin(); ig!=fg; ++ig )
    if( (*ig).second == object )
      {
	removeItem( (*ig).first );
	break;
      }
}


Q3ListViewItem* QObjectBrowserWidget::insertObject( AObject* obj )
{
  // cout << "QObjectBrowserWidget::insertObject( AObject* obj )\n";
  Q3ListViewItem	*item = itemFor( obj );

  if( !item )
    {
      item = new Q3ListViewItem( this );

      _aobjects[ item ] = obj;
      _itemTypes[ item ] = AOBJECT;
    }

  decorateItem( item, obj );
  describeAObject( obj, item );
  return( item );
}


Q3ListViewItem* QObjectBrowserWidget::insertObject( Q3ListViewItem* parent, 
						   AObject* obj )
{
  Q3ListViewItem	*item = itemFor( obj );

  if( !item )
    {
      item = new Q3ListViewItem( parent );

      _aobjects[ item ] = obj;
      _itemTypes[ item ] = AOBJECT;
    }

  decorateItem( item, obj );
  return( item );
}


void QObjectBrowserWidget::decorateItem( Q3ListViewItem* item, AObject* obj )
{
  enum { nameCol, typeCol, ValueCol };
  map<int, QPixmap>::const_iterator	ip, fp=QObjectTree::TypeIcons.end();
  map<int, string>::const_iterator	it, ft=QObjectTree::TypeNames.end();

  item->setText( nameCol, obj->name().c_str() );

  ip = QObjectTree::TypeIcons.find( obj->type() );
  if( ip != fp )
    item->setPixmap( nameCol, (*ip).second );
  it = QObjectTree::TypeNames.find( obj->type() );
  if( it != ft )
    item->setText( typeCol, (*it).second.c_str() );
  else
    {
      char	str[20];
      sprintf( str, "? (%d)", obj->type() );
      item->setText( typeCol, str );
    }

  AttributedAObject	*aao = dynamic_cast<AttributedAObject *>( obj );
  if( aao )
    {
      SyntaxedInterface	*si = aao->attributed()
        ->getInterface<SyntaxedInterface>();
      if( si && si->hasSyntax() )
        item->setText( ValueCol, si->getSyntax().c_str() );
    }
}


void QObjectBrowserWidget::removeObject( Q3ListViewItem* item, AObject* )
{
  removeItem( item );
  delete item;
}


void QObjectBrowserWidget::removeItem( Q3ListViewItem* item )
{
  map<Q3ListViewItem *, ItemType>::iterator	it = _itemTypes.find( item );

  if( it != _itemTypes.end() )
    {
      switch( (*it).second )
	{
	case AOBJECT:
	  {
	    map<Q3ListViewItem *, AObject *>::iterator 
	      ia = _aobjects.find( item );
	    if( ia == _aobjects.end() )
	      {
		cerr << "QObjectBrowserWidget::removeItem : " 
		     <<"AObject not found\n";
		break;
	      }
	    _aobjects.erase( ia );
	  }
	  break;
	case GOBJECT:
	  {
	    map<Q3ListViewItem *, GenericObject *>::iterator 
	      ig = _gobjects.find( item );
	    if( ig == _gobjects.end() )
	      {
		cerr << "QObjectBrowserWidget::removeItem : " 
		     << "GObject not found\n";
		break;
	      }
	    _gobjects.erase( ig );
	  }
	  break;
	default:
	  break;
	}
      _itemTypes.erase( it );
    }

  unsigned	i, n = item->childCount();

  for( i=0, item = item->firstChild(); i<n; ++i )
    {
      removeItem( item );
      item = item->nextSibling();
    }
}


QObjectBrowserWidget::ItemType 
QObjectBrowserWidget::typeOf( Q3ListViewItem * item ) const
{
  map<Q3ListViewItem *, ItemType>::const_iterator 
    it = _itemTypes.find( item );

  if( it == _itemTypes.end() )
    return( UNKNOWN );
  else
    return( (*it).second );
}


void QObjectBrowserWidget::describeAObject( AObject* obj, 
					    Q3ListViewItem* parent )
{
  // cout << "describeAObject " << obj << ": " << obj->name() << endl;
  map<int, ObjectHelper>::const_iterator 
    ih = objectHelpers.find( obj->type() );

  if( ih != objectHelpers.end() )
    (*ih).second( this, obj, parent );
  else
    {
#if 1
      AttributedAObject	*aao = dynamic_cast<AttributedAObject *>( obj );
      if( aao )
	{
	  AttDescr		*attd = AttDescr::descr();
	  attd->describeAttributes( this, parent, aao->attributed() );
	}
      else
	{
          //#endif
	  const PythonAObject 
	    *da = dynamic_cast<const PythonAObject *>( obj );
	  if( da )
	    {
              const GenericObject	*go = da->attributed();
              if( go )
                {
                  QPythonPrinter pp( parent, AttDescr::descr()->syntaxSet() );
                  pp.write( *go, true );
                }
	    }
          //#if 0
	}
#endif

      if( obj->isMultiObject() )
	{
	  MObject		*mo = (MObject *) obj;
	  MObject::iterator	io, fo = mo->end();

	  for( io=mo->begin(); io!=fo; ++io )
	    {
	      insertObject( parent, *io );
	    }
	}
    }
}


void QObjectBrowserWidget::describeGraph( QObjectBrowserWidget* br, 
					  AObject* obj,
                                          Q3ListViewItem* parent )
{
  Graph			*gr = ((AGraph *) obj)->graph();
  AttDescr		*attd = AttDescr::descr();
  Graph::const_iterator	iv, fv=gr->end();
  Vertex		*v;
  string		name, synt;
  Q3ListViewItem		*item, *gratt, *nodes;
  char			id[20];
  float			sz;
  bool			newitem = false;

  gratt = br->itemFor( parent, tr( "Global attributes" ).utf8().data() );
  if( !gratt )
    gratt = new Q3ListViewItem( parent, tr( "Global attributes" ) );
  if( !gr->getProperty( "name", name ) )
    name = tr( "Graph nodes" ).utf8().data();
  sprintf( id, "%d %s", (int) gr->order(), tr( "nodes" ).utf8().data() );
  nodes = br->itemFor( parent, name );
  if( !nodes )
    {
      nodes = new Q3ListViewItem( parent, name.c_str(), 
				 gr->getSyntax().c_str(), id );
      newitem = true;
    }
  else
    {
      nodes->setText( 1, gr->getSyntax().c_str() );
      nodes->setText( 2, id );
    }
  attd->describeAttributes( br, gratt, gr );

  Material mat;
  shared_ptr<AObject> o;

  for( iv=gr->begin(); iv!=fv; ++iv )
  {
    v = *iv;
    name = attd->objectName( v );

    synt = v->getSyntax();
    o.reset( shared_ptr<AObject>::Weak, 0 );
    v->getProperty( "ana_object", o );

    if( !newitem )
      item = br->itemFor( nodes, v );
    if( newitem || !item )
    {
      item = new Q3ListViewItem( nodes, name.c_str(), synt.c_str() );
      br->_itemTypes[ item ] = GOBJECT;
      br->_gobjects[ item ] = v;

      if( o.get() )
      {
        MObject		*mo = (MObject *) o.get();
        MObject::iterator	io, fo = mo->end();

        for( io=mo->begin(); io!=fo; ++io )
          br->insertObject( item, *io );
      }
    }
    else
    {
      item->setText( 0, name.c_str() );
      item->setText( 1, synt.c_str() );
    }
    if( v->getProperty( "size", sz ) )
    {
      sprintf( id, "%7.1f", sz );
      item->setText( 2, id );
    }
    if( v->getProperty( "label", name ) )
      item->setText( 3, name.c_str() );
    if( o.get() && GraphParams::recolorLabelledGraph(
        static_cast<AGraph *>( obj ),
        static_cast<AGraphObject *>( o.get() ), mat ) )
    {
      /* cout << mat.Diffuse(0) << ", " << mat.Diffuse(1) << mat.Diffuse(2)
          << endl; */
      QPixmap cpix
          = AttDescr::rgbPixmap( QColor( int( mat.Diffuse(0) * 255 ),
                                  int( mat.Diffuse(1) * 255 ),
                                  int( mat.Diffuse(2) * 255 ) ) );
      item->setPixmap( 0, cpix );
    }
    else
      item->setPixmap( 0, QPixmap() );
    attd->describeAttributes( br, item, v );
  }
}


void QObjectBrowserWidget::describeHierarchy( QObjectBrowserWidget* br,
					      AObject* obj, 
					      Q3ListViewItem* parent )
{
  AttDescr::descr()->describeTreeInside( br, ((Hierarchy *)obj)->tree(), 
					 parent, true );
}


void QObjectBrowserWidget::clear()
{
  _aobjects.erase( _aobjects.begin(), _aobjects.end() );
  _gobjects.erase( _gobjects.begin(), _gobjects.end() );
  _itemTypes.erase( _itemTypes.begin(), _itemTypes.end() );
  Q3ListView::clear();
}


void QObjectBrowserWidget::registerAttribute( Q3ListViewItem* item )
{
  _itemTypes[ item ] = ATTRIBUTE;
}


void QObjectBrowserWidget::registerAObject( Q3ListViewItem* item, AObject* obj )
{
  _itemTypes[ item ] = AOBJECT;
  _aobjects[ item ] = obj;
}


void QObjectBrowserWidget::registerGObject( Q3ListViewItem* item, 
					    GenericObject* obj )
{
  _itemTypes[ item ] = GOBJECT;
  _gobjects[ item ] = obj;
}


Q3ListViewItem* QObjectBrowserWidget::itemFor( const AObject* obj )
{
  map<Q3ListViewItem *, AObject *>::iterator	io, fo=_aobjects.end();

  for( io=_aobjects.begin(); io!=fo && (*io).second!=obj; ++io ) {}
  if( io == fo )
    return( 0 );	// not found

  return( (*io).first );
}


Q3ListViewItem* QObjectBrowserWidget::itemFor( Q3ListViewItem* parent, 
					      const AObject* )
{
  unsigned	n = parent->childCount(), i;
  Q3ListViewItem	*item;
  map<Q3ListViewItem *, AObject *>::iterator	io, fo=_aobjects.end();

  for( item = parent->firstChild(), i=0; i<n; ++i, item=item->nextSibling() )
    {
      io = _aobjects.find( item );
      if( io != fo )
	return( item );
    }
  return( 0 );
}


Q3ListViewItem* QObjectBrowserWidget::itemFor( Q3ListViewItem* parent, 
					      const GenericObject* ao, 
					      bool regist )
{
  if( !regist )
    {
      const SyntaxedInterface	*si = ao->getInterface<SyntaxedInterface>();
      if( si && si->hasSyntax() )
        return itemFor( parent, si->getSyntax() );
      else
        return itemFor( parent, ao->type() );
    }
  unsigned	n = parent->childCount(), i;
  Q3ListViewItem	*item;
  map<Q3ListViewItem *, GenericObject *>::iterator	ia, fa=_gobjects.end();

  for( item = parent->firstChild(), i=0; i<n; ++i, item=item->nextSibling() )
    {
      ia = _gobjects.find( item );
      if( ia != fa && (*ia).second == ao )
	return( item );
    }
  return( 0 );
}


Q3ListViewItem* QObjectBrowserWidget::itemFor( const GenericObject* ao )
{
  map<Q3ListViewItem *, GenericObject *>::iterator	ia, fa=_gobjects.end();

  for( ia=_gobjects.begin(); ia!=fa && (*ia).second!=ao; ++ia ) {}
  if( ia == fa )
    return( 0 );	// not found

  return( (*ia).first );
}


Q3ListViewItem* QObjectBrowserWidget::itemFor( Q3ListViewItem* parent, 
					      ItemType, 
					      const string & ff, bool regist )
{
  if( !regist )
    return( itemFor( parent, ff ) );
  unsigned	n = parent->childCount(), i;
  Q3ListViewItem	*item;
  map<Q3ListViewItem *, ItemType>::iterator	it, ft=_itemTypes.end();

  for( item = parent->firstChild(), i=0; i<n; ++i, item=item->nextSibling() )
    {
      it = _itemTypes.find( item );
      if( it != ft && ff == item->text( 0 ).utf8().data() )
	return( item );
    }
  return( 0 );
}


Q3ListViewItem* QObjectBrowserWidget::itemFor( Q3ListViewItem* parent, 
					      const string & ff )
{
  unsigned	n = parent->childCount(), i;
  Q3ListViewItem	*item;

  for( item = parent->firstChild(), i=0; i<n; ++i, item=item->nextSibling() )
    {
      if( ff == item->text( 0 ).utf8().data() )
	return( item );
    }
  return( 0 );
}


void QObjectBrowserWidget::whatIs( Q3ListViewItem* item, 
				   ItemDescr & descr ) const
{
  map<Q3ListViewItem *, AObject *>::const_iterator	io, fo=_aobjects.end();
  map<Q3ListViewItem *, GenericObject *>::const_iterator 
    ig, fg=_gobjects.end();
  AttributedAObject	*aao;
  shared_ptr<AObject>	obj;

  descr.type = typeOf( item );
  // cout << "Browser::whatIs: type: " << descr.type << endl;

  switch( descr.type )
    {
    case AOBJECT:
      io = _aobjects.find( item );
      assert( io != fo );
      descr.obj = (*io).second;
      descr.att = "";
      aao = dynamic_cast<AttributedAObject *>( (*io).second );
      if( aao )
	descr.ao = aao->attributed();
      else
	descr.ao = 0;
      break;
    case GOBJECT:
      ig = _gobjects.find( item );
      assert( ig != fg );
      descr.ao = (*ig).second;
      if( (*ig).second->getProperty( "ana_object", obj ) )
	descr.obj = obj.get();
      else
	descr.obj = 0;
      descr.att = "";
      break;
    case ATTRIBUTE:
      descr.att = item->text( 0 ).utf8().data();
      descr.ao = 0;
      descr.obj = 0;
      // cout << "attribute: " << descr.att << endl;
      break;
    default:
      // cout << "unknonw descr.type " << descr.type << endl;
      descr.att = "";
      descr.ao = 0;
      descr.obj = 0;
      break;
    }

  Q3ListViewItem	*par = item->parent();
  ItemType	t = par ? typeOf( par ) : UNKNOWN;

  while( par && ( t == UNKNOWN || t == OTHER ) )
    {	// find next significant parent (skip decorative items)
      par = par->parent();
      if( par )
	t = typeOf( par );
    }

  if( !par )
    {
      // cout << "no parent\n";
      descr.ptype = OTHER;
      descr.pao = 0;
      descr.pobj = 0;
      descr.ttype = OTHER;
      descr.tao = 0;
      descr.tobj = 0;
      return;
    }
  // else cout << "parent type: " << t << endl;

  descr.ptype = t;
  switch( t )
    {
    case AOBJECT:
      io = _aobjects.find( par );
      assert( io != fo );
      descr.pobj = (*io).second;
      aao = dynamic_cast<AttributedAObject *>( (*io).second );
      if( aao )
	descr.pao = aao->attributed();
      else
	descr.pao = 0;
      break;
    case GOBJECT:
      //cout << "graph object\n";
      ig = _gobjects.find( par );
      assert( ig != fg );
      descr.pao = (*ig).second;
      if( (*ig).second->getProperty( "ana_object", obj ) )
	descr.pobj = obj.get();
      else
	descr.pobj = 0;
      //cout << "ana_object: " << obj << endl;
      break;
    case ATTRIBUTE:
      descr.pao = 0;
      descr.pobj = 0;
      break;
    default:
      // cout << "unknonw descr.ptype " << descr.ptype << endl;
      descr.pao = 0;
      descr.pobj = 0;
      break;
    }

  Q3ListViewItem	*par2;

  // find top-level parent item
  // cout << "par: " << par << endl;
  for( par2=par->parent(); par2; par=par2, par2=par2->parent() ) {}
  // cout << "ancestor: " << par << endl;
  descr.ttype = typeOf( par );
  // cout << "ancestor type: " << descr.ttype << endl;
  if( descr.ttype != AOBJECT )	// top-level should be AObjects
    {
      // cout << "not an AObject\n";
      descr.tobj = 0;
      descr.tao = 0;
      return;
    }
  else
    {
      io = _aobjects.find( par );
      if( io == fo )
        {
          // cout << "AObject not found\n";
          descr.tobj = 0;
          descr.tao = 0;
          return;
        }
    }
  // cout << "descr.tobj: " << io->second << endl;
  descr.tobj = (*io).second;
  aao = dynamic_cast<AttributedAObject *>( (*io).second );
  if( aao )
    descr.tao = aao->attributed();
  else
    descr.tao = 0;
  // cout << "attributed: " << descr.tao << endl;
}


void QObjectBrowserWidget::keyPressEvent( QKeyEvent* ev )
{
  if( ev->key() == Key_Delete || ev->key() == Key_A )
    {
      // delegate del and A (ctrl-A actually) to the Control system
      ev->ignore();
      return;
    }
  QAListView::keyPressEvent( ev );
  /* if( ev->key() == Key_A && ev->state() == ControlButton )
     ev->accept(); */
}


