#ifndef SERVER_DIALOG_H
#define SERVER_DIALOG_H
#include <final/final.h>
#include "opcua.h"
#include "listview.h"
#include "dict.h"
#include "basewindow.h"


namespace fc = finalcut::fc;
using finalcut::FPoint;
using finalcut::FSize;


class ServerDialog final : public BaseWindow
{
	public:
		explicit ServerDialog (finalcut::FWidget* = nullptr, OpcUa* a_connection=nullptr );
		ServerDialog (const finalcut::FWidget&) = delete;
		~ServerDialog() override;
		struct TreeItem
		{
			const char* const* begin() const
			{ return &name; }

			const char* const* end() const
			{ return reinterpret_cast<const char* const*>(&child_element); }

			// Data members
			const char* name;
			const char* info;	//put info here
			TreeItem*   child_element;
		};
		void insertItems( uamodel::Object& root  );
		static void* addToTree( void*dlg, void* data, uamodel::Object& obj );
		void showInfo( uamodel::Object a_obj );
		void clearInfo();
		bool callMethod( uamodel::Object& a_obj, uamodel::Object& a_method );
		bool setVariable( uamodel::Object& a_obj  );
		string getServer() { return m_connection->m_server; };

		void onShow (finalcut::FShowEvent*) override;
		void onTimer (finalcut::FTimerEvent*) override; 
	private:
		ListView listview{this};
		finalcut::FListBox infolist{this};
		finalcut::FListViewItem::iterator current;
		finalcut::FLabel m_status{this};
		uamodel::Object m_dict;
		OpcUa* m_connection;
};



#endif
