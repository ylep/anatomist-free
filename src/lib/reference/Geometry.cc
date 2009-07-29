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


//--- header files ------------------------------------------------------------

#include <anatomist/reference/Geometry.h>


using namespace anatomist;
using namespace std;


//--- methods -----------------------------------------------------------------

Geometry::Geometry()
{
  _size[0] = 0.;
  _size[1] = 0.;
  _size[2] = 0.;
  _dimMin[0] = 0;
  _dimMin[1] = 0;
  _dimMin[2] = 0;
  _dimMin[3] = 0;
  _dimMax[0] = 0;
  _dimMax[1] = 0;
  _dimMax[2] = 0;
  _dimMax[3] = 0;
}

Geometry::Geometry(Point3df size,Point4d dimMin,Point4d dimMax)
{
  _size[0] = size[0];
  _size[1] = size[1];
  _size[2] = size[2];
  _dimMin[0] = dimMin[0];
  _dimMin[1] = dimMin[1];
  _dimMin[2] = dimMin[2];
  _dimMin[3] = dimMin[3];
  _dimMax[0] = dimMax[0];
  _dimMax[1] = dimMax[1];
  _dimMax[2] = dimMax[2];
  _dimMax[3] = dimMax[3];
}

Geometry::Geometry( const Geometry & g )
{
  *this = g;
}

Geometry::~Geometry()
{ 
}

Geometry & Geometry::operator = ( const Geometry & g )
{
  if( &g != this )
    {
      _size = g._size;
      _dimMin = g._dimMin;
      _dimMax = g._dimMax;
    }

  return( *this );
}


void Geometry::SetSize( Point3df size )
{
  _size[0] = size[0];
  _size[1] = size[1];
  _size[2] = size[2];
}

void Geometry::SetDimMin( Point4d dimMin )
{
  _dimMin[0] = dimMin[0];
  _dimMin[1] = dimMin[1];
  _dimMin[2] = dimMin[2];
  _dimMin[3] = dimMin[3];
}

void Geometry::SetDimMax( Point4d dimMax )
{
  _dimMax[0] = dimMax[0];
  _dimMax[1] = dimMax[1];
  _dimMax[2] = dimMax[2];
  _dimMax[3] = dimMax[3];
}
