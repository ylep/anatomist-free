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


#ifndef ANATOMIST_WINDOW_GLWIDGET_H
#define ANATOMIST_WINDOW_GLWIDGET_H

#include <anatomist/window/glcontext.h>
#include <anatomist/window/glwidgetmanager.h>


class QAGLWidget : public carto::GLWidget,
  public anatomist::GLWidgetManager
{
  Q_OBJECT

public:
  QAGLWidget( anatomist::AWindow* win, QWidget* parent = 0,
              const char* name = 0,
              const QGLWidget * shareWidget = 0, Qt::WindowFlags f=0 );
  virtual ~QAGLWidget();

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;
  virtual std::string name() const;

  int width()
  {
    return carto::GLWidget::width();
  }
  void setBackgroundAlpha( float a );

  int height()
  {
    return carto::GLWidget::height();
  }
  
public slots:
  /// to be reimplemented in "public slots"
  virtual void updateGL();

protected:
#if QT_VERSION >= 0x040600
  virtual bool event( QEvent * );
#endif
  virtual void initializeGL();
  virtual void resizeGL( int w, int h );
  virtual void paintGL();
  virtual void mousePressEvent( QMouseEvent* me );
  virtual void mouseReleaseEvent( QMouseEvent* me );
  virtual void mouseMoveEvent( QMouseEvent* me );
  virtual void mouseDoubleClickEvent( QMouseEvent * );
  virtual void keyPressEvent( QKeyEvent* ev );
  virtual void keyReleaseEvent( QKeyEvent* ev );
  virtual void focusInEvent( QFocusEvent * );
  virtual void focusOutEvent( QFocusEvent * );
  virtual void wheelEvent( QWheelEvent * );
};


#endif
