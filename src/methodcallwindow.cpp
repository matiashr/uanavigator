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

#include "methodcallwindow.h"
#include "main.h"
namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;


MethodCallWindow::MethodCallWindow (finalcut::FWidget* parent, OpcUa* a_connection, uamodel::Object& a_obj, uamodel::Object& a_method, uamodel::Object& a_inputs, uamodel::Object& a_outputs) : 
	BaseWindow{parent}, m_obj(a_obj), m_method(a_method), m_inputs(a_inputs), m_outputs(a_outputs), m_connection(a_connection)
{
	//const auto& wc = getColorTheme();
	setText("MethodCall ");

	setShadow();
//------------------------------------------
// create dynamic gui here
//------------------------------------------
	int x=11;
	int y =2;
	m_inputsLbl.setGeometry (FPoint{x-10, y}, FSize{17, 1});
	m_inputsLbl= "Inputs:";
//	m_invalues = std::vector<finalcut::FLineEdit*>(20);
//	m_outvalues = std::vector<finalcut::FLineEdit*>(20);

	y+=2;
	for(auto i:m_inputs.getChildren() ) {
		auto lbl = new finalcut::FLineEdit( "", this);
		lbl->setLabelText( (*i).getIdentifier());
		lbl->setGeometry(FPoint{x, y}, FSize{27, 1});
		y+=2;
		m_invalues.push_back(lbl);
	}

	y+=5;
	m_outputsLbl.setGeometry (FPoint{x-10, y}, FSize{17, 1});
	m_outputsLbl= "Outputs:";
	y+=2;
	for(auto i:m_outputs.getChildren() ) {
		finalcut::FLineEdit* lbl = new finalcut::FLineEdit( "", this);
		lbl->setGeometry(FPoint{x, y}, FSize{27, 1});
		lbl->setLabelText( (*i).getIdentifier());
		lbl->setReadOnly();
		y+=2;
		m_outvalues.push_back(lbl);
	}
//------------------------------------------
	y+=4;
	m_status.setGeometry (FPoint{x-10, y}, FSize{37, 1});
	m_status= "Status:";
	y+=5;
	ok.setGeometry (FPoint{x+20, y}, FSize{10, 1});
	
//	finalcut::FMessageBox::info ( getMainWindow(), "c Info" , m_obj.toString() +"/"+ m_inputs.toString()+" | "+m_outputs.toString() );	
	ok.addCallback ( "clicked",  [](MethodCallWindow& dlg) {
				std::vector<std::string> args;
				std::vector<std::string> r_args;
				std::string r_status;
				finalcut::FString argstr;
				for(auto i:dlg.m_invalues ) {
					finalcut::FString v = i->getText();
					args.push_back( std::string(v.c_str()) );
					argstr << i->getLabelObject()->getText() << "=" << i->getText() <<"\n";
				}
				int pos=0;
				for(auto i: dlg.m_inputs.getChildren() ) {
					uamodel::Value val( dlg.m_invalues.at(pos++)  );
					(*i).setValue(val);
				}
		
				if( !dlg.m_connection->callMethod( &dlg.m_obj, &dlg.m_method,  dlg.m_inputs, r_args, r_status)) {
					finalcut::FMessageBox::info ( getMainWindow(), "Error" ,"Call failed for: "+ dlg.m_obj.toString()+" Args:"+argstr+"=>"+r_status );	
					dlg.m_status = "Failed:"+r_status;
				} else {
					dlg.m_status = "Called method Ok";	
					dlg.updateOutputs( r_args );
				}
				dlg.redraw();
			}, 
			std::ref(*this) 
	);

	cancel.setGeometry (FPoint{x, y}, FSize{10, 1});
	cancel.addCallback ( "clicked",  [](MethodCallWindow& dlg) {
				dlg.close();
			}, 
			std::ref(*this) 
	);

	setGeometry (FPoint{10, 2}, FSize{ (size_t)x+40,  (size_t)y+4});
	setActiveWindow(this);
	setFocus();

}

// bug if no outputs are created! TODO
void MethodCallWindow::updateOutputs( std::vector<std::string> av)
{
	int pos=0;
	for(auto i: av) {
		finalcut::FLineEdit* lbl = m_outvalues.at(pos);
		finalcut::FString v = i;
		lbl->setText(v); 
	}

}

MethodCallWindow::~MethodCallWindow()
{ }

void MethodCallWindow::adjustSize()
{
	finalcut::FDialog::adjustSize();
}

void MethodCallWindow::onShow (finalcut::FShowEvent*)
{
	ok.setFocus();
}

void MethodCallWindow::onTimer (finalcut::FTimerEvent*)
{
}


