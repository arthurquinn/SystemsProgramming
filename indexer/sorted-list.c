/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 3 - Indexer
 */

#include <stdio.h>
#include "sorted-list.h"

#define NEW_OBJ_SMALLER comparator(current->data, newObj) == 1
#define NEW_OBJ_LARGER comparator(current->data, newObj) == -1
#define NEW_OBJ_EQUAL comparator(current->data, newObj) == 0
#define DOES_NOT_EXIST == NULL
#define EXISTS != NULL
#define NEW_SORTED_LIST_PTR (SortedListPtr) malloc(sizeof(struct SortedList))
#define NEW_LIST_NODE_PTR (ListNodePtr) malloc(sizeof(struct ListNode))
#define NEW_SORTED_LIST_ITERATOR_PTR (SortedListIteratorPtr) malloc(sizeof(struct SortedListIterator))

SortedListPtr SLCreate(CompareFuncT cf, DestructFuncT df)
{
	SortedListPtr sl = NEW_SORTED_LIST_PTR; //allocates memory on heap for struct SortedList and returns pointer to sl //malloc was used here
	if(sl DOES_NOT_EXIST)
		return NULL; //if malloc failed, returns null
	sl->head = NULL; //initialzes sl->head to null
	sl->cf = cf; //stores comparator function in sl->cf (CompareFuncT type)
	sl->df = df; //stores destructor function in sl->df (DestructFuncT type)
	return sl; //returns the sorted list struct pointer
}

void SLDestroy(SortedListPtr list)
{
	DestructFuncT destroy = list->df; //sets the function pointer destroy equal to the destruct function pointer stored in the sorted list
	ListNodePtr current = list->head; //set current and previous to head node
	ListNodePtr previous = list->head;
	while(current EXISTS) //loop continues iteration until both current (and consequently previous) are null
	{
		current = current->next; //current advances to next node (or null)
		if(previous->ptrcount == 1)
		{
			destroy(previous->data); //frees the data stored in the previous node
			free(previous); //previous frees the node after current advances if its ptrcount is equal to 1
		}
		else previous->ptrcount--; //if the previous node ptr count is not equal to 1 (is being pointed to by an iterator) simply decrease pointer count and the 
		//iterator will take care of its deallocation
		previous = current; //previous catches up to current for next iteration (or becomes null for iteration end)
	}
	free(list); //frees the SortedList structure
}

//when called this function will create a new ListNode containing a given data item
ListNodePtr CreateNode(void * newObj)
{
	ListNodePtr p = NEW_LIST_NODE_PTR; //allocates memory for a new list node pointer //malloc was used here
	if(p DOES_NOT_EXIST) return NULL; //if malloc fails return 0 to signal no new node creation
	p->next = NULL; //initializes next to null
	p->data = newObj; //initializes data to the newObj being inserted
	p->ptrcount = 1; //initializes the ptrcount to 1 (to count the pointer used by the caller)
	return p;
}

//ptrcount for each ListNode is never incremented or decremented in this function since all pointers created here will be destroyed after the function call has ended
//for example, after the function call previous and current will be destroyed and will have no effect on ptrcount
//since an iterator can not be called during the exeuction of this function, to increment and then decrement the pointers here would be redundant
//therefore all ptrcounts for pointers created here will remain at ptrcount = 1 after execution of this function
int SLInsert(SortedListPtr list, void * newObj)
{
	CompareFuncT comparator = list->cf; //sets comparator equal to the comparator function pointer stored in struct SortedList
	ListNodePtr previous = list->head; //sets previous to the SortedList head (previous will remain 1 position in the linked list behind current after the first iteration of while
	ListNodePtr current = list->head; //sets current to the SortedList head                                          
	if(list->head DOES_NOT_EXIST) //if the current node (which currently is pointing to the SortedList head) is null, then this will be the first item added to list
	{
		list->head = CreateNode(newObj); //attempts to create a new ListNode for the SortedList head pointer
		return list->head DOES_NOT_EXIST ? 0 : 1; //returns 0 if malloc failed to allocate memory for the head ListNode (insert failure), or 1 if the insert was a success
	}
	while(current->next EXISTS && NEW_OBJ_SMALLER) //while current->next node exists and while the current node's data is larger than newObj
	{
		previous = current;
		current = current->next; //advances current to next node, previous remains 1 node behind
	}
	//after while loop iteration all of the following cases are possible
	if(NEW_OBJ_EQUAL) return 0; //return insert failure if a duplicate is found
	if(current == previous && NEW_OBJ_LARGER) //if the while never executed and newObj is larger, the larger newObj will be inserted as new head node
	{
		list->head = CreateNode(newObj); //a new head node is created containing the new data item
		if(list->head DOES_NOT_EXIST) return 0; //if failed to allocate memory for new ListNode, return 0 for insert failure
		list->head->next = current; //reattaches new head to remainder of the SortedList
	}
	else if(NEW_OBJ_SMALLER) //if the newObj is smaller than the current data item, the insert must occur at the end of the list
	{
		current->next = CreateNode(newObj); //inserts a new ListNode containing the new data item at the end of the list
		if(current->next DOES_NOT_EXIST) return 0; //if the insert of the new item was unsuccessful due to a malloc failure return 0
	}
	else //the final case is that the insertion occurs anywhere in the middle of the list
	{
		previous->next = CreateNode(newObj); //previous is attached to a new ListNode containing the new data item
		if(previous->next DOES_NOT_EXIST) return 0; //if the insert of the new item was unsuccessful due to a malloc failure return 0
		previous->next->next = current; //reattach the remainder of the list to the newly inserted item
	}
	return 1; //returns 1 for successful insert
}

//ptrcounts are never increased when the new pointers current and previous point to list nodes, these pointers are not relevant to ptrcount
//the only pointers that ptrcount is concerned with are pointers that maintain the linked list, such as the head node and each ListNode's next pointer,
//and current pointers from any potential iterators
//the pointers here current and previous are temporary and will never maintain the integrity of the list therefore ptrcount is never adjusted to account
//for them
int SLRemove(SortedListPtr list, void *newObj)
{
	CompareFuncT comparator = list->cf; //sets a comparator function pointer to the comparator function found in the given SortedList data structure
	DestructFuncT destroy = list->df; //sets a destroy function pointer to the destroyer function found in the given SortedList data structure
	ListNodePtr current = list->head; //sets list node pointer current to point to head node
	ListNodePtr previous = list->head; //sets list node pointer previous to point to head node
	if(list->head DOES_NOT_EXIST) return 0; //returns 0 if the list is empty to signal removal failure
	if(NEW_OBJ_EQUAL) //special case if the head node contains the matching data item 
	{
		list->head = current->next; //sets the head node to the second item in the linked list maintaining the integrity of the list
		if(current->ptrcount == 1)
		{
			destroy(current->data); //destroys the data item contained in the current node
			free(current); //frees what was previously the head node if ptrcount is 1 (no iterators are pointing to it)
		}
		else 
		{
			current->ptrcount--; //else it decrememnts the pointer count so signify one less pointer pointing to this node
			if(list->head EXISTS) list->head->ptrcount++; //increments the pointer count of the new head node unless the new head node points to null and thus the list is empty
			//this is done because (unless it is null) the new head node will now be pointed to by the previous head node and the new head node pointer
		}
		//if ptrcount was greater than 1 an iterator was pointing to the head node, since the pointers for the SortedList struct were 
		//adjusted the item was effectively removed from the list, however the node itself was not deallocated since it still may be used
		//by the iterator that is pointing to it
		return 1; //return 1 to signal successful removal
	}
	while(current EXISTS && !NEW_OBJ_EQUAL) //continues iteration while current exists and while current->data does not equal the newObj
	{
		previous = current;
		current = current->next; //advances current list node pointer one item down linked list, keeping previous one item behind
	}
	if(current DOES_NOT_EXIST) return 0; //if the item was not found in the list, returns 0 to signal removal failure
	if(NEW_OBJ_EQUAL && current->next DOES_NOT_EXIST) //if after iteration the new object is equal to the current data item and the current->next node pointer is null
	{
		if(current->ptrcount == 1)
		{
			destroy(current->data); //destroys the data item contained in the current node
			free(current); //since current->next does not exist we are guaranteed that current will be positioned at the end of the list
		}
		else current->ptrcount--; //if the ptrcount was not 1, we decrement to signify that the previous pointer will not longer point to it, effectively removing it from the list
		previous->next = NULL; //to remove it we simply free the current pointer (if the current ptrcount is 1) and set the previous pointer to NULL to ensure the current
		return 1; //list nodes complete removal from the linked list, then we return 1 to signal a successful removal
	}
	else if(NEW_OBJ_EQUAL) //if the data item at the current list node is equal to the new object
	{
		previous->next = current->next; //in this case there is at least one more list node following the node slated for removal
		if(current->ptrcount == 1)
		{
			destroy(current->data); //destroys the data item contained in the current node
			free(current); //if the current node (node to be removed) has a ptrcounter of 1 it is deallocated
		}
		else
		{
			current->ptrcount--; //if the removal nodes ptrcount was not equal to 1, we simply decrement the pointer to signify one less pointer
			current->next->ptrcount++; //since both the previous node and the current node will point to the next node (but only the previous node remains in the list)
			//we increment the next nodes pointer count by one
			//the iterators pointing to current willl be responsible for its deallocation
		}
		return 1; //return 1 to signal a sucessful removal
	}
	return 0; //statement should never execute, kept for default purposes

}

SortedListIteratorPtr SLCreateIterator(SortedListPtr list)
{
	SortedListIteratorPtr iter = NEW_SORTED_LIST_ITERATOR_PTR; //allocate space for new SortedListIterator struct
	if(iter DOES_NOT_EXIST) return NULL; //if malloc failed to allocate memory for iter return null to signal creation failure
	if(list == NULL) return NULL;
	iter->sl = list; //stores the given SortedListPtr into the iterator struct
	iter->current = list->head; //stores the head node of the list into the current pointer within iterator
	iter->current->ptrcount++; //increments the ptrcount of the current node (head node) (since we now have a new iterator struct pointing to it)
	iter->item = NULL;
	iter->itemcount = 0; //item count will count the number of items the iterator has iterated through
	return iter; //returns pointer to the newly created iterator struct
}

void SLDestroyIterator(SortedListIteratorPtr iter)
{
	DestructFuncT destroy = iter->sl->df; //sets the function pointer destroy equal to the function pointer df stored within the list of this iterator
	ListNodePtr previous = iter->current; //set a ListNodePtr previous equal to current that will remain one pointer behind current during deletion
	while(iter->current EXISTS && iter->current->ptrcount == 1) //if the current node exists there is a list of nodes that may potentially need deallocation
	{
		//the nodes that need deallocation are defined as the nodes with ptrcount equal to 1
		iter->current = iter->current->next; //advance the current pointer to the next node
		destroy(previous->data); //destroys the data item stored within previous
		free(previous); //frees the previous node
	}
	//if after deleting all nodes with a ptr counter equal to one, iter->current still exists, decrement its ptrcount, as this iterator
	if(iter->current EXISTS) iter->current->ptrcount--; //will now be destroyed (current will no longer point to it)
	free(iter); //the iterator struct is deallocated and destruction is now complete
}

void * SLNextItem(SortedListIteratorPtr iter)
{
	DestructFuncT destroy = iter->sl->df; //sets the function pointer destroy equal to the function pointer df stored within the list of this iterator
	ListNodePtr tempdel = NULL; //temporary list node pointer for use in case a node requires deletion
	if(iter->current DOES_NOT_EXIST) return NULL; //if the iterator is empty return null
	if(iter->itemcount == 0) //special case if this is the first item being retrieved by iterator
	{
		iter->item = iter->current->data; //since this is the first item in iterator to be returned, simply store the item in iter->item pointer
		iter->itemcount++; //increases the number of items retrieved to 1
	}
	else if(iter->current->next EXISTS) //if the next node in iterator exists begin data retrieval
	{
		iter->current->ptrcount--; //reduces the ptrcount of the current node as we will now advance the node pointer to the next item
		if(iter->current->ptrcount == 0) tempdel = iter->current; //if the ptrcount has reached zero, store the current node in tempdel for deletion
		iter->current = iter->current->next; //advances the current node pointer to the next node within the list
		if(tempdel EXISTS)
		{
			destroy(tempdel->data); //destroys the data item stored in tempdel
			free(tempdel); //if there was a node slated for deletion within tempdel, deallocate it
		}
		else iter->current->ptrcount++; //if the previous node was not slated for deletion, increase current ptrcount by one to signify the additional pointer pointing to it
		//this is because if the item previous in the iterator was deleted, the number of pointers pointing to next remains the same, it only icreases by one if the previous
		//pointer was not deleted
		iter->item = iter->current->data; //store the data item of the current node in iter->item pointer
		iter->itemcount++; //increases the number of items retrieved by 1
	}
	else //case where the end of the list has been reached
	{

		iter->current->ptrcount--; //if there is no next node we decrement the ptrcount by one to signify this iterator will no longer point to this node
		if(iter->current->ptrcount == 0)
		{
			destroy(iter->current->data); //destroys the data item stored in the current node of the iterator
			free(iter->current); //if the ptrcount of this node has reached 0, it is no longer needed and we deallocate it
		}
		iter->current = NULL; //set the current node pointer of iter to null, this will signify to future calls of SLNextItem that list iteration has terminated
		//first if statement in this function will recognize and return NULL...since iter->current begins at head (or NULL if the list is empty)
		//we can be positive that every time this function is called, if iter->current is equal to NULL, list iteration has terminated
		iter->item = NULL; //set the item equal to null so that SLGetItem will recognize either A. the iterator has not been used yet
		//																						B. iteration has terminated
	}
	return iter->item; //this will return the item if an item was successfully retrieved, or NULL if list iteration has terminated
}

void * SLGetItem(SortedListIteratorPtr iter)
{
	return iter->item; //simply returns item which is the item most recently returned by SLNextItem or null which signifies no more items in list
}