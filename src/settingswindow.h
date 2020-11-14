#ifndef SET_CON_WINDOW_H
#define SET_CON_WINDOW_H
#include <final/final.h>
#include "basewindow.h"
#include "listview.h"

//----------------------------------------------------------------------
// class 
//----------------------------------------------------------------------
class SettingsWindow final : public BaseWindow
{
	public:
		explicit SettingsWindow(finalcut::FWidget* = nullptr);
		SettingsWindow(const SettingsWindow&) = delete; 		// Disable copy constructor
		~SettingsWindow() override;
		SettingsWindow& operator = (const SettingsWindow&) = delete;
	private:
		void adjustSize() override;
		// Event handlers
		void onShow (finalcut::FShowEvent*) override;
		void onTimer (finalcut::FTimerEvent*) override;
		finalcut::FButton ok{"&OK", this};
		finalcut::FButton cancel{"&Cancel", this};
		finalcut::FLineEdit m_server {this};
		finalcut::FLineEdit m_port {this};

};


#endif

