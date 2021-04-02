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
#include <utility>
#include <vector>
#include <iostream>
#include <final/final.h>
#include "basewindow.h"
#include "mainwindow.h"
#include "main.h"

namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;

static MainWindow* g_mainWindow;

MainWindow* getMainWindow() { return g_mainWindow; }


void connectTo(std::string a_remote)
{
	std::string raw;
	std::string host;
	uint32_t port;
	raw = a_remote.substr(6, a_remote.length() );
	host = raw.substr(0, raw.find(":"));
	port = atoi( raw.substr(raw.find(":")+1, raw.length() ).c_str() );
	g_mainWindow->openConnection(host, port );
	std::cout << "starting with connection";
}


int main (int argc, char* argv[])
{
	std::string srv="";
	for(int i=1; i < argc; i++ ) {
		std::string arg = argv[i];
		if( arg == "--h" ) {
			std::cout << "Usage: " << argv[0] << " " << "\
-server opc://<servername|ip>:4840\n\
-h gui options\n\
--help this help\n\
If noargs are provided, gui will show up\n";
			exit(0);
		} else if( arg.find("-server",0) == 0 ) {
			srv = argv[i+1];
			std::cout << "USING SERVER: " << srv << "\n" ;
			break;
		}
	}
	finalcut::FApplication app {argc, argv};
	g_mainWindow = new MainWindow(&app);
	g_mainWindow->setText ("UA Navigator");
	finalcut::FWidget::setMainWidget (g_mainWindow);
	g_mainWindow->show();
	if( srv !="") {
		connectTo( srv );
	}
	finalcut::FWidget::setMainWidget(g_mainWindow);
	return app.exec();
}

