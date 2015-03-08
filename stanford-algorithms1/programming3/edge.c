#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

#include "edge.h"

struct _edge_element {
    int ref;
    vertex_element * left;
    vertex_element * right;
    edge_element * previous;
    edge_element * next;
};

static edge_element * head = NULL;
static edge_element * tail = NULL;

static int size = 0;

static edge_element * edge_new (vertex_element * left, vertex_element * right)
{
    if (left == NULL || right == NULL) return NULL;

    edge_element * edge = malloc (sizeof(edge_element));
    if (edge == NULL) return NULL;

    bzero (edge, sizeof(edge_element));
    edge->left = left;
    edge->right = right;
    //printf ("edge[%i - %i] created!\n", vertex_id(left), vertex_id(right));
    return edge;
}

static void edge_free (edge_element * edge)
{
    //printf("edge_free: %p\n", edge);
    free (edge);
}

static void edge_append (edge_element * edge)
{
    size++;

    edge->next = NULL;

    edge_ref(edge);

    if (tail == NULL) {
        head = tail = edge;
        edge->previous = NULL;
    } else {
        tail->next = edge;
        edge->previous = tail;
        tail = edge;
    }
}

void edge_remove (edge_element * edge)
{
    size--;

    if (edge == head) {
        // edge is the first
        head = edge->next;
        if (head == NULL)
        {
            // list is empty
            tail = NULL;
            edge_unref(edge);
            return;
        }
        head->previous = NULL;
        edge_unref(edge);
        return;
    }  

    if (edge == tail) {
        // edge is the last
        tail = edge->previous;
        tail->next = NULL;
        edge_unref(edge);
        return;
    }

    // edge is in the middle
    edge->previous->next = edge->next;
    edge->next->previous = edge->previous;
    edge_unref(edge);
}

static edge_element * edge_find (vertex_element * left, vertex_element * right)
{
    edge_element * edge = head;
    while(edge) {
        if ((edge->left == left  && edge->right == right) ||
            (edge->left == right && edge->right == left)) {
            //printf ("edge[%i - %i] found!\n", vertex_id(left), vertex_id(right));
            return edge;
        }
        edge = edge->next;
    }
    return NULL;
}

edge_element * edge_ref (edge_element * edge)
{
    if (edge == NULL) return NULL;

    edge->ref++;
    return edge;
}

void edge_unref (edge_element * edge)
{
    if (edge == NULL) return;
    if (--edge->ref == 0) edge_free (edge);
}

int edge_size ()
{
    return size;
}

void edge_make (vertex_element * left, vertex_element * right)
{
    edge_element * edge = edge_find (left, right);
    if (edge != NULL) return;

    edge = edge_new (left, right);
    if (edge == NULL) return;

    edge_append (edge);

    vertex_add_edge (left, edge);
    vertex_add_edge (right, edge);
    return ;
}

edge_element * edge_index (long int index)
{
    edge_element * edge = head;

    while (index-- > 0 && edge) {
        edge = edge->next;
    }

    return edge;
}

vertex_element * edge_left (edge_element * edge)
{
    return edge?edge->left:NULL;
}

vertex_element * edge_right (edge_element * edge)
{
    return edge?edge->right:NULL;
}

void edge_change_vertex (edge_element * edge, vertex_element * old_vertex, vertex_element * new_vertex)
{
    if (edge == NULL) return;

    //printf("\t\tchange vertex: edge[%i-%i] old[%i] new[%i]\n",
    //       vertex_id(edge->left), vertex_id(edge->right),
    //       vertex_id(old_vertex), vertex_id(new_vertex));

    if (edge->left == old_vertex) {
        edge->left = new_vertex;
        return;
    }

    if (edge->right == old_vertex) {
        edge->right = new_vertex;
        return;
    }

    //printf ("======> algo esta errado\n");
}

void edge_dump ()
{
    printf ("#edges: %i\n", size);
    edge_element * element = head;
    while (element) {
        printf("\t%i-%i\n", vertex_id(element->left), vertex_id(element->right));
        element = element->next;
    }
}

void edge_remove_self_loops ()
{
    //printf("removendo edge - self loop\n");
    edge_element * edge = head;
    while (edge) {
        //printf("\tedge: %p\n", edge);
        if (edge->left == edge->right) {
            //printf("\t\tremoving edge: %p\n", edge);
            edge_element * next = edge->next;
            edge_remove (edge);
            edge = next;
            continue;
        }
        edge = edge->next;
    }
}
