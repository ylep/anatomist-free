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


#ifndef ANA_SURFACE_TRANSFORMEDOBJECT_H
#define ANA_SURFACE_TRANSFORMEDOBJECT_H


#include <anatomist/mobject/objectVector.h>
#include <anatomist/surface/glcomponent.h>


namespace anatomist
{

  class Referential;
  class Geometry;


  /** A container object which displays its children either with a fixed
     orientation, or at a fixed position in the window corner.
   */
  class TransformedObject : public ObjectVector, public GLComponent
  {
  public:
    TransformedObject( const std::vector<AObject *> &,
                       bool followorientation=true,
                       bool followposition=false,
                       const Point3df & pos=Point3df( 1, 1, 0 ),
                       bool strongrefs=false );
    TransformedObject( const std::vector<carto::shared_ptr<AObject> > &,
                       bool followorientation=true,
                       bool followposition=false,
                       const Point3df & pos=Point3df( 1, 1, 0 ) );
    virtual ~TransformedObject();

    virtual bool renderingIsObserverDependent() const;
    virtual bool render( PrimList &, const ViewState & );
    void setPosition( const Point3df & );
    /// position in model view coords
    Point3df position() const;
    void setOffset( const Point3df & );
    /// 3D constant offset in camera coords
    Point3df offset() const;
    void setDynamicOffsetFromPoint( const Point3df & );
    void removeDynamicOffset();
    bool usesDynamicOffset() const;
    Point3df dynamicOffsetFromPoint() const;
    void setScale( float scale );
    float scale() const;
    bool followsOrientation() const;
    void setFollowOrientation( bool x );
    bool followsPosition() const;
    void setFollowPosition( bool x );
    virtual const GLComponent* glAPI() const { return ObjectVector::glAPI(); }
    virtual GLComponent* glAPI() { return ObjectVector::glAPI(); }


  protected:
    void setupTransforms( GLPrimitives &, const ViewState & );
    void popTransformationMatrixes( GLPrimitives & );

  private:
    struct Private;
    Private *d;
  };

}

#endif

