#ifndef LISTVIEW_H
#define LISTVIEW_H
#include <final/final.h>
#include "opcua.h"
namespace fc = finalcut::fc;

class ListView final  : public finalcut::FListView
{
	public:
		explicit ListView( FWidget* parent = nullptr): finalcut::FListView(parent ),
		m_parent((ServerDialog*)parent)
	{
	};
	void onKeyPress (finalcut::FKeyEvent* ev);

	private:
		ServerDialog* m_parent;
};

#include "serverdialog.h"
#endif
