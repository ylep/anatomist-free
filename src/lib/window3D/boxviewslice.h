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


#ifndef ANATOMIST_WINDOW3D_BOXVIEWSLICE_H
#define ANATOMIST_WINDOW3D_BOXVIEWSLICE_H


#include <aims/vector/vector.h>
#include <cartobase/smart/rcptr.h>

class QGraphicsView;
class QString;

namespace anatomist
{
  class Action;
  class AObject;
  class Referential;

  class BoxViewSlice
  {
  public:
    BoxViewSlice( Action* action );
    virtual ~BoxViewSlice();

    virtual void beginTrackball( int x, int y );
    virtual void moveTrackball( int x, int y );
    virtual void endTrackball( int x, int y );

    void setCubeColor( float r, float g, float b, float a );
    void setPlaneColor( float r, float g, float b, float a );
    void setTextColor( float r, float g, float b );
    void setObjectsReferential( Referential* ref );
    void enablePlane( bool );
    void enableText( bool );

    void initOjects();
    void buildSmallBox();
    void buildCube();
    void buildPlane();
    void removeObjects();
    QGraphicsView* graphicsView();
    void updateText( const QString & );
    void addObject( carto::rc_ptr<AObject> obj );
    void removeObject( carto::rc_ptr<AObject> obj );
    void clearAdditionalObjects();

    void drawText( float posx, float posy, const QString & text );
    float objectsSize();
    void updateRect();
    /** Intersect plane (pnorm, d) with line (linedir, x0)
    pnorm: normal vector to plane
    d: plane equation offset
    linedir: line vector
    x0: point in the line
    returns: pair( valid, lambda coef ) (pos=x0+lambda*linedir)
    */
    std::pair<bool, float> lineIntersect( const Point3df & pnorm, float d,
      const Point3df & linedir, const Point3df & x0 );
    void clearTmpItems();

  private:
    struct Private;
    Private *d;
  };

}

#endif

