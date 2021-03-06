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

#include <anatomist/volume/slice.h>
#include <anatomist/object/actions.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <anatomist/surface/glcomponent.h>
#include <aims/resampling/quaternion.h>
#include <anatomist/window/viewstate.h>
#include <qpixmap.h>
#include <graph/tree/tree.h>
#include <qtranslator.h>

using namespace anatomist;
using namespace carto;
using namespace std;


int Slice::registerClass()
{
  int	type = registerObjectType( "Slice" );
  return type;
}


Slice::Slice( const vector<AObject *> & obj ) 
  : ObjectVector(), SelfSliceable()
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
    {
      string str = Settings::findResourceFile( "icons/list_slice.xpm" );
      if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
      {
        QObjectTree::TypeIcons.erase( _type );
        cerr << "Icon " << str.c_str() << " not found\n";
      }

      QObjectTree::TypeNames[ _type ] = "Slice";
    }

  vector<AObject *>::const_iterator	io, fo=obj.end();
  vector<AObject *>			vol;
  AObject				*o = 0;
  GLComponent				*c;

  for( io=obj.begin(); io!=fo; ++io )
    if( ( c = (*io)->glAPI() ) && c->sliceableAPI() )
    {
      o = *io;
      insert( o );
      break;
    }

  if( size() > 0 )
  {
    o = *begin();
    setReferentialInheritance( o );
    cout << "init ref: " << o->getReferential() << endl;

    vector<float> bmin, bmax;
    o->boundingBox2D( bmin, bmax );
    _offset = Point3df( ( bmin[0] + bmax[0] ) / 2,
                        ( bmin[1] + bmax[1] ) / 2,
                        ( bmin[2] + bmax[2] ) / 2 );
  }
}


Slice::~Slice()
{
  iterator	i = begin();
  erase( i );
}


int Slice::classType()
{
  static int	_classType = registerClass();
  return _classType;
}


const AObject* Slice::volume() const
{
  return *begin();
}


AObject* Slice::volume()
{
  return *begin();
}


bool Slice::render( PrimList & prim, const ViewState & state )
{
  // cout << "Slice::render " << quaternion().vector() << endl;
  AObject	*obj = volume();
  aims::Quaternion    q = quaternion();
  Geometry	geom = AWindow3D::setupWindowGeometry( _data, q );
  bool firstlist = false;
  PrimList::iterator ip = prim.end();
  if( ip == prim.begin() )
    firstlist = true;
  else
    --ip;
  SliceViewState  svs( state.timedims, true, offset(), &q,
                       obj->getReferential(), &geom,
                       state.sliceVS() ? state.sliceVS()->vieworientation : 0,
                       state.window );
  if( volume()->render( prim, svs ) )
  {
    const Referential *ref = getReferential();
    GLPrimitives p2 = GLComponent::glHandleTransformation( state, ref );
    bool hastr = !p2.empty();
    if( hastr )
    {
      if( !firstlist )
        ++ip;
      else
        ip = prim.begin();
      prim.insert( ip, p2.begin(), p2.end() );
      p2 = GLComponent::glPopTransformation( state, ref );
      prim.insert( prim.end(), p2.begin(), p2.end() );
    }
    return true;
  }
  return false;
}


Tree* Slice::optionTree() const
{
  static Tree*	_optionTree = 0;

  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ) );
      t2->setProperty( "callback", &ObjectActions::saveStatic );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
                                              "Rename object" ) );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );
    }
  return( _optionTree );
}


Material & Slice::GetMaterial()
{
  return( (*begin())->GetMaterial() );
}


void Slice::SetMaterial( const Material & mat )
{
  (*begin())->SetMaterial( mat );
}


const AObjectPalette* Slice::palette() const
{
  const AObject	*functional = volume();
  return( functional->palette() );
}


AObjectPalette* Slice::palette()
{
  AObject	*functional = volume();
  return( functional->palette() );
}


void Slice::setPalette( const AObjectPalette & palette )
{
  AObject	*functional = volume();
  functional->setPalette( palette );
  setChanged();
}


void Slice::sliceChanged()
{
  volume()->setChanged();
  setChanged();
}


void Slice::update( const Observable *observable, void * )
{
  // cout << "Slice::update\n";

  if( observable == volume() )
  {
    // cout << "vol: " << observable << " / " << volume() << endl;
    const AObject *obj = static_cast<const AObject *>( observable );
    // const GLComponent *g = glAPI();
    // cout << "obj: " << obj << ", g: " << g << endl;

    if( obj->obsHasChanged( GLComponent::glTEXIMAGE ) )
    {
      setChanged();
      // g->glSetTexImageChanged( true, 0 );
    }
    if( obj->obsHasChanged( GLComponent::glTEXENV ) )
    {
      setChanged();
      // g->glSetTexEnvChanged( true, 0 );
    }
    if( obj->obsHasChanged( GLComponent::glREFERENTIAL )
        || obj->obsHasChanged( GLComponent::glBODY ) )
    {
      setChanged();
      // g->glSetChanged( GLComponent::glBODY );
    }
    if( obj->obsHasChanged( GLComponent::glMATERIAL ) )
    {
      setChanged();
      // g->glSetChanged( GLComponent::glMATERIAL );
    }
    if( obj->obsHasChanged( GLComponent::glREFERENTIAL )
        || obj->obsHasChanged( GLComponent::glGEOMETRY ) )
    {
      setChanged();
      // g->glSetChanged( GLComponent::glGEOMETRY );
    }
    updateSubObjectReferential( obj );
    AObject::update( observable, 0 );
  }

  notifyObservers( this );
}


Object Slice::makeHeaderOptions() const
{
  Object opts = ObjectVector::makeHeaderOptions();
  makeSliceHeaderOptions( opts );
  return opts;
}


void Slice::setProperties( Object options )
{
  ObjectVector::setProperties( options );
  setSliceProperties( options );
}

