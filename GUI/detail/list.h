#ifndef OMFG_GUI_LIST_H
#define OMFG_GUI_LIST_H

#include "wnd.h"
#include "llist.h"

#include <string>
//#include <list>

namespace OmfgGUI
{

class List : public Wnd
{
public:
	struct Node;
	
	// typedef std::list<Node> list_t;
	typedef LList<Node> list_t;
	typedef list_t::iterator node_iter_t;
	typedef list_t::reference node_ref_t;
	static const long rowHeight = 12;
	
	struct ColumnHeader
	{
		ColumnHeader(std::string const& name_, double widthFactor_)
		: name(name_), widthFactor(widthFactor_)
		{
		}
		
		std::string name;
		double      widthFactor;
	};
	
	struct Node : public LNodeImp<Node>
	{
		friend class List;
		
		Node(std::string const& text)
			: selected(false), expanded(true),
			  parent(0), level(0)
		{
			columns.push_back(text);
		}
		
		node_iter_t push_back(Node* node)
		{
			children.insert(node);
			
			node->level = level + 1;
			node->parent = this;

			return node_iter_t(node);
		}
		
		void render(Renderer* renderer, long& y, List& list);
		//void renderChildren(Renderer* aRenderer, long& y, List& list);
		void renderFrom(Renderer* renderer, long& y, List& list);
		static node_iter_t findByIdx(node_iter_t i, long aIdx);
		
		void setText(unsigned int column, std::string const& text)
		{
			if(column < columns.size())
				columns[column] = text;
		}
				
		//std::string text;
		std::vector<std::string> columns;
		bool        selected;
		bool        expanded;
		//list_t*     parentList;
		Node*       parent;
		//bool        hasParent;
		long        level;
		//TODO: columns

		list_t      children;
	};
	
	friend struct Node;
	
	List(Wnd* parent, std::string const& tagLabel, std::string const& className,
	  std::string const& id, std::map<std::string, std::string> const& attributes)
	: Wnd(parent, tagLabel, className, id, attributes, ""), m_RootNode("root")
	{
		m_MainSel = m_Base = node_iter_t(0);
		//addColumn(ColumnHeader("Moo", 0.5));
	}
	
	void addColumn(ColumnHeader const& column);
	
	node_iter_t push_back(Node* node)
	{
		node->columns.resize(m_columnHeaders.size());
		node_iter_t i = m_RootNode.children.insert(node);
		node->parent = 0; // Just to be sure
		node->level = m_RootNode.level + 1;

		return i;
	}
	
	void clear()
	{
		m_RootNode.children.clear();
	}
	
	bool isValid()
	{
		return false;
	}

	virtual bool render(Renderer* renderer);
	virtual bool mouseDown(ulong newX, ulong newY, Context::MouseKey::type button);

	virtual int classID();
	
private:
	//list_t           m_Nodes;
	Node             m_RootNode;
	list_t::iterator m_Base;
	list_t::iterator m_MainSel;
	std::vector<ColumnHeader> m_columnHeaders;
};

}

#endif //OMFG_GUI_LIST_H
