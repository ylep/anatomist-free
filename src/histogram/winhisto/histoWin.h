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


#ifndef HISTO_WIN_H
#define HISTO_WIN_H

#include <anatomist/window/qwindow.h>
#include <anatomist/winhisto/histo.h>
#include <set>
#include <map>


class QwtPlot;


class QAHistogramWindow : public QAWindow
{
  Q_OBJECT

public:

  enum HistogramSize
  {
    histogramWidth  = 512,
    histogramHeight = 256,
  };

  QAHistogramWindow( QWidget *parent=0, const char *name=0, 
                     carto::Object options = carto::none(), 
                     Qt::WFlags f = Qt::WType_TopLevel | Qt::WDestructiveClose );
  virtual ~QAHistogramWindow();

  static AWindow *createHistogramWindow( void *, carto::Object );
  virtual Type type() const { return (Type)_classType; }

  virtual void displayClickPoint();

  virtual void refreshNow();

  virtual void registerObject( anatomist::AObject* object, 
			       bool temporaryObject = false );
  virtual void unregisterObject( anatomist::AObject* object, 
                                 bool temporaryObject = false );

  virtual const std::set<unsigned> & typeCount() const;
  virtual std::set<unsigned> & typeCount();
  virtual const std::string & baseTitle() const;

protected:

  virtual void Draw( bool flush = true );

private:
  struct Private;
  Private	*d;

  QwtPlot *graphic;

  QAHistogram qaHisto;

  double *x_curve;
  std::map< anatomist::AObject *, double * > phisto;

  void initX();
  QRgb newColor( int );

  static int registerClass();

  static std::set< unsigned > _histCount;
  static std::string          _baseTitle;
  static int             _classType;
};

#endif
