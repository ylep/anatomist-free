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


#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#include <anatomist/surface/fusiontexsurf.h>
#include <anatomist/fusion/fusionChooser.h>


using namespace anatomist;
using namespace std;


FusionFactory		*FusionFactory::_theFactory = 0;
set<FusionMethod *>	FusionFactory::_methods;

static FusionFactory	*_theFusionFactory = new FusionFactory;



FusionMethod::~FusionMethod()
{
}


bool FusionMethod::orderingMatters() const
{
  return true;
}


//

FusionFactory::FusionFactory()
{
  delete _theFactory;
  _theFactory = this;

  if( _methods.empty() )	// base methods not registered yet ?
    {
      registerMethod( new Fusion2dMethod );
      registerMethod( new Fusion3dMethod );
      registerMethod( new PlanarFusion3dMethod );
      registerMethod( new FusionTexSurfMethod );
      registerMethod( new FusionTextureMethod );
      registerMethod( new FusionMultiTextureMethod );
      registerMethod( new FusionCutMeshMethod );
      registerMethod( new FusionSliceMethod );
      registerMethod( new FusionRGBAVolumeMethod );
    }
}


FusionFactory::~FusionFactory()
{
  _theFactory = 0;

  set<FusionMethod *>::iterator	im, em = _methods.end();
  for( im=_methods.begin(); im!=em; ++im )
    delete *im;
}


bool FusionFactory::registerMethod( FusionMethod* method )
{
  set<FusionMethod *>::const_iterator	im, fm=_methods.end();
  string	id = method->ID();

  for( im=_methods.begin(); im!=fm; ++im )
    if( (*im)->ID() == id )
      return( false );	// already registered

  _methods.insert( method );
  return( true );
}


bool FusionFactory::canFusion( const set<AObject *> & objects )
{
  set<FusionMethod *>::const_iterator	im, fm=_methods.end();

  for( im=_methods.begin(); im!=fm; ++im )
    if( (*im)->canFusion( objects ) )
      return( true );
  return( false );
}


FusionMethod* FusionFactory::chooseMethod( const set<AObject *> & objects )
{
  vector<AObject *>	obj;
  obj.reserve( objects.size() );
  set<AObject *>::const_iterator	io, eo = objects.end();
  for( io=objects.begin(); io!=eo; ++io )
    obj.push_back( *io );
  return chooseMethod( obj, true );
}


FusionMethod* FusionFactory::chooseMethod( vector<AObject *> & objects, 
                                           bool ordering )
{
  set<FusionMethod *>			sm;
  set<FusionMethod *>::const_iterator	im, fm=_methods.end();
  set<AObject *>	objs;

  objs.insert( objects.begin(), objects.end() );
  for( im=_methods.begin(); im!=fm; ++im )
    if( (*im)->canFusion( objs ) )
      {
        sm.insert( *im );
        ordering |= (*im)->orderingMatters();
      }

  if( sm.empty() )
    return 0;	// not found

  if( sm.size() == 1 && ( !ordering || objects.size() == 1 ) )
    return( *sm.begin() );

  vector<AObject *>	*obj = 0;
  if( ordering )
  {
    if( objects.size() != 1 )
      obj = &objects;
    else
      ordering = false;
  }

  // several fusions avalable
  FusionChooser	fc( sm, 0, 0, true, 0, obj );
  if( !fc.exec() )
    return( 0 );	// cancelled

  if( ordering )
    // reorder objects
    objects = fc.objects();
  return fc.selectedMethod();
}


FusionMethod* FusionFactory::method( const string & name ) const
{
  set<FusionMethod *>::const_iterator	im, em = _methods.end();

  for( im=_methods.begin(); im!=em && (*im)->ID()!=name; ++im ) {}
  if( im != em )
    return( *im );
  else
    return( 0 );
}
