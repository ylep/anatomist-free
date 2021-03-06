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

#include <anatomist/commands/cCamera.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <fstream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


CameraCommand::CameraCommand( const std::set<AWindow *> & win, 
			      const Point3df *observerpos, 
			      const Quaternion* q, float zoom, 
			      const Quaternion* sliceq, bool forcedraw, 
			      const Point4df* curpos, const Point3df* bbmin, 
                              const Point3df* bbmax,
                              const Point3df* sliceplane )
  : RegularCommand(), _win( win ), _zoom( zoom ), _forcedraw( forcedraw )
{
  if( observerpos )
    {
      _observerpos = *observerpos;
      _obspos = true;
    }
  else
    _obspos = false;

  if( q )
    {
      _viewq = true;
      _viewquat = *q;
    }
  else
    _viewq = false;

  if( sliceq )
    {
      _sliceq = true;
      _slicequat = *sliceq;
    }
  else
    _sliceq = false;
  if( curpos )
    {
      _curpos = true;
      _cursorpos = *curpos;
    }
  else
    _curpos = false;
  if( bbmin )
    {
      _bbmin = *bbmin;
      _bbminset = true;
    }
  else
    _bbminset = false;
  if( bbmax )
    {
      _bbmax = *bbmax;
      _bbmaxset = true;
    }
  else
    _bbmaxset = false;
  if( sliceplane )
  {
    _sliceplane = *sliceplane;
    _hassliceplane = true;
  }
  else
    _hassliceplane = false;
}


CameraCommand::~CameraCommand()
{
}


bool CameraCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "Camera" ];
  
  s[ "windows"           ] = Semantic( "int_vector", true );
  s[ "observer_position" ] = Semantic( "float_vector", false );
  s[ "view_quaternion"   ] = Semantic( "float_vector", false );
  s[ "slice_quaternion"  ] = Semantic( "float_vector", false );
  s[ "zoom"              ].type = "float";
  s[ "force_redraw"      ].type = "int";
  s[ "cursor_position"   ].type = "float_vector";
  s[ "boundingbox_min"   ].type = "float_vector";
  s[ "boundingbox_max"   ].type = "float_vector";
  s[ "slice_orientation" ] = Semantic( "float_vector", false );
  Registry::instance()->add( "Camera", &read, ss );
  return( true );
}


void
CameraCommand::doit()
{
  // cout << "Camera::doit\n";
  set<AWindow *>::const_iterator	iw, ew = _win.end();
  AWindow3D				*w;
  bool					fullrefresh = false;
  for( iw=_win.begin(); iw!=ew; ++iw )
    if( (w = dynamic_cast<AWindow3D *>( *iw ) ) )
    {
      if( w->needsRedraw() )
        fullrefresh = true;
      if( _sliceq )
      {
        w->setSliceQuaternion( _slicequat );
        fullrefresh = true;
      }
      if( _hassliceplane )
      {
        w->setSliceOrientation( _sliceplane );
        fullrefresh = true;
      }
      if( _curpos )
      {
        Point3df	oldpos = w->getPosition();
        Point3df	pos( _cursorpos[0], _cursorpos[1], _cursorpos[2] );
        if( oldpos != pos || _cursorpos[3] != w->getTime() )
        {
          w->setPosition( pos, 0 );
          w->setTime( _cursorpos[3] );
          fullrefresh = true;
        }
      }
      GLWidgetManager	*v = dynamic_cast<GLWidgetManager *>( w->view() );
      if( v )
      {
        if( _obspos )
          v->setRotationCenter( _observerpos );
        if( _viewq )
          v->setQuaternion( _viewquat );
        if( _zoom > 0 )
          v->setZoom( _zoom );
        if( _bbminset || _bbmaxset )
        {
          Point3df	bbmin, bbmax;
          if( _bbminset )
            bbmin = _bbmin;
          else
            bbmin = v->windowBoundingMin();
          if( _bbmaxset )
            bbmax = _bbmax;
          else
            bbmax = v->windowBoundingMax();
          v->setWindowExtrema( bbmin, bbmax );
        }
        if( !fullrefresh )
          w->refreshLightViewNow();
      }
      if( fullrefresh )
      {
        if( _forcedraw )
          w->refreshNow();
        else
          w->Refresh();
      }
    }
}


Command* CameraCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		win;
  float			zoom = 0;
  vector<float>		pos, vq, sq, cpos, bbmin, bbmax, so;
  int			forcedraw = 0;
  bool			hasbbmin, hasbbmax;

  if( !com.getProperty( "windows", win ) )
    return( 0 );
  com.getProperty( "observer_position", pos );
  com.getProperty( "cursor_position", cpos );
  com.getProperty( "zoom", zoom );
  com.getProperty( "view_quaternion", vq );
  com.getProperty( "slice_quaternion", sq );
  com.getProperty( "slice_orientation", so );
  com.getProperty( "force_redraw", forcedraw );
  hasbbmin = com.getProperty( "boundingbox_min", bbmin );
  hasbbmax = com.getProperty( "boundingbox_max", bbmax );

  set<AWindow *>	wins;
  unsigned		i, n = win.size();
  void			*ptr;

  for( i=0; i<n; ++i )
    {
      ptr = context->unserial->pointer( win[i], "AWindow" );
      if( ptr )
	wins.insert( (AWindow *) ptr );
      else
	cerr << "window id " << win[i] << " not found\n";
    }

  Point3df	p, bbminf, bbmaxf;
  Point4df	cp;
  if( pos.size() >= 3 )
    p = Point3df( pos[0], pos[1], pos[2] );
  if( cpos.size() >= 3 )
    {
      cp = Point4df( cpos[0], cpos[1], cpos[2], 0 );
      if( cpos.size() >= 4 )
	cp[3] = cpos[3];
      else
	cp[3] = 0;
    }
  Quaternion	viewq, sliceq;
  if( vq.size() >= 4 )
    viewq = Quaternion( vq[0], vq[1], vq[2], vq[3] );
  if( sq.size() >= 4 )
    sliceq = Quaternion( sq[0], sq[1], sq[2], sq[3] );
  if( hasbbmin )
    bbminf = Point3df( bbmin[0], bbmin[1], bbmin[2] );
  if( hasbbmax )
    bbmaxf = Point3df( bbmax[0], bbmax[1], bbmax[2] );
  Point3df sliceo;
  if( so.size() >= 3 )
    sliceo = Point3df( so[0], so[1], so[2] );
  return( new CameraCommand( wins, pos.size() >= 3 ? &p : 0, 
                             vq.size() >=4  ? &viewq : 0, zoom,
                             sq.size() >= 4 ? &sliceq : 0,
                             (bool) forcedraw, cpos.size() >=3 ? &cp : 0,
                             hasbbmin ? &bbminf : 0, hasbbmax ? &bbmaxf : 0,
                             so.size() >= 3 ? &sliceo : 0 ) );
}


void CameraCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  set<AWindow *>::iterator	iw;
  vector<int>			win;
  vector<float>			pos, vq, sq;

  for( iw=_win.begin(); iw!=_win.end(); ++iw )
    win.push_back( ser->serialize( *iw ) );

  t->setProperty( "windows", win );
  if( _zoom > 0 )
    t->setProperty( "zoom", _zoom );
  if( _obspos )
    {
      pos.push_back( _observerpos[0] );
      pos.push_back( _observerpos[1] );
      pos.push_back( _observerpos[2] );
      t->setProperty( "observer_position", pos );
    }
  if( _curpos )
    {
      pos.clear();
      pos.push_back( _cursorpos[0] );
      pos.push_back( _cursorpos[1] );
      pos.push_back( _cursorpos[2] );
      t->setProperty( "cursor_position", pos );
    }
  if( _viewq )
    {
      Point4df	v = _viewquat.vector();
      vq.push_back( v[0] );
      vq.push_back( v[1] );
      vq.push_back( v[2] );
      vq.push_back( v[3] );
      t->setProperty( "view_quaternion", vq );
    }
  if( _sliceq )
    {
      Point4df	v = _slicequat.vector();
      sq.push_back( v[0] );
      sq.push_back( v[1] );
      sq.push_back( v[2] );
      sq.push_back( v[3] );
      t->setProperty( "slice_quaternion", sq );
    }
  if( _hassliceplane )
  {
    vector<float> so( 3 );
    so[0] = _sliceplane[0];
    so[1] = _sliceplane[1];
    so[2] = _sliceplane[2];
    t->setProperty( "slice_orientation", so );
  }
  if( _forcedraw )
    t->setProperty( "force_redraw", (int) 1 );
  if( _bbminset )
    {
      pos.clear();
      pos.push_back( _bbmin[0] );
      pos.push_back( _bbmin[1] );
      pos.push_back( _bbmin[2] );
      t->setProperty( "boundingbox_min", pos );
    }
  if( _bbmaxset )
    {
      pos.clear();
      pos.push_back( _bbmax[0] );
      pos.push_back( _bbmax[1] );
      pos.push_back( _bbmax[2] );
      t->setProperty( "boundingbox_max", pos );
    }

  com.insert( t );
}


