#ifndef _vtkQtRenderWindowInteractor2_h
#define _vtkQtRenderWindowInteractor2_h

#include <qgl.h>
#include <qpaintdevice.h>
#include <qtimer.h>

#ifdef Q_WS_X11
#include "vtkXOpenGLRenderWindow.h"
#endif

#include "vtkRenderWindowInteractor.h"

#ifdef WIN32
  #define VTK_QT_EXPORT __declspec (dllexport)
#else
  #define VTK_QT_EXPORT
#endif

#ifndef VTK_QT_EXPORT
#ifdef _WIN_32
  #define VTK_QT_EXPORT __declspec (dllexport)
#else
  #define VTK_QT_EXPORT
#endif
#endif

class vtkRenderWindow;

class VTK_QT_EXPORT vtkQtRenderWindowInteractor2 : public QGLWidget, virtual public vtkRenderWindowInteractor {
  Q_OBJECT
    
    public:

  vtkQtRenderWindowInteractor2();
  vtkQtRenderWindowInteractor2(QWidget* parent=0, const char* name=0,
			       const QGLWidget* shareWidget = 0, Qt::WFlags f=0 );
  vtkQtRenderWindowInteractor2(const QGLFormat& format, QWidget* parent=0, const char* name=0,
			       const QGLWidget* shareWidget = 0, Qt::WFlags f=0 );
  
  
  static int IsTypeOf(const char *type)
  {
    if ( !strcmp("vtkQtRenderWindowInteractor2",type) )
      return 1;
    return vtkRenderWindowInteractor::IsTypeOf(type);
  }
  virtual int IsA(const char *type)
  {
    return this->IsTypeOf(type);
  }
  
  virtual ~vtkQtRenderWindowInteractor2();
  
  static vtkQtRenderWindowInteractor2 *New(); // inline

  void PrintSelf(ostream& os, vtkIndent indent);

  
  // vtkRenderWindowInteractor overrides
  void Initialize();
  void Enable();
  void Disable();
  void Start();
  void UpdateSize(int x, int y);
  void TerminateApp() {};
  void Render();

  void SetRenderWindow (vtkRenderWindow*);
  
  // timer methods
  virtual void SetTimerDuration (unsigned long);
  virtual int CreateTimer(int timertype);
  virtual int DestroyTimer(void);


  virtual QSizePolicy sizePolicy() const { return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding); }
  virtual QSize sizeHint()         const { return QSize(500, 500); }
  virtual QSize minimumSizeHint()  const { return QSize(500, 500); }
  
  
  //virtual void initializeGL();
  virtual void paintGL();
  virtual void resizeGL(int, int);
  virtual void focusInEvent(QFocusEvent*){};
  virtual void focusOutEvent(QFocusEvent*){};

#ifdef Q_WS_X11
  int GetDesiredDepth() { return QPaintDevice::x11Depth(); }
  Colormap GetDesiredColormap() { return QPaintDevice::x11Colormap(); }
  Visual *GetDesiredVisual() { return (Visual*) QPaintDevice::x11Visual(); }
#endif


  int    UpdateRenderWindow;
  
    
  protected slots:
  void timer();
    
    
 protected:
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void keyPressEvent(QKeyEvent*);
  virtual void wheelEvent (QWheelEvent*);

 private:
  virtual const char* GetClassNameInternal() const
    { return "vtkQtRenderWindowInteractor2"; }

  void InitRenderWindowInteractor (void);
  
  QTimer qTimer;
  long   Handle;

#if defined (Q_WS_MAC) && QT_VERSION < 0x040000
  void macFixRect(void);
#endif
  
};

#endif


