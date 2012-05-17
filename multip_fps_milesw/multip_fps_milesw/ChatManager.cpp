#include "NetworkManager.hpp"
#include "ChatManager.hpp"

#include "CEGUIFormattedListBoxTextItem.hpp"

ChatManager::ChatManager()
:	m_pChatOutput(NULL)
,	m_pChatInput(NULL)
,	m_pPeerList(NULL)
{
	//
}

void ChatManager::SetChatOutput(CEGUI::Listbox* a_pNewChatOutput)
{
	m_pChatOutput = a_pNewChatOutput;
}

void ChatManager::SetChatInput(CEGUI::Editbox* a_pNewChatInput)
{
	m_pChatInput = a_pNewChatInput;
}

void ChatManager::SetPeerList(CEGUI::Listbox* a_pNewPeerList)
{
	m_pPeerList = a_pNewPeerList;
}


void ChatManager::PrintTextLocal(std::string a_Message, CEGUI::colour a_Colour)
{
	//this will hold the message
	CEGUI::FormattedListboxTextItem* newItem = NULL;

	//display the message only to the user
	newItem = new CEGUI::FormattedListboxTextItem(a_Message, CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
	newItem->setTextColours(a_Colour);	// Set the text color
	m_pChatOutput->addItem(newItem);	// Add the new ListBoxTextItem to the ListBox
}

void ChatManager::SayText(std::string a_DisplayName, std::string a_Message, CEGUI::colour a_Colour, bool a_IsHost)
{
	//this will hold the message
	CEGUI::FormattedListboxTextItem* newItem = NULL;

	//format the message
	std::string output = "";
	if(a_IsHost)
	{
		output += "Host~";
		output += a_DisplayName;
		output += ": ";
		output += a_Message;
		//
		a_Colour = CEGUI::colour(255,0,255);
	}
	else
	{
		output += a_DisplayName;
		output += ": ";
		output += a_Message;
	}

	//display the message
	newItem = new CEGUI::FormattedListboxTextItem(output,CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
	newItem->setTextColours(a_Colour);	// Set the text color
	m_pChatOutput->addItem(newItem);	// Add the new ListBoxTextItem to the ListBox
}

void ChatManager::ClearChatOutput()
{
	short numItems = m_pChatOutput->getItemCount();
	for(short i=0; i<numItems; i++)
	{
		m_pChatOutput->removeItem(m_pChatOutput->getListboxItemFromIndex(0));
	}
}

void ChatManager::ProcTextBoxContents()
{
	if(m_pChatInput)
	{
		std::string msg = m_pChatInput->getText().c_str();
		m_pChatInput->setText("");

		//check if the user is talking, or using a command
		if( msg.find("/") < msg.size() )
		{
			msg = msg.substr(1,msg.size());
			std::string command = msg.substr(0, msg.find(" "));
			std::string argument = msg.substr(msg.find(" ") + 1);
			if(!command.compare("nick"))
			{
				NetworkManager::GetInstance().m_NodeName = argument;
				PrintTextLocal("You have changed your nick to " + argument + ".", CEGUI::colour(0,0,255));
				NetworkManager::GetInstance().UpdateAll();
			}
			else
			{
				PrintTextLocal("Unknown command \"" + command + "\". Did you forget that commands are case sensitive?", CEGUI::colour(0,0,255));
			}
		}
		else
		{
			NetworkManager::GetInstance().EnqueueChatMessage(msg);
		}
	}
}

void ChatManager::ClearChatInputChar()
{
	std::string boxText = m_pChatInput->getText().c_str();
	m_pChatInput->setText(boxText.substr(0,boxText.size()+1).c_str());
}

void ChatManager::ClearChatInput()
{
	if(m_pChatInput)
		m_pChatInput->setText("");
}

void ChatManager::AddClientName(std::string name, bool isSelf, bool isHost)
{
	if(m_pPeerList)
	{
		if(isHost)
		{
			//display the message
			CEGUI::FormattedListboxTextItem* newItem = new CEGUI::FormattedListboxTextItem(name,CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
			newItem->setTextColours(CEGUI::colour(255,0,255));	// Set the text color
			m_pPeerList->addItem(newItem);	// Add the new ListBoxTextItem to the ListBox
		}
		else if(isSelf)
		{
			//display the message
			CEGUI::FormattedListboxTextItem* newItem = new CEGUI::FormattedListboxTextItem(name,CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
			newItem->setTextColours(CEGUI::colour(0,0,255));	// Set the text color
			m_pPeerList->addItem(newItem);	// Add the new ListBoxTextItem to the ListBox
		}
		else
		{
			//display the message
			CEGUI::FormattedListboxTextItem* newItem = new CEGUI::FormattedListboxTextItem(name,CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
			newItem->setTextColours(CEGUI::colour(0,0,0));	// Set the text color
			m_pPeerList->addItem(newItem);	// Add the new ListBoxTextItem to the ListBox
		}
	}
}

void ChatManager::RemoveClientName(std::string name)
{
	if(m_pPeerList)
	{
		CEGUI::ListboxItem* target = m_pPeerList->findItemWithText(name, m_pPeerList->getListboxItemFromIndex(0));
		m_pPeerList->removeItem(target);
	}
}
