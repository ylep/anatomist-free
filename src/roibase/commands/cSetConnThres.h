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


#ifndef ROI_COMMANDS_CSETCONNTHRES_H
#define ROI_COMMANDS_CSETCONNTHRES_H

#include <anatomist/processor/Command.h>


#include <anatomist/processor/Command.h>
#include <set>


namespace anatomist
{

class AObject;
class AWindow;

///	Assign a referential to objects and/or windows
class SetConnThresholdCommand : public RegularCommand, 
				public SerializingCommand
{
public:
  /**	@param	ref	pointer to a referential. 0 = new ref or no ref.
	@param	obj	objects to be assigned this referential
	@param	win	windows to be assigned this referential
	@param	refId	referential Id for created referential, 
			used only if ref is null for loaded commands. 
			If RefId < 0, the ID is not recorded into the 
			unserializer. 
			Both ref and refId null means no ref at all
	@param	unser	Unserializer pointer
  */
  SetConnThresholdCommand( AObject* image, AWindow* win, bool assignLowThreshold, 
			   float lowThres, bool assignHighThreshold, float highThres,
			   CommandContext* context = 0 ) ;
  
  virtual ~SetConnThresholdCommand();

  virtual std::string name() const { return( "SetConnThreshold" ); }
  virtual void write( Tree & com, Serializer* ser ) const;
  
protected:
  virtual void doit();

private:
  AObject *             _image ;
  AWindow *             _win ;
  bool                  _assignLowThreshold ;
  float                 _lowThres ;
  bool                  _assignHighThreshold ;
  float                 _highThres ;

  friend class StdModule;
  static Command* read( const Tree & com, CommandContext* context );
  static bool initSyntax();
};

}
#endif
