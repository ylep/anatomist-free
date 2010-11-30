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

#ifndef ANATOMIST_WINDOW3D_WSURFPAINTTOOLS_H
#define ANATOMIST_WINDOW3D_WSURFPAINTTOOLS_H

//#include <anatomist/observer/Observer.h>

#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/surface/surface.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/color/objectPalette.h>
#include <aims/rgb/rgb.h>

#include <aims/utility/converter_texture.h>

#include <anatomist/application/Anatomist.h>
#include <anatomist/object/objectConverter.h>

#include <anatomist/control/surfpaintcontrol.h>

#include <anatomist/observer/Observer.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/controler/controlmanager.h>
#include <qwidget.h>
#include <qspinbox.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/application/settings.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qvgroupbox.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qbuttongroup.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <aims/qtcompat/qtoolbutton.h>
#include <aims/qtcompat/qbutton.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <qcombobox.h>
#include <qaction.h>
#include <iostream>
#include <qmenubar.h>

#include <aims/def/path.h>
#include <cartobase/config/version.h>
#include <cartobase/config/paths.h>
#include <cartobase/stream/fileutil.h>

#include <iostream>
#include <string.h>
#include <fstream>
#include <aims/mesh/curv.h>
#include <aims/mesh/surfaceOperation.h>
#include <aims/mesh/surfacegen.h>
#include <aims/mesh/geometric.h>

#include <aims/geodesicpath/geodesic_mesh.h>

using namespace aims;
using namespace std;
using namespace geodesic;
using namespace anatomist;
using namespace carto;

namespace anatomist
{
  class SurfpaintTools: public QWidget/*, public anatomist::Observer*/
  {
    Q_OBJECT

//    protected:
//      virtual void unregisterObservable(anatomist::Observable*);

    public:

      SurfpaintTools();
      virtual ~SurfpaintTools() ;

      static SurfpaintTools* instance() ;

//      virtual void update( const anatomist::Observable* observable, void* arg );

      void initSurfPaintModule(AWindow3D *w3);
      void addToolBarControls(AWindow3D *w3);
      void removeToolBarControls(AWindow3D *w3);
      void addToolBarInfosTexture(AWindow3D *w3);
      void removeToolBarInfosTexture(AWindow3D *w3);

      void setPolygon(int p){IDPolygonSpinBox->setValue(p);}
      void setMaxPoly(int max){IDPolygonSpinBox->setRange(-1,max);}
      void setVertex(int v){IDVertexSpinBox->setValue(v);}
      void setMaxVertex(int max){IDVertexSpinBox->setRange(-1,max);}

      void setMinMaxTexture(float min, float max){textureFloatSpinBox->setRange(min,max);}
      float getTextureValueFloat(void){return textureFloatSpinBox->value();}
      void setTextureValueFloat(double v){textureFloatSpinBox->setValue(v);}
      void updateTextureValue(int indexVertex, float value);
      void restoreTextureValue(int indexVertex);
      void floodFillStart(int indexVertex);
      void floodFillStop(void);
      void floodFillMove(int indexVertex, float newTextureValue, float oldTextureValue);

      string getPathType(void){return shortestPathSelectedType;}

      void setClosePath(bool c){pathClosed = c;}
      bool pathIsClosed(void){return pathClosed;}

      geodesic::Mesh getMeshStructSP() {return meshSP;}
      geodesic::Mesh getMeshStructGyriP() {return meshGyriCurvSP;}
      geodesic::Mesh getMeshStructSulciP() {return meshSulciCurvSP;}

      void addGeodesicPath(int indexNearestVertex,Point3df positionNearestVertex);

      void changeControl(int control){IDActiveControl = control;}
      int getActiveControl(void){return IDActiveControl;}
      AWindow3D* getWindow3D(void){return win3D;}

    public slots:

      void colorPicker();
      void magicSelection();
      void path();
      void shortestPath();
      void sulciPath();
      void gyriPath();
      void brush();
      void fill();
      void clearPath();
      void clearRegion();
      void clearAll();
      void erase();
      void save();

      void updateConstraintList();
      void loadConstraintsList();
      void changeToleranceSpinBox(int v);
      void changeConstraintPathSpinBox(int v);

    private :
      void popAllButtonPaintToolBar();

    private:

      Texture1d *surfpaintTexInit;

      AWindow3D *win3D;

      static SurfpaintTools * my_instance ;

      AObject *objselect;
      ATexSurface *go;
      AObject *tex;
      AObject *surf;
      ATexture *at;
      ATriangulated *as;

      QToolBar  *tbTextureValue;
      QDoubleSpinBox *textureFloatSpinBox;

      //QWidget *textureSpinBox;

      string textype;
      string objtype;

      QToolBar  *tbInfos3D;
      QSpinBox *IDPolygonSpinBox;
      QSpinBox *IDVertexSpinBox;

      QToolBar  *tbControls;

      QToolButton *colorPickerAction;
      QToolButton *selectionAction;
      QToolButton *pathAction;
      QAction     *shortestPathAction;
      QAction     *sulciPathAction;
      QAction     *gyriPathAction;
      QToolButton *paintBrushAction;
      QToolButton *fillAction;
      QToolButton *eraseAction;
      QToolButton *clearPathAction;
      QToolButton *saveAction;

      QComboBox *constraintList;

      QSpinBox *toleranceSpinBox;
      QLabel *toleranceSpinBoxLabel;
      QSpinBox *constraintPathSpinBox;
      QLabel *constraintPathSpinBoxLabel;

      GLuint constraintPathValue;
      GLuint toleranceValue;
      float stepToleranceValue;

      int IDActiveControl;
      string shortestPathSelectedType;

      float *texCurvature;
      geodesic::Mesh meshSP;
      geodesic::Mesh meshGyriCurvSP;
      geodesic::Mesh meshSulciCurvSP;
      std::vector<std::set<uint> >  neighbours;
      bool pathClosed;

      std::vector<geodesic::SurfacePoint> pathSP;
      std::vector<int> listIndexVertexPathSP;
      std::vector<int> listIndexVertexPathSPLast;
      std::vector<int> listIndexVertexSelectSP;
      map<int,float> listVertexChanged;

      std::vector<int> listIndexVertexSelectFill;

      std::vector<ATriangulated*> pathObject;
      std::vector<ATriangulated*> fillObject;
  };
//
//  class SurfpaintToolsWindow: public QWidget, public anatomist::Observer
//  {
//    Q_OBJECT
//
//    typedef Mesh* mesh_pointer;
//
//    public:
//      SurfpaintToolsWindow(AWindow3D *win, string t, AObject *surf, AObject *tex);
//      virtual ~SurfpaintToolsWindow();
//
//      virtual void update( const anatomist::Observable* observable, void* arg );
//
//      virtual const std::string & baseTitle() const;
//
//      virtual anatomist::View* view();
//      virtual const anatomist::View* view() const;
//
//      void fillRegionOrPath();
//      void changeMode(int mode);
//      void saveTexture();
//
//      AObject* getSurface() {return _surf;};
//      geodesic::Mesh getMeshStructSP() {return meshSP;};
//      geodesic::Mesh getMeshStructGyriP() {return meshGyriCurvSP;};
//      geodesic::Mesh getMeshStructSulciP() {return meshSulciCurvSP;};
//      //std::vector<std::set<uint> >  neighbours;
//
//    private :
//      void popAllButtonPaintToolBar();
//
//    public slots:
//
//      void colorPicker();
//      void magicSelection();
//      void path();
//      void shortestPath();
//      void sulciPath();
//      void gyriPath();
//      void brush();
//      void fill();
//      void clear();
//      void save();
//
//
//      float getTextureValue();
//      void setTextureValue(float v);
//      void setTextureValueInt(int v);
//      void setTextureValueFloat(double v);
//
//      void setMinMaxTexture(float min, float max);
//
//      void setPolygon(int p);
//      void setMaxPoly(int max);
//      void setVertex(int v);
//      void setMaxVertex(int max);
//

//    protected:
//      AWindow3D *_window;
//      static std::string    _baseTitle;
//
//      virtual void unregisterObservable(anatomist::Observable*);
//
//    public :
//
//
//    private:
//
//      QToolButton *colorPickerAction;
//      QToolButton *selectionAction;
//      QToolButton *pathAction;
//      QAction     *shortestPathAction;
//      QAction     *sulciPathAction;
//      QAction     *gyriPathAction;
//      QToolButton *paintBrushAction;
//      QToolButton *fillAction;
//      QToolButton *clearAction;
//      QToolButton *saveAction;
//

//
//      QWidget *textureSpinBox;
//      QSpinBox *IDPolygonSpinBox;
//      QSpinBox *IDVertexSpinBox;
//      string _textype;
//      bool destroying;
//
//      AObject *_surf;
//      AObject *_tex;
//
//      float *texCurvature;
//      geodesic::Mesh meshSP;
//      geodesic::Mesh meshGyriCurvSP;
//      geodesic::Mesh meshSulciCurvSP;
//      std::vector<std::set<uint> >  neighbours;
//
//      string shortestPathSelectedType;
//  };
}

#endif
