
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
#include "serverdialog.h"
#include "methodcallwindow.h"
#include "main.h"
#include "dict.h"
#include "writewindow.h"


using namespace uamodel;

ServerDialog::ServerDialog(finalcut::FWidget* parent, OpcUa* a_connection)
  : BaseWindow{parent}, m_connection(a_connection)
{
	setText ("OPC-UA Server connection");
	setShadow();
	setGeometry (FPoint{1, 1}, FSize{160, 112});

	listview.setGeometry(FPoint{2, 2}, FSize{63, 63});
	listview.addColumn ("Address space", 40);
	listview.setColumnAlignment (0, fc::alignRight);
	listview.setTreeView();

	infolist.setGeometry(FPoint{65, 2}, FSize{160, 59});
	infolist.setText ("Data Access View");
	infolist.insert("Info");
	
	m_status.setGeometry(FPoint{4, 1 }, FSize{40, 10});
	m_status = "OK";

	zoomWindow();
	listview.setHeight (getHeight() - 6, true);
//	setFocusWidget( &listview);
	setActiveWindow(this);
	setFocus();
	// browse target system
	if( m_connection->browse( m_dict ) ) {
		insertItems( m_dict );
		m_dict.mkGraph("graph.dot");
		 addTimer( 1000); 	//once a sec check connection
	} else {
		finalcut::FMessageBox::info ( this , "Error" , "Failed to browse target");	
	}

}
 
ServerDialog::~ServerDialog() {

}

void ServerDialog::onShow (finalcut::FShowEvent*) { listview.setFocus(); }

void ServerDialog::onTimer (finalcut::FTimerEvent*) {
	static int n=0;
	std::string alive;
	if(n == 0 ) {
		alive=".";
	} else if ( n == 1) {
		alive="*";
	} else if ( n== 2 ) {
		alive="o";
	} else if( n == 3) {
		alive = "O";
	} else {
		n=-1;
	}
	n++;
	std::string msg;
	m_connection->execute();
	if( !m_connection->isConnected( msg ) ) {
		finalcut::FMessageBox::info ( this , "Error" , "Server is not connected");	
		m_status="Not connected to server";
	} else {
		m_status="Established: "+alive;
	}
	redraw();
}


void ServerDialog::showInfo( uamodel::Object a_obj )
{
	if( a_obj.NodeId.Identifier == "RootFolder") {
		return;
	}
	clearInfo();
	auto node = a_obj;
	finalcut::FString data;
	if( node.NodeId.IdType == uamodel::Numeric ) {
		data  << " NodeId : "  << node.NodeId.IdentifierNum;
		infolist.insert(data);
		data.clear();
		data << " Idtype: "  << (int)node.NodeId.IdType << " (Numeric)";
		infolist.insert(data);
		data.clear();

	} else if(  node.NodeId.IdType == uamodel::String ) {
		data  << " NodeId : "  << node.NodeId.Identifier;
		infolist.insert(data);
		data.clear();
		data << " Idtype: "  << (int)node.NodeId.IdType << " (string)";
		infolist.insert(data);
		data.clear();
	}
	data << "\n NodeClass: " <<(int)node.NodeClass;
	infolist.insert(data);
	data.clear();
	data << "\n Namespace : " << node.NodeId.NamespaceIndex;
	infolist.insert(data);
	data.clear();
	data << "\n BrowseName: " << node.BrowseName;
	infolist.insert(data);
	data.clear();
	data << "\n Description: "<< node.Description;
	infolist.insert(data);
	data.clear();

	infolist.insert("");
	std::string result;
	m_connection->readVariable( a_obj, result );
	data << "\n Value:: "<< result;
	infolist.insert(data);
	data.clear();
	infolist.redraw();
}

void ServerDialog::clearInfo()
{
	infolist.clear();
}



void* ServerDialog::addToTree( void* dlg, void* data, uamodel::Object& obj )
{
	finalcut::FListViewItem::iterator*  root = (finalcut::FListViewItem::iterator*)data;
	ServerDialog* where = (ServerDialog*)dlg;
	finalcut::FListViewItem::iterator* added = new 	finalcut::FListViewItem::iterator;		//TODO: delete this where ??
	finalcut::FListViewItem* item = new finalcut::FListViewItem (  finalcut::FStringList({ (char*)obj.getBrowseName().c_str() }), obj, *root);
	*added  = where->listview.insert( item, *root );
	return added;
}


void ServerDialog::insertItems( uamodel::Object& a_dict )
{
	finalcut::FListViewIterator root; 
	this->current = listview.insert (finalcut::FStringList({m_connection->m_server}), root);
	a_dict.forEach( (void*)this, &this->current, a_dict, addToTree );
}



bool ServerDialog::callMethod( uamodel::Object& a_obj, uamodel::Object& a_method )
{
	uamodel::Object inputs;
	uamodel::Object outputs;
	if(!m_connection->browseMethod(  &a_method, inputs, outputs) ) {
		finalcut::FMessageBox::info ( this , "Error" , "Failed to read method arguments");	
		return false;
	}
	MethodCallWindow* w = new MethodCallWindow(this, m_connection, a_obj, a_method, inputs, outputs);
	getMainWindow()->addWindow(w);
	w->show();
	redraw();                    
	finalcut::FWindow::raiseWindow(w);
	return true;
}

bool ServerDialog::setVariable( uamodel::Object& a_obj )
{
//	finalcut::FMessageBox::info ( this , "Info" , a_obj.toString() );	
	std::string value;
	if(!m_connection->readVariable( a_obj, value) ) {
		finalcut::FMessageBox::info ( this , "Error" , "Failed to read current value");	
		value = "";
	}

	WriteWindow* w = new WriteWindow(this, a_obj, m_connection );
	std::string dtype=uamodel::dataType2String( a_obj.getDataType().Identifier, 
						   a_obj.getDataType().isAbstract );
	w->setValue("Value ("+dtype+")", value);		//display current value
	getMainWindow()->addWindow(w);
	w->show();
	return true;
}



#if 0
	finalcut::FUserEvent user_event(fc::User_Event, 0);
	user_event.setData(this);
	finalcut::FApplication::sendEvent (getMainWidget(), &user_event);
#endif
