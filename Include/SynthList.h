/////////////////////////////////////////////////////
// BasicSynth double-linked list class
//
/// @file SynthList.h Linked list template.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////
/// @addtogroup grpGeneral
//@{
#ifndef _SYNTHLIST_
#define _SYNTHLIST_

/////////////////////////////////////////////////////
/// Linked list item template. 
/// A variety of BasicSynth classes derive from this base.
/// It is used wherever the library code needs to store things in a 
/// dynamic list.
/// @note This is NOT a general purpose linked list class.
/// This is low-overhead, fast and simple, which is what 
/// the synthesizer needs. For example, 
/// the Insert() and InsertBefore() methods take the NEW node that
/// should be added to the list. You CANNOT do:
/// @code
/// newNode->Insert(oldNode)
/// @endcode
///////////////////////////////////////////////////////
template<class T> class SynthList
{
public:
	T *next;
	T *prev;

	SynthList()
	{
		next = prev = NULL;
	}

	/// Insert the new node after this node.
	/// @param pnew pointer to the new node
	T *Insert(T *pnew)
	{
		pnew->next = next;
		pnew->prev = (T*)this;
		if (next)
			next->prev = pnew;
		next = pnew;
		return pnew;
	}

	/// Insert the new node before this node.
	/// @param pnew pointer to the new node
	T *InsertBefore(T *pnew)
	{
		pnew->prev = prev;
		pnew->next = (T*)this;
		if (prev)
			prev->next = pnew;
		prev = pnew;
		return pnew;
	}

	/// Remove this node from the list.
	/// This does not destroy the node; you must still call delete.
	T *Remove()
	{
		T *pold = next;
		if (next)
			next->prev = prev;
		if (prev)
			prev->next = next;
		next = NULL;
		prev = NULL;
		return pold;
	}
};

/// Enumerated list of SynthList items.
/// T must be derived from SynthList<> and must
/// implement a constructor without arguments.
template <class T> class SynthEnumList
{
public:
	T head;
	T tail;

	SynthEnumList()
	{
		head.Insert(&tail);
	}

	~SynthEnumList()
	{
		Clear();
	}

	void Clear()
	{
		T *itm;
		while ((itm = head.next) != &tail)
		{
			itm->Remove();
			delete itm;
		}
	}

	/// Enumerate the list.
	/// Pass a null pointer to get the first
	/// item in the list. Thereafter pass the
	/// previous item:
	/// @code
	/// T *itm = 0;
	/// while ((itm = list.EnumItem(itm)) != 0)
	///     ;
	/// @endcode
	/// @returns pointer to item or null
	T *EnumItem(T *itm)
	{
		if (itm == 0)
			itm = &head;
		itm = itm->next;
		if (itm == &tail)
			return 0;
		return itm;
	}

	/// Add a new item to the linked list.
	/// @returns pointer to new item
	T *AddItem()
	{
		T *itm = new T;
		if (itm)
			tail.InsertBefore(itm);
		return itm;
	}

	T *AddItem(T *itm)
	{
		tail.InsertBefore(itm);
		return itm;
	}
};

//@}
#endif
