#ifndef HLP_CON_WINDOW_H
#define HLP_CON_WINDOW_H
#include <final/final.h>
#include "basewindow.h"

//----------------------------------------------------------------------
// class 
//----------------------------------------------------------------------
class HelpWindow final : public BaseWindow
{
	public:
		explicit HelpWindow(finalcut::FWidget* = nullptr);
		HelpWindow(const HelpWindow&) = delete; 		// Disable copy constructor
		~HelpWindow() override;
		HelpWindow& operator = (const HelpWindow&) = delete;
	private:
		void adjustSize() override;
		// Event handlers
		void onShow (finalcut::FShowEvent*) override;
		void onTimer (finalcut::FTimerEvent*) override;
		finalcut::FButton ok{"&OK", this};
		finalcut::FLabel m_text{this};

};


#endif

