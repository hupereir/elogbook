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

#include <QAction>
#include <QApplication>
#include <QCursor>

#include "ApplicationManager.h"
#include "ArgList.h"
#include "Counter.h"



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
    assert( selection_frame_
 );
    return *selection_frame_; 
  }
  
  //! retrieve AttachmentFrame singleton
  AttachmentFrame & attachmentFrame( void ) const
  { 
    assert( attachment_frame_
 );
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
  
  //!@name actions
  //@{
  
  //! about
  QAction& aboutAction( void ) const
  { return *about_action_; }

  //! about
  QAction& aboutQtAction( void ) const
  { return *aboutqt_action_; }
  
  //! configuration
  QAction& configurationAction( void ) const
  { return *configuration_action_; }
  
  //! exit safely
  QAction& closeAction( void ) const
  { return *close_action_; }
  
  //@}
  
  signals:
  
  //! request widget to save their current configuration
  void saveConfiguration( void );
  
  //! configuration has changed
  void configurationChanged( void );
  
  public slots:
  
  //! show splash screen
  void showSplashScreen( void );
  
  private slots:
  
  //! configuration
  void _configuration( void );
  
  //! Update Configuration
  void _updateConfiguration( void );
      
  //! about eLogbook
  void _about( void );
  
  //! exit safely
  void _exit( void );
  
  //! process request from application manager
  void _processRequest( const ArgList&);
  
  //! application manager state is changed
  void _applicationManagerStateChanged( SERVER::ApplicationManager::State );
  
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

  //!@name actions
  //@{
  
  //! about
  QAction* about_action_;
  
  //! about qt
  QAction* aboutqt_action_;
  
  //! configure
  QAction* configuration_action_;
  
  //! close
  QAction* close_action_;
  
  //@}
  
};

#endif  
