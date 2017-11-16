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


#include <anatomist/winprof/profY.h>


using namespace anatomist;


double *QAProfileY::abscisse( Point4df& pmin, int pdim )
{
  double *x = new double[ pdim ];

  for ( int t=0; t<pdim; t++ )
    x[ t ] = (double)t + pmin[1];

  return x;
}


/*
double *QAProfileY::doit( AObject *d, Point3df& pt, float t, Point4df& pmin, 
			  int pdim )
{
  Point3df	bmin, bmax;
  d->boundingBox( bmin, bmax );
  vector<float> vs = d->voxelSize();
  float sy = vs[ 1 ];
  int dY = (int)( ( bmax[1] - bmin[1] ) / sy );
  int omin = (int)( bmin[1] / sy );
  double *y = new double[ pdim ];

  for ( int i=0; i<dY; i++ )
    {
      Point3df pos( pt[0], (float)i, pt[2] );
      y[ omin + i - (int)( pmin[1] / sy ) ] = d->mixedTexValue( pos, t );
    }

  return y;
}
*/


int QAProfileY::size( Point4df& pmin, Point4df& pmax )
{
  return (int)( pmax[1] - pmin[1] + 1 );
}


double QAProfileY::markerPos( Point3df& pt, float, Point4df& pmin )
{
  return (double)( pt[1] - pmin[1] );
}
