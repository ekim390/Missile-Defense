//=================================================================
// Implementation for DLL module.
//
// Copyright 2020 Georgia Tech.  All rights reserved.
// The materials provided by the instructor in this course are for
// the use of the students currently enrolled in the course.
// Copyrighted course materials may not be further disseminated.
// This file must not be made publicly available anywhere.
//=================================================================
#include <stdlib.h>
#include <stdio.h>
#include "doubly_linked_list.h"
#include "globals.h"

LLNode* create_llnode(void* data) {
    // creates a single node
    LLNode* newNode = (LLNode*)malloc(sizeof(LLNode));
    newNode->prev = NULL;
    newNode->next = NULL;
    newNode->data = data;
    return newNode;
}

DLinkedList* create_dlinkedlist(void) {
    DLinkedList* newList = (DLinkedList*)malloc(sizeof(DLinkedList));
    newList->head = NULL;
    newList->tail = NULL;
    newList->size = 0;
    return newList;
}

void insertHead(DLinkedList* dLinkedList, void* data){
    //inserts node at the head of the list
    LLNode* newNode = create_llnode(data);
    newNode->next = dLinkedList->head;
    if (dLinkedList->head != NULL)
        dLinkedList->head->prev = newNode;
    else
        dLinkedList->tail = newNode;
    dLinkedList->head = newNode;
    dLinkedList->size++;
}

void deleteNode(DLinkedList* dLinkedList, LLNode* Node){
    // if node to be deleted is only node in the list
    if ((dLinkedList->head == dLinkedList->tail) && (dLinkedList->head == Node))
    {
        dLinkedList->head = NULL;
        dLinkedList->tail = NULL;
        free(Node);
    }
    // if node to be deleted is first node
    else if (dLinkedList->head == Node) 
    {
        dLinkedList->head = Node->next; // next node is in front of list
        dLinkedList->head->prev = NULL;
        free(Node);
    }
    // if node to be deleted is last node of list
    else if (dLinkedList->tail == Node)
    {
        dLinkedList->tail = Node->prev; // previous node is at end of list
        dLinkedList->tail->next = NULL;
        free(Node);
    }
    //if node to be deleted in neither the first or last node of list
    else 
    {
        LLNode* current = (LLNode*)malloc(sizeof(LLNode));
        current = dLinkedList->head;
        while (current != Node) {
            current = current->next;
        }
        current->prev->next = current->next;
        if (current->next != NULL) {
            current->next->prev = current->prev;
        }
        free(current);
    }
    dLinkedList->size--;
}

void destroyList(DLinkedList* dLinkedList){
  LLNode* Node = dLinkedList->head;
  LLNode* Next;
  while(Node){
    free(Node->data);
    Next = Node->next;
    free(Node);
    Node = Next;
    }
  free(dLinkedList);
}

int getSize(DLinkedList* dLinkedList){
  return dLinkedList->size;
}

// Check for common errors and ASSERT_P to raise the error code.
void testDLL(void){
  int n = 4;
  LLItem* m[n];
  DLinkedList* myList = create_dlinkedlist();
  while (n--)
    m[n] = (LLItem*) malloc(sizeof(LLItem));
  for(n = 0; n<3; n++){
    insertHead(myList, m[n]);
    if ((myList->head->data != m[n]) || (getSize(myList) != n+1))
      uLCD.printf("Complete and debug DLL to get rid of this error\n");
    ASSERT_P(myList->head->data == m[n], ERROR_DLL_INSERT_HEAD);
    ASSERT_P(getSize(myList) == n+1, ERROR_DLL_INSERT_HEAD);
  }
  // [2 1 0]
  LLNode* current = myList->head;
  for (n = 2; n>=0; n--){
    if (current->data != m[n])
      uLCD.printf("Complete and debug DLL to get rid of this error\n");
    ASSERT_P(current->data == m[n], ERROR_DLL_INSERT_HEAD);
    current = current->next;
  }
  pc.printf("OK DLL 3 x insertHead: [2 1 0]\n");
  uLCD.printf("OK DLL 3 inserts:\n    [2 1 0]\n");
  deleteNode(myList, myList->head->next);  // delete middle node
  if (getSize(myList) != 2) 
    uLCD.printf("Complete and debug DLL to get rid of this error\n");
  ASSERT_P(getSize(myList) == 2, ERROR_DLL_DELETE);
  // [2 0]
  current = myList->head;
  for (n = 2; n>=0; n=n-2){
    if (current->data != m[n]) 
      uLCD.printf("Complete and debug DLL to get rid of this error\n");
    ASSERT_P(current->data == m[n], ERROR_DLL_DELETE);
    current = current->next;
  }
  pc.printf("OK DLL deleteNode middle: [2 0]\n");
  uLCD.printf("OK DLL del mid:\n    [2 0]\n");
  insertHead(myList, m[3]); // 3 2 0
  if (getSize(myList) != 3) 
    uLCD.printf("Complete and debug DLL to get rid of this error\n");
  ASSERT_P(getSize(myList) == 3, ERROR_DLL_INSERT_HEAD);
  pc.printf("OK DLL insertHead: [3 2 0]\n");
  uLCD.printf("OK DLL 1 ins:\n    [3 2 0]\n");
  deleteNode(myList, myList->head);  // delete head
  if (getSize(myList) != 2)
    uLCD.printf("Complete and debug DLL to get rid of this error\n");
  ASSERT_P(getSize(myList) == 2, ERROR_DLL_DELETE);
  current = myList->head;
  // [2 0]
  for (n = 2; n>=0; n=n-2){
    if (current->data != m[n]) 
      uLCD.printf("Complete and debug DLL to get rid of this error\n");
    ASSERT_P(current->data == m[n], ERROR_DLL_DELETE);
    current = current->next;
  }
  pc.printf("OK DLL deleteNode head: [2 0]\n");
  uLCD.printf("OK DLL del head:\n    [2 0]\n");
  destroyList(myList);
}
