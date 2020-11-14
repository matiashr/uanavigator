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

#include "writewindow.h"
#include "main.h"
namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;


WriteWindow::WriteWindow (finalcut::FWidget* parent, const uamodel::Object& a_obj, OpcUa* a_connection) : BaseWindow{parent},m_obj(a_obj),m_connection(a_connection)
{
	setText("Write variable ");
	setShadow();
	setGeometry (FPoint{40, 10}, FSize{80, 12});
	ok.setGeometry (FPoint{54, 8}, FSize{10, 1});
	
	ok.addCallback ( "clicked",  [](WriteWindow& dlg, const uamodel::Object& obj) {
		//		finalcut::FMessageBox::info ( getMainWindow(), "c Info" , obj.toString() );	
				std::string result="";
				if( dlg.m_connection->writeVariable( dlg.m_obj, dlg.m_value.getText().c_str() , result ) ) {
					dlg.m_status = "Ok";
				} else {
					dlg.m_status = "-> "+result;
				}
				dlg.redraw();
			}, 
			std::ref(*this) , std::ref(m_obj)
	);
	cancel.setGeometry (FPoint{10, 8}, FSize{10, 1});
	cancel.addCallback ( "clicked",  [](WriteWindow& dlg) {
				dlg.close();
			}, 
			std::ref(*this) 
	);
	m_valuedesc = "";
	m_valuedesc.setGeometry (FPoint{4, 1}, FSize{45, 1});
	m_value.setLabelText (L"&=");
	m_value.setGeometry (FPoint{20, 3}, FSize{45, 1});
	m_value = "";
	m_status.setGeometry(FPoint{10, 5}, FSize{76, 5});
	setActiveWindow(this);
	setFocus();
}

WriteWindow::~WriteWindow()
{ }


void WriteWindow::setValue( std::string a_label, std::string a_val)
{
	m_valuedesc = a_label;
	m_value = a_val;
}


void WriteWindow::adjustSize()
{
	finalcut::FDialog::adjustSize();
}

void WriteWindow::onShow (finalcut::FShowEvent*)
{
	ok.setFocus();
}

void WriteWindow::onTimer (finalcut::FTimerEvent*)
{
}


