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

#include <anatomist/reference/wReferential.h>
#include <anatomist/reference/wChooseReferential.h>
#include <anatomist/application/fileDialog.h>
#include <anatomist/control/wControl.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/refpixmap.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cLoadTransformation.h>
#include <anatomist/commands/cSaveTransformation.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/misc/error.h>
#include <aims/def/general.h>
#include <aims/qtcompat/qpopupmenu.h>
#include <aims/qtcompat/qmouseevent.h>
#include <cartobase/object/pythonwriter.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <map>
#include <stdio.h>
#include <math.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace
{

  class RefToolTip
#if QT_VERSION < 0x040000
  : public QToolTip
#endif
  {
  public:
#if QT_VERSION >= 0x040000
    RefToolTip( ReferentialWindow* parent );
#else
    RefToolTip( ReferentialWindow* parent, QToolTipGroup* group = 0 );
#endif
    virtual ~RefToolTip();

    virtual void maybeTip( const QPoint & p );

  private:
    ReferentialWindow *_refwin;
  };

#if QT_VERSION >= 0x040000
  RefToolTip::RefToolTip( ReferentialWindow* parent )
  : _refwin( parent )
#else
  RefToolTip::RefToolTip( ReferentialWindow* parent, QToolTipGroup* group )
  : QToolTip( parent, group ), _refwin( parent )
#endif
  {
  }

  RefToolTip::~RefToolTip()
  {
  }


  QString headerPrint( PythonHeader & ph,
                       const set<string> & exclude = set<string>() )
  {
    QString text;
    Object  i;;
    string  key, val;
    int     l = 12, x;
    set<string>::const_iterator printable = exclude.end();
    for( i=ph.objectIterator(); i->isValid(); i->next() )
    {
      key = i->key();
      if( exclude.find( key ) == printable )
        try
        {
          val = i->currentValue()->getString();
          text += "\n";
          text += key.c_str();
          text += ": ";
          for( x=key.length()+2; x<l; ++x )
            text += " ";
          text += val.c_str();
        }
        catch( ... )
        {
          try
          {
            PythonWriter  pw;
            ostringstream ss;
            pw.attach( ss );
            pw.setSingleLineMode( true );
            pw.write( i->currentValue(), false, false );
            text += "\n";
            text += key.c_str();
            text += ": ";
            for( x=key.length()+2; x<l; ++x )
              text += " ";
            text += ss.str().c_str();
          }
          catch( ... )
          {
          }
        }
    }
    return text;
  }


  void RefToolTip::maybeTip( const QPoint & p )
  {
    QPoint  pos;
    Referential *r = _refwin->refAt( p, pos );
    if( r )
    {
      string  name;
      PythonHeader  & ph = r->header();
      if( !ph.getProperty( "name", name ) )
        name = "<unnamed>";
      QString text( "Referential:  " );
      text += name.c_str();
      text += "\nUUID        :  ";
      text += r->uuid().toString().c_str();
      set<string> exclude;
      exclude.insert( "name" );
      exclude.insert( "uuid" );
      text += headerPrint( ph, exclude );
#if QT_VERSION >= 0x040000
        QToolTip::showText( _refwin->mapToGlobal( p ), text );
#else
        tip( QRect( p, p + QPoint( 20, 20 ) ), text );
#endif
    }
    else
    {
      Transformation  *t = _refwin->transfAt( p );
      if( t )
      {
        QString text( "Transformation:\n" );
        AimsData<float> r = t->motion().rotation();
        text += "R = " + QString::number( r( 0,0 ) ) + " "
            + QString::number( r( 0,1 ) ) + " "
            + QString::number( r( 0,2 ) ) + "\n      "
            + QString::number( r( 1,0 ) ) + " "
            + QString::number( r( 1,1 ) ) + " "
            + QString::number( r( 1,2 ) ) + "\n      "
            + QString::number( r( 2,0 ) ) + " "
            + QString::number( r( 2,1 ) ) + " "
            + QString::number( r( 2,2 ) ) + "\n"
            + "T = " + QString::number( t->Translation( 0 ) ) + " "
            + QString::number( t->Translation( 1 ) ) + " "
            + QString::number( t->Translation( 2 ) );
        PythonHeader  *ph = t->motion().header();
        if( ph )
          text += headerPrint( *ph );
#if QT_VERSION >= 0x040000
        QToolTip::showText( _refwin->mapToGlobal( p ), text );
#else
        tip( QRect( p, p + QPoint( 20, 20 ) ), text );
#endif
      }
    }
  }

}

// --------

namespace anatomist
{

  struct ReferentialWindow_PrivateData
  {
    ReferentialWindow_PrivateData() 
      : srcref( 0 ), dstref( 0 ), trans( 0 ), tracking( false ), refmenu( 0 ),
      bgmenu( 0 )
    {}

    Referential			*srcref;
    Referential			*dstref;
    Transformation		*trans;
    QPoint			pos;
    bool			tracking;
    map<Referential *, QPoint>	refpos;
    QPopupMenu			*refmenu;
    QPopupMenu                  *bgmenu;
    RefToolTip                  *tooltip;
  };

}


ReferentialWindow::ReferentialWindow( QWidget* parent, const char* name, 
				      Qt::WFlags f )
  : QLabel( parent, name, f ), 
  pdat( new ReferentialWindow_PrivateData )
{
  setCaption( tr( "Referentials" ) );
  resize( 256, 256 );
  setPixmap( QPixmap( width(), height() ) );
  pdat->tooltip = new RefToolTip( this );
#if QT_VERSION >= 0x040000
  setAttribute( Qt::WA_PaintOutsidePaintEvent );
#endif
}

ReferentialWindow::~ReferentialWindow()
{
  if (theAnatomist->getControlWindow() != 0)
    theAnatomist->getControlWindow()->enableRefWinMenu( true );
  delete pdat;
}


void ReferentialWindow::closeEvent( QCloseEvent * ev )
{
  QLabel::closeEvent( ev );
  if( theAnatomist->getControlWindow() )
    theAnatomist->getControlWindow()->enableRefWinMenu( true );
}


void ReferentialWindow::resizeEvent( QResizeEvent* ev )
{
  QLabel::resizeEvent( ev );
  refresh();
}


void ReferentialWindow::openSelectBox()
{
  QString filter = tr( "Transformation" );
  filter += " (*.trm *TO*);; ";
  filter += ControlWindow::tr( "All files" );
  filter += " (*)";
  QFileDialog	& fd = fileDialog();
  fd.setFilters( filter );
  fd.setCaption( tr( "Open transformation" ) );
  fd.setMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;
  QString filename = fd.selectedFile();
  if ( !filename.isEmpty() )
    {
      loadTransformation( filename.utf8().data() );
    }
}


void ReferentialWindow::saveTransformation( Transformation* trans )
{
  QString filter = tr( "Transformation" );
  filter += " (*.trm *TO*);;";
  filter += ControlWindow::tr( "All files" );
  filter += " (*)";
  QFileDialog	& fd = fileDialog();
  fd.setFilters( filter );
  fd.setCaption( tr( "Save transformation" ) );
  fd.setMode( QFileDialog::AnyFile );
  if( !fd.exec() )
    return;
  QString filename = fd.selectedFile();
  if ( !filename.isEmpty() )
    {
      pdat->trans = trans;
      saveTransformation( filename.utf8().data() );
    }
}


void ReferentialWindow::loadTransformation( const string & filename )
{
  LoadTransformationCommand	*com 
    = new LoadTransformationCommand( filename, pdat->srcref, pdat->dstref );
  theProcessor->execute( com );
  refresh();
}


void ReferentialWindow::saveTransformation( const string & filename )
{
  //cout << "saveTransformation " << filename << endl;
  /*cout << "src: " << pdat->srcref << ", dst: " << pdat->dstref << endl;
    cout << "trans: " << pdat->trans << endl;*/

  Transformation 
    *t = pdat->trans;

  if( t )
    {
      SaveTransformationCommand	*com 
	= new SaveTransformationCommand( filename, t );
      theProcessor->execute( com );
    }
  else
    cerr << "No transformation to save\n";
}


void ReferentialWindow::refresh()
{
  pdat->refpos.clear();

  set<Referential *>	refs = theAnatomist->getReferentials();
  set<Transformation *>	trans 
    = ATransformSet::instance()->allTransformations();
  unsigned		n = refs.size(), i;
  set<Referential *>::const_iterator	ir, fr=refs.end();
  set<Transformation *>::const_iterator	it, ft=trans.end();
  AimsRGB		col;
  Referential		*ref;
  Transformation	*tr;
  unsigned		x, y, sz = 20;
  int			w = width(), h = height(), R = w, Rmin = 50;
  QPixmap		pix( w, h );

  pix.resize( w, h );

  QPainter		p( &pix );

  p.setPen( QPen( Qt::black ) );
  p.eraseRect( 0, 0, w, h );
  if( h < R )
    R = h;
  R = R/2 - 50;
  if( R < 20 )
    R = 20;

  for( ir=refs.begin(), i = 0; ir!=fr; ++ir )
    if( (*ir)->index() == 0 )
      {
	--n;
	break;
      }

  for( ir=refs.begin(), i = 0; ir!=fr; ++ir )
    {
      ref = *ir;
      if( ref->index() == 0 )
	{
	  x = (unsigned) ( w * 0.45 );
	  y = (unsigned) ( h * 0.45 );
	}
      else
	{
	  x = (unsigned) ( R * cos( ( (float) i / n )*2.*M_PI ) + w/2 );
	  y = (unsigned) ( R * sin( ( (float) i / n )*2.*M_PI ) + h/2 );
	  ++i;
	}
      col = ref->Color();
      //p.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
      //p.drawEllipse( x-sz/2, y-sz/2, sz, sz );
      p.drawPixmap( x-sz/2, y-sz/2,
                    ReferencePixmap::referencePixmap( ref, false, sz ) );
      pdat->refpos[ref] = QPoint( x, y );
    }

  for( it=trans.begin(); it!=ft; ++it )
    {
      tr = *it;
      if( !tr->isGenerated() )
	{
	  if( pdat->refpos.find( tr->source() ) == pdat->refpos.end() 
	      || pdat->refpos.find( tr->destination() ) == pdat->refpos.end() )
	    cerr << "Transformation from " << tr->source() << " to " 
		 << tr->destination() << " : ref not registered !\n";
	  else
          {
            QPoint p1 = pdat->refpos[ tr->source() ],
              p2 = pdat->refpos[ tr->destination() ],
              p3 = ( p1 - p2 );
            p3 *= 1000. / sqrt( float( p3.x() ) * p3.x() 
                                + float( p3.y() ) * p3.y() );
            p1 -= p3 * double( sz ) / 2000.;
            p2 += p3 * double( sz ) / 2000.;
	    p.drawLine( p1, p2 );
            if( R >= Rmin )
            {
              p.drawLine( p2, p2 + p3 * 0.015 
                  + QPoint( int( -p3.y() * 0.005), int( p3.x() * 0.005 ) ) );
              p.drawLine( p2, p2 + p3 * 0.015 
                  + QPoint( int( p3.y() * 0.005 ), int( -p3.x() * 0.005 ) ) );
            }
          }
	}
    }

  p.end();
  setPixmap( pix );
}


void ReferentialWindow::mousePressEvent( QMouseEvent* ev )
{
  pdat->trans = 0;

  switch( ev->button() )
    {
    case Qt::LeftButton:
      pdat->srcref = refAt( ev->pos(), pdat->pos );
      if( pdat->srcref )
	{
	  pdat->tracking = true;
	  pdat->dstref = 0;
	}
      break;
    case Qt::RightButton:
      {
	QPoint	dummy;
	pdat->srcref = refAt( ev->pos(), dummy );
	if( pdat->srcref )
	  {
	    /*cout << "ref : " << ref->Color().r << ", " << ref->Color().g 
	      << ", " << ref->Color().b << endl;*/
	    popupRefMenu( ev->globalPos() );
	    break;
	  }
	else
	  {
	    Transformation	*tr = transfAt( ev->pos() );
	    if( tr )
	      {
		/*Referential	*src = tr->source(), *dst = tr->destination();
		cout << "trans : " << src->Color().r << ", " 
		     << src->Color().g << ", " << src->Color().b 
		     << " -> " << dst->Color().r << ", " << dst->Color().g 
		     << ", " << dst->Color().b << endl;*/
		pdat->trans = tr;
		popupTransfMenu( ev->pos() );
		break;
	      }
            else
              popupBackgroundMenu( ev->pos() );
	  }
      }
      break;
    default:
      break;
    }
}


void ReferentialWindow::mouseReleaseEvent( QMouseEvent* ev )
{
  if( pdat->tracking )
    {
      pdat->tracking = false;
      QPainter	p( this );
      p.drawPixmap( 0, 0, *pixmap() );

      QPoint		dummy;
      pdat->dstref = refAt( ev->pos(), dummy );

      if( pdat->dstref && pdat->srcref != pdat->dstref 
	  && !ATransformSet::instance()->transformation( pdat->srcref, 
							 pdat->dstref ) )
	{
	  if( ev->state() & Qt::ControlButton )
	    {
	      float	matrix[4][3];
	      matrix[0][0] = 0;
	      matrix[0][1] = 0;
	      matrix[0][2] = 0;
	      matrix[1][0] = 1;
	      matrix[1][1] = 0;
	      matrix[1][2] = 0;
	      matrix[2][0] = 0;
	      matrix[2][1] = 1;
	      matrix[2][2] = 0;
	      matrix[3][0] = 0;
	      matrix[3][1] = 0;
	      matrix[3][2] = 1;

	      LoadTransformationCommand	*com 
		= new LoadTransformationCommand( matrix, pdat->srcref, 
						 pdat->dstref );
	      theProcessor->execute( com );
	      refresh();
	    }
	  else
	    openSelectBox();
	}
    }
}


void ReferentialWindow::mouseMoveEvent( QMouseEvent* ev )
{
  if( pdat->tracking )
    {
      QPainter	p( this );
      p.drawPixmap( 0, 0, *pixmap() );
      p.drawLine( pdat->pos, ev->pos() );
    }
}


Referential* ReferentialWindow::refAt( const QPoint & pos, QPoint & newpos )
{
  map<Referential*, QPoint>::const_iterator	ir, fr = pdat->refpos.end();
  Referential		*ref;
  int			sz = 10;
  QPoint		rpos;

  for( ir=pdat->refpos.begin(); ir!=fr; ++ir )
    {
      ref = (*ir).first;
      rpos = (*ir).second;
      if( pos.x() >= rpos.x()-sz && pos.x() < rpos.x()+sz 
	  && pos.y() >= rpos.y()-sz && pos.y() < rpos.y()+sz )
	{
	  newpos = rpos;
	  return( ref );
	}
    }
  return( 0 );
}


Transformation* ReferentialWindow::transfAt( const QPoint & pos )
{
  vector<Transformation *>              trat = transformsAt( pos );
  if( trat.empty() )
    return 0;
  return trat[0];
}


vector<Transformation*> ReferentialWindow::transformsAt( const QPoint & pos )
{
  set<Transformation *>                 trans 
    = ATransformSet::instance()->allTransformations();
  set<Transformation *>::const_iterator it, ft=trans.end();
  Transformation                        *t;
  QPoint                                rvec, relp;
  float                                 x, y, normv, norm;
  vector<Transformation *>              trat;

  // 1st pass on loaded transformations
  for( it=trans.begin(); it!=ft; ++it )
    {
      t = *it;
      if( !t->isGenerated() )
        {
          const QPoint & rpos1 = pdat->refpos[ t->source() ];
          rvec = pdat->refpos[ t->destination() ] - rpos1;
          relp = pos - rpos1;
          // project
          x = relp.x() * rvec.x() + relp.y() * rvec.y();
          norm = rvec.x() * rvec.x() + rvec.y() * rvec.y();
          normv = sqrt( norm );
          //cout << "x : " << ((float) x) / norm << endl;
          if( x >= 0 && x <= norm )     // between 2 ref points
            {
              // distance from line
              y = relp.y() * rvec.x() - relp.x() * rvec.y();
              //cout << "y : " << ((float) y) / normv << endl;
              if( y >= -normv * 5 && y <= normv * 5 )
                trat.push_back( t );
            }
        }
    }

  // 2nd pass on generated transformations
  for( it=trans.begin(); it!=ft; ++it )
    {
      t = *it;
      if( t->isGenerated() )
        {
          const QPoint & rpos1 = pdat->refpos[ t->source() ];
          rvec = pdat->refpos[ t->destination() ] - rpos1;
          relp = pos - rpos1;
          // project
          x = relp.x() * rvec.x() + relp.y() * rvec.y();
          norm = rvec.x() * rvec.x() + rvec.y() * rvec.y();
          normv = sqrt( norm );
          //cout << "x : " << ((float) x) / norm << endl;
          if( x >= 0 && x <= norm )     // between 2 ref points
            {
              // distance from line
              y = relp.y() * rvec.x() - relp.x() * rvec.y();
              //cout << "y : " << ((float) y) / normv << endl;
              if( y >= -normv * 5 && y <= normv * 5 )
                trat.push_back( t );
            }
        }
    }

  return trat;
}


void ReferentialWindow::popupRefMenu( const QPoint & pos )
{
  QPopupMenu	*pop = pdat->refmenu;

  if( !pop )
    {
      pop = new QPopupMenu( this );
      pdat->refmenu = pop;

      pop->insertItem( QPixmap( 16, 16 ), 0 );
      pop->insertSeparator();
      pop->insertItem( tr( "Delete referential" ), this, 
			   SLOT( deleteReferential() ), 0, 10 );
      pop->insertItem( tr( "Load referential information" ), this,
                       SLOT( loadReferential() ), 0, 11 );
    }
  pop->setItemEnabled( 10, pdat->srcref->index() != 0 );
  pop->setItemEnabled( 11, pdat->srcref->index() != 0 );

  QPixmap	pix( 16, 16 );
  QPainter	p( &pix );
  AimsRGB	col = pdat->srcref->Color();

  p.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
  p.fillRect( 0, 0, 16, 16, pop->backgroundColor() );
  p.drawEllipse( 0, 0, 16, 16 );
  p.end();
  pix.setMask( pix.createHeuristicMask() );
  pop->changeItem( 0, pix );

  pop->popup( pos );
}


void ReferentialWindow::popupTransfMenu( const QPoint & pos )
{
  vector<Transformation *>  trans = transformsAt( pos );
  if( trans.empty() )
    return;

  unsigned        i, n = trans.size();
  Transformation  *t;
  QPopupMenu      pop( this );
  vector<ReferentialWindow_TransCallback *> cbks;
  ReferentialWindow_TransCallback           *cbk;

  cbks.reserve( n );

  for( i=0; i<n; ++i )
  {
    t = trans[i];

    QPixmap     pix( 64, 16 );

    QPainter    p( &pix );
    AimsRGB     col = t->source()->Color();
    //cout << "source: " << t->source() << ": " << col << endl;

    p.setBackgroundMode( Qt::OpaqueMode );
    p.fillRect( 0, 0, 64, 16, pop.backgroundColor() );
    p.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
    p.drawEllipse( 0, 0, 16, 16 );
    col = t->destination()->Color();
    // cout << "dest: " << t->source() << ": " << col << endl;
    p.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
    p.drawEllipse( 48, 0, 16, 16 );
    p.drawLine( 16, 8, 48, 8 );
    p.drawLine( 40, 4, 48, 8 );
    p.drawLine( 40, 12, 48, 8 );
    p.end();
    pix.setMask( pix.createHeuristicMask() );
    pop.insertItem( pix, 0 );

    pop.insertSeparator();
    cbk = new ReferentialWindow_TransCallback( this, t );
    cbks.push_back( cbk );
    if( !t->isGenerated() )
    {
      pop.insertItem( tr( "Delete transformation" ), cbk,
                      SLOT( deleteTransformation() ) );
      pop.insertItem( tr( "Invert transformation" ), cbk,
                      SLOT( invertTransformation() ) );
      pop.insertItem( tr( "Reload transformation" ), cbk,
                      SLOT( reloadTransformation() ) );
    }
    pop.insertItem( tr( "Save transformation..." ), cbk,
                    SLOT( saveTransformation() ) );
    if( i < n-1 )
      pop.insertSeparator();
  }

  pop.exec( mapToGlobal( pos ) );

  for( i=0; i<n; ++i )
    delete cbks[i];
}


void ReferentialWindow::popupBackgroundMenu( const QPoint & pos )
{
  QPopupMenu    *pop = pdat->bgmenu;

  if( !pop )
  {
    pop = new QPopupMenu( this );
    pdat->bgmenu = pop;

    pop->insertItem( tr( "New referential" ), this,
                          SLOT( newReferential() ) );
    pop->insertItem( tr( "Load referential" ), this,
                          SLOT( loadReferential() ) );
    pop->insertItem( tr( "Load transformation" ), this,
                          SLOT( loadNewTransformation() ) );
  }

  pop->popup( mapToGlobal( pos ) );
}


void ReferentialWindow::deleteReferential()
{
  delete pdat->srcref;
  theAnatomist->Refresh();
}


void ReferentialWindow::deleteTransformation( Transformation* trans )
{
  delete trans;
  refresh();

  set<AWindow *>		win = theAnatomist->getWindows();
  set<AWindow*>::iterator	iw, fw = win.end();

  for( iw=win.begin(); iw!=fw; ++iw )
    (*iw)->SetRefreshFlag();
  theAnatomist->Refresh();
}


void ReferentialWindow::invertTransformation( Transformation* trans )
{
  Transformation	*other
    = ATransformSet::instance()->transformation( trans->destination(),
						 trans->source() );
  if( !other )
    {
      cerr << "BUG: inverse transformation not found\n";
      return;
    }
  *other = *trans;
  trans->invert();

  set<AWindow *>		win = theAnatomist->getWindows();
  set<AWindow*>::iterator	iw, fw = win.end();

  for( iw=win.begin(); iw!=fw; ++iw )
    (*iw)->SetRefreshFlag();
  theAnatomist->Refresh();
}


void ReferentialWindow::reloadTransformation( Transformation* trans )
{
  pdat->srcref = trans->source();
  pdat->dstref = trans->destination();
  openSelectBox();
}


void ReferentialWindow::newReferential()
{
  set<AObject *>  o;
  set<AWindow *>  w;
  AssignReferentialCommand	*com
      = new AssignReferentialCommand( 0, o, w, -1 );
  theProcessor->execute( com );
}


void ReferentialWindow::loadReferential()
{
  QString filter = tr( "Referential" );
  filter += " (*.referential);; ";
  filter += ControlWindow::tr( "All files" );
  filter += " (*)";
  QFileDialog   & fd = fileDialog();
  fd.setFilters( filter );
  fd.setCaption( tr( "Load referential information" ) );
  fd.setMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;
  QString filename = fd.selectedFile();
  set<AObject *> o;
  set<AWindow *> w;
  AssignReferentialCommand  *com
      = new AssignReferentialCommand( pdat->srcref, o, w, -1, 0,
                                      filename.utf8().data() );
  theProcessor->execute( com );
}


void ReferentialWindow::loadNewTransformation()
{
  QString filter = tr( "Transformation" );
  filter += " (*.trm *TO*);; ";
  filter += ControlWindow::tr( "All files" );
  filter += " (*)";
  QFileDialog   & fd = fileDialog();
  fd.setFilters( filter );
  fd.setCaption( tr( "Open transformation" ) );
  fd.setMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;
  QString filename = fd.selectedFile();
  pdat->srcref = 0;
  pdat->dstref = 0;
  loadTransformation( filename.utf8().data() );
}


#if QT_VERSION >= 0x040000
bool ReferentialWindow::event( QEvent* event )
{
  if (event->type() == QEvent::ToolTip)
  {
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
    pdat->tooltip->maybeTip( helpEvent->pos() );
  }
  return QWidget::event(event);
}
#endif


// -----------

ReferentialWindow_TransCallback::ReferentialWindow_TransCallback
  ( ReferentialWindow* rw, Transformation* t )
  : QObject(), refwin( rw ), trans( t )
{
}

ReferentialWindow_TransCallback::~ReferentialWindow_TransCallback()
{
}

void ReferentialWindow_TransCallback::deleteTransformation()
{
  refwin->deleteTransformation( trans );
}

void ReferentialWindow_TransCallback::invertTransformation()
{
  refwin->invertTransformation( trans );
}

void ReferentialWindow_TransCallback::reloadTransformation()
{
  refwin->reloadTransformation( trans );
}

void ReferentialWindow_TransCallback::saveTransformation()
{
  refwin->saveTransformation( trans );
}
