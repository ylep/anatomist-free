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
#include "mipShader.h"
#include "shaderFactory.h"

#include <anatomist/window/glcaps.h>

using namespace anatomist;

Vr::MIPShader::MIPShader() : Shader() 
{
}


std::string Vr::MIPShader::getName() const 
{ 

  return getStaticName();

}


std::string Vr::MIPShader::getStaticName()
{

  return "MIPShader";

}


void Vr::MIPShader::setBlending()
{

  glEnable( GL_BLEND );
  GLCaps::glBlendEquation( GL_MAX );
  glBlendFunc( GL_SRC_ALPHA, GL_DST_ALPHA );

}


std::map< float, std::list< Vr::Vector3d > >& 
Vr::MIPShader::getSlices( const float* m, const Vr::Vector3d&, const int )
{

  return sl.getSlices( m );

}


RegisterShader( MIPShader );
