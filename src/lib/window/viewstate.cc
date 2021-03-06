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

#include <anatomist/window/viewstate.h>
#include <anatomist/window/Window.h>

using namespace anatomist;
using namespace aims;
using namespace std;


ViewState::ViewState( float t, AWindow* win,
                      glSelectRenderMode selectrendermode )
  : timedims( 1, t ), window( win ), selectRenderMode( selectrendermode )
{
}


ViewState::ViewState( const vector<float> & timedims, AWindow* win,
                      glSelectRenderMode selectrendermode )
  : timedims( timedims ), window( win ), selectRenderMode( selectrendermode )
{
}


ViewState::~ViewState()
{
}


SliceViewState::SliceViewState( float t, bool slicewanted, 
                                const Point3df & pos, 
                                const Quaternion *orient, 
                                const Referential* wref, 
                                const Geometry* wgeom,
                                const Quaternion* vorient,
                                AWindow* win,
                                glSelectRenderMode selectrendermode )
  : ViewState( t, win, selectrendermode ), wantslice( slicewanted ),
    position( pos ), orientation( orient ), winref( wref ), wingeom( wgeom ),
    vieworientation( vorient )
{
  if( win )
  {
    if( !wref )
      winref = win->getReferential();
    if( !wgeom )
      wingeom = win->windowGeometry();
  }
}


SliceViewState::SliceViewState( const vector<float> & timedims,
                                bool slicewanted,
                                const Point3df & pos,
                                const Quaternion *orient,
                                const Referential* wref,
                                const Geometry* wgeom,
                                const Quaternion* vorient,
                                AWindow* win,
                                glSelectRenderMode selectrendermode )
  : ViewState( timedims, win, selectrendermode ), wantslice( slicewanted ),
    position( pos ), orientation( orient ), winref( wref ), wingeom( wgeom ),
    vieworientation( vorient )
{
  if( win )
  {
    if( !wref )
      winref = win->getReferential();
    if( !wgeom )
      wingeom = win->windowGeometry();
  }
}


SliceViewState::~SliceViewState()
{
}


