
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
#include "connectwindow.h"
#include "serverdialog.h"
#include "main.h"
#include "opcua.h"

namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;


ConnectWindow::ConnectWindow (finalcut::FWidget* parent) : BaseWindow{parent}
{
	const auto& wc = getColorTheme();
	const wchar_t arrow_up = fc::BlackUpPointingTriangle;
	const wchar_t arrow_down = fc::BlackDownPointingTriangle;

	left_arrow = arrow_up;
	left_arrow.setForegroundColor (wc->label_inactive_fg);
	left_arrow.setEmphasis();
	left_arrow.ignorePadding();
	left_arrow.setGeometry (FPoint{2, 2}, FSize{1, 1});
	setText("Connect server");

	setShadow();
	setGeometry (FPoint{4, 2}, FSize{45, 22});
	ok.setGeometry (FPoint{24, 18}, FSize{10, 1});
	
	ok.addCallback ( "clicked",  [](ConnectWindow& dlg) {
				OpcUa* ua = new OpcUa();
				finalcut::FString serverLbl = dlg.m_server.getText();
				std::string server = std::string("opc.tcp://")+serverLbl.c_str();
				uint32_t port = atoi(dlg.m_port.getText().c_str());
				if( !ua->connect( (char*)server.c_str(), port )  ) {
					dlg.redraw();
					finalcut::FMessageBox::info (&dlg, "Error" , "Failed to connect to: opc.tcp://"+server );
				} else {
				//	finalcut::FMessageBox::info (&dlg, "Info" , "connected" );
					auto  s = new ServerDialog( getMainWindow(),  ua);
					s->show();
				}
				dlg.close();
				getMainWindow()->delWindow(&dlg);
			}, 
			std::ref(*this) 
	);

	cancel.setGeometry (FPoint{12, 18}, FSize{10, 1});
	cancel.addCallback ( "clicked",  [](ConnectWindow& dlg) {
				dlg.close();
			}, 
			std::ref(*this) 
	);
	m_server.setLabelText (L"&Server:");
	m_server.setGeometry (FPoint{11, 4}, FSize{27, 1});
	m_server = "";

	m_port.setLabelText (L"&Port:");
	m_port.setGeometry (FPoint{11, 6}, FSize{27, 1});
	m_port= "4840";

	setActiveWindow(this);
	setFocus();

}

ConnectWindow::~ConnectWindow()
{ 

}

void ConnectWindow::setFields(std::string host, uint32_t port )
{
	m_server = host;
	m_port = std::to_string(port);

}

void ConnectWindow::adjustSize()
{
	finalcut::FDialog::adjustSize();
}

void ConnectWindow::onShow (finalcut::FShowEvent*)
{
	ok.setFocus();
}

void ConnectWindow::onTimer (finalcut::FTimerEvent*)
{
	left_arrow.unsetEmphasis();
	left_arrow.redraw();
}


