/************************************************************************
* This file is part of UaNavigator					*	
*                                                                      	*
* Copyright 2020 Matias Henttunen					*
*                                                                      	*
* FINAL CUT is free software; you can redistribute it and/or modify    	*
* it under the terms of the GNU Lesser General Public License as       	*
* published by the Free Software Foundation; either version 3 of       	*
* the License, or (at your option) any later version.                  	*
*                                                                      	*
* FINAL CUT is distributed in the hope that it will be useful, but     	*
* WITHOUT ANY WARRANTY; without even the implied warranty of           	*
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        	*
* GNU Lesser General Public License for more details.                  	*
*                                                                      	*
* You should have received a copy of the GNU Lesser General Public     	*
* License along with this program.  If not, see                        	*
* <http://www.gnu.org/licenses/>.                                      	*
*************************************************************************/


#include <algorithm>
#include "mainwindow.h"
#include "basewindow.h"
#include "connectwindow.h"
#include "settingswindow.h"
#include "helpwindow.h"
#include "serverdialog.h"
#include "writewindow.h"
#include "opcua.h"



MainWindow::MainWindow (finalcut::FWidget* parent) : finalcut::FDialog{parent}
{
	FDialog::setSize ({40, 6});
	File.setStatusbarMessage ("File management commands");
	DglList.setStatusbarMessage ("List of all the active dialogs");
	configureFileMenuItems();
	configureDialogButtons();
	Statusbar.setMessage("Status bar message");
	active = nullptr;
	port = 4840;
	host = "cx-18ec19";

}

MainWindow::~MainWindow()
{
}


void MainWindow::openConnection( std::string a_host, uint32_t a_port )
{
	port = a_port;
	host = a_host;
	startWithConnection=true;

}

void MainWindow::configureFileMenuItems()
{
	New.setStatusbarMessage ("Open");
	Close.setStatusbarMessage ("Close");
	Line1.setSeparator();
	Next.addAccelerator (fc::Fmkey_npage);  			// Meta/Alt + PgDn
	Next.setStatusbarMessage ("Switch to the next window");
	Previous.addAccelerator (fc::Fmkey_ppage); 	 		// Meta/Alt + PgUp
	Previous.setStatusbarMessage ("Switch to the previous window");
	Line2.setSeparator();
	Quit.addAccelerator (fc::Fmkey_x);  				// Meta/Alt + X
	Quit.setStatusbarMessage ("Exit the program");

	Settings.addAccelerator (fc::Fmkey_s);  				// Meta/Alt + S
	Settings.setStatusbarMessage ("Exit the program");

	About.addAccelerator (fc::Fmkey_h);  				// Meta/Alt + h
	About.setStatusbarMessage ("About");



	addClickedCallback (&New, this, &MainWindow::cb_connectWindows);
	addClickedCallback (&Close, this, &MainWindow::cb_closeWindows);
	addClickedCallback (&Next, this, &MainWindow::cb_next);
	addClickedCallback (&Previous, this, &MainWindow::cb_previous);
	addClickedCallback ( &Quit , finalcut::getFApplication() , &finalcut::FApplication::cb_exitApp , this );
	addClickedCallback ( &Settings ,  this, &MainWindow::cb_settings );
	addClickedCallback (&About, this, &MainWindow::cb_about);

	if( startWithConnection) {
		OpcUa* connection = new OpcUa();
		if( connection->connect( (char*)host.c_str(), port ) )  {
			ServerDialog* d = new ServerDialog(this, connection );
			addWindow(d);
			d->show();
		}
	}
}

void MainWindow::configureDialogButtons()
{
	CreateButton.setGeometry (FPoint{2, 2}, FSize{9, 1});
	CreateButton.setText (L"&Connect");
	addClickedCallback (&CreateButton, this, &MainWindow::cb_connectWindows); // Add button callback
}

void MainWindow::activateWindow (finalcut::FDialog* win) const
{
	if ( ! win || win->isWindowHidden() || win->isWindowActive() ) {
		return;
	}
	const bool has_raised = finalcut::FWindow::raiseWindow(win);
	win->activateDialog();

	if ( has_raised ) {
		win->redraw();
	}

	updateTerminal();
}

void MainWindow::adjustSize()
{
	finalcut::FDialog::adjustSize();
	const std::size_t w = getDesktopWidth();
	const std::size_t h = getDesktopHeight();
	const auto X = int(1 + (w - 40) / 2);
	auto Y = int(1 + (h - 22) / 2);
	const int dx = ( w > 80 ) ? int(w - 80) / 2 : 0;
	const int dy = ( h > 24 ) ? int(h - 24) / 2 : 0;

	if ( Y < 2 )
		Y = 2;

	setPos (FPoint{X, Y});
	const auto& first = winlist.begin();
	auto iter = first;

	while ( iter != winlist.end() )
	{
		if ( (*iter)->hasFocus() )
		{
			const auto n = int(std::distance(first, iter));
			const int x = dx + 5 + (n % 3) * 25 + int(n / 3) * 3;
			const int y = dy + 11 + int(n / 3) * 3;
			(*iter)->setPos (FPoint{x, y});
		}
		++iter;
	}
}

	template <typename InstanceT , typename CallbackT , typename... Args>
void MainWindow::addClickedCallback ( finalcut::FWidget* widget
		, InstanceT&& instance
		, CallbackT&& callback
		, Args&&... args )
{
	widget->addCallback ( "clicked", std::bind ( std::forward<CallbackT>(callback) , std::forward<InstanceT>(instance) , std::forward<Args>(args)... ));
}

	template <typename IteratorT>
finalcut::FDialog* MainWindow::getNext (IteratorT iter)
{
	auto next_element = iter;
	finalcut::FDialog* next{nullptr};

	do {
		++next_element;
		if ( next_element == getDialogList()->end() )
			next_element = getDialogList()->begin();

		next = static_cast<finalcut::FDialog*>(*next_element);
	} while ( ! next->isEnabled()
			|| ! next->acceptFocus()
			|| ! next->isVisible()
			|| ! next->isWindowWidget() );
	return next;
}

	template <typename IteratorT>
finalcut::FDialog* MainWindow::getPrevious (IteratorT iter)
{
	auto prev_element = iter;
	finalcut::FDialog* prev;

	do {
		if ( prev_element == getDialogList()->begin() )
			prev_element = getDialogList()->end();

		--prev_element;
		prev = static_cast<finalcut::FDialog*>(*prev_element);
	} while ( ! prev->isEnabled()
			|| ! prev->acceptFocus()
			|| ! prev->isVisible()
			|| ! prev->isWindowWidget() );

	return prev;
}

void MainWindow::onClose (finalcut::FCloseEvent* ev)
{
	finalcut::FApplication::closeConfirmationDialog (this, ev);
}


void MainWindow::cb_closeWindows()
{
	auto iter = getDialogList()->end();
	const auto& first = getDialogList()->begin();
	activateWindow(this);
	if( active != nullptr) {
		if( active != this ) {
			//		activateWindow(active);
			active->close(); 
		}
	}
}

void MainWindow::cb_next()
{
	if ( ! getDialogList() || getDialogList()->empty() ) {
		return;
	}

	auto iter = getDialogList()->begin();

	while ( iter != getDialogList()->end() )
	{
		if ( static_cast<finalcut::FWindow*>(*iter)->isWindowActive() ) {
			auto next = getPrevious(iter);
			activateWindow(next);
			active = next;
			return;
		}
		++iter;
	}
	active = nullptr;
}

void MainWindow::cb_previous()
{
	if ( ! getDialogList() || getDialogList()->empty() ) {
		return;
	}
	auto iter = getDialogList()->end();
	do {
		--iter;
		if ( (*iter)->isDialogWidget() && static_cast<finalcut::FWindow*>(*iter)->isWindowActive() ) {
			auto prev = getPrevious(iter);
			activateWindow(prev);
			active = prev;
			return;
		}
	}
	while ( iter != getDialogList()->begin() );

	active = nullptr;
}

void MainWindow::cb_destroyBaseWindow (BaseWindow* a_win) {
	delWindow(a_win);
//	delete a_win;
}


void MainWindow::cb_connectWindows()
{
	auto win = new ConnectWindow(this);
	win->setFields(host, port);
	addWindow(win);
	hide();		//TODO; really ?
}


void MainWindow::cb_settings()
{
	auto win = new SettingsWindow(this);
	addWindow(win);
}

void MainWindow::cb_about()
{
	auto win = new HelpWindow(this);
	addWindow(win);
}


void MainWindow::addWindow( BaseWindow* a_win )
{
	a_win->setMinimumSize (FSize{20, 8});
	a_win->setResizeable();
	a_win->show();
	a_win->addCallback ( "destroy", this, &MainWindow::cb_destroyBaseWindow, a_win );
	winlist.push_back(a_win);
	return;
}

void MainWindow::delWindow( BaseWindow* a_win )
{
	if ( ! getDialogList() || getDialogList()->empty() )
		return;
	winlist.erase(std::remove(winlist.begin(), winlist.end(), a_win), winlist.end());
}

void MainWindow::onUserEvent (finalcut::FUserEvent* ev) 
{
	finalcut::FMessageBox::info ( this , "info" , "Event");   
}

