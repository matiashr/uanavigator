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

#include "basewindow.h"
#include "main.h"

BaseWindow::BaseWindow (finalcut::FWidget* parent) : finalcut::FDialog{parent}
{
	const auto& wc = getColorTheme();
	const wchar_t arrow_up = fc::BlackUpPointingTriangle;
	const wchar_t arrow_down = fc::BlackDownPointingTriangle;

	left_arrow = arrow_up;
	left_arrow.setForegroundColor (wc->label_inactive_fg);
	left_arrow.setEmphasis();
	left_arrow.ignorePadding();
	left_arrow.setGeometry (FPoint{2, 2}, FSize{1, 1});
	setActiveWindow(this);
	setFocus();
}

BaseWindow::~BaseWindow()
{ }

void BaseWindow::adjustSize()
{
	finalcut::FDialog::adjustSize();
}

void BaseWindow::onShow (finalcut::FShowEvent*)
{
}

void BaseWindow::onTimer (finalcut::FTimerEvent*)
{
	left_arrow.unsetEmphasis();
	left_arrow.redraw();
	updateTerminal();
	delOwnTimers();
}

void BaseWindow::onClose (finalcut::FCloseEvent* ev)
{
	getMainWindow()->delWindow(this);
	ev->accept();
}

