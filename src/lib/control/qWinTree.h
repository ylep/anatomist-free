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


#ifndef ANAQT_CONTROL_QWINTREE_H
#define ANAQT_CONTROL_QWINTREE_H


#include <qwidget.h>
#include <map>
#include <string>
#include <set>
#include <qglobal.h>

class QPixmap;
class QTreeWidgetItem;
class QTreeWidget;


namespace anatomist
{
  class AWindow;
}


/**	Qt window tree widget for the control wondow
 */
class QWindowTree : public QWidget
{
  Q_OBJECT

public:
  QWindowTree( QWidget* parent, const char *name );
  virtual ~QWindowTree();

  virtual void registerWindow( anatomist::AWindow* obj );
  virtual void unregisterWindow( anatomist::AWindow* obj );
  virtual void NotifyWindowChange( anatomist::AWindow* obj );

  virtual std::set<anatomist::AWindow *> *SelectedWindows() const;
  std::set<int> SelectedGroups() const;
  virtual void SelectWindow( anatomist::AWindow *obj );
  virtual bool isWindowSelected( anatomist::AWindow* obj ) const;
  virtual void UnselectAll();
  void SelectGroup( int group );

  ///	Are reference colors markers visible ?
  virtual bool ViewingRefColors() const;
  virtual void ToggleRefColorsView();
  virtual void DisplayRefColors();
  virtual void UndisplayRefColors();

  static unsigned 			RefPixSize;
  static QPixmap			*LinkIcon;

signals:
  void selectionChanged();
  void doubleClicked( anatomist::AWindow * );

protected:
  virtual QTreeWidgetItem* insertWindow( QTreeWidgetItem* item, 
                                         anatomist::AWindow* obj );
  virtual QTreeWidgetItem* insertWindow( QTreeWidget* lview, 
                                         anatomist::AWindow* obj );
  virtual void decorateItem( QTreeWidgetItem* item, anatomist::AWindow* obj );
  virtual void dragEnterEvent( QDragEnterEvent* );
  virtual void dragMoveEvent( QDragMoveEvent* );
  bool dragMoveEventFromObject( QDragMoveEvent* );
  bool dragMoveEventFromWindow( QDragMoveEvent* );
  virtual void dragLeaveEvent( QDragLeaveEvent* );
  virtual void dropEvent( QDropEvent* );
  void dropEventFromObject( QDropEvent* );
  void dropEventFromWindow( QDropEvent* );
  void highlightWindow( anatomist::AWindow *win, bool state );
  void clearWindowsHighlights();

public slots:
  virtual void startDragging( QTreeWidgetItem*, Qt::MouseButtons,
                              Qt::KeyboardModifiers );
  void sortIndicatorChanged( int, Qt::SortOrder );

protected slots:
  void doubleClickedSlot( QTreeWidgetItem *, int );
  void unselectInvisibleItems();
  void itemChanged( QTreeWidgetItem *, int );
  void raiseDropWindows();

private:
  struct Private;
  Private *d;
};


#endif
