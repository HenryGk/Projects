// Implements a dictionary's functionality
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dictionary.h"

// Represents number of buckets in a hash table
#define N 26

// Represents a node in a hash table
typedef struct node
{
    char word[LENGTH + 1];
    struct node *next;
}
node;

// predefine destroy function
void destroy(node *node);

// Represents a hash table
node *hashtable[N];

// Hashes word to a number between 0 and 25, inclusive, based on its first letter
unsigned int hash(const char *word)
{
    return tolower(word[0]) - 'a';
}

// Loads dictionary into memory, returning true if successful else false
bool load(const char *dictionary)
{
    // Initialize hash table
    for (int i = 0; i < N; i++)
    {
        hashtable[i] = NULL;
    }

    // Open dictionary
    FILE *file = fopen(dictionary, "r");
    if (file == NULL)
    {
        unload();
        return false;
    }

    // Buffer for a word
    char word[LENGTH + 1];

    // Insert words into hash table
    while (fscanf(file, "%s", word) != EOF)  // takes dictionary file and put in word
    {
        // create a new node
        node *new_node = malloc(sizeof(node));
        if (new_node == NULL) // in case we run out of memory, unload and return
        {
            unload();
            return false;
        }
        // copy word scanned from dictionary to node->word
        strcpy(new_node->word, word);
        //get hash code index for word
        int hashIndex = hash(word);

        // isert new node in the beggining of linked list
        new_node->next = hashtable[hashIndex]; // new node should point to where head is pointing to
        hashtable[hashIndex] = new_node;    // then head will point to new node
    }

    // Close dictionary
    fclose(file);

    // Indicate success
    return true;
}

// Returns number of words in dictionary if loaded else 0 if not yet loaded
unsigned int size(void)
{
    // initialize count var as 0;
    int count = 0;

    // iterate over all Hashtble keys(N)
    for (int i = 0; i < N; i++)
    {
        // assign currentNode to first element of linked list of Hash table;;
        node *currentNode = hashtable[i];

        // iterate over all linked list nodes and increment count each time
        while (currentNode != NULL)
        {
            count++;
            // reassign currentNode to the next node
            currentNode = currentNode->next;
        }
    }

    return count;
}

// Returns true if word is in dictionary else false
bool check(const char *word)
{
    // store word length in variable
    int len = strlen(word);

    // initialize and allocate memory for variable
    char *lword = malloc(len + 1);
    if (lword == NULL) // in case we run out of memory, unload and return
    {
        unload();
        return false;
    }

    // copy lowercase of word into lword
    for (int i = 0; i <= len; i++)
    {
        lword[i] = tolower(word[i]);
    }

    // get the hash index of word
    int hashIndex = hash(word);

    // assign currentNode to first node of hashtable index hashIndex's linked list
    node *currentNode = hashtable[hashIndex];

    // iterate over linked list and check if it contains lword
    while (currentNode != NULL)
    {
        if (strcmp(currentNode->word, lword) == 0)
        {
            free(lword);
            return true;
        }
        // re-assign currentNode to next node
        currentNode = currentNode->next;
    }

    // free allocated memory
    free(lword);

    return false;
}

// Unloads dictionary from memory, returning true if successful else false
bool unload(void)
{
    // iterate over all Hashtble keys(N)
    for (int i = 0; i < N; i++)
    {
        // destroy the linked list starting at hashtable[i]
        destroy(hashtable[i]);
    }

    return true;
}

void destroy(node *node)
{
    // if we reach to end stop
    if (node == NULL)
    {
        return ;
    }

    // call self with next node
    destroy(node->next);
    // free current node
    free(node);
}