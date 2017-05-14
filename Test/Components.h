#pragma once

#include <memory>
#include <map>
#include <random>
#include <time.h>

#include "wx\wx.h"
#include "wx\textfile.h"


namespace TextEditor
{
	template <class T>
	class AppData
	{
	public:

		friend class App;
		friend class Frame;

		static void Create()
		{
			if (instance == nullptr)
			{
				instance = new AppData<T>();
				srand(time(NULL));
			}
				
		}
		static std::shared_ptr<AppData<T>> getShared()
		{
			if (instance == nullptr)
			{
				Create();
				srand(time(NULL));
			}
				
				
			return std::shared_ptr<AppData<T>>(instance);
		}

		static wxStandardID getGroupID()
		{
			wxStandardID id = static_cast<wxStandardID>(std::rand());
			while (instance->frameMap.find(id) != instance->frameMap.end())
				id = static_cast<wxStandardID>(std::rand());
			
			return id;
		}

		static void Destroy()
		{
			if (instance != nullptr)
			{
				instance->frameMap.clear();
				instance = nullptr;
			}
		}

	private:
		
		static AppData* instance;
		std::map<wxStandardID, std::vector<T*>> frameMap;
	};

	class Frame : public wxFrame
	{
	public:
		Frame(std::shared_ptr<AppData<Frame>> data, wxStandardID groupID);
		virtual ~Frame();

		void ResetSavedCheck( const wxString& title );

		bool isNotSaved;
		bool isOpen;

	private:

		wxTextCtrl* frameText;

		wxStandardID frameGroupID;
		std::shared_ptr<AppData<Frame>> dataRef;

		void OnNew(wxCommandEvent& wx);
		void OnOpen(wxCommandEvent& wx);
		void OnSave(wxCommandEvent& wx);
		void OnSaveAs(wxCommandEvent& wx);
		void OnExit(wxCommandEvent& wx);
		void OnDuplicate(wxCommandEvent& wx);
		void OnClose(wxCommandEvent& wx);
		void OnCloseAll(wxCommandEvent& wx);

		void UpdateText();
		void UpdateSelection();
	};

	class App: public wxApp
	{
	public:

		virtual bool OnInit();
		virtual ~App()
		{
			AppData<Frame>::Destroy();
			pAppData.reset();
		}

		std::shared_ptr<AppData<Frame>> pAppData;
	};
}

