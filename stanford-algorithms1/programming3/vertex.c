#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

#include "vertex.h"
#include "edge.h"

typedef struct _edge_list edge_list;

struct _edge_list {
    edge_element * edge;
    edge_list * previous;
    edge_list * next;
};

struct _vertex_element {
    int id;
    edge_list * edges_head;
    edge_list * edges_tail;
    int edges;
    vertex_element * previous;
    vertex_element * next;
};

static vertex_element * head = NULL;
static vertex_element * tail = NULL;

static int size;

static vertex_element * vertex_new (int id)
{
    //printf ("vertex[%i] created!\n", id);
    vertex_element * vertex = malloc (sizeof(vertex_element));
    if (vertex == NULL) return NULL;
    bzero (vertex, sizeof(vertex_element));
    vertex->id = id;
    return vertex;
}

static void vertex_free (vertex_element * vertex)
{
    //printf("vertex %i free\n", vertex->id);
/*    
    edge_list * element = vertex->edges_head;
    while (element != NULL) {
        edge_list * next = element->next;
        edge_list_free (element);
        element = next;
    }
*/
    free (vertex);
}

static void vertex_append (vertex_element * vertex)
{
    size++;

    vertex->next = NULL;

    if (tail == NULL) {
        head = tail = vertex;
        vertex->previous = NULL;
    } else {
        tail->next = vertex;
        vertex->previous = tail;
        tail = vertex;
    }
}

static void vertex_remove (vertex_element * vertex)
{
    size--;

    if (vertex == head) {
        // vertex is the first
        head = vertex->next;
        if (head == NULL)
        {
            // list is empty
            tail = NULL;
            return;
        }
        head->previous = NULL;
        return;
    }  

    if (vertex == tail) {
        // vertex is the last
        tail = vertex->previous;
        tail->next = NULL;
        return;
    }

    // vertex is in the middle
    vertex->previous->next = vertex->next;
    vertex->next->previous = vertex->previous;
}

static vertex_element * vertex_find (int id)
{
    vertex_element * vertex = head;
    while(vertex) {
        if (vertex->id == id) {
            //printf ("vertex[%i] found!\n", id);
            return vertex;
        }
        vertex = vertex->next;
    }
    return NULL;
}

static edge_list * edge_list_new (edge_element * edge)
{
    edge_list * element = malloc (sizeof(edge_list));
    if (element == NULL) return NULL;

    bzero (element, sizeof(edge_list));
    element->edge = edge_ref(edge);
    return element;
}

static void edge_list_free (edge_list * element)
{
    if (element == NULL) return;
    //printf("edge_list_free\n");

    edge_unref(element->edge);
    free (element);
}

static void edge_list_append (vertex_element * vertex, edge_list * element)
{
    vertex->edges++;

    element->next = NULL;

    if (vertex->edges_tail == NULL) {
        vertex->edges_head = vertex->edges_tail = element;
        element->previous = NULL;
    } else {
        vertex->edges_tail->next = element;
        element->previous = vertex->edges_tail;
        vertex->edges_tail = element;
    }
}

static void edge_list_remove (vertex_element * vertex, edge_list * element)
{
    vertex->edges--;

    if (element == vertex->edges_head) {
        // element is the first
        vertex->edges_head = element->next;
        if (vertex->edges_head == NULL)
        {
            // element is empty
            vertex->edges_tail = NULL;
            return;
        }
        vertex->edges_head->previous = NULL;
        return;
    }  

    if (element == vertex->edges_tail) {
        // element is the last
        vertex->edges_tail = element->previous;
        vertex->edges_tail->next = NULL;
        return;
    }

    // element is in the middle
    element->previous->next = element->next;
    element->next->previous = element->previous;
}

static edge_list * edge_list_find (vertex_element * vertex, edge_element * edge)
{
    edge_list * element = vertex->edges_head;
    while(element) {
        if (element->edge == edge) {
            return element;
        }
        element = element->next;
    }
    return NULL;
}

static void edge_list_dump (vertex_element * vertex)
{
    printf("\t#edges: %i\n", vertex->edges);
    edge_list * element = vertex->edges_head;
    while(element) {
        printf("\t\t%i-%i\n", edge_left(element->edge)->id, edge_right(element->edge)->id);
        element = element->next;
    }
}

static void vertex_dump (vertex_element * vertex)
{
    printf ("vertex: %i\n", vertex->id);
    edge_list_dump (vertex);    
}

vertex_element * vertex_make (int id)
{
    vertex_element * vertex = vertex_find (id);
    if (vertex != NULL) return vertex;

    vertex = vertex_new (id);
    if (vertex == NULL) return NULL;

    vertex_append (vertex);
    return vertex;
}

void vertex_destroy (vertex_element * vertex)
{
    if (vertex == NULL) return;

    vertex_remove (vertex);
    vertex_free (vertex);   
}

int vertex_id (vertex_element * vertex)
{
    if (vertex == NULL) return -1;
    return vertex->id;
}

int vertex_size ()
{
    return size;
}

int vertex_num_edges(vertex_element * vertex)
{
    return vertex->edges;
}

void vertex_add_edge (vertex_element * vertex, edge_element * edge)
{
    if (vertex == NULL) return;
    if (edge == NULL) return;

    edge_list * element = edge_list_find (vertex, edge);
    if (element != NULL) return;

    element = edge_list_new (edge);
    if (element == NULL) return;

    edge_list_append (vertex, element);
}

void edge_contract_vertices (edge_element * edge)
{
    if (edge == NULL) return;

    vertex_element * left  = edge_left (edge);
    vertex_element * right = edge_right (edge);

    if (left == NULL) return;    
    if (right == NULL) return;

    //printf ("merge %i %i\n", vertex_id (left), vertex_id (right));
    //vertex_dump (left);
    //vertex_dump (right);

    edge_list * element = right->edges_head;
    while (element) {
        edge_change_vertex (element->edge, right, left);
        element = element->next;
    }

    left->edges_tail->next = right->edges_head;
    right->edges_head->previous = left->edges_tail;
    left->edges_tail = right->edges_tail;
    left->edges += right->edges;

    //vertex_dump (left);

    right->edges = 0;
    right->edges_head = NULL;
    right->edges_tail = NULL;
    vertex_destroy (right);

    element = left->edges_head;
    while (element) {

        if (element->edge == edge ||
            edge_left (element->edge) == edge_right (element->edge)) {
            //printf("removendo edge list\n");
            edge_list * next = element->next;
            edge_list_remove (left, element);
            edge_list_free (element);
            element = next;
            continue;
        }

        element = element->next;
    }

    //edge_remove (edge);
    edge_remove_self_loops ();

    //vertex_dump (left);
    //edge_dump ();
}

void vertex_vertices_dump ()
{
    printf("#vertices: %i\n\n", size);
    vertex_element * element = head;
    while (element) {
        vertex_dump (element);
        element = element->next;
    }
}
