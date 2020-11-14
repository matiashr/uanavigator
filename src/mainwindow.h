#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H
#include <final/final.h>
#include "basewindow.h"
namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;

//----------------------------------------------------------------------
// class MainWindow
//----------------------------------------------------------------------
class MainWindow final : public finalcut::FDialog
{
	public:
		explicit MainWindow (finalcut::FWidget* = nullptr);
		MainWindow (const MainWindow&) = delete; 			// Disable copy constructor
		~MainWindow() override;
		MainWindow& operator = (const MainWindow&) = delete; 		// Disable copy assignment operator (=)

		void addWindow( BaseWindow* a_win );
		void delWindow( BaseWindow* a_win );
	public:
		void openConnection( std::string host, uint32_t port );
		std::string host;
		uint32_t port;
		bool startWithConnection{false};
	private:
		FDialog* active;
		void configureFileMenuItems();
		void configureDialogButtons();
		void activateWindow (finalcut::FDialog*) const;
		void adjustSize() override;
		template <typename InstanceT , typename CallbackT , typename... Args>
			void addClickedCallback ( finalcut::FWidget* , InstanceT&&, CallbackT&&, Args&&... );
		template <typename IteratorT> finalcut::FDialog* getNext (IteratorT); template <typename IteratorT>
			finalcut::FDialog* getPrevious (IteratorT iter);


		// Event handlers
		void onClose (finalcut::FCloseEvent*) override;
		void onUserEvent (finalcut::FUserEvent* ev) override;
    
		// Callback methods
		void cb_connectWindows();
		void cb_closeWindows();
		void cb_next();
		void cb_previous();
		void cb_settings();
		void cb_about();
		//void cb_destroyWindow (MainWindow*, WindowData*) const;
		void cb_destroyBaseWindow (BaseWindow*);

		std::vector<BaseWindow*>    winlist{};
		finalcut::FString         drop_down_symbol{fc::BlackDownPointingTriangle};
		finalcut::FMenuBar        Menubar{this};
		finalcut::FMenu           File{"&File", &Menubar};
		finalcut::FDialogListMenu DglList{drop_down_symbol, &Menubar};
		finalcut::FStatusBar      Statusbar{this};
		finalcut::FMenuItem       New{"&Connect", &File};
		finalcut::FMenuItem       Close{"&Close", &File};
		finalcut::FMenuItem       Line1{&File};
		finalcut::FMenuItem       Next{"Ne&xt window", &File};
		finalcut::FMenuItem       Previous{"&Previous window", &File};
		finalcut::FMenuItem       Line2{&File};
		finalcut::FMenuItem       Settings{"&Settings", &File};
		finalcut::FMenuItem       Quit{"&Quit", &File};

		finalcut::FMenu           Help{"&Help", &Menubar};
		finalcut::FMenuItem       About{"&About", &Help};
		finalcut::FButton         CreateButton{this};
};



#endif

