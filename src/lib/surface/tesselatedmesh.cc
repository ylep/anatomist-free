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

#include <anatomist/surface/tesselatedmesh.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/reference/Referential.h>
#include <graph/tree/tree.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/fusion/defFusionMethods.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/object/actions.h>
#include <aims/mesh/surfaceOperation.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <qpixmap.h>
#include <qtranslator.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


int TesselatedMesh::registerClass()
{
  int   type = registerObjectType( "TesselatedMesh" );
  return type;
}


struct TesselatedMesh::Private
{
  Private() : polychanged( true ), tesselator( 0 ), polytess( 0 )
  {}

  bool  polychanged;
  GLUtesselator *tesselator;
  GLUtesselator *polytess;
  list<Point3dd> tmppts;
};


TesselatedMesh::TesselatedMesh( const vector<AObject *> & obj )
  : GLObjectVector(), d( new TesselatedMesh::Private )
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::globalPath() + "/icons/list_tesselatedmesh.xpm";
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str.c_str() << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "Tesselated Mesh";
  }

  vector<AObject *>::const_iterator     io, fo=obj.end();
  ViewState vs;
  ATriangulated *surf = new ATriangulated;
  surf->setName( theAnatomist->makeObjectName( "TessSubMesh" ) );
  theAnatomist->registerObject( surf, false );
  surf->setSurface( new AimsSurfaceTriangle );
  insert( surf );
  AObject       *s = 0;

  for( io=obj.begin(); io!=fo; ++io )
  {
    GLComponent *gc = (*io)->glAPI();
    if( gc )
    {
      if( !s )
        s = *io;
      if( gc->glPolygonSize( vs ) == 2 )
        insert( *io );
    }
  }

  if( s )
  {
    setReferentialInheritance( s );
    surf->setReferentialInheritance( s );
    surf->SetMaterial( s->GetMaterial() );
  }
}


TesselatedMesh::~TesselatedMesh()
{
  if( d->tesselator )
  {
    gluDeleteTess( d->polytess );
    gluDeleteTess( d->tesselator );
  }
  iterator      i = begin();
  AObject *o = *i;
  erase( i );
  if( o->Parents().empty() )
    delete o;
  delete d;
}


int TesselatedMesh::classType()
{
  static int    _classType = registerClass();
  return _classType;
}


const AObject* TesselatedMesh::tesselatedMesh() const
{
  return *begin();
}


AObject* TesselatedMesh::tesselatedMesh()
{
  return *begin();
}


const AObject* TesselatedMesh::firstPolygon() const
{
  const_iterator i = begin();
  ++i;
  return *i;
}


AObject* TesselatedMesh::firstPolygon()
{
  iterator i = begin();
  ++i;
  return *i;
}


GLComponent* TesselatedMesh::glGeometry()
{
  return glAPI();
}


GLComponent* TesselatedMesh::glTexture( unsigned )
{
  return tesselatedMesh()->glAPI();
}


const GLComponent* TesselatedMesh::glGeometry() const
{
  return glAPI();
}


const GLComponent* TesselatedMesh::glTexture( unsigned ) const
{
  return tesselatedMesh()->glAPI();
}


/*
bool TesselatedMesh::render( PrimList & prim, const ViewState & state )
{
  cout << "TesselatedMesh::render\n";
  if( d->polychanged )
    tesselate();

  bool ok = GLObjectVector::render();

  cout << "TesselatedMesh::render done\n";
  clearHasChangedFlags();
  return ok;
}
*/


namespace
{

  void tessVertex( GLvoid* vertex, void* /*client*/ )
  {
    // TesselatedMesh* tmesh = reinterpret_cast<TesselatedMesh *>( client );
    glVertex3dv( (GLdouble *) vertex );
  }


  void tessBegin( GLenum polytype, void* /*client*/ )
  {
    // TesselatedMesh* tmesh = reinterpret_cast<TesselatedMesh *>( client );
    glBegin( polytype );
  }


  void tessEnd( void* /*client*/ )
  {
    glEnd();
  }


  void tessError( GLenum errcode, void* /*client*/ )
  {
    // TesselatedMesh* tmesh = reinterpret_cast<TesselatedMesh *>( client );
    cout << "Tesselation error: " << gluErrorString( errcode ) << endl;
  }


  void tessCombine( GLdouble coords[3], GLdouble* vdata[4], GLfloat weight[4],
    GLdouble **outdata, void* /*client*/ )
  {
    // TesselatedMesh* tmesh = reinterpret_cast<TesselatedMesh *>( client );
    GLdouble *vertex = (GLdouble *) malloc( 6 * sizeof( GLdouble ) );
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    for( unsigned i=3; i<6; ++i )
    {
      GLdouble & v = vertex[i];
      if( vdata[0] )
        v = weight[0] * vdata[0][i];
      else
        v = 0;
      if( vdata[1] )
        v += weight[1] * vdata[1][i];
      if( vdata[2] )
        v += weight[2] * vdata[2][i];
      if( vdata[3] )
        v += weight[3] * vdata[3][i];
    }
    *outdata = vertex;
  }


  void tessEdgeFlag( bool flag, void* /*client*/ )
  {
    glEdgeFlag( flag );
  }


  void tessPolyVertex( GLvoid* vertex, void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );
    GLdouble *v = (GLdouble *) vertex, *v2;
    gluTessVertex( d->tesselator, v, v );
    glVertex3dv( (GLdouble *) vertex );
  }


  void tessPolyBegin( GLenum polytype, void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );
    gluTessBeginContour( d->tesselator );
    glBegin( polytype );
  }


  void tessPolyEnd( void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );
    gluTessEndContour( d->tesselator );
    glEnd();
  }


  void tessPolyError( GLenum errcode, void* /*client*/ )
  {
    cout << "Poly Tesselation error: " << gluErrorString( errcode ) << endl;
  }


  void tessPolyCombine( GLdouble coords[3], GLdouble* vdata[4],
                        GLfloat weight[4], GLdouble **outdata,
                        void* /*client*/ )
  {
    GLdouble *vertex = (GLdouble *) malloc( 6 * sizeof( GLdouble ) );
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    for( unsigned i=3; i<6; ++i )
      vertex[i] = weight[0] * vdata[0][i] + weight[1] * vdata[1][i]
      + weight[2] * vdata[2][i] + weight[3] * vdata[3][i];
    *outdata = vertex;
  }


  unsigned reorderPolygon( unsigned npol, const GLuint* poly,
                           list<list<GLuint> > & orderedlist )
  {
    if( npol == 0 )
      return 0;

    map<GLuint, AimsVector<GLuint,2> > neigh;
    map<GLuint, AimsVector<GLuint,2> >::iterator in, jn, en = neigh.end();
    const GLuint *iv, *ev = poly + npol*2;
    GLuint i, j;

    for( iv=poly; iv!=ev; ++iv )
    {
      i = *iv;
      ++iv;
      j = *iv;

      in = neigh.find( i );
      if( in == en )
        in = neigh.insert( make_pair( i, AimsVector<GLuint, 2>( -1, -1 ) )
          ).first;
      AimsVector<GLuint,2> & v = in->second;
      if( v[0] == (unsigned) -1 )
        v[0] = j;
      else
        v[1] = j;
      in = neigh.find( j );
      if( in == en )
        in = neigh.insert( make_pair( j, AimsVector<GLuint, 2>( -1, -1 ) )
          ).first;
      AimsVector<GLuint,2> & w = in->second;
      if( w[0] == (unsigned) -1 )
        w[0] = i;
      else
        w[1] = i;
    }

    unsigned n = neigh.size(), ndone = 0, nv = 0;
    list<GLuint> todo;
    set<GLuint> done;
    set<GLuint>::iterator notdone = done.end();
    in = neigh.begin();
    bool newcurve;
    while( ndone != n )
    {
      for( ; done.find( in->first ) != notdone; ++in )
      {}
      todo.push_back( in->first );
      newcurve = true;
      while( !todo.empty() )
      {
        i = todo.front();
        todo.pop_front();
        done.insert( i );
        if( newcurve )
        {
          newcurve = false;
          orderedlist.push_back( list<GLuint>() );
        }
        list<GLuint> & ordered = orderedlist.back();
        ++ndone;
        ++nv;
        AimsVector<GLuint,2> & nx = neigh[i];
        ordered.push_back( i );
        j = nx[1];
        if( j != (unsigned) -1 )
        {
          if( done.find( j ) == notdone )
          {
            todo.push_back( j );
            continue;
          }
        }
        bool ngb = (j != (unsigned) -1);
        j = nx[0];
        if( j != (unsigned) -1 )
          if( done.find( j ) == notdone )
            todo.push_back( j );
        else if( !ngb )
        {
          cout << "vertex " << i << " without neighbour\n";
          ordered.pop_back();
          --nv;
        }
      }
    }
    return nv;
  }

}


void TesselatedMesh::tesselate( const ViewState & vs )
{
  if( !d->tesselator )
  {
    d->tesselator = gluNewTess();
    gluTessCallback( d->tesselator, GLU_TESS_VERTEX_DATA,
                     (void (*)()) tessVertex );
    gluTessCallback( d->tesselator, GLU_TESS_BEGIN_DATA,
                     (void (*)()) tessBegin );
    gluTessCallback( d->tesselator, GLU_TESS_END_DATA,
                     (void (*)()) tessEnd );
    gluTessCallback( d->tesselator, GLU_TESS_ERROR_DATA,
                     (void (*)()) tessError );
    gluTessCallback( d->tesselator, GLU_TESS_COMBINE_DATA,
                     (void (*)()) tessCombine );
    gluTessCallback( d->tesselator, GLU_TESS_EDGE_FLAG_DATA,
                     (void (*)()) tessEdgeFlag );
    gluTessProperty( d->tesselator, GLU_TESS_WINDING_RULE,
                     GLU_TESS_WINDING_ODD );
    d->polytess = gluNewTess();
    gluTessCallback( d->polytess, GLU_TESS_VERTEX_DATA,
                     (void (*)()) tessPolyVertex );
    gluTessCallback( d->polytess, GLU_TESS_BEGIN_DATA,
                     (void (*)()) tessPolyBegin );
    gluTessCallback( d->polytess, GLU_TESS_END_DATA,
                     (void (*)()) tessPolyEnd );
    gluTessCallback( d->polytess, GLU_TESS_ERROR_DATA,
                     (void (*)()) tessPolyError );
    gluTessCallback( d->polytess, GLU_TESS_COMBINE_DATA,
                     (void (*)()) tessPolyCombine );
    gluTessProperty( d->polytess, GLU_TESS_BOUNDARY_ONLY, GL_TRUE );
    gluTessProperty( d->polytess, GLU_TESS_WINDING_RULE,
                     GLU_TESS_WINDING_ODD );
    ASurface<2> *surf = dynamic_cast<ASurface<2> *>( firstPolygon() );
    if( surf && surf->isPlanar() )
    {
      GLComponent *gl = surf->glAPI();
      if( gl )
      {
        const GLfloat* narr = gl->glNormalArray( vs );
        if( narr )
        {
          gluTessNormal( d->tesselator, narr[0], narr[1], narr[2] );
          gluTessNormal( d->polytess, narr[0], narr[1], narr[2] );
        }
      }
    }
  }
  iterator io = begin(), eo = end();
  unsigned nvert = 0, v = 0;

  list<list<list<GLuint> > > ordered;
  d->tmppts.clear();
  // count / order vertices on all polygons
  for( ++io; io!=eo; ++io )
  {
    GLComponent *gl = (*io)->glAPI();
    if( gl )
    {
      ordered.push_back( list<list<GLuint> >() );
      nvert += reorderPolygon( gl->glNumPolygon( vs ),
                               gl->glPolygonArray( vs ), ordered.back() );
    }
  }
  vector<GLdouble> vertices;
  vertices.reserve( nvert * 3 );
  list<list<list<GLuint> > >::iterator iov = ordered.begin();

  gluTessBeginPolygon( d->polytess, d );
  gluTessBeginPolygon( d->tesselator, this );
  for( io=begin(), ++io; io!=eo; ++io )
  {
    GLComponent *gl = (*io)->glAPI();
    if( !gl )
      continue;
    list<list<GLuint> > & overt = *iov;
    list<list<GLuint> >::iterator iiov, eiov = overt.end();
    const GLfloat* vert = gl->glVertexArray( vs );

    list<GLuint>::iterator iord, eord;
    for( iiov=overt.begin(); iiov!=eiov; ++iiov )
    {
      gluTessBeginContour( d->polytess );
      // we have to convert coords to double and keep them all in memory
      for( iord=iiov->begin(), eord=iiov->end(); iord!=eord; ++iord )
      {
        unsigned ind = 3 * *iord;
        vertices.push_back( vert[ ind + 0 ] );
        vertices.push_back( vert[ ind + 1 ] );
        vertices.push_back( vert[ ind + 2 ] );
        gluTessVertex( d->polytess, &vertices[v], &vertices[v] );
        v += 3;
      }
      gluTessEndContour( d->polytess );
    }
    ++iov;
  }
  gluTessEndPolygon( d->polytess );
  gluTessEndPolygon( d->tesselator );
  d->tmppts.clear();
  cout << "tesselate done\n";
}


bool TesselatedMesh::glMakeBodyGLL( const ViewState & state,
                                    const GLList & gllist ) const
{
  // GLComponent::glMakeBodyGLL();
  glNewList( gllist.item(), GL_COMPILE );
  const_cast<TesselatedMesh *>( this )->tesselate( state );
  glEndList();

  return true;
}


void TesselatedMesh::update( const Observable* observable, void* arg )
{
  cout << "TesselatedMesh::update " << this << " " << size() << endl;
  GLComponent::glSetChanged( glGEOMETRY );
}


Tree* TesselatedMesh::optionTree() const
{
  static Tree*  _optionTree = 0;

  if( !_optionTree )
    {
      //Tree    *t, *t2;
      _optionTree = new Tree( true, "option tree" );

      Tree      *t = new Tree( true, "Referential" );
      _optionTree->insert( t );
      Tree      *t2 = new Tree( true, "Load" );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );
    }
  return( _optionTree );
}


void TesselatedMesh::clearHasChangedFlags()
{
  d->polychanged = false;
  GLMObject::clearHasChangedFlags();
}


void TesselatedMesh::SetMaterial( const Material & mat )
{
  AObject::SetMaterial( mat );
  AObject       *cm = tesselatedMesh();
  if( cm )
    cm->SetMaterial( mat );
}


/*
Material & TesselatedMesh::GetMaterial()
{
  AObject       *cm = cutMesh();
  if( cm )
    return cm->GetMaterial();
  return AObject::GetMaterial();
}
*/


AObject* TesselatedMesh::fallbackReferentialInheritance() const
{
  return const_cast<AObject *>( firstPolygon() );
}


