
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
#include "settingswindow.h"
namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;


SettingsWindow::SettingsWindow (finalcut::FWidget* parent) : BaseWindow{parent}
{
	const auto& wc = getColorTheme();
	const wchar_t arrow_up = fc::BlackUpPointingTriangle;
	const wchar_t arrow_down = fc::BlackDownPointingTriangle;
	setText("Settings ");

	setShadow();
	setGeometry (FPoint{4, 2}, FSize{55, 22});
	ok.setGeometry (FPoint{34, 18}, FSize{10, 1});
	
	ok.addCallback ( "clicked",  [](SettingsWindow& dlg) {
				//dlg.hide();
			}, 
			std::ref(*this) 
	);

	cancel.setGeometry (FPoint{22, 18}, FSize{10, 1});
	cancel.addCallback ( "clicked",  [](SettingsWindow& dlg) {
				dlg.close();
			}, 
			std::ref(*this) 
	);
	m_server.setLabelText (L"&Default Server:");
	m_server.setGeometry (FPoint{21, 12}, FSize{27, 1});
	m_server = "cx-18ec19";

	m_port.setLabelText (L"&Default Port:");
	m_port.setGeometry (FPoint{21, 10}, FSize{27, 1});
	m_port= "4840";

	setActiveWindow(this);
	setFocus();

}

SettingsWindow::~SettingsWindow()
{ }

void SettingsWindow::adjustSize()
{
	finalcut::FDialog::adjustSize();
}

void SettingsWindow::onShow (finalcut::FShowEvent*)
{
	ok.setFocus();
}

void SettingsWindow::onTimer (finalcut::FTimerEvent*)
{
}


