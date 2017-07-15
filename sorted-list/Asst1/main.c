/*
 * main.c
 * Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 1 - Sorted List
 */

#include <stdlib.h>
#include <stdio.h>
#include "sorted-list.h"

//defining the student structure
typedef struct Student_
{
	int id;
	char * name;
	int credits;
}Student;

//defining the book structure
typedef struct Book_
{
	char * title;
	char * author;
	char * publishdate;
}Book;

//function to compare basic integer pointers
int compareintptrs(void * ptr1, void * ptr2)
{
	int x = *((int *) ptr1);//function simply casts the void pointers as int pointers
	int y = *((int *) ptr2);	
	if(x < y) return -1;//and returns a value based on their comparison
	else if(y < x) return 1;
	else return 0;
}

//this function simply casts the given pointer as an int and destroys it
void destroyintptrs(void * x)
{
	free((int *) x);
}

void printints(SortedListPtr list)
{
	SortedListIteratorPtr intiter = SLCreateIterator(list); //creates an iterator to iterate through a sorted list of int pointers
	Student * tmp = SLNextItem(intiter); //stores the next item
	while(tmp != NULL)
	{
		printf("|Data: %2d|Ptrcount: %2d|\n", *((int *) tmp),
											 intiter->current->ptrcount - 1); //prints the int value and the current ptrcount of the node
																			  //(subtract 1 to account for the iterator in use here)
	 	tmp = SLNextItem(intiter); //stores the next item
	}
	SLDestroyIterator(intiter);	//destroys iterator when iteration is completed
}

//function to compare student structures. student structures are compared based on their id numbers
int comparestudentsptrs(void * ptr1, void * ptr2)
{
	Student * st1 = (Student *) ptr1; //casts the void pointers to Student pointers
	Student * st2 = (Student *) ptr2;
	if(st1->id < st2->id) return -1;//compares their id fields and returns a value based on that comparison
	else if(st2->id < st1->id) return 1;
	else return 0;
}

//this function simply casts the void pointer as a student pointer and frees it
void destroystudentptrs(void * ptr)
{
	Student * st = (Student *) ptr;
	free(st);
}

//this function prints out a sorted list of student structs using a sorted list iterator
void printstudents(SortedListPtr list)
{
	SortedListIteratorPtr studentiter = SLCreateIterator(list); //creates an iterator to iterate through a sorted list of students
	Student * tmp = SLNextItem(studentiter); //stores the next item
	while(tmp != NULL)
	{
		printf("|ID: %d|Name: %-8s|Credits: %d|\n", tmp->id,
	 	   		                  					tmp->name,
	 										  	  	tmp->credits); //prints the next item
	 	tmp = SLNextItem(studentiter); //stores the next item
	}
	SLDestroyIterator(studentiter);	//destroys iterator when iteration is completed
}

//this function compares two book structs. the books are compared alphabetically based on their title
int comparebookptrs(void * ptr1, void * ptr2)
{
	Book * bk1 = (Book *) ptr1; //cast the two void pointers as book pointers
	Book * bk2 = (Book *) ptr2;
	int cmp = strcmp(bk1->title, bk2->title); //use strcmp to determine the order of the books alphabetically
	if(cmp < 1) return -1;//return a value based on the above comparison
	else if(cmp > 1) return 1;
	else return 0;

}

//function used to destroy book pointers
//this function simply casts the void pointer as a book, and frees it
void destroybookptrs(void * ptr)
{
	Book * bk = (Book *) ptr;
	free(bk);
}

//this function uses a sorted list iterator to print out a sorted list of books
//the books are ordered in reverse alphabetical order (Z through A)
void printbooks(SortedListPtr list)
{
	SortedListIteratorPtr bookiter = SLCreateIterator(list); //create a sorted list iterator to iterate through book list
	Book * tmp = SLNextItem(bookiter); //stores next book in tmp
	while(tmp != NULL)
	{
		printf("|Title: %-17s|Author: %-20s|Release Date: %-4s|\n", tmp->title,
	 	   		                  									 tmp->author,
	 										  	  					 tmp->publishdate); //prints out the current book
	 	tmp = SLNextItem(bookiter); //gets next book
	}
	SLDestroyIterator(bookiter); //destroys the book iterator
}

int main()
{
	int i = 0;
	int * tmp = NULL;
	char keystroke = 0;
	int (*Comparator)(void *, void *) = compareintptrs; //declares function pointers for integer comparison to be passed to SLCreate
	void (*Destroyer)(void *) = destroyintptrs;
	SortedListPtr list = SLCreate(Comparator, Destroyer); //creates a sorted list of integers

	int ** intarray = malloc(sizeof(int *) * 20); //the array that will hold the integer pointers to be inserted

	for(i = 0; i < 20; i++) //this loop inserts the integer pointers into the sorted list
	{
		tmp = (int *) malloc(sizeof(int));
		*tmp = i + 1;
		intarray[i] = tmp;
		SLInsert(list, (void *) intarray[i]);
	}

	//this is the beginning of the testing phase of the sorted list
	printf("1. The following is a test of the sorted list using integer data objects value 1 through 20\nPress Enter to continue...");
	keystroke = getchar();

	printints(list); //prints out the list in its current state

	printf("The above is a Sorted List containing 20 integer data objects.\n");
	printf("\n2. To test the remove functionality, items from the beginning, middle and end of the list will be removed");
	printf("\nRemoving values: 1 13 20\nPress Enter to continue...");
	keystroke = getchar();

	//attempts to remove the following items from the sorted list
	SLRemove(list, (void *) intarray[12]); //test removal from the middle of list (data = 13)
	SLRemove(list, (void *) intarray[0]); //test removal from the end of the list (data = 1)
	SLRemove(list, (void *) intarray[19]); //test removal from the beginning of the list (data = 20)
	
	printints(list);

	intarray[12] = (int *) malloc(sizeof(int)); //the following statements recreate the previously freed integer objects for reinsertion into the sorted list
	intarray[0] = (int *) malloc(sizeof(int)); //these integers were freed when they were removed as per the specifications of the sorted list
	intarray[19] = (int *) malloc(sizeof(int));
	*intarray[12] = 13;
	*intarray[0] = 1;
	*intarray[19] = 20;

	printf("As seen above, the nodes containing the values 1, 13 and 20 have been successfully removed.");
	printf("\n\n3. Testing for removal of item that does not exist within the list: Attempt to remove a node containing the value 21.");
	printf("\nPress Enter to continue...");
	keystroke = getchar();

	//creates an integer pointer to be removed from the sorted list
	int * wrongval = (int *) malloc(sizeof(int));
	*wrongval = 21;
	printf(SLRemove(list, (void *) wrongval) == 0 ? "Removal failed. Item does not exist.\n" : "Removal Succeeded.\n");
	printints(list);

	printf("The list remains unchanged after an unsuccessful removal attempt.");
	printf("\n\n4. Test of insert functionality: Insert an item at the beginning, middle, and end of the list");
	printf("\nItems to be inserted: 1 13 20\nPress Enter to conitnue...");
	keystroke = getchar();


	SLInsert(list, intarray[12]); //test insertion in the middle of the list (data = 13)
	SLInsert(list, intarray[0]); //test insertion at the end of the list (data = 1)
	SLInsert(list, intarray[19]); //test insertion at the beginning of the list (data = 20)

	printints(list);

	printf("Above nodes successfully inserted.");
	printf("\n\n5. Attempt to insert duplicate item: 13.\nPress Enter to continue...");
	keystroke = getchar();

	printf(SLInsert(list, (void *) intarray[12]) == 0 ? "Insert unsuccessful. Duplicate value found.\n" : "Insert successful.\n");
	printints(list);

	printf("List unchanged after unsuccessful insert.\n");
	printf("\n\n6. Functionality test of sorted list iterators:\n");
	printf("A node with a pointer counter greater than or equal to 2 indicates one or more iterators pointing to that list node.\n");
	printf("Creating two iterators that will point to the sorted list's head node.\nPress Enter to continue...");
	keystroke = getchar();

	//creates two new sorted list iterators for use by the sorted list
	SortedListIteratorPtr iterator1 = SLCreateIterator(list);
	SortedListIteratorPtr iterator2 = SLCreateIterator(list);
	printints(list);

	printf("The pointer counter of the head node was increased to 3. This indicates that both the head node pointer of the sorted list and");
	printf("\ntwo current pointers of two iterators are pointing to this node.");
	printf("\n\n7. Test functionality of iterators iterating through list:");
	printf("\nUse the first iterator to return each data value until it reaches 13. The results of iteration will be printed above the list");
	printf("\nPress Enter to continue...");
	keystroke = getchar();

	//prints out the return values of the sorted list iterator function SLNextItem
	printf("Iterator return values: ");
	while(*((int *) tmp) != 13)
	{
		tmp = SLNextItem(iterator1);
		printf("%2d ", *((int *) tmp));
	}
	printf("\n");
	printints(list);

	printf("There are now two pointers pointing to the node containing data value 13. This indicates that not only is the previous node");
	printf("\nin the sorted list pointing to 13, but also the iterator. Also, the pointer counter of the node containing the value 20 reduced to 2");
	printf("\nindicating there is now only 1 iterator pointing to this node.\n");
	printf("\n8.The specifications of this program require that our iterators be \"sticky\", i.e., even if the current node that the iterator is pointing");
	printf("\nto is removed from the list, the iterator must \"stick\" to that data item and continue iteration as normal through the rest of the list.");
	printf("\nTo test this functionality: Remove the data value 13.\nPress Enter to continue...");
	keystroke = getchar();

	//removes the value 13 from the sorted list
	SLRemove(list, (void *) intarray[12]);
	printints(list);

	printf("The node containing 12 now has two pointers pointing to it, indicating that not only is the previous node in the list");
	printf("\npointing to it, but also the node containing 13 that the iterator \"stuck\" to even though it was removed from the list.");
	printf("\nReturn value of SLGetItem for iterator 1 is still 13. \n\tSLGetItem returns: %d", *((int *) SLGetItem(iterator1)));
	printf("\nPress Enter to continue...");
	keystroke = getchar();

	printf("\n9. Use iterator 1 to continue iteration until the end of the list. When iteration terminates a null value is returned");
	printf("\nand the iterator is destroyed by this main. It is the users reponsibility to call the Sorted List Iterator's SLDestroyIterator function");
	printf("\nwhen a null value is reached. Otherwise SLNextItem will continually return null.");
	printf("\nReturn values of SLNextItem displayed below.\nPress Enter to continue...");
	keystroke = getchar();

	//prints out the return values of the sorted list iterator
	printf("Iterator return values: ");
	tmp = SLNextItem(iterator1);
	while(tmp != NULL)
	{
		printf("%2d ", *((int *) tmp));
		tmp = SLNextItem(iterator1);
	}
	printf("\n");

	//destroys the sorted list iterator iterator1
	SLDestroyIterator(iterator1);
	printints(list);

	printf("First iterator destroyed.");
	printf("\n\n10. Test for removal of the head node while iterator 2 is still pointing to it.");
	printf("\nPress Enter to continue...");
	keystroke = getchar();

	//removes value 20 from the sorted list (the value within the head node)
	SLRemove(list, intarray[19]);
	printints(list);
	printf("The node containing the value 19 has two pointers pointing to it. These are the sorted lists head node pointer, and the node containing");
	printf("\nthe value 20. Note that the node containing the value 20 was not deallocated - it still had an iterator pointing to it. While it was removed");
	printf("\nfrom the list, it was not deallocated as it is still being pointed to by iterator 2. It will be deallocated when iterator 2 is destroyed, or when");
	printf("\niterator 2 passes over it during iteration.");
	printf("\n\n11. Destroy iterator 2: The data node containing the value 19 will once again only have 1 pointer pointing to it.");
	printf("\nPress Enter to continue...");
	keystroke = getchar();

	//destroys the second iterator
	SLDestroyIterator(iterator2);
	printints(list);

	printf("No iterators remaining.");
	printf("\n\n12. Create 3 more iterators to continue testing.\nPress Enter to continue...");
	keystroke = getchar();

	//creates 3 new iterators for use by the sorted list
	iterator1 = SLCreateIterator(list);
	iterator2 = SLCreateIterator(list);
	SortedListIteratorPtr iterator3 = SLCreateIterator(list);

	printints(list);
	printf("There are 4 pointers pointing to the list node containing the value 19. These are the head node pointer and our three iterators.");
	printf("\n\n13. Iterate the following...\n\tIterator 1 to the node containing value 11\n\tIterator 2 to the node containing value 10");
	printf("\n\tIterator 3 to the node containing value 9.\nPress Enter to continue...");
	keystroke = getchar();

	//the following three do while loops print out the sorted list iterators next items until they reach their respective stop values
	printf("Iterator 1 return values: ");
	do
	{
		tmp = SLNextItem(iterator1);
		printf("%d ", *((int *) tmp));
	}while(*((int *) tmp) != 11);
	printf("\niterator 2 return values: ");
	do
	{
		tmp = SLNextItem(iterator2);
		printf("%d ", *((int *) tmp));
	}while(*((int *) tmp) != 10);
	printf("\nIterator 3 return values: ");
	do
	{
		tmp = SLNextItem(iterator3);
		printf("%d ", *((int *) tmp));
	}while(*((int *) tmp) != 9);
	printf("\n");

	printints(list);

	printf("There are two pointers pointing to each of the nodes containing the data values 9 10 and 11.");
	printf("\n\n14. Test removal of successive nodes that all have iterators: Remove the values 9 10 and 11 entirely from the list.");
	printf("\nPress Enter to continue...");
	keystroke = getchar();

	//removes the values 11, 10, and 9 from the sorted list
	SLRemove(list, intarray[10]);
	SLRemove(list, intarray[9]);
	SLRemove(list, intarray[8]);
	printints(list);

	printf("There appears to be only 1 iterator pointing to the list node containing the value 8. This is because iterator 1 is pointing to the");
	printf("\nnode containing 11, iterator 2 is pointing to the node containing 10, and iterator 3 is pointing to the node containing 9. In fact it is not");
	printf("\neven an iterator that is pointing to the node containing 8. Rather, it is the node containing 9 that was just removed from the list. Since");
	printf("\niterator 3 was pointing to 9 when it was removed, the node containing 9 was \"stickied\" and thus is still pointing to 8 as well as the previous");
	printf("\nnode in the list (12), thus 8's pointer count is 2.");
	printf("\nAll of the nodes that were removed have iterators that are indeed still pointing to them, but we see no evidence of this in the list output since");
	printf("\nthe nodes are no longer in the list. Here is evidence that they are still \"stuck\":");
	printf("\n\tIterator 1 SLGetItem: %d\n\tIterator 2 SLGetItem: %d\n\tIterator 3 SLGetItem: %d\n", *((int *) SLGetItem(iterator1)), 
																								   *((int *) SLGetItem(iterator2)), 
																								   *((int *) SLGetItem(iterator3)));
	printf("\n15. Return SLNextItem for each iterator.\nPress Enter to continue...");
	keystroke = getchar();

	//prints out each of the iterators next item
	printf("\tIterator 1 SLNextItem: %d", *((int *) SLNextItem(iterator1)));
	printf("\n\tIterator 2 SLNextItem: %d", *((int *) SLNextItem(iterator2)));
	printf("\n\tIterator 3 SLNextItem: %d\n", *((int *) SLNextItem(iterator3)));
	printints(list);

	printf("Iterator 1 returned 10 even though it was no longer in the list, and iterator 2 returned 9 even though it was no longer in the list.");
	printf("\nThis is becuase those values were all \"stickied\" to their iterators despite their removal from the list. We also see the pointer count of");
	printf("\nthe node 8 is now 3. This signifies that the node containing 9 is still pointing to it (because 9 is now \"stickied\" by iterator 2),");
	printf("\niterator 3's current node is pointing to it (as that is the current node of iterator 3), and the previous node in the sorted list is pointing");
	printf("\nto it. Thus it now has a pointer count of 3. At this time, the node containing 11 has been deallocated as it is no longer \"stickied\" by");
	printf("\niterator 1.");
	printf("\n\n16. Return SLNextItem values for each iterator.\nPress Enter to continue...");
	keystroke = getchar();

	//prints out each of the iterators next item
	printf("\tIterator 1 SLNextItem: %d", *((int *) SLNextItem(iterator1)));
	printf("\n\tIterator 2 SLNextItem: %d", *((int *) SLNextItem(iterator2)));
	printf("\n\tIterator 3 SLNextItem: %d\n", *((int *) SLNextItem(iterator3)));
	printints(list);

	printf("The returned nodes are once again the next consecutive values - including those that had been removed due to the \"sticky\" functionality. We also");
	printf("\nsee evidence of the iterators once again pointing to items within the list through their increased pointer counts.");
	printf("\nThe node containing 7 is pointed to by 2 pointers, the previous node in the list, and the third iterator. The node containing 8 is pointed to");
	printf("\nby 3 pointers, the node containing 9 which is still \"stickied\", the second iterator, and the previous item in the sorted list itself. Iterator 1");
	printf("\nis pointing to the node containing 9 but we see no evidence of this as 9 is no longer in the list. At this point the node containing 10 has been");
	printf("\ndeallocated as it is no longer \"stickied\" by any iterator.");
	printf("\n\n17. Return SLNextItem for each iterator.\nPress Enter to continue...");
	keystroke = getchar();

	//prints out each of the iterators next item
	printf("\tIterator 1 SLNextItem: %d", *((int *) SLNextItem(iterator1)));
	printf("\n\tIterator 2 SLNextItem: %d", *((int *) SLNextItem(iterator2)));
	printf("\n\tIterator 3 SLNextItem: %d\n", *((int *) SLNextItem(iterator3)));
	printints(list);
	printf("All of the consecutive values have been returned due to the \"sticky\" functionality. It is important to note at this time the nodes that were");
	printf("\npreviously \"stuck\", i.e., the nodes containing the values 11 10 and 9, have all been deallocated, as there are no longer any iterators");
	printf("\npointing to them. Iterator 1 is pointing to the node with value 8, iterator 2 is pointing to the node with value 7, and iterator 3");
	printf("\nis pointing to the node with value 6. All of the nodes that were \"sticked\" have been iterated over and are no longer needed. And, as they");
	printf("\nwere also not being pointed to by any node within the list, they have all been properly deallocated from the heap.");
	printf("\n\n18. Destroy iterators 2 and 3. Iterate iterator 1 to the last value in the sorted list.\nPress Enter to continue...");
	keystroke = getchar();

	///destroys the two iterators
	SLDestroyIterator(iterator2);
	SLDestroyIterator(iterator3);

	//prints out the next item of the iterator until it reaches the value 1 (last node in the list)
	printf("Iterator return values: ");
	do
	{
		tmp = SLNextItem(iterator1);
		printf("%2d ", *((int *) tmp));
	} while((*(int *) tmp) != 1);
	printf("\n");
	printints(list);
	printf("Iterators 2 and 3 destroyed. Iterator 1 iterated to final value.");
	printf("\nThere are now 2 pointers pointing to the last node in the sorted list: The previous item in the sorted list and the first iterator.");
	printf("\n\n19. Remove the last node in the last (data item 1).\nPress Enter to continue...");
	keystroke = getchar();

	//removes the last node in the list
	SLRemove(list, (void *) intarray[0]);
	printints(list);

	printf("The last node was removed from the list but the iterator should still be \"stickied\" to it. Check the return value of SLGetItem.");
	printf("\n\tIterator SLGetItem: %d", *((int *) SLGetItem(iterator1)));;
	printf("\nThe iterator is still \"stuck\" to 1 even though it was removed from the list. This should also be the last value returned by SLNextItem.");
	printf("\nSLNextItem should now return NULL:");
	printf(SLNextItem(iterator1) == NULL ? "\n\tSLNextItem returned NULL" : "\n\tSLNextItem did not return NULL");
	printf("\nPress Enter to continue...");
	keystroke = getchar();


	printf("\n\nThis concludes the in depth functional analysis of the Sorted List and Sorted List Iterator. The following are more brief tests using");
	printf("\nmore complex data such as structs. The current list and all iterators will be destroyed.\n\n");
	SLDestroyIterator(iterator1);
	
	SLDestroy(list);

	/* The above was an extensive test to demonstrate the functionality of our sorted list and sorted list iterators using int pointers. Its
	 * purpose was to be very vigorous in its testing to prove its capabilities.
	 * The following will be a less intensive approach to demonstrate the "generality" of our sorted list
	 * That is, we will be doing a less thorough examination of our sorted list data structure using strings and student structs to demonstrate
	 * that our sorted list can indeed handle any kind of data type pointers as long as it is provided with a compare and destroy function capable
	 * of comparing and destroying those data types. 
	 * The following will test only construction of a list, removal of items, and iteration through the list. For the in-depth analysis, please refer above
	 */

	 //creates 5 student structures items to insert into a student sorted list
	Student * student1 = (Student *) malloc(sizeof(Student));
	Student * student2 = (Student *) malloc(sizeof(Student));
	Student * student3 = (Student *) malloc(sizeof(Student));
	Student * student4 = (Student *) malloc(sizeof(Student));
	Student * student5 = (Student *) malloc(sizeof(Student));
	student1->id = 10980;
	student2->id = 61450;
	student3->id = 30786;
	student4->id = 41823;
	student5->id = 90134;
	student1->name = "Steve";
	student2->name = "Tony";
	student3->name = "Bruce";
	student4->name = "Natasha";
	student5->name = "Clint";
	student1->credits = 13;
	student2->credits = 14;
	student3->credits = 12;
	student4->credits = 16;
	student5->credits = 14;

	//declares function pointers for use by the student sorted list
	int (*comparatorstudents)(void *, void *) = comparestudentsptrs;
	void (*destroyerstudents)(void *) = destroystudentptrs;
	SortedListPtr studentlist = SLCreate(comparatorstudents, destroyerstudents); //creates a sorted list for the student structures

	
	//inserts the 5 student structures into the sorted list and prints the list
	printf("Inserting 5 students into the list. Students are ordered by their ID number.\nPress Enter to see the results of insertion...");
	keystroke = getchar();
	SLInsert(studentlist, (void *) student1);
	SLInsert(studentlist, (void *) student2);
	SLInsert(studentlist, (void *) student3);
	SLInsert(studentlist, (void *) student4);
	SLInsert(studentlist, (void *) student5);
	printstudents(studentlist);

	printf("\nRemoving student with ID number: 30768. Press Enter to see results of removal...");
	keystroke = getchar();

	//removes the given student from the sorted list and prints the resulting list
	SLRemove(studentlist, (void *) student3);
	printstudents(studentlist);

	printf("\nAttempting to add repeat of student with ID number: 61450. Press Enter to see results of insertion...");
	keystroke = getchar();

	//attempts to insert a repeat item and prints out the resulting list
	printf(SLInsert(studentlist, (void *) student2) == 0 ? "Insertion failed. Repeat Item.\n" : "Insertion succeeded.\n");
	printstudents(studentlist);

	//destroys the student sorted list
	SLDestroy(studentlist);

	printf("\nPrinting of the above student structures was done using a SortedListIterator.");
	printf("\n\nTest the sorted list structure using book structs. Books are ordered by their title.");

	//creates 5 new book structs to be used in the book sorted list
	Book * book1 = (Book *) malloc(sizeof(Book));
	Book * book2 = (Book *) malloc(sizeof(Book));
	Book * book3 = (Book *) malloc(sizeof(Book));
	Book * book4 = (Book *) malloc(sizeof(Book));
	Book * book5 = (Book *) malloc(sizeof(Book));
	book1->title = "Moby Dick";
	book2->title = "War and Peace";
	book3->title = "The Great Gatsby";
	book4->title = "Don Quixote";
	book5->title = "1984";
	book1->author = "Herman Melville";
	book2->author = "Leo Tolstoy";
	book3->author = "F. Scott Fitzgerald";
	book4->author = "Miguel de Cervantes";
	book5->author = "George Orwell";
	book1->publishdate = "1851";
	book2->publishdate = "1869";
	book3->publishdate = "1925";
	book4->publishdate = "1605";
	book5->publishdate = "1949";

	//declares two function pointers that will be passed to SLCreate to create the book sorted list
	int (*bookcomparator)(void *, void *) = comparebookptrs;
	void (*bookdestroyer)(void *) = destroybookptrs;
	SortedListPtr booklist = SLCreate(bookcomparator, bookdestroyer);

	//inserts the 5 book structs into the sorted list and prints the result
	SLInsert(booklist, (void *) book1);
	SLInsert(booklist, (void *) book2);
	SLInsert(booklist, (void *) book3);
	SLInsert(booklist, (void *) book4);
	SLInsert(booklist, (void *) book5);
	printf("\nBooks inserted into sorted list. Press Enter to see insertion results...");
	keystroke = getchar();
	printbooks(booklist);

	printf("\nAttempt to remove a book not included in the list. Attempting to remove book with title: \"To Kill a Mockingbird\".");
	printf("\nPress enter to see the result of the removal...");
	keystroke = getchar();

	//attempts remove a book that does not exist within the list from the list and prints the resulting list
	Book * wrongbook = (Book *) malloc(sizeof(Book));
	wrongbook->title = "To Kill a Mockingbird";
	wrongbook->author = "Harper Lee";
	wrongbook->publishdate = "1960";
	printf(SLRemove(booklist, (void *) wrongbook) == 0 ? "Item not found in list. Removal Failed.\n" : "Book successfully removed.\n");
	printbooks(booklist);

	printf("\nA sorted list iterator was used to print the sorted list of books.");
	printf("\nThis concludes testing of both the sorted list structure and the sorted list iterator.\nPress Enter to exit...");
	keystroke = getchar();
	//destroys the book sorted list and exits the program
	SLDestroy(booklist);

	return 0;
}	