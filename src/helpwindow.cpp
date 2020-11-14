
/************************************************************************
* This file is part of UaNavigator					*	
*                                                                      	*
* Copyright 2020 Matias Henttunen					*
*                                                                      	*
* UA Navigator is free software; you can redistribute it and/or modify    	*
* it under the terms of the GNU Lesser General Public License as       	*
* published by the Free Software Foundation; either version 3 of       	*
* the License, or (at your option) any later version.                  	*
*                                                                      	*
* UA Navigator is distributed in the hope that it will be useful, but     	*
* WITHOUT ANY WARRANTY; without even the implied warranty of           	*
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        	*
* GNU Lesser General Public License for more details.                  	*
*                                                                      	*
* You should have received a copy of the GNU Lesser General Public     	*
* License along with this program.  If not, see                        	*
* <http://www.gnu.org/licenses/>.                                      	*
*************************************************************************/
#include "helpwindow.h"
namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;


HelpWindow::HelpWindow (finalcut::FWidget* parent) : BaseWindow{parent}
{
	setText("About UA Navigator");
	setShadow();
	setGeometry (FPoint{4, 2}, FSize{55, 22});
	ok.setGeometry (FPoint{34, 18}, FSize{10, 1});
	
	ok.addCallback ( "clicked",  [](HelpWindow& dlg) {
				dlg.close();
			}, 
			std::ref(*this) 
	);

	//zoomWindow();
	m_text 
		<< "------------------------------------\n"
		<< "-      UA Navigator                -\n"
		<< "- Matias Henttunen 2020 (c)        -\n"
		<< "------------------------------------\n"
		<< "<F10>            Activate menu bar\n"
		<< "<F1>             Show selected\n"
		<< "<F3>             Set value of selected\n"
		<< "<Ctrl>+<o>       Open connection \n"
		<< "<Ctrl>+<Space>   Activate menu bar\n"
		<< "<Menu>           Activate menu bar\n"
		<< "<Esc>            Close active window\n"
		<< "<Shift>+<Menu>   Open dialog menu\n"
		<< "<Shift>+<F10>    Open window menu\n"
		<< "<Meta>+<n>       Next window\n"
		<< "<Meta>+<X>       Exit\n";
	
	m_text.setGeometry(FPoint{2, 1}, FSize{56, 20});
	setActiveWindow(this);
	setFocus();
}

HelpWindow::~HelpWindow()
{ }

void HelpWindow::adjustSize()
{
	finalcut::FDialog::adjustSize();
}

void HelpWindow::onShow (finalcut::FShowEvent*)
{
	ok.setFocus();
}

void HelpWindow::onTimer (finalcut::FTimerEvent*)
{
}


