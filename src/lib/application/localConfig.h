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


#ifndef ANATOMIST_APPLICATION_LOCALCONFIG_H
#define ANATOMIST_APPLICATION_LOCALCONFIG_H


namespace anatomist
{

  class GlobalConfiguration;


  /**	Local configurator: used by program parts (modules, ...) to setup 
	their part of the global config \see GlobalConfiguration
  */
  class LocalConfiguration
  {
  public:
    LocalConfiguration() {}
    virtual ~LocalConfiguration();

    /**	applies configuration to the program part. This function must be 
	derived by inherited classes, and is aimed to pick the specific 
	attributes in the global config and apply these settings to 
	the program part (module...) */
    virtual void apply( const GlobalConfiguration* cfg ) = 0;
    /**	Updates the global config with the specific local options.
	This function is not absolutely needed if the global config is 
	maintained up to date by program parts each time a setting changes */
    virtual void update( GlobalConfiguration* ) {}
  };

}


#endif
