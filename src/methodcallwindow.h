#ifndef METHOD_CALL_H
#define METHOD_CALL_H
#include <final/final.h>
#include "basewindow.h"
#include "opcua.h"
#include "dict.h"

//----------------------------------------------------------------------
// class 
//----------------------------------------------------------------------
class MethodCallWindow final : public BaseWindow
{
	public:
		MethodCallWindow(finalcut::FWidget* , OpcUa* a_connection, uamodel::Object& a_obj,  uamodel::Object& a_method,  uamodel::Object& a_in, uamodel::Object& a_out);
		MethodCallWindow(const MethodCallWindow&) = delete; 		// Disable copy constructor
		~MethodCallWindow() override;
		MethodCallWindow& operator = (const MethodCallWindow&) = delete;
	private:
		void adjustSize() override;
		void onShow (finalcut::FShowEvent*) override;
		void onTimer (finalcut::FTimerEvent*) override;
		finalcut::FButton ok{"&OK", this};
		finalcut::FButton cancel{"&Cancel", this};
		finalcut::FLabel m_inputsLbl{this};
		finalcut::FLabel m_outputsLbl{this};
		finalcut::FLabel m_status{this};

		std::vector<finalcut::FLineEdit*> m_invalues;
		std::vector<finalcut::FLineEdit*> m_outvalues;
	private:
		void updateOutputs( std::vector<std::string> av);
		uamodel::Object m_inputs;
		uamodel::Object m_outputs;
		uamodel::Object m_obj;
		uamodel::Object m_method;

	private:
		OpcUa* m_connection;


};


#endif

