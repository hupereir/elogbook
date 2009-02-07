// $Id$

/******************************************************************************
*                        
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>            
*                        
* This is free software; you can redistribute it and/or modify it under the    
* terms of the GNU General Public License as published by the Free Software    
* Foundation; either version 2 of the License, or (at your option) any later  
* version.                            
*                         
* This software is distributed in the hope that it will be useful, but WITHOUT 
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        
* for more details.                    
*                         
* You should have received a copy of the GNU General Public License along with 
* software; if not, write to the Free Software Foundation, Inc., 59 Temple    
* Place, Suite 330, Boston, MA  02111-1307 USA                          
*                        
*                        
*******************************************************************************/

#ifndef Application_h
#define Application_h

/*!
  \file    Application.h
  \brief  Main Window singleton object
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "BaseApplication.h"
#include "CommandLineArguments.h"
#include "CommandLineParser.h"
#include "Config.h"
#include "Counter.h"

class AttachmentWindow;
class FileList;
class LogEntry;
class MainWindow;

/*!
  \class  Application
  \brief  Main Window singleton object
*/

class Application: public BaseApplication, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:
    
  //! command line help
  static void usage( void );
  
  //! constructor
  Application( CommandLineArguments ); 
  
  //! destructor
  ~Application( void );
  
  //! window title for modified logbook
  static const QString MAIN_TITLE_MODIFIED;

  //! default window title
  static const QString MAIN_TITLE;

  //! default window title
  static const QString ATTACHMENT_TITLE;
  
  //! application manager
  virtual void initApplicationManager( void );
  
  //! create all widgets
  virtual bool realizeWidget( void );
  
  //! file list
  FileList& recentFiles( void ) const
  { 
    assert( recent_files_ );
    return *recent_files_;
  }
  
  //! retrieve AttachmentWindow singleton
  AttachmentWindow & attachmentWindow( void ) const
  { 
    assert( attachment_window_ );
    return *attachment_window_; 
  }
  
  //! retrieve MainWindow singleton
  MainWindow& mainWindow( void ) const
  { 
    assert( main_window_ );
    return *main_window_; 
  }

  protected slots:
      
  //! about eLogbook
  virtual void _about( void )
  { BaseApplication::_about( MAIN_TITLE, VERSION, BUILD_TIMESTAMP ); }
  
  //! configuration
  virtual void _configuration( void );
  
  //! exit safely
  void _exit( void );
  
  //! process request from application manager
  void _processRequest( const CommandLineArguments&);
  
  private:
 
  //! recent files
  FileList* recent_files_;
  
  //! toplevel attachment frame
  AttachmentWindow* attachment_window_;
  
  //! main window entry selection frame
  MainWindow* main_window_; 

};

#endif  
