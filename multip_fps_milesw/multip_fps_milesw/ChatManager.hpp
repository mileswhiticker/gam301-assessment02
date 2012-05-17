#ifndef CHAT_HPP
#define CHAT_HPP

#include <string>
#include <CEGUI/CEGUIColour.h>

namespace CEGUI
{
	class Listbox;
	class Editbox;
};

class ChatManager
{
public:
	static ChatManager& GetInstance()
	{
		static ChatManager instance;
		return instance;
	}
	void SetChatOutput(CEGUI::Listbox* a_pNewChatOutput);
	void SetChatInput(CEGUI::Editbox* a_pNewChatInput);
	void SetPeerList(CEGUI::Listbox* a_pNewPeerList);
	//
	void ClearChatOutput();
	void ClearChatInput();
	void ClearChatInputChar();
	//
	void ProcTextBoxContents();
	void SayText(std::string a_DisplayName, std::string a_Message, CEGUI::colour a_Colour = CEGUI::colour(255,255,255), bool a_IsHost = false);
	void PrintTextLocal(std::string a_Message, CEGUI::colour a_Colour = CEGUI::colour(255,255,255));
	//
	void AddClientName(std::string name, bool isSelf = false, bool isHost = false);
	void RemoveClientName(std::string name);
private:
	ChatManager();
	CEGUI::Editbox* m_pChatInput;
	CEGUI::Listbox* m_pChatOutput;
	CEGUI::Listbox* m_pPeerList;
	//
};

#endif	//CHAT_HPP