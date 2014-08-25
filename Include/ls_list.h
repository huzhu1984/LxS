#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"

/*
 * Function to process a node's data.
 * pData: Data attached to node.
 */
typedef LS_VOID *(*LIST_NODE_PROC_FUNC_F)(LS_VOID* pData);

typedef struct CLASS_LIST_T
{
    /*
     * Append a node to the tail of list.
     * pData[in]: Data to append.
     * Return: LS_OK or LS_ERR.
     */
    LS_S32 (*append)(struct CLASS_LIST_T *pThis, LS_VOID *pData);

    /*
     * Set iterator to the start of list.
     */
    LS_VOID (*startIter)(struct CLASS_LIST_T *pThis);

    /*
     * Set iterator to the end of list.
     */
    LS_VOID (*endIter)(struct CLASS_LIST_T *pThis);

    /*
     * Set iterator to the next node.
     */
    LS_VOID (*nextIter)(struct CLASS_LIST_T *pThis);

    /*
     * Get node pointed by current iterator.
     * Return: Data in node.
     */
    LS_VOID *(*getIter)(struct CLASS_LIST_T *pThis);

    /*
     * Get node pointed by current iterator, iterator moves to next.
     * Return: Data in node.
     */
    LS_VOID *(*getIterNext)(struct CLASS_LIST_T *pThis);

    /*
     * Insert a node to where the current iterator points.
     * pData[in]: Data to insert.
     * Return: LS_OK or LS_ERR.
     */
    LS_S32 (*insertIter)(struct CLASS_LIST_T *pThis, LS_VOID *pData);

    /*
     * Remove a node to where the current iterator points, iterator moves to next.
     * Return: LS_OK or LS_ERR.
     */
    LS_S32 (*removeIter)(struct CLASS_LIST_T *pThis);

    /*
     * Call [pfProc] to process each node's data. Iterator will no be changed.
     * pfProc[in]: Process function.
     */
    LS_VOID (*forEach)(struct CLASS_LIST_T *pThis, LIST_NODE_PROC_FUNC_F pfProc);

    LS_VOID (*del)(struct CLASS_LIST_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_LIST_T;

CLASS_LIST_T *new_CLASS_LIST_T();

#ifdef __cplusplus
}
#endif

#endif // LIST_H
