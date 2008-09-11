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


#include <anatomist/object/oReader.h>
#include <aims/io/process.h>
#include <aims/io/reader.h>
#include <aims/io/finder.h>
#include <aims/mesh/texture.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/texture.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/object/actions.h>
#include <aims/data/pheader.h>
#include <aims/utility/converter_texture.h>
#include <aims/def/path.h>
#include <aims/io/fileFormat.h>
#include <aims/io/readerasobject.h>
#include <cartobase/stream/fileutil.h>
#include <time.h>
#include <zlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


//	default reader


ObjectReader::ObjectReader()
{
  delete _theReader;
  _theReader = this;

  //registerLoader( "arg", readGraph );
}


ObjectReader::~ObjectReader()
{
  _theReader = 0;
}


namespace
{

  template<class T> 
  bool loadVolume( Process & p, const string & fname, Finder & f );
  template<class T> 
  bool loadData( T & obj, const string & fname, Finder & f );
  template<long D>
  bool loadMesh( Process & p, const string & fname, Finder & f );
  bool loadTex1d( Process & p, const string & fname, Finder & f );
  bool loadTex2d( Process & p, const string & fname, Finder & f );
  template<typename T>
  bool loadTex1dInt( Process & p, const string & fname, Finder & f );
  template<typename T>
  bool loadTex2dInt( Process & p, const string & fname, Finder & f );
  bool loadBucket( Process & p, const string & fname, Finder & f );
  bool loadGraph( Process & p, const string & fname, Finder & f );


  class AimsLoader : public Process
  {
  public:
    AimsLoader( Object opts ) : Process(), object( 0 ), options( opts )
    {
      Object	restricted;
      bool	restr = false;
      if( !options.isNull() )
        try
          {
            restricted = options->getProperty( "restrict_object_types" );
            restr = true;
          }
        catch( ... )
          {
          }

      bool	r2 = false;
      Object	rtypes;

      set<string>			types;
      set<string>::const_iterator	eir = types.end();
      if( restr )
        try
          {
            rtypes = restricted->getProperty( "Volume" );
            Object	i = rtypes->objectIterator();
            for( ; i->isValid(); i->next() )
              types.insert( i->currentValue()->getString() );
            r2 = false;
          }
        catch( ... )
          {
            r2 = true;
          }
      bool	vols16 = false;
      if( !r2 )
        {
          if( !restr || types.find( "S8" ) != eir )
            registerProcessType( "Volume", "S8", &loadVolume<int8_t> );
          if( !restr || types.find( "U8" ) != eir )
            registerProcessType( "Volume", "U8", &loadVolume<uint8_t> );
          if( !restr || types.find( "S16" ) != eir )
            {
              registerProcessType( "Volume", "S16", &loadVolume<int16_t> );
              vols16 = true;
            }
          if( !restr || types.find( "U16" ) != eir )
            registerProcessType( "Volume", "U16", &loadVolume<uint16_t> );
          if( !restr || types.find( "S32" ) != eir )
            registerProcessType( "Volume", "S32", &loadVolume<int32_t> );
          if( !restr || types.find( "U32" ) != eir )
            registerProcessType( "Volume", "U32", &loadVolume<uint32_t> );
          if( !restr || types.find( "FLOAT" ) != eir )
            registerProcessType( "Volume", "FLOAT", &loadVolume<float> );
          if( !restr || types.find( "DOUBLE" ) != eir )
            registerProcessType( "Volume", "DOUBLE", &loadVolume<double> );
          if( !restr || types.find( "RGB" ) != eir )
            registerProcessType( "Volume", "RGB", &loadVolume<AimsRGB> );
          if( !restr || types.find( "RGBA" ) != eir )
            registerProcessType( "Volume", "RGBA", &loadVolume<AimsRGBA> );
        }
      r2 = restr && !restricted->hasProperty( "Segments" );
      if( !r2 )
        registerProcessType( "Segments", "VOID", &loadMesh<2> );
      r2 = restr && !restricted->hasProperty( "Mesh" );
      if( !r2 )
        registerProcessType( "Mesh", "VOID", &loadMesh<3> );
      r2 = restr && !restricted->hasProperty( "Mesh4" );
      if( !r2 )
        registerProcessType( "Mesh4", "VOID", &loadMesh<4> );
      r2 = restr && !restricted->hasProperty( "Bucket" );
      if( !r2 )
        registerProcessType( "Bucket", "VOID", &loadBucket );

      if( restr )
        try
          {
            types.clear();
            rtypes = restricted->getProperty( "Texture" );
            Object	i = rtypes->objectIterator();
            for( ; i->isValid(); i->next() )
              types.insert( i->getString() );
            r2 = false;
          }
        catch( ... )
          {
            r2 = true;
          }
      else
        r2 = false;
      if( !r2 )
        {
          if( !restr || types.find( "FLOAT" ) != eir )
            registerProcessType( "Texture", "FLOAT", &loadTex1d );
          if( !restr || types.find( "POINT2DF" ) != eir )
            registerProcessType( "Texture", "POINT2DF", &loadTex2d );
          if( !restr || types.find( "S16" ) != eir )
            registerProcessType( "Texture", "S16", &loadTex1dInt<int16_t> );
          if( !restr || types.find( "S32" ) != eir )
            registerProcessType( "Texture", "S32", &loadTex1dInt<int32_t> );
          if( !restr || types.find( "U32" ) != eir )
            registerProcessType( "Texture", "U32", &loadTex1dInt<uint32_t> );
          if( !restr || types.find( "POINT2D" ) != eir )
            registerProcessType( "Texture", "POINT2D", &loadTex2dInt<short> );
        }

      r2 = restr && !restricted->hasProperty( "Graph" );
      if( !r2 )
        {
          registerProcessType( "Graph", "VOID", &loadGraph );
          registerProcessType( "Bundles", "any", &loadGraph );
          if( !vols16 )
            registerProcessType( "Volume", "S16", &loadGraph );
        }
    }

    AObject	*object;
    Object	options;
  };


  template<class T>
  bool loadVolume( Process & p, const string & fname, Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    AVolume<T>	*vol = new AVolume<T>( fname.c_str() /*, type*/ );
    if( !loadData( *vol->volume(), fname, f ) )
      {
        delete vol;
        return( false );
      }
    ap.object = vol;
    vol->setFileName( fname );
    vol->SetExtrema();
    vol->adjustPalette();
    return true;
  }

  template<class T> 
  bool loadData( T & obj, const string & fname, Finder & f )
  {
    Reader<T>	reader( fname );
    reader.setAllocatorContext( AllocatorContext
                                ( AllocatorStrategy::ReadOnly, 
                                  DataSource::none(), false, 0.5 ) );
    string	format = f.format();
    try
      {
        reader.read( obj, 0, &format );
      }
    catch( exception & e )
      {
        cerr << e.what() << endl;
        return false;
      }
    return( true );
  }

  template<long D>
  bool loadMesh( Process & p, const string & fname, Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    AimsTimeSurface<D, Void>	*surf = new AimsTimeSurface<D, Void>;
    if( !loadData( *surf, fname, f ) )
      {
        delete surf;
        return( false );
      }
    ASurface<D>	*ao = new ASurface<D>( fname.c_str() );
    ao->setSurface( surf );
    ap.object = ao;
    ao->setFileName( fname );
    string name = FileUtil::removeExtension( FileUtil::basename( fname ) );

    Object		otex;
    carto::ObjectVector	*tex = 0;
    unsigned		i, j, n, m;

    // cout << "check textures\n";
    try
      {
        otex = surf->header().getProperty( "textures" );
        tex = &otex->GenericObject::value<carto::ObjectVector>();
        // cout << "existing textures: " << tex->size() << endl;
      }
    catch( ... )
      {
      }
    try
      {
        Object	tfiles = surf->header().getProperty( "texture_filenames" );
        Object	iter;
        string	dirn = FileUtil::dirname( fname ) + FileUtil::separator();
        if( !tex )
          n = 0;
        else
          n = tex->size();
        for( i=0, iter=tfiles->objectIterator(); i<n && iter->isValid(); 
             ++i, iter->next() ) {}
        if( !tex && iter->isValid() )
          {
            otex = Object::value( ObjectVector() );
            surf->header().setProperty( "textures", otex );
            tex = &otex->GenericObject::value<ObjectVector>();
          }
        for( ; iter->isValid(); ++i, iter->next() )
          {
            string	texfname = iter->currentValue()->getString();
            if( !FileUtil::isAbsPath( texfname ) )
              texfname = dirn + texfname;
            ReaderAsObject	ord( texfname );
            try
              {
                tex->push_back( ord.read() );
              }
            catch( ... )
              {
              }
          }
        // cout << "textures loaded, now:" << tex->size() << endl;
      }
    catch( ... )
      {
      }

    if( tex && tex->size() > 0 )
      try
        {
          // cout << "textures found\n";
          vector<string>	texnames, texdisp;
          vector<Object>	otd;
          map<string, int>	texd;

          try
            {
              Object	tn = surf->header().getProperty( "texture_names" );
              Object	iter;
              texnames.reserve( tn->size() );
              for( iter=tn->objectIterator(); iter->isValid(); iter->next() )
                {
                  string	x;
                  try
                    {
                      x = iter->currentValue()->getString();
                    }
                  catch( ... )
                    {
                    }
                  texnames.push_back( x );
                }
            }
          catch( ... )
            {
            }
          n = texnames.size();
          if( surf->header().getProperty( "texture_displayed", otd ) )
            {
              m=otd.size();
              texdisp.reserve( m );
              for( i=0; i<m; ++i )
                try
                  {
                    texdisp.push_back( otd[i]->getString() );
                  }
                catch( ... )
                  {
                  }
              if( texnames.empty() )
                texnames = texdisp;
            }
          else
            texdisp = texnames;

          m = texdisp.size();
          for( i=0; i<n; ++i )
            {
              for( j=0; j<m; ++j )
                if( texnames[i] == texdisp[j] )
                  {
                    texd[ texnames[i] ] = j;
                    break;
                  }
            }

          n = tex->size();
          m = texnames.size();
          vector<AObject *>	atex;
          ATexture		*at;
          map<string, int>::const_iterator	itm, etm = texd.end();
          atex.reserve( n );
          string		tname;
          bool		toapply;

          for( i=0; i<n; ++i )
            {
              at = new ATexture;
              toapply = false;
              if( texd.empty() )
                toapply = true;
              if( i < m )
                {
                  tname = texnames[i];
                  if( texd.find( tname ) != etm )
                    toapply = true;
                  tname = string( "texture_" ) + name + '_' + tname;
                }
              else
                tname = string( "texture_" ) + name;
              if( (*tex)[i]->type() == DataTypeCode<Texture1d>::name() )
                at->setTexture( rc_ptr<Texture1d>( new Texture1d
                    ( (*tex)[i]->GenericObject::value<Texture1d>() ) ) );
              else if( (*tex)[i]->type() == DataTypeCode<Texture2d>::name() )
                at->setTexture( rc_ptr<Texture2d>( new Texture2d
                    ( (*tex)[i]->GenericObject::value<Texture2d>() ) ) );
              // else... ?
              at->normalize();
              // at->setFileName( "texture" );
              at->setHeaderOptions();
              at->setName( theAnatomist->makeObjectName( tname ) );
              if( toapply )
                {
                  theAnatomist->registerObject( at, false );
                  atex.push_back( at );
                }
              else
                theAnatomist->registerObject( at, true );
            }
          AObject		*tex = 0;
          FusionFactory	*ff = FusionFactory::factory();
          FusionMethod	*fm;
          if( n > 1 )
            {
              if( n >= 3 )
                {
                  int	ired = -1, igreen = -1, iblue = -1;
                  itm = texd.find( "red" );
                  if( itm != etm )
                    ired = itm->second;
                  itm = texd.find( "green" );
                  if( itm != etm )
                    igreen = itm->second;
                  itm = texd.find( "blue" );
                  if( itm != etm )
                    iblue = itm->second;
                  if( ired >= 0 && igreen >= 0 && iblue >= 0 )
                    {
                      const PaletteList & pall = theAnatomist->palettes();
                      rc_ptr<APalette>
                        pal = pall.find( "multitex-geom-red-mask" );
                      AObjectPalette	*opal;
                      if( pal )
                        {
                          atex[ired]->getOrCreatePalette();
                          opal = atex[ired]->palette();
                          opal->setRefPalette( pal );
                          opal->fill();
                          atex[ired]->setPalette( *opal );
                        }
                      pal = pall.find( "multitex-geom-green-mask" );
                      if( pal )
                        {
                          atex[igreen]->getOrCreatePalette();
                          opal = atex[igreen]->palette();
                          opal->setRefPalette( pal );
                          opal->fill();
                          atex[igreen]->setPalette( *opal );
                        }
                      pal = pall.find( "multitex-geom-blue-mask" );
                      if( pal )
                        {
                          atex[iblue]->getOrCreatePalette();
                          opal = atex[iblue]->palette();
                          opal->setRefPalette( pal );
                          opal->fill();
                          atex[iblue]->setPalette( *opal );
                        }
                    }
                }
              fm = ff->method( "FusionMultiTextureMethod" );
              tex = fm->fusion( atex );
              tex->setName( theAnatomist->makeObjectName
                            ( string( "mtexture_" ) + name ) );
              theAnatomist->registerObject( tex, false );
            }
          else
            tex = atex[0];
          vector<AObject *>	ts(2);
          ts[0] = ao;
          ts[1] = tex;
          fm = ff->method( "FusionTexSurfMethod" );
          AObject	*tso = fm->fusion( ts );
          tso->setName( theAnatomist->makeObjectName( name + "_texsurface" ) );
          tso->setHeaderOptions();
          theAnatomist->registerObject( tso, false );
          /* tso->setName( theAnatomist->makeObjectName
             ( string( "texsurface_" ) + name ) ); */
          surf->header().removeProperty( "textures" );
          // copy material from mesh to texsurf.
          tso->SetMaterial( ao->GetMaterial() );
          ap.object = tso;
        }
      catch( ... )
        {
        }
    return true;
  }

  bool loadTex1d( Process & p, const string & fname, 
                  Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    rc_ptr<Texture1d>	tex( new Texture1d );
    if( !loadData( *tex, fname, f ) )
      return false;
    ATexture	*ao = new ATexture;
    ao->setTexture( tex );
    ao->normalize();
    ap.object = ao;
    return( true );
  }


  template<typename T>
  bool loadTex1dInt( Process & p, const string & fname, 
                     Finder & f )
  {
    AimsLoader		& ap = (AimsLoader &) p;
    TimeTexture<T>	ts;

    if( !loadData( ts, fname, f ) )
      return false;
    rc_ptr<Texture1d>	tex( new Texture1d );

    Converter<TimeTexture<T>, Texture1d>	c;
    c.convert( ts, *tex );

    ATexture	*ao = new ATexture;
    ao->setTexture( tex );
    ao->normalize();
    ap.object = ao;
    return( true );
  }


  bool loadTex2d( Process & p, const string & fname, 
                  Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    rc_ptr<Texture2d>	tex( new Texture2d );
    if( !loadData( *tex, fname, f ) )
      return false;
    ATexture	*ao = new ATexture;
    ao->setTexture( tex );
    ao->normalize();
    ap.object = ao;
    return( true );
  }


  template<typename T>
  bool loadTex2dInt( Process & p, const string & fname, 
                     Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    TimeTexture<AimsVector<T, 2> > ts;

    if( !loadData( ts, fname, f ) )
      return( false );
    rc_ptr<Texture2d> tex( new Texture2d );
    typename TimeTexture<AimsVector<T, 2> >::const_iterator 
      it, et = ts.end();
    unsigned	i, n;

    for( it=ts.begin(); it!=et; ++it )
    {
      Texture<Point2df>	& tx = (*tex)[ it->first ];
      n = it->second.nItem();
      tx.reserve( n );
      for( i=0; i<n; ++i )
        tx.push_back( Point2df( it->second.item( i )[0],
                                it->second.item( i )[1] ) );
    }
    tex->header() = ts.header();

    ATexture	*ao = new ATexture;
    ao->setTexture( tex );
    ao->normalize();
    ap.object = ao;
    return( true );
  }


  bool loadBucket( Process & p, const string & fname, Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    Bucket		*ao = new Bucket( fname.c_str() );
    if( !loadData( ao->bucket(), fname, f ) )
      {
        delete ao;
        return( false );
      }
    ap.object = ao;
    ao->setFileName( fname );
    ao->setGeomExtrema();
    return( true );
  }


  bool loadGraph( Process & p, const string & fname, Finder & )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    AObject	*ao = AGraph::LoadGraph( fname.c_str() );
    if( ao )
      {
        ap.object = ao;
        return( true );
      }
    return( false );
  }


  bool checkCopyFile( const string & filein, const string & extension, 
                      set<string> & tempFiles, const string & fbase );
  bool readCompressed( const string & filein, const string & fileout );
  template <typename T>
  void checkFormats( const string & ext, set<string> & exts );

  bool checkCompressedFiles( const string & filename, string & newFilename, 
                             set<string> & tempFiles )
  {
    if( ( filename.length() < 3 
          || filename.substr( filename.length() - 2, 2 ) != ".Z" )
        && ( filename.length() < 4 
             || filename.substr( filename.length() - 3, 3 ) != ".gz" ) )
      return( false );	// not compressed

    string	fbase 
      = FileUtil::removeExtension( FileUtil::basename( filename ) );
    string	ext = FileUtil::extension( fbase );

    int		fd;
    newFilename = FileUtil::temporaryFile( fbase, fd );
    if( fd == -1 )
      return false;
    ::close( fd );

    if( !readCompressed( filename, newFilename ) )
      return( 0 );	// pas r�ssi �d�ompresser
    tempFiles.insert( newFilename );  // retenir d'effacer le fichier �la fin

    //	look for other files going together with the 1st compressed one
    set<string>	others;

    checkFormats<AimsData<char> >( ext, others );
    checkFormats<AimsData<byte> >( ext, others );
    checkFormats<AimsData<short> >( ext, others );
    checkFormats<AimsData<ushort> >( ext, others );
    checkFormats<AimsData<long> >( ext, others );
    checkFormats<AimsData<ulong> >( ext, others );
    checkFormats<AimsData<float> >( ext, others );
    checkFormats<AimsData<double> >( ext, others );
    checkFormats<AimsData<AimsRGB> >( ext, others );
    checkFormats<AimsData<AimsRGBA> >( ext, others );
    checkFormats<AimsSurfaceTriangle>( ext, others );
    checkFormats<BucketMap<Void> >( ext, others );
    checkFormats<Texture1d>( ext, others );
    checkFormats<Texture2d>( ext, others );
    checkFormats<TimeTexture<short> >( ext, others );
    checkFormats<TimeTexture<int> >( ext, others );
    checkFormats<TimeTexture<unsigned> >( ext, others );
    checkFormats<TimeTexture<Point2d> >( ext, others );
    checkFormats<AimsSurfaceTriangle>( ext, others );
    checkFormats<AimsTimeSurface<2,Void> >( ext, others );
    checkFormats<AimsTimeSurface<4,Void> >( ext, others );
    checkFormats<Graph>( ext, others );

    //	check extensions
    set<string>::iterator	ie, ee = others.end();

    for( ie=others.begin(); ie!=ee; ++ie )
      {
        if( *ie != ext )
          checkCopyFile( filename, string( "." ) + *ie, tempFiles, 
                         newFilename );
        checkCopyFile( filename, string( "." ) + *ie + ".minf", tempFiles, 
                       newFilename );
      }

    return true;
  }


  bool checkCopyFile( const string & filein, const string & extension, 
                      set<string> & tempFiles, const string & fbase )
  {
    string	filename = filein, file2, zext = FileUtil::extension( filein );
    string	fb = FileUtil::removeExtension( fbase ) + extension;

    if( zext == "Z" || zext == "gz" )
      filename = FileUtil::removeExtension( filename );
    filename = FileUtil::removeExtension( filename ) + extension;

    cout << "checking file " << filename << endl;

    struct stat	s;

    if( !stat( filename.c_str(), &s ) )
      {
        cout << "file found\n";

        FILE	*f = fopen( filename.c_str(), "rb" );
        FILE	*d;
        int	c;

        if( !f )
          {
            cerr << "cannot open file " << filename << endl;
            return( false );
          }

        cout << "copying file " << filename << " to " << fb << endl;

        d = fopen( fb.c_str(), "wb" );
        if( !d )
          {
            cerr << "cannot open output file\n";
            return( false );
          }

        char	buffer[ 65536 ];
        while( ( c = fread( buffer, 1, 65536, f ) ) == 65536 )
          fwrite( buffer, 1, 65536, d );
        if( c > 0 )
          fwrite( buffer, 1, c, d );
        fclose( d );
        fclose( f );

        tempFiles.insert( fb );
        return true;
      }
    else if( !stat( (file2 = filename + ".Z").c_str(), &s ) 
             || !stat( (file2 = filename + ".gz" ).c_str(), &s ) )
      {
        if( readCompressed( file2, fb ) )
          {
            tempFiles.insert( fb );
            return true;
          }
        else
          return false;
      }
    return false;
  }


  bool readCompressed( const string & filein, const string & fileout )
  {
    FILE		*f;
    bool		zformat = false;

    if( FileUtil::extension( filein ) == ".Z" )
      zformat = true;	// .Z, not .gz

    cout << "uncompressing file " << filein << " into " << fileout << endl;

    f = fopen( fileout.c_str(), "wb" );
    if( !f )
      {
        cerr << "cannot open output file\n";
        return( false );
      }

    int		c, errnum = Z_OK;

    if( zformat )
      {
        cout << "Z format\n";

        /*unsigned char	ibuf[65536], obuf[65536];
          FILE		*ist = fopen( filein, "rb" );
          int		st;
          z_stream		zstr;
          bool		initdone = false;

          if( !ist )
          {
	  cerr << "cannot open compressed " << filein << endl;
	  fclose( f );
	  remove( );
	  return( false );
          }

          zstr.total_in = 0;
          zstr.total_out = 0;
          zstr.msg = 0;
          zstr.zalloc = Z_NULL;
          zstr.zfree = Z_NULL;
          zstr.opaque = 0;
          zstr.data_type = Z_BINARY;

          while( !feof( ist ) )
          {
	  c = fread( ibuf, 1, 65536, ist );
	  if( c <= 0 )
          break;
	  cout << "read " << c << " bytes\n";
	  zstr.next_in = ibuf;
	  zstr.avail_in = c;
	  zstr.next_out = obuf;
	  zstr.avail_out = 65536;

	  if( !initdone && inflateInit( &zstr ) != Z_OK )
          {
          cerr << "uncompress init failed. I can't read .Z format\n";
          fclose( f );
          fclose( ist );
          remove( .. );
          return( false );
          }

	  do
          {
          st = inflate( &zstr, Z_SYNC_FLUSH );
          fwrite( obuf, 1, zstr.next_out - obuf, f );
          zstr.next_out = obuf;
          zstr.avail_out = 65536;
          }
	  while( ( st == Z_OK || st == Z_STREAM_END ) && zstr.avail_in > 0 );

	  inflateEnd( &zstr );

	  if( st != Z_OK || st != Z_STREAM_END )
          {
          cerr << "Error " << st << " in uncompress\n";
          fclose( f );
          fclose( ist );
          remove( .. );
          return( false );
          }
          }
          fclose( f );
          fclose( ist ); */

        // bon, j'y arrive pas, on fait simple.
        fclose( f );
        string	cmd = "uncompress -c ";
        int	err;
        cmd += filein + " > " + fileout;
        cout << "executing : " << cmd << endl;
        err = system( cmd.c_str() );

        if( err )
          {
            remove( fileout.c_str() );
            return( false );
          }
      }
    else
      {
        unsigned char		buffer[65536];
        gzFile	gf = gzopen( filein.c_str(), "rb" );
        if( !gf )
          {
            cout << "cannot open gzipped " << filein << endl;
            fclose( f );
            unlink( fileout.c_str() );
            return( false );
          }

        while( ( c = gzread( gf, buffer, 65536 ) ) == 65536 )
          {
            gzerror( gf, &errnum );
            if( errnum != Z_OK )
              break;
            fwrite( buffer, 1, 65536, f );
          }
        if( c > 0 )
          fwrite( buffer, 1, c, f );

        gzclose( gf );
        fclose( f );
        if( errnum != Z_OK && errnum != Z_STREAM_END )
          {
            cerr << "error in decompression. Not in gzip format ?\n";
            unlink( fileout.c_str() );
            return false;
          }
      }

    return true;
  }


  void delFiles( const set<string> & files )
  {
    set<string>::const_iterator	is, fs=files.end();

    for( is = files.begin(); is!=fs; ++is )
      {
        remove( (*is).c_str() );
      }
  }


  template<typename T>
  void checkFormats( const string & ext, set<string> & exts )
  {
    const map<string, list<string> >	& extf
      = FileFormatDictionary<T>::extensions();
    map<string, list<string> >::const_iterator
      ie = extf.find( ext ), ee = extf.end();
    if( ie == ee )
      return;	// ext not found for this type

    set<string>			fmts;
    set<string>::iterator	ef = fmts.end();
    list<string>::const_iterator  ief, eef = ie->second.end();

    for( ief=ie->second.begin(); ief!=eef; ++ief )
      fmts.insert( *ief );

    for( ie=extf.begin(); ie!=ee; ++ie )
      for( ief=ie->second.begin(), eef=ie->second.end(); ief!=eef; ++ief )
        if( fmts.find( *ief ) != ef )
          exts.insert( ie->first );
  }

}

AObject* ObjectReader::load_internal( const string & filename, 
                                      Object options ) const
{
  AObject       *object = 0;

  string extension = FileUtil::extension( filename );
  if( !extension.empty() )
  {
    map<string, LoadFunction>::const_iterator
        im = _loaders.find( extension );
    if( im != _loaders.end() )
      object = (*im).second( filename, options );
  }

  if( !object )
  //  extension not recognized : let's try the Aims finder
    object = ObjectReader::readAims( filename, options );

  return object;
}


AObject* ObjectReader::load( const string & filename, bool notifyFail, 
                             Object options ) const
{
  AObject       *object = load_internal( filename, options );
  if( !object )
  {
    //	Tentative de lecture des .Z et .gz

    string	newFilename;
    set<string>	tempFiles;

    if( checkCompressedFiles( filename, newFilename, tempFiles ) )
    {
      object = load_internal( newFilename, options );
      delFiles( tempFiles );
    }
  }

  if( object )
  {
    object->setFileName( filename );
    object->setName( theAnatomist->makeObjectName(
        FileUtil::basename( filename ) ) );
    object->setHeaderOptions();
    bool autoref = false;
    try
    {
      Object autorefo = theAnatomist->config()->getProperty(
          "setAutomaticReferential" );
      if( !autorefo.isNull() )
        autoref = (bool) autorefo->getScalar();
    }
    catch( ... )
    {
    }
    /* int	spmo = 0;
    theAnatomist->config()->getProperty( "useSpmOrigin", spmo );
    */
    if( autoref )
    {
      set<AObject *> so;
      so.insert( object );
      ObjectActions::setAutomaticReferential( so );
    }
    object->setLoadDate( time( 0 ) );
    object->notifyObservers( (void *) this );
  }
  else
  {
    cerr << "Could not read object in file " << filename << " !\n";
    if( notifyFail )
    {
        // warn message box...
    }
  }

  return object;
}


bool ObjectReader::reload( AObject* object, bool notifyFail, 
                           bool /* onlyoutdated TODO */ ) const
{
  string	fname = object->fileName();
  if( !object->loadable() || fname.empty() )
    return false;

  //	Tentative de lecture des .Z et .gz

  set<string>	tempFiles;

  bool  status = false;
  try
  {
    status = object->reload( fname /*, options*/ );
  }
  catch( ... )
  {
  }

  if( !status )
  {
    string        file;
    if( checkCompressedFiles( fname, file, tempFiles ) )
    {
      status = object->reload( file /*, options*/ );
      delFiles( tempFiles );
    }
  }

  object->setLoadDate( time( 0 ) );

  if( status )
    {
      GLComponent	*c = object->glAPI();
      if( c )
        {
          c->glSetChanged( GLComponent::glGEOMETRY );
          c->glSetChanged( GLComponent::glTEXIMAGE );
          c->glSetChanged( GLComponent::glTEXENV );
        }
      else
        object->setChanged();
      object->setHeaderOptions();
      object->notifyObservers( object );
    }
  else
    {
      cout << "Reload failed !\n";
      if( notifyFail )
	{
	  //  message box etc.
	}
    }

  return( status );
}


AObject* ObjectReader::readAims( const string & file, Object options ) const
{
  AimsLoader		proc( options );

  try
    {
      if( !proc.execute( file ) )
	return( 0 );
      return( proc.object );
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
  return 0;
}


AObject* ObjectReader::readGraph( const string & filename, Object /*options*/ )
{
  AObject	*graph = AGraph::LoadGraph( filename.c_str() );
  return graph;
}


ObjectReader::LoadFunction 
ObjectReader::registerLoader( const string & extension, LoadFunction newFunc )
{
  map<string, LoadFunction>::iterator	im = _loaders.find( extension );

  if( im != _loaders.end() )
    {
      LoadFunction	oldfunc = (*im).second;
      (*im).second = newFunc;
      return( oldfunc );
    }
  else
    {
      _loaders[ extension ] = newFunc;
      return( 0 );
    }
}

