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


#include <cstdlib>
#include <anatomist/application/settings.h>
#include <anatomist/application/Anatomist.h>
#include <cartobase/config/paths.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/stream/directory.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


string Settings::globalPath()
{
  string result;
#ifdef USE_SHARE_CONFIG
  const char *path = getenv( "ANATOMIST_PATH" );
  char	s = FileUtil::separator();
  Directory	d( "/" );

  if ( path ) {
    result = string( path );
    d.chdir( result );
    if( ! d.isValid() ) {
      result.clear();
    }
  }
  if( result.empty() ) {
    result = Paths::globalShared() + FileUtil::separator() + "anatomist-"
        + theAnatomist->libraryVersionString();
    d.chdir( result );
    if( !d.isValid() ) {
      result = Paths::globalShared() + FileUtil::separator()
               + "anatomist";
    }
  }
#else // #ifdef USE_SHARE_CONFIG
  const char *path = getenv( "ANATOMIST_PATH" );
  char	s = FileUtil::separator();
  Directory	d( "/" );

  if ( path )
    {
      result = string( path ) + s + "shared";
      d.chdir( result );
      if( !d.isValid() )
        {
          result = string( path );
          d.chdir( result );
          if( !d.isValid() )
            result = "";
        }
    }
  if( result.empty() )
    {
      result = Paths::globalShared() + FileUtil::separator() + "anatomist-"
        + theAnatomist->libraryVersionString();
      d.chdir( result );
      if( !d.isValid() )
        {
          result = Paths::globalShared() + FileUtil::separator()
              + "anatomist-main";
          d.chdir( result );
          if( !d.isValid() )
            result = Paths::globalShared() + FileUtil::separator()
                + "anatomist";
        }
    }
#endif // #ifdef USE_SHARE_CONFIG
  return result;
}


string Settings::localPath()
{
  string	p = userProfile();
  if( p.empty() )
    return Paths::home() + FileUtil::separator() + ".anatomist";
  else
    return Paths::home() + FileUtil::separator() + ".anatomist"
      + "-" + p;
}


string Settings::docPath()
{
  char	s = FileUtil::separator();
  string p = Paths::globalShared() + s + "doc" + s + "anatomist";
  string p2 = p + '-' + theAnatomist->libraryVersionString();
  Directory d( p2 );
  if( d.isValid() )
    return p2;
  d.chdir( p );
  if( d.isValid() )
    return p;
  return globalPath() + s + "doc";
}


string & Settings::userProfile()
{
  static string	p;
  return p;
}


void Settings::setUserProfile( const string & p )
{
  userProfile() = p;
}


string Settings::findResourceFile( const string & filename )
{
  return Paths::findResourceFile( filename, "anatomist",
    theAnatomist->libraryVersionString() );
}


