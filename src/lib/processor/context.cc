/* Copyright (c) 1995-2006 CEA
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

#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <iostream>
#include <unistd.h>

using namespace anatomist;
using namespace carto;
using namespace std;


CommandContext::CommandContext()
  : MutexedObject(), 
    ostr( 0 ), ownstream( false ), istr( 0 ), infd( -1 ), unserial( 0 ), 
    evfilter( new OutputEventFilter )
{
  contextsP().insert( this );
}


CommandContext::~CommandContext()
{
  close();
  closein();
  delete evfilter;
  contextsP().erase( this );
}


void CommandContext::attach( ostream* stm, bool own )
{
  close();
  ostr = stm;
  ownstream = own;
}


void CommandContext::close()
{
  if( ownstream )
    delete ostr;
  ostr = 0;
  ownstream = false;
}


void CommandContext::closein()
{
  delete istr;
  istr = 0;
  if( infd >= 0 )
    {
      ::close( infd );
      infd = -1;
    }
}


CommandContext & CommandContext::defaultContext()
{
  static	CommandContext	cc;
  if( !cc.ostr )
    {
      cc.ostr = &cout;
      cc.ownstream = false;
      cc.unserial = rc_ptr<Unserializer>( new Unserializer );
    }
  return cc;
}


set<CommandContext *> & CommandContext::contextsP()
{
  static set<CommandContext *> contexts;
  return contexts;
}


const set<CommandContext *> & CommandContext::contexts()
{
  return contextsP();
}


