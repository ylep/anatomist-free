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


#include <anatomist/control/windowdrag.h>
#include <string>

using namespace anatomist;
using namespace std;


QAWindowDrag::QAWindowDrag( const set<AWindow *> & o, QWidget * dragSource, 
			    const char * name )
  : QStoredDrag( "AWindow", dragSource, name ) //, _objects( o )
{
  // cout << "QAWindowDrag::QAWindowDrag\n";

  QByteArray	ba;

  ba.resize( sizeof( AWindow * ) * o.size() + sizeof( unsigned ) );
#if QT_VERSION >= 0x040000
  QDataStream	ds( &ba, IO_ReadWrite );
#else
  QDataStream	ds( ba, IO_ReadWrite );
#endif
  ds << (unsigned) o.size();

  set<AWindow *>::const_iterator	io, eo=o.end();
  for( io=o.begin(); io!=eo; ++io )
    {
      // cout << "obj " << *io << endl;
      ds.writeRawBytes( (const char *) &*io, sizeof( AWindow * ) );
    }
  setEncodedData( ba );
}


QAWindowDrag::~QAWindowDrag()
{
  //cout << "QAWindowDrag::~QAWindowDrag\n";
}


bool QAWindowDrag::canDecode( const QMimeSource * e )
{
  // cout << "QAWindowDrag::canDecode\n";
  if( string( e->format() ) == "AWindow" )
    return( true );
  return( false );
}


bool QAWindowDrag::decode( const QMimeSource * e, set<AWindow*> & o )
{
  // cout << "QAWindowDrag::decode\n";
  if( string( e->format() ) != "AWindow" )
    {
      // cout << "Wrong format: " << e->format() << endl;
      return false;
    }
  QByteArray	ba = e->encodedData( e->format() );
#if QT_VERSION >= 0x040000
  QDataStream	s( &ba, IO_ReadOnly );
#else
  QDataStream	s( ba, IO_ReadOnly );
#endif
  unsigned	i, n;
  AWindow	*ao;
  s >> n;
  for( i=0; i<n; ++i )
    {
      s.readRawBytes( (char *) &ao, sizeof( AWindow * ) );
      // cout << ao << endl;
      o.insert( ao );
    }
  return( true );
}


