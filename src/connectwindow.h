#ifndef CON_WINDOW_H
#define CON_WINDOW_H
#include <final/final.h>
#include "basewindow.h"

//----------------------------------------------------------------------
// class BaseWindow
//----------------------------------------------------------------------
class ConnectWindow final : public BaseWindow
{
	public:
		explicit ConnectWindow(finalcut::FWidget* = nullptr);
		ConnectWindow(const ConnectWindow&) = delete; 		// Disable copy constructor
		~ConnectWindow() override;
		ConnectWindow& operator = (const ConnectWindow&) = delete;
		void setFields(std::string host, uint32_t port );
	private:
		void adjustSize() override;
		// Event handlers
		void onShow (finalcut::FShowEvent*) override;
		void onTimer (finalcut::FTimerEvent*) override;
		finalcut::FLabel left_arrow{this};
		finalcut::FButton ok{"&OK", this};
		finalcut::FButton cancel{"&Cancel", this};
		finalcut::FLineEdit m_server {this};
		finalcut::FLineEdit m_port {this};

};


#endif

