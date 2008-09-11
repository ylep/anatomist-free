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


#include <anatomist/fusion/fusionChooser.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <aims/qtcompat/qhbox.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/object/Object.h>
#include <aims/listview/orderinglist.h>
#include <qtextedit.h>
#include <aims/qtcompat/qlistbox.h>


using namespace anatomist;
using namespace aims::gui;
using namespace std;


struct FusionChooser::Private
{
  Private( const vector<AObject *>* objects );

  vector<FusionMethod *>		methods;
  FusionMethod				*selectedMethod;
  QTextEdit				*help;
  QOrderingListBox			*order;
  vector<AObject *>			objects;
  map<QListBoxItem *, AObject *>	objectsmap;
};


FusionChooser::Private::Private( const vector<AObject *>* obj )
  : selectedMethod( 0 ), help( 0 ), order( 0 )
{
  if( obj )
    {
      vector<AObject *>::const_iterator	i, e = obj->end();
      objects.reserve( obj->size() );
      for( i=obj->begin(); i!=e; ++i )
        objects.push_back( *i );
    }
}


FusionChooser::FusionChooser( const set<FusionMethod *> & methods, 
			      QWidget* parent, const char *name, 
			      bool modal, Qt::WFlags f, 
                              const vector<AObject *>* objects )
  : QDialog( parent, name, modal, f ), d( new Private( objects ) )
{
  set<FusionMethod *>::const_iterator	ims, ems = methods.end();
  d->methods.reserve( methods.size() );
  for( ims=methods.begin(); ims!=ems; ++ims )
    d->methods.push_back( *ims );

  setCaption( tr( "Which fusion ?" ) );
  QVBoxLayout	*mainlay = new QVBoxLayout( this, 5, 5 );
  QComboBox	*met = new QComboBox( false, this );
  d->help = new QTextEdit( this );
  d->help->setReadOnly( true );
  if( objects && objects->size() > 1 )
    d->order = new QOrderingListBox( this );
  QHBox		*butts = new QHBox( this );
  butts->setSpacing( 10 );
  QPushButton	*ok = new QPushButton( tr( "OK" ), butts );
  QPushButton	*cancel = new QPushButton( tr( "Cancel" ), butts );

  mainlay->addWidget( met );
  mainlay->addWidget( d->help );
  if( d->order )
    mainlay->addWidget( d->order );
  mainlay->addWidget( butts );
  ok->setDefault( true );

  vector<FusionMethod *>::const_iterator	im, fm=d->methods.end();
  for( im=d->methods.begin(); im!=fm; ++im )
    met->insertItem( (*im)->ID().c_str() );
  if( !d->methods.empty() )
    d->selectedMethod = d->methods[0];

  setMethod( 0 );

  if( d->order )
    {
      vector<AObject *>::const_iterator	i, e = d->objects.end();
      int				x = 0;
      for( i=d->objects.begin(); i!=e; ++i, ++x )
        {
          d->order->qListBox()->insertItem( (*i)->name().c_str() );
          d->objectsmap[ d->order->qListBox()->item( x ) ] = *i;
        }
    }

  connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( met, SIGNAL( activated( int ) ), this, SLOT( setMethod( int ) ) );
}


FusionChooser::~FusionChooser()
{
  delete d;
}


void FusionChooser::setMethod( int meth )
{
  d->selectedMethod = d->methods[ meth ];
  d->help->setText( tr( d->methods[meth]->ID().c_str() ) );
}


FusionMethod* FusionChooser::selectedMethod() const
{
  return d->selectedMethod;
}


vector<AObject *> FusionChooser::objects() const
{
  vector<AObject *>	obj;
  if( !d->order )
    return obj;

  unsigned		i, n = d->order->qListBox()->count();

  obj.reserve( n );
  for( i=0; i<n; ++i )
    obj.push_back( d->objectsmap[ d->order->qListBox()->item( i ) ] );
  return obj;
}


