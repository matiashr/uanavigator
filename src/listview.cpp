#include "listview.h"
#include "dict.h"


static char* nodetype( char t ) {
	switch(t) 
	{
		case 'n': return "numeric"; break;
		case 's': return "string"; break;
	}
	return "??";
}



void ListView::onKeyPress (finalcut::FKeyEvent* ev) {
	const FKey key = ev->key();
	auto selected = getCurrentItem();

	// catch the enter key
	if ( key == fc::Fkey_return || key == fc::Fkey_enter ) {
		if( selected->getText(1)  == m_parent->getServer() ) { 
			ev->ignore(); 
			//ignore root node operations
			return; 
		}	
		setFocusWidget(this);
		auto method = selected->getData<uamodel::Object>();
		m_parent->showInfo( method );
		if( method.NodeClass == uamodel::METHOD ) {
			m_parent->callMethod( *method.getParentNode(), method );
		}
		if(!selected->isExpand() ) {
			selected->expand();
		} else {
			selected->collapse();
		}
		redraw();
		ev->accept();
	} else if ( key == fc::Fkey_f1 ) { 							// catch the f1 key
		auto node = selected->getData<uamodel::Object>();
		finalcut::FString data;
		finalcut::FString idData;
		if( node.NodeId.IdType == uamodel::Numeric) {
			idData << "\n NodeId :"     << node.NodeId.Identifier;
		} else if( node.NodeId.IdType == uamodel::String) {
			idData << "\n NodeId : "    << node.NodeId.IdentifierNum;
		}  
		data 	<< " Name : "	     << selected->getText(1)
			<< idData
			<< "\n Idtype: "     << (int)node.NodeId.IdType
			<< "\n Namespace : " << node.NodeId.NamespaceIndex
			<< "\n BrowseName: " << node.BrowseName
			<< "\n Description: "<< node.Description;
		finalcut::FMessageBox::info ( this , "info" , data );	
		setFocusWidget(this);
		ev->accept();
	} else if ( key == fc::Fkey_f3 ) {							// catch the f3 key
		if( selected->getText(1)  == m_parent->getServer() ) {
			if(!selected->isExpand() ) {
				 selected->expand();
			} else {
				selected->collapse();
			}
			redraw();
			ev->ignore(); 
			return; 								//ignore root node operations
		}
		auto node = selected->getData<uamodel::Object>();
		if( node.NodeClass == uamodel::VARIABLE) {
			m_parent->setVariable( node );
		} else if( node.NodeClass == uamodel::OBJECT ) {
			if(!selected->isExpand() ) {
				selected->expand();
			} else {
				selected->collapse();
			}

			m_parent->showInfo( node );
			redraw();
		}
		ev->accept();
	} else {
		// other events forwarded, up, down etc
	//	ev->ignore();
		finalcut::FListView::onKeyPress(ev );
	//	finalcut:: FApplication::getLog()->error("Ignore");
	}
}


