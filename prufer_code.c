/**
 * @date    27/12/2022
 * @file	prufer_code.c
 * @author	Simone Arcari
 *
 * @note	main file
 * @brief	questo programma genere alberi a partire dal loro prufer code
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define BUFFER_SIZE 256
#define FATHER_CODE_ROWS 2


/**
 * @typedef	tree_t
 * 
 * @brief struttura che rappresenta un albero senza tener conto del vertice radice,
 *        di fatto è un grafo aciclico minimamente connesso costruito con vertici e archi
 *
 * @param	edge_number	    number of tree's edge
 * @param	vertex_number   number of tree's vertex
 * @param   father_code     tiene traccia degli archi di un albero/grafo tramite coppie [figlio, padre] è una matrice 2x(edge_number)
*/
typedef struct tree {
    u_int32_t edge_number;
    u_int32_t vertex_number;
    u_int32_t **father_code;
} tree_t;


/**
 * @typedef	queue_t
 *
 * @brief Contains all queue properties. The structure is that of a node 
 *        in a list, but designed to be of the FIFO type only.
 *
 * @param	data	information that the node stores (it's a queue of void*)
 * @param	next	link to the next node in list
*/
typedef struct queue {
	void *data;
	struct queue *next;
}queue_t;


/**
 * @name    create_queue
 * 
 * @brief This function allocates a pointer to queue_t (* head) in the
 *        heap and returns the pointer to pointer to queue_t (** head).
 *		
 * @param 	void	No parameters.
 * 
 * @return	A double pointer to queue.
*/
queue_t **create_queue(void) {

	queue_t **head = malloc(sizeof(queue_t *));
	if(head==NULL) {
		fprintf(stderr, "ERR: [ malloc(sizeof(queue_t *)) ]\n");
	        return NULL;  
	}
	
	*head = NULL;
	return head;
}


/**
 * @name enqueue
 * @brief This function always inserts a node at the end, it's like normal
 *		  insertion in the tail for the lists.
 *		
 * @param 	head	Double pointer to queue.
 * @param 	data	Pointer to data.
 * 
 * @return 	Pointer to new node inserted.
*/
queue_t *enqueue(queue_t **head, void *data) {
	queue_t *new_tail = malloc(sizeof(queue_t));
	if(new_tail==NULL) {
		fprintf(stderr, "ERR: [ malloc(sizeof(queue_t)) ]\n");
		return NULL;
	}
	new_tail->data = data;
	new_tail->next = NULL;

	queue_t *c = *head;	// c points to first node
    queue_t *pos = NULL;

	while (c != NULL) {
		pos = c;
		c = c->next;	
	}
	
	if (pos == NULL)
		*head = new_tail;
	else
		pos->next = new_tail;
	
	return new_tail;
}


/**
 * @name    dequeue
 * 
 * @brief This function always takes a node at the head, it's like normal
 *		  remove from the head for the lists.
 *		
 * @param 	head	Double pointer to queue.
 * 
 * @return 	Pointer to data cointained, if empty return NULL.
*/
void *dequeue(queue_t **head) {

	if(head==NULL) {
		fprintf(stderr, "ERR head == NULL in function dequeue()");
		return NULL;
	}
	
	if(*head==NULL) return NULL;	// is empty
	
	void* res = (*head)->data;
    queue_t* old = *head;
	*head = (*head)->next;

    free(old);
	return res;
}


/**
 * @name element_in_queue
 * @brief Funzione che verifica se un elemento è presente in un coda
 *		
 * @param 	element	Elemento da trovare.
 * @param 	head	Double pointer to queue.
 * 
 * @return 	true se l'elemento è presente, false altrimneti
*/
bool element_in_queue(u_int32_t element, queue_t **head) {
    queue_t *current = *head;	// current points to first node
    u_int32_t cur_data;

	while (current != NULL) {   // Scorro tutti gli elementi del vettore
        cur_data = (u_int32_t)(current->data);

        if(cur_data == element) {   // Se trovo l'elemento, restituisco true
            return true;
        }

		current = current->next;	
	}

    // Se non trovo l'elemento, restituisco false
    return false;
}


/**
 * @name element_in_array
 * @brief Funzione che verifica se un elemento è presente in un vettore
 *		
 * @param 	element	Elemento da trovare.
 * @param 	array	Vettore elementi.
 * @param   len     lunghezza del vettore
 * 
 * @return 	true se l'elemento è presente, false altrimneti
*/
bool element_in_array(u_int32_t element, u_int32_t* array, size_t len) {
    for (int i = 0; i < len; i++) { // Scorro tutti gli elementi del vettore
        if (array[i] == element) {  // Se trovo l'elemento, restituisco true
            return true;
        }
    }

    // Se non trovo l'elemento, restituisco false
    return false;
}


/**
 * @name 	make_tree
 * 
 * @brief genere un albero in base al prufer code passato come input
 *
 * @param 	prufer_code vettore di uint32_t che contine il prufer code
 * @param   length      lunghezza del vettore       
 * 
 * @return	puntatore a struttura albero generato dal prufer code
 */
tree_t* make_tree(u_int32_t *prufer_code, u_int32_t length) {
    tree_t *my_tree = (tree_t*)malloc(sizeof(tree_t));
    if(my_tree == NULL) {
        fprintf(stderr, "non è stato possibile allocare memoria per l'albero\n");
        exit(EXIT_FAILURE);
    }

    my_tree->vertex_number = length + 2;    // questa formula viene dalla teoria del prufer code
    my_tree->edge_number = length + 1;      // questa formula viene indirettamente dalla teoria degli alberi (vertex_number - 1)

    size_t rows = FATHER_CODE_ROWS;
    size_t cols = my_tree->edge_number;

    u_int32_t **father_code_matrix = malloc(rows * sizeof(u_int32_t*));  // Alloco memoria per il numero di righe
    for (int i = 0; i < rows; i++) {
        father_code_matrix[i] = malloc(cols * sizeof(u_int32_t));  // Alloco memoria per il numero di colonne
    }
    my_tree->father_code = father_code_matrix;

    /*PRUFER CODE ALGORITHM*/

    queue_t **pr_code = create_queue(); // creo una coda per contenere gli elementi del prufer code

    for(int i=0; i<length; i++) {
        enqueue(pr_code, (void*)prufer_code[i]);   // inserisco nella coda gli elementi del prufer code in ordine crescente di posizione
    }

    size_t len = my_tree->vertex_number;

    u_int32_t V[len];        // vettore che deve contenere tutti i vertici possibili
    bool V_enabled[len];     // la i-esima componente indica con false se V[i] non è attualmente valido, con true se V[i] è valido
    bool V_consumed[len];    // la i-esima componente indica con true se V[i] è stato già consumato, con false se V[i] se è ancora disponibile

    for(int i=0; i<len; i++) {  // riempio V con i valori (dei vertici) da 0 a len-1 e li dichiaro non consumati
        V[i] = i;
        V_consumed[i] = false;
    }

    for(int i=0; i<len; i++) {  // imposto come validi gli elementi di V che non compaiono negli elementi del prufer code
        if(element_in_array(V[i], prufer_code, length)==true) {
            V_enabled[i] = false;
        }else {
            V_enabled[i] = true;
        }
    }

    size_t col_index = 0; // serve per tenere traccia del ciclo while i-esimo qui sotto

    while(*pr_code != NULL) {   // finchè la coda degli elimenti del prufer code non è vuota esegui il ciclo
        u_int32_t father = (u_int32_t)dequeue(pr_code); // prelevo il primo elemento della coda
        u_int32_t child;

        for(int i=0; i<len; i++) {  // prendo come child il primo elemto di V che è sia valido che non consumato
            if(V_enabled[i]==true && V_consumed[i]==false) {
                child = V[i];
                V_consumed[i] = true;
                goto exit_here;
            }
        }

        fprintf(stderr, "qualcosa è andato storto!!!\n");
        exit(EXIT_FAILURE);

exit_here:
        for(int i=0; i<len; i++) {  // imposto come validi gli elementi di V che non compaiono negli elementi del prufer code rimasti in coda
            if(element_in_queue(V[i], pr_code)==false) {
                V_enabled[i] = true;
            }
        }

        father_code_matrix[0][col_index] = child;
        father_code_matrix[1][col_index] = father;

        col_index++;
    }
    free(pr_code);

    // cercare gli ultimi due elementi rimasti non consumati nel vettore V e fare l'ultima coppia padre figlio
    int counter = 0;
    for(int i=0; i<len; i++) {
        if(V_consumed[i]==false) {

            switch(counter)
            {
            case 0:
                father_code_matrix[0][col_index] = V[i];
                break;
            
            case 1:
                father_code_matrix[1][col_index] = V[i];
                break;
            
            default:
                break;
            }

            counter++;
        }
    }

    return my_tree;
}

int main(void) {
    char *token;
    char buffer[BUFFER_SIZE];
    u_int32_t prufer_code[BUFFER_SIZE];

    printf("inserisci il prufer code separando i vari elementi con degli spazi:\n");
    fgets(buffer, BUFFER_SIZE, stdin);
    puts("");

    int i = 0;
    token = strtok(buffer, " ");
    prufer_code[i] = (u_int32_t)atoi(token);

    while (token != NULL) {
        prufer_code[i] = (u_int32_t)atoi(token);
        printf("prufer_code[%d]: %d\n", i, prufer_code[i]);

        token = strtok(NULL, " ");
        i++;
    }

    tree_t* T = make_tree(prufer_code, i);
    int rows = FATHER_CODE_ROWS;
    int cols = T->edge_number;

    for(int i=0; i<cols; i++) {
        printf("| %d |  ", T->father_code[0][i]);
    }
    puts("");

    for(int i=0; i<cols; i++) {
        printf("| %d |  ", T->father_code[1][i]);
    }
    puts("");



    
    return 0;
}