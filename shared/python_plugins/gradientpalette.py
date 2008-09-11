try:
  try:
    from soma.gui.api import chooseMatplotlibBackend
    chooseMatplotlibBackend()
  except:
    pass
  from neurospy.ndview.gui import gradwidget
  ok = True
except:
  print 'gradient widget plugin disabled because neurospy.ndview.gui ' \
  'cannot be imported.'
  ok = False

import anatomist.cpp as anatomist
import qt
from soma import aims
import sip, weakref, os, sys

class GWManager( qt.QObject ):
  gradwidgets = set()

if ok:
  gwManager = GWManager()


class GradientPaletteWidget( qt.QVBox ):
  class GradientObserver( anatomist.Observer ):
    def __init__( self, gradpal ):
      anatomist.Observer.__init__( self )
      self._gradpal = weakref.proxy( gradpal )

    def registerObservable( self, obs ):
      anatomist.Observer.registerObservable( self, obs )
      self._gradpal._objects.append( anatomist.weak_ptr_AObject(obs) )

    def unregisterObservable( self, obs ):
      self._gradpal._objects.remove( obs )
      anatomist.Observer.unregisterObservable( self, obs )

    def update( self, observable, args ):
      self._gradpal.update( observable, args )

  class GradientObjectParamSelect( anatomist.ObjectParamSelect ):
    def __init__( self, objects, parent ):
      anatomist.ObjectParamSelect.__init__( self, objects, parent )

    def filter( self, obj ):
      glapi = obj.glAPI()
      if not glapi:
        return False
      if glapi.glNumTextures() > 0:
        te = glapi.glTexExtrema( 0 );
        if te.minquant.size() > 0 and te.maxquant.size() > 0:
          return True
      return False

  def __init__( self, objects, parent = None, name = None, flags = 0 ):
    qt.QVBox.__init__( self, parent, name, flags )
    self._objsel = GradientPaletteWidget.GradientObjectParamSelect( objects,
      self )
    self._gradw = gradwidget.GradientWidget( self, 'gradientwidget', '', 0, 1 )
    hb = qt.QGrid( 2, self )
    hb.setMargin( 5 )
    hb.setSpacing( 10 )
    savebtn = qt.QPushButton( self.tr( 'Save palette image...' ), hb )
    editbtn = qt.QPushButton( self.tr( 'Edit gradient information' ), hb  )
    keepbtn = qt.QPushButton( self.tr( 'Keep as static palette' ), hb )
    modecb = qt.QComboBox( hb )
    modecb.insertItem( self.tr( 'RGB mode' ) )
    modecb.insertItem( self.tr( 'HSV mode' ) )
    modecb.setCurrentItem( 0 )
    hb.setSizePolicy( qt.QSizePolicy.Preferred, qt.QSizePolicy.Fixed )
    self._objects = []
    self._observer = GradientPaletteWidget.GradientObserver( self )
    self._gradw.setHasAlpha( True )
    self._changing = False
    o0 = None
    for obj in objects:
      if o0 is None:
        o0 = obj
      obj.addObserver( self._observer )
    self._initial = [] + self._objects
    self.connect( self._gradw, qt.SIGNAL( 'gradientChanged( QString )' ),
      self.gradientChanged )
    global gwManager
    gwManager.gradwidgets.add( self )
    if o0 is not None:
      self.update( o0, None )
    self.connect( self._objsel, qt.SIGNAL( 'selectionStarts()' ),
      self.chooseObject );
    self.connect( self._objsel,
      qt.SIGNAL( 'objectsSelected( const std::set<anatomist::AObject *> & )' ),
      self.objectsChosen );
    self.connect( savebtn, qt.SIGNAL( 'clicked()' ), self.save )
    self.connect( keepbtn, qt.SIGNAL( 'clicked()' ), self.makeStaticPalette )
    self.connect( editbtn, qt.SIGNAL( 'clicked()' ), self.editGradient )
    self.connect( modecb, qt.SIGNAL( 'activated(int)' ), self.setMode )

  def close( self, alsodelete=True ):
    gwManager.gradwidgets.remove( self )
    return True

  def gradientChanged( self, s ):
    a = anatomist.Anatomist()
    paldim = 512
    pal = anatomist.APalette( 'CustomGradient', paldim )
    pdata = pal.volume()
    rgbp = self._gradw.fillGradient( paldim, True )
    rgb = rgbp.data()
    if sys.byteorder == 'little':
      for i in xrange(paldim):
        pdata.setValue( aims.AimsRGBA( ord(rgb[i*4+2]), ord(rgb[i*4+1]),
          ord(rgb[i*4]), ord(rgb[i*4+3]) ), i )
    else:
      for i in xrange(paldim):
        pdata.setValue( aims.AimsRGBA( ord(rgb[i*4+1]), ord(rgb[i*4+2]),
          ord(rgb[i*4+3]), ord(rgb[i*4]) ), i )
    pal.header()[ "palette_gradients" ] \
      = self._gradw.getGradientString().latin1()
    if self._gradw.isHsv():
      pal.header()[ "palette_gradients_mode" ] = 'HSV'
    else:
      pal.header()[ "palette_gradients_mode" ] = 'RGB'
    for obj in self._objects:
      opal = obj.getOrCreatePalette()
      opal.setRefPalette( pal )
      opal.fill()
      obj.setPalette( opal )
      self._changing = True
      obj.notifyObservers()
      self._changing = False

  def update( self, observable, args ):
    if self._changing:
      return
    if len( self._objects ) > 0:
      obj = self._objects[0]
      pal = obj.getOrCreatePalette()
      glapi = obj.glAPI()
      if glapi:
        extr = glapi.glTexExtrema()
        sz = extr.maxquant[0] - extr.minquant[0]
        pmin = extr.minquant[0] + sz * pal.min1()
        pmax = extr.minquant[0] + sz * pal.max1()
        self._gradw.setBounds( pmin, pmax )
      self._changing = True
      if pal:
        rpal = pal.refPalette()
        try:
          grads = rpal.header()[ 'palette_gradients' ]
        except:
          grads = None
        if grads:
          try:
            gradmode = rpal.header()[ 'palette_gradients_mode' ]
          except:
            gradmode = None
          if gradmode == 'HSV':
            self._gradw.newHsv()
          else:
            self._gradw.newRgb()
          self._gradw.setGradient( grads )
        else:
          self._gradw.setGradient( '0;0;1;1#0;0;1;1#0;0;1;1#0.5;1' )
      else:
        self._gradw.setGradient( '0;0;1;1#0;0;1;1#0;0;1;1#0.5;1' )
      self._gradw.update()
      self._changing = False

  def chooseObject( self ):
    self._initial = [ x for x in self._initial \
      if x.get() and not sip.isdeleted( x.get() ) ]
    self._objsel.selectObjects( [ x.get() for x in self._initial ],
        [ x.get() for x in self._objects ] );

  def objectsChosen( self, objects ):
    objects = self._objsel.selectedObjects()
    for x in [] + self._objects:
      x.deleteObserver( self._observer )
    for x in objects:
      x.addObserver( self._observer )
    self.update( None, None )

  def save( self ):
    if len( self._objects ) > 0:
      obj = self._objects[0]
      pal = obj.getOrCreatePalette().refPalette()
      apal = anatomist.AObjectConverter.anatomist( pal.volume() )
      apal.setName( pal.name() )
      a = anatomist.Anatomist()
      apal.setFileName( os.path.join( a.anatomistHomePath().latin1(), 'rgb',
        pal.name() ) )
      anatomist.ObjectActions.saveStatic( [ apal ] )
      a = anatomist.Anatomist()
      a.releaseObject( apal )

  def editGradient( self ):
    d = qt.QDialog( None, 'gradient', True )
    d.setCaption( self.tr( 'Gradient definition:' ) )
    l = qt.QVBoxLayout( d )
    t = qt.QTextEdit( d )
    l.addWidget( t )
    t.setWordWrap( t.WidgetWidth )
    t.setWrapPolicy( t.AtWordOrDocumentBoundary )
    t.setText( self._gradw.getGradientString() )
    d.connect( t, qt.SIGNAL( 'returnPressed()' ), d.accept )
    res = d.exec_loop()
    if res:
      self._gradw.setGradient( t.text() )
      self._gradw.update()
      self.gradientChanged( t.text() )

  def makeStaticPalette( self ):
    obj = self._objects[0]
    a = anatomist.Anatomist()
    pall = a.palettes()
    pali = obj.getOrCreatePalette().refPalette()
    d = qt.QDialog( None, 'paletteName', True )
    d.setCaption( self.tr( 'Palette name:' ) )
    l = qt.QVBoxLayout( d )
    t = qt.QLineEdit( pali.name(), d )
    l.addWidget( t )
    d.connect( t, qt.SIGNAL( 'returnPressed()' ), d.accept )
    res = d.exec_loop()
    if res:
      pal = anatomist.APalette( pali.get() )
      pal.setName( t.text().latin1() )
      pall.push_back( pal )

  def setMode( self, mode ):
    if mode == 0:
      if self._gradw.isHsv():
        self._gradw.newRgb()
    else:
      if not self._gradw.isHsv():
        self._gradw.newHsv()


# ---

class GradientPaletteModule( anatomist.Module ):
  def name( self ):
    return 'GradientPalette'

  def description(self):
    return 'Gradient palette'

class GradientPaletteCallback( anatomist.ObjectMenuCallback ):
  def __init__( self ):
    anatomist.ObjectMenuCallback.__init__(self)

  def doit( self, objects ):
    w = GradientPaletteWidget( objects, None, 'gradientpalette',
      qt.Qt.WDestructiveClose )
    w.show()

callbacks_list = []

class GradientPaletteMenuRegistrer( anatomist.ObjectMenuRegistrerClass ):
  def doit( self, otype, menu ):
    glapi = otype.glAPI()
    if glapi and glapi.glNumTextures() != 0:
      if menu is None:
        menu = anatomist.ObjectMenu()
      gradpalette = GradientPaletteCallback()
      menu.insertItem( ['Color'], 'gradient palette', gradpalette )
    return menu


def init():
  r = GradientPaletteMenuRegistrer()
  callbacks_list.append( r )
  anatomist.AObject.addObjectMenuRegistration( r )


if ok:
  gp = GradientPaletteModule()
  init()

del ok

