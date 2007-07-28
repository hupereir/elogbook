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

#ifndef MainFrame_h
#define MainFrame_h

/*!
  \file    MainFrame.h
  \brief  Main Window singleton object
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QApplication>
#include <QCursor>
#include <QMessageBox>

#include "ApplicationManager.h"
#include "ArgList.h"
#include "Counter.h"
#include "Exception.h"

class AttachmentFrame;
class LogEntry;
class SelectionFrame;

/*!
  \class  MainFrame
  \brief  Main Window singleton object
*/

class MainFrame: public QApplication, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:
  
  //! command line help
  static void usage( void );
  
  //! constructor
  MainFrame( int argc, char*argv[] ); 
  
  //! destructor
  ~MainFrame( void );
  
  //! window title for modified logbook
  static const QString MAIN_TITLE_MODIFIED;

  //! default window title
  static const QString MAIN_TITLE;

  //! default window title
  static const QString ATTACHMENT_TITLE;
    
  //! initialize application manager
  void initApplicationManager( void );
  
  //! create all widgets
  void realizeWidget( void );
  
  //! retrieve SelectionFrame singleton
  SelectionFrame& selectionFrame( void ) const
  { 
    Exception::checkPointer( selection_frame_, DESCRIPTION("selection_frame_ not initialized") );
    return *selection_frame_; 
  }
  
  //! retrieve AttachmentFrame singleton
  AttachmentFrame & attachmentFrame( void ) const
  { 
    Exception::checkPointer( attachment_frame_, DESCRIPTION("attachment_frame_ not initialized") );
    return *attachment_frame_; 
  }
  
  //! set application busy
  void busy( void ) 
  {
    setOverrideCursor( Qt::WaitCursor ); 
    processEvents(); 
  }
  
  //! set application idle
  void idle( void )
  { restoreOverrideCursor(); }
  
  signals:
  
  // configuration has changed
  void configurationChanged( void );
  
  public slots:
  
  //! configuration
  void configuration( void );
  
  //! Update Configuration
  void updateConfiguration( void );
      
  //! about eLogbook
  void about( void );
  
  //! show splash screen
  void showSplashScreen( void );
  
  //! exit safely
  void exit( void );
  
  private slots:
  
  //! process request from application manager
  void _processRequest( const ArgList&);
  
  //! application manager state is changed
  void _applicationManagerStateChanged( SERVER::ApplicationManager::State );
  
  //! actions called when application is about to quit
  void _aboutToQuit( void );
          
  private:
  
  //! command line arguments
  ArgList args_;
  
  //! application manager
  SERVER::ApplicationManager* application_manager_;
  
  //! toplevel attachment frame
  AttachmentFrame* attachment_frame_;
  
  //! main window entry selection frame
  SelectionFrame* selection_frame_; 
  
  //! true when Realized Widget has been called.
  bool realized_; 

};

#endif  
