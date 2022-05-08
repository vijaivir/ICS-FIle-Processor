/*
 * linkedlist.c
 *
 * Based on the implementation approach described in "The Practice 
 * of Programming" by Kernighan and Pike (Addison-Wesley, 1999).
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ics.h"
#include "emalloc.h"
#include "listy.h"


node_t *new_node(event_t *val) {
    assert( val != NULL);

    node_t *temp = (node_t *)emalloc(sizeof(node_t));

    temp->val = val;
    temp->next = NULL;
    temp->prev = NULL;    

    return temp;
}


node_t *add_front(node_t *list, node_t *new) {
    new->next = list;
    return new;
}


node_t *add_end(node_t *list, node_t *new) {
    node_t *curr;

    if (list == NULL) {
        new->next = NULL;
        return new;
    }

    for (curr = list; curr->next != NULL; curr = curr->next);
    curr->next = new;
    new->next = NULL;
    return list;
}


node_t *insert(node_t *list, node_t *new){
    node_t *cur;
    node_t *pre = NULL;
    
    if(list == NULL){
        new->next = NULL;
        new->prev = NULL;
        return new;
    }

    cur = list;

    if(atoi(new->val->dtstart) == atoi(cur->val->dtstart) &&
       atoi(cur->val->tmstart) < atoi(new->val->tmstart)){
        while(cur != NULL &&
              atoi(new->val->dtstart) == atoi(cur->val->dtstart) &&
              atoi(cur->val->tmstart) < atoi(new->val->tmstart)){
            pre = cur;
            cur = cur->next;
        }
    }else{
        while(cur != NULL && atoi(new->val->dtstart) > atoi(cur->val->dtstart)){
            pre = cur;
            cur = cur->next;
        }
        while(cur != NULL &&
              atoi(new->val->dtstart) == atoi(cur->val->dtstart) &&
              atoi(cur->val->tmstart) < atoi(new->val->tmstart)){
            pre = cur;
            cur = cur->next;
        }
    }
    
    if(pre == NULL){
        new->next = cur;
        new->prev = NULL;
        cur->prev = new;
        return new;
    }else if(cur == NULL){
        new->next = cur;
        new->prev = pre;
        pre->next = new;
        return list;
    }else{
        new->next = cur;
        pre->next = new;
        cur->prev = new;
        new->prev = pre;
        return list;
    }
}


node_t *peek_front(node_t *list) {
    return list;
}


node_t *remove_front(node_t *list) {
    if (list == NULL) {
        return NULL;
    }

    return list->next;
}


void p_apply(node_t *list, void (*fn)(node_t *list, void *, int, int, int),
             void *arg, int from, int to, int op){
    for( ; list != NULL; list = list->next) {
        (*fn)(list, arg, from, to, op);
    }
}


void apply(node_t *list, void (*fn)(node_t *list, void *, int, int),
           void *arg, int from, int to){
    for( ; list != NULL; list = list->next) {
        (*fn)(list, arg, from, to);
    }
}


void r_apply(node_t *list, void (*fn)(node_t *list, void *), void *arg){
    for( ; list != NULL; list = list->next){
        (*fn)(list, arg);
    }
}
