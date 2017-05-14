#include "Components.h"

namespace TextEditor
{
	template <class T>
	AppData<T>* AppData<T>::instance = nullptr;

	bool App::OnInit()
	{
		pAppData = AppData<Frame>::getShared();

		wxStandardID firstID = pAppData->getGroupID();
		pAppData->frameMap.insert(std::make_pair(firstID, std::vector<Frame*>()));
		pAppData->frameMap[firstID].push_back(new Frame(pAppData, firstID));

		pAppData->frameMap[firstID][0]->Show(true);

		return true;
	}

	Frame::Frame(std::shared_ptr<AppData<Frame>> data, wxStandardID groupID) : wxFrame(NULL, wxID_ANY, wxString("Blank"))
	{
		dataRef = data;
		frameGroupID = groupID;

		isNotSaved = false;
		isOpen = false;

		wxMenu* fileMenu = new wxMenu;
		fileMenu->Append(wxID_NEW);
		fileMenu->Append(wxID_OPEN);
		fileMenu->AppendSeparator();
		fileMenu->Append(wxID_SAVE);
		fileMenu->Append(wxID_SAVEAS);
		fileMenu->AppendSeparator();
		fileMenu->Append(wxID_EXIT);

		wxMenu* windowMenu = new wxMenu;
		windowMenu->Append(wxID_DUPLICATE, "New");
		windowMenu->AppendSeparator();
		windowMenu->Append(wxID_CLOSE);
		windowMenu->Append(wxID_CLOSE_ALL, "Close All");

		wxMenuBar* menuBar = new wxMenuBar;
		menuBar->Append(fileMenu, "File");
		menuBar->Append(windowMenu, "Window");

		SetMenuBar(menuBar);

		Bind(wxEVT_MENU, &Frame::OnNew, this, wxID_NEW);
		Bind(wxEVT_MENU, &Frame::OnOpen, this, wxID_OPEN);
		Bind(wxEVT_MENU, &Frame::OnSave, this, wxID_SAVE);
		Bind(wxEVT_MENU, &Frame::OnSaveAs, this, wxID_SAVEAS);
		Bind(wxEVT_MENU, &Frame::OnExit, this, wxID_EXIT);
		Bind(wxEVT_MENU, &Frame::OnDuplicate, this, wxID_DUPLICATE);
		Bind(wxEVT_MENU, &Frame::OnClose, this, wxID_CLOSE);
		Bind(wxEVT_MENU, &Frame::OnCloseAll, this, wxID_CLOSE_ALL);

		frameText = new wxTextCtrl(this, wxID_ANY, "Testing", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH );

		frameText->Bind(wxEVT_TEXT, [=](wxCommandEvent&) { UpdateText(); }, wxID_ANY);

		frameText->Bind(wxEVT_LEFT_UP, [=](wxMouseEvent&) { UpdateSelection(); }, wxID_ANY);
	}

	Frame::~Frame()
	{
		dataRef.reset();
	}

	void Frame::ResetSavedCheck(const wxString& title)
	{
		auto it = dataRef->frameMap.find(frameGroupID);
		if (it != dataRef->frameMap.end())
		{
			wxString currentTitle = GetTitle();

			if (title.CompareTo(currentTitle) != 0 && !title.empty())
			{
				for (Frame* frame : it->second)
				{
					frame->SetTitle(title);
					frame->isNotSaved = false;
				}
			}
			else if (title.empty())
			{
				for (Frame* frame : it->second)
				{
					frame->SetTitle(currentTitle.SubString(0, currentTitle.Length() - 2));
					frame->isNotSaved = false;
				}
			}
			
		}
	}

	void Frame::UpdateText()
	{
		long pos;
		pos = frameText->GetInsertionPoint();

		auto it = dataRef->frameMap.find(frameGroupID);
		if (it != dataRef->frameMap.end())
		{
			wxString newTitle = GetTitle();
			
			for (Frame* frame : it->second)
			{
				if (isOpen)
				{
					frame->SetTitle(newTitle);
				}

				if (!frame->isNotSaved && !isOpen)
				{
					frame->SetTitle(newTitle + "*");
					frame->isNotSaved = true;
				}
				
				frame->frameText->ChangeValue(frameText->GetValue());
				frame->frameText->SetInsertionPoint(pos);
			}
		}
	}

	void Frame::UpdateSelection()
	{
		long start, stop;
		frameText->GetSelection(&start, &stop);

		auto it = dataRef->frameMap.find(frameGroupID);
		if (it != dataRef->frameMap.end())
		{
			for (Frame* frame : it->second)
			{
				frame->frameText->SetSelection(start, stop);
			}
		}
	}

	void Frame::OnNew(wxCommandEvent& wx)
	{
		wxStandardID newID = dataRef->getGroupID();
		dataRef->frameMap.insert(std::make_pair(newID, std::vector<Frame*>()));
		dataRef->frameMap[newID].push_back(new Frame(dataRef, newID));

		dataRef->frameMap[newID][0]->Show(true);
	}

	void Frame::OnOpen(wxCommandEvent& wx)
	{
		wxString title = GetTitle();
		bool isFileExists = wxFileExists(title.SubString(0, title.Length() - 2));

		if (isFileExists || isNotSaved)
		{
			if (wxYES == wxMessageBox("Save document?", "Save", wxYES_NO | wxICON_NONE))
			{
				if (isFileExists && isNotSaved)
					OnSave(wx);
				else if (isNotSaved)
					OnSaveAs(wx);
			}
		}
			
		wxFileDialog dialog(this,"Open text document","","","text files (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

		if (dialog.ShowModal() == wxID_OK)
		{
			wxTextFile textFile;
			textFile.Open(dialog.GetPath());

			SetTitle(dialog.GetPath());
			isOpen = true;

			if (textFile.IsOpened())
			{
				frameText->Clear();
				
				frameText->AppendText(textFile.GetFirstLine() + "\n");
				while (!textFile.Eof())
				{
					frameText->AppendText(textFile.GetNextLine() + "\n");
				}
			}
			
			isOpen = false;
			textFile.Close();
		}
	}

	void Frame::OnSave(wxCommandEvent& wx)
	{
		wxString filePath = GetTitle();

		if (isNotSaved)
			filePath = filePath.SubString(0, filePath.Length() - 2);
		
		wxTextFile textFile;

		if (wxFileExists(filePath))
		{
			if (isNotSaved)
			{
				textFile.Open(filePath);
				textFile.Clear();

				if (textFile.IsOpened())
				{
					for (int lineIdx = 0; lineIdx < frameText->GetNumberOfLines(); lineIdx++)
						textFile.AddLine(frameText->GetLineText(lineIdx));

					textFile.Write();
				}

				textFile.Close();

				ResetSavedCheck("");
			}
		}
		else
		{
			if (wxYES == wxMessageBox("Save document?", "Save", wxYES_NO | wxICON_NONE))
				OnSaveAs(wx);
		}
	}

	void Frame::OnSaveAs(wxCommandEvent& wx)
	{
		wxFileDialog dialog(this, "Save text file", "", "","text files (*.txt)|*.txt", wxFD_SAVE );
		
		if (dialog.ShowModal() == wxID_OK)
		{
			ResetSavedCheck(dialog.GetPath());

			wxTextFile textFile;

			if (!wxFileExists(dialog.GetPath()))
				textFile.Create(dialog.GetPath());

			textFile.Open(dialog.GetPath());

			if (textFile.IsOpened())
			{
				textFile.Clear();

				for (int lineIdx = 0; lineIdx < frameText->GetNumberOfLines(); lineIdx++)
					textFile.AddLine(frameText->GetLineText(lineIdx));
				
				textFile.Write();
			}

			textFile.Close();
		}
	}

	void Frame::OnExit(wxCommandEvent& wx)
	{
		wxString title = GetTitle();
		bool isFileExists = wxFileExists(title.SubString(0, title.Length() - 2));

		if (isFileExists || isNotSaved)
		{
			if (wxYES == wxMessageBox("Save document?", "Save", wxYES_NO | wxICON_NONE))
			{
				if (isFileExists && isNotSaved)
					OnSave(wx);
				else if (isNotSaved)
					OnSaveAs(wx);
			}
		}
		
		if (!dataRef->frameMap.empty())
		{
			for (auto& group : dataRef->frameMap)
			{
				for (Frame* frame : group.second)
					frame->Close();
			
				group.second.clear();
			}
		}
	}

	void Frame::OnDuplicate(wxCommandEvent& wx)
	{
		auto it = dataRef->frameMap.find(frameGroupID);
		if (it != dataRef->frameMap.end())
		{
			it->second.push_back(new Frame(dataRef, frameGroupID));
			it->second[it->second.size() - 1]->frameText->ChangeValue(frameText->GetValue());
			it->second[it->second.size() - 1]->SetTitle(GetTitle());
			it->second[it->second.size() - 1]->isNotSaved = isNotSaved;

			long start, stop;
			frameText->GetSelection(&start, &stop);
			it->second[it->second.size() - 1]->frameText->SetSelection(start, stop);
			
			it->second[it->second.size() - 1]->Show();

		}
		else
		{
			OnNew(wx);
		}
	}

	void Frame::OnClose(wxCommandEvent& wx)
	{
		auto itf = dataRef->frameMap.find(frameGroupID);
		
		if (itf->second.size() == 1)
		{
			wxString title = GetTitle();
			bool isFileExists = wxFileExists(title.SubString(0, title.Length() - 2));

			if (isFileExists || isNotSaved)
			{
				if (wxYES == wxMessageBox("Save document?", "Save", wxYES_NO | wxICON_NONE))
				{
					if (isFileExists && isNotSaved)
						OnSave(wx);
					else if (isNotSaved)
						OnSaveAs(wx);
				}
			}
		}
		
		if (itf != dataRef->frameMap.end())
		{
			wxWindowID fID = GetId();
			auto it = std::find_if(dataRef->frameMap[frameGroupID].begin(),dataRef->frameMap[frameGroupID].end(),
				[fID](Frame* f) { return f->GetId() == fID; });

			if (it != dataRef->frameMap[frameGroupID].end())
			{	
				Close();
				dataRef->frameMap[frameGroupID].erase(it);
			}

			if (itf->second.empty())
				dataRef->frameMap.erase(itf);
		}
	}

	void Frame::OnCloseAll(wxCommandEvent& wx)
	{
		wxString title = GetTitle();
		bool isFileExists = wxFileExists(title.SubString(0, title.Length() - 2));

		if (isFileExists || isNotSaved)
		{
			if (wxYES == wxMessageBox("Save document?", "Save", wxYES_NO | wxICON_NONE))
			{
				if (isFileExists && isNotSaved)
					OnSave(wx);
				else if (isNotSaved)
					OnSaveAs(wx);
			}
		}
		
		auto it = dataRef->frameMap.find(frameGroupID);
		if (it != dataRef->frameMap.end())
		{
			for (Frame* frame : it->second)
				frame->Close();

			it->second.clear();
			dataRef->frameMap.erase(it);
		}
	}
}
