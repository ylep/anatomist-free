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


#include <anatomist/winprof/profStrategy.h>


using namespace anatomist;


double *QAProfileStrategy::abscisse( Point4df&, int )
{
  return (double *)0;
}


double *QAProfileStrategy::doit( AObject *d, Point3df& pt, float t, 
                                 Point4df& pmin, int pdim, 
                                 const Point4df & increment )
{
  Point3df vs = d->VoxelSize();
  float sx = vs[ 0 ];
  double *y = new double[ pdim ];
  Point3df pos, incs( increment[0], increment[1], increment[2] );
  float inct = increment[3];
  int i;

  pos = pt;
  pos[0] /= vs[0];
  pos[1] /= vs[1];
  pos[2] /= vs[2];
  incs[0] /= vs[0];
  incs[1] /= vs[1];
  incs[2] /= vs[2];

  for( i=0; i<pdim; i++, pos += incs, t += inct )
  {
    y[ i ] = d->mixedTexValue( pos, t );
  }

  return y;
}




int QAProfileStrategy::size( Point4df&, Point4df& )
{
  return 0;
}


double QAProfileStrategy::markerPos( Point3df&, float, Point4df& )
{
  return 0.0;
}
