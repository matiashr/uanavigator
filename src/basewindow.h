#ifndef BASE_WINDOW_H
#define BASE_WINDOW_H
#include <final/final.h>
namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;


//----------------------------------------------------------------------
// class BaseWindow
//----------------------------------------------------------------------
class BaseWindow : public finalcut::FDialog
{
	public:
		explicit BaseWindow (finalcut::FWidget* = nullptr);
		BaseWindow (const BaseWindow&) = delete; 		// Disable copy constructor
		~BaseWindow() override;
		BaseWindow& operator = (const BaseWindow&) = delete;
	private:
		void adjustSize() override;
		// Event handlers
		void onShow (finalcut::FShowEvent*) override;
		void onClose (finalcut::FCloseEvent*) override;
		void onTimer (finalcut::FTimerEvent*) override;
		finalcut::FLabel left_arrow{this};
};


#endif

