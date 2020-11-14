#ifndef WRITECALL_H
#define WRITECALL_H
#include <final/final.h>
#include "basewindow.h"
#include "opcua.h"
#include "dict.h"

//----------------------------------------------------------------------
// class 
//----------------------------------------------------------------------
class WriteWindow final : public BaseWindow
{
	public:
		WriteWindow(finalcut::FWidget* , const uamodel::Object& a_var, OpcUa* a_conection);
		WriteWindow(const WriteWindow&) = delete; 		// Disable copy constructor
		~WriteWindow() override;
		WriteWindow& operator = (const WriteWindow&) = delete;
		void setValue( std::string a_label, std::string a_val);
	private:
		void adjustSize() override;
		void onShow (finalcut::FShowEvent*) override;
		void onTimer (finalcut::FTimerEvent*) override;
		finalcut::FButton ok{"&OK", this};
		finalcut::FButton cancel{"&Cancel", this};
		finalcut::FLabel m_valuedesc{this};
		finalcut::FLineEdit m_value{this};
		finalcut::FLabel m_status{this};
		const uamodel::Object m_obj;
		OpcUa* m_connection;

};


#endif

