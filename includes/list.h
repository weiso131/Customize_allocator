#ifndef LIST_H
#define LIST_H

#include <stddef.h>

/**
 * Feature detection for 'typeof':
 * - Supported as a GNU extension in GCC/Clang.
 * - Part of C23 standard (ISO/IEC 9899:2024).
 *
 * Reference: https://gcc.gnu.org/onlinedocs/gcc/Typeof.html
 */
#if defined(__GNUC__) || defined(__clang__) ||         \
    (defined(__STDC__) && defined(__STDC_VERSION__) && \
     (__STDC_VERSION__ >= 202311L)) /* C23 ?*/
#define __LIST_HAVE_TYPEOF 1
#else
#define __LIST_HAVE_TYPEOF 0
#endif

/**
 * struct list_head - Node structure for a circular doubly-linked list
 * @next: Pointer to the next node in the list.
 * @prev: Pointer to the previous node in the list.
 *
 * Defines both the head and nodes of a circular doubly-linked list. The head's
 * @next points to the first node and @prev to the last node; in an empty list,
 * both point to the head itself. All nodes, including the head, share this
 * structure type.
 *
 * Nodes are typically embedded within a container structure holding actual
 * data, accessible via the list_entry() helper, which computes the container's
 * address from a node pointer. The list_* functions and macros provide an API
 * for manipulating this data structure efficiently.
 */
struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

/**
 * container_of() - Calculate address of structure that contains address ptr
 * @ptr: pointer to member variable
 * @type: type of the structure containing ptr
 * @member: name of the member variable in struct @type
 *
 * Return: @type pointer of structure containing ptr
 */
#ifndef container_of
#if __LIST_HAVE_TYPEOF
#define container_of(ptr, type, member)                         \
    __extension__({                                             \
        const typeof(((type *) 0)->member) *__pmember = (ptr);  \
        (type *) ((char *) __pmember - offsetof(type, member)); \
    })
#else
#define container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) - offsetof(type, member)))
#endif
#endif

/**
 * list_add - Insert a node at the beginning of a circular list
 * @node: Pointer to the list_head structure to add.
 * @head: Pointer to the list_head structure representing the list head.
 *
 * Adds the specified @node immediately after @head in a circular doubly-linked
 * list, effectively placing it at the beginning. The existing first node, if
 * any, shifts to follow @node, and the list's circular structure is maintained.
 */
static inline void list_add(struct list_head *node, struct list_head *head)
{
    struct list_head *next = head->next;

    next->prev = node;
    node->next = next;
    node->prev = head;
    head->next = node;
}

/**
 * list_entry() - Get the entry for this node
 * @node: pointer to list node
 * @type: type of the entry containing the list node
 * @member: name of the list_head member variable in struct @type
 *
 * Return: @type pointer of entry containing node
 */
#define list_entry(node, type, member) container_of(node, type, member)

/**
 * INIT_LIST_HEAD() - Initialize empty list head
 * @head: Pointer to the list_head structure to initialize.
 *
 * It sets both @next and @prev to point to the structure itself. The
 * initialization applies to either a list head or an unlinked node that is
 * not yet part of a list.
 *
 * Unlinked nodes may be passed to functions using 'list_del()' or
 * 'list_del_init()', which are safe only on initialized nodes. Applying these
 * operations to an uninitialized node results in undefined behavior, such as
 * memory corruption or crashes.
 */
static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

/**
 * list_del - Remove a node from a circular doubly-linked list
 * @node: Pointer to the list_head structure to remove.
 *
 * Removes @node from its list by updating the adjacent nodes’ pointers to
 * bypass it. The node’s memory and its containing structure, if any, are not
 * freed. After removal, @node is left unlinked and should be treated as
 * uninitialized; accessing its @next or @prev pointers is unsafe and may cause
 * undefined behavior.
 *
 * Even previously initialized but unlinked nodes become uninitialized after
 * this operation. To reintegrate @node into a list, it must be reinitialized
 * (e.g., via INIT_LIST_HEAD).
 *
 * If LIST_POISONING is enabled at build time, @next and @prev are set to
 * invalid addresses to trigger memory access faults on misuse. This feature is
 * effective only on systems that restrict access to these specific addresses.
 */
static inline void list_del(struct list_head *node)
{
    struct list_head *next = node->next;
    struct list_head *prev = node->prev;

    next->prev = prev;
    prev->next = next;

#ifdef LIST_POISONING
    node->next = NULL;
    node->prev = NULL;
#endif
}

/**
 * list_for_each - Iterate over list nodes
 * @node: list_head pointer used as iterator
 * @head: pointer to the head of the list
 *
 * The nodes and the head of the list must be kept unmodified while
 * iterating through it. Any modifications to the the list will cause undefined
 * behavior.
 */
#define list_for_each(node, head) \
    for (node = (head)->next; node != (head); node = node->next)

#endif