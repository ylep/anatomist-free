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


#include <anatomist/mobject/globjectvector.h>

#include <anatomist/application/Anatomist.h>
#include <anatomist/object/actions.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


GLObjectVector::GLObjectVector() : GLMObject()
{
  _type = AObject::VECTOR;
}


GLObjectVector::~GLObjectVector()
{
  cleanup();
  datatype::iterator	it;

  for( it=_data.begin(); it!=_data.end(); ++it )
    {
      (*it)->UnregisterParent( this );
      (*it)->deleteObserver((Observer*)this);
      if( !(*it)->Visible() ) 
	theAnatomist->mapObject( it->get() );
    }
  _data.erase( _data.begin(), _data.end() );
}


MObject::const_iterator GLObjectVector::find( const AObject *obj ) const
{
  using carto::shared_ptr;

  return find( shared_ptr<AObject>( shared_ptr<AObject>::Weak,
               const_cast<AObject *>( obj ) ) );
}


MObject::const_iterator
GLObjectVector::find( const carto::shared_ptr<AObject> & obj ) const
{
  datatype::const_iterator i;

  i = ::find( _data.begin(), _data.end(), obj );
  if( i!=_data.end() ) return iterator( new const_ObjectVectorIterator( i ) );
  else return end();
}


void GLObjectVector::erase( MObject::iterator & i )
{
  //	ajouter un test du type d'iterateur avant le casting brutal
  datatype::iterator & it = ((ObjectVectorIterator&)i.subIterator())._dataIt;
  (*it)->UnregisterParent( this );
  (*it)->deleteObserver((Observer*)this);
  _eraseObject( it->get() );
  _data.erase( it );
  _contentHasChanged = true;
  setChanged();
}


void GLObjectVector::insert( AObject* x, int pos )
{
  using carto::shared_ptr;

  insert( shared_ptr<AObject>( shared_ptr<AObject>::WeakShared, x ), pos );
}


void GLObjectVector::insert( const carto::shared_ptr<AObject> & x, int pos )
{
  if( pos < 0 )
    pos = _data.size() + pos + 1;
  if( pos < 0 )
    pos = 0;
  if( pos >= (int) _data.size() )
  {
    _data.push_back( x );
    _insertObject( x.get() );
    return;
  }

  datatype::iterator	io = _data.begin();
  int	i;

  for( i=0; i<pos; ++i, ++io ) {}
  _data.insert( io, x );
  _insertObject( x.get() );
}


bool GLObjectVector::CanRemove( AObject * obj )
{
  using carto::shared_ptr;

  iterator  it
      = find( shared_ptr<AObject>( shared_ptr<AObject>::Weak, obj ) );
  if( it == end() )
    return true;  // not mine: so I don't care if you remove it...
  datatype::iterator & vit = ((ObjectVectorIterator&)it.subIterator())._dataIt;
  return vit->referenceType() != shared_ptr<AObject>::Strong;
}


