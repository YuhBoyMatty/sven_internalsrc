// Hash Table

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Hash Table: key (only integer type) - value (any data)
//-----------------------------------------------------------------------------

template <typename KeyT, class ValueT>
class CHashTable
{
public:
	typedef struct HashPair_s
	{
		KeyT key;
		ValueT value;
		HashPair_s *next;

	} HashPair_t;

public:
	CHashTable(int iBucketsCount);
	~CHashTable();

	ValueT *Find( const KeyT key ) const;
	bool Insert( const KeyT key, const ValueT &value, void (*pfnOnInsertFailed)(ValueT *pFoundValue, ValueT *pInsertValue) = NULL );
	bool Remove( const KeyT key, bool (*pfnOnRemove)(ValueT *pRemoveValue, ValueT *pUserValue) = NULL, ValueT *pUserValue = NULL );

	void RemoveAll();
	void Purge();

	void Clear();

	void IterateEntries( void (*pfnCallback)(const KeyT key, ValueT &value) );

	unsigned int Count() const { return m_Size; }
	unsigned int Size() const { return m_Size; }

private:
	unsigned int HashKey(const KeyT key) const;

protected:
	HashPair_t **m_Buckets = NULL;
	unsigned int m_Size = 0;
};

template <typename KeyT, class ValueT>
CHashTable<KeyT, ValueT>::CHashTable(int iBucketsCount)
{
	if ((iBucketsCount & (iBucketsCount - 1)) == 0) // power of two
		m_Size = iBucketsCount - 1;
	else
		m_Size = iBucketsCount;

	m_Buckets = (HashPair_t **)calloc(m_Size, sizeof(HashPair_t *));
}

template <typename KeyT, class ValueT>
CHashTable<KeyT, ValueT>::~CHashTable()
{
	Purge();
}

template <typename KeyT, class ValueT>
inline ValueT *CHashTable<KeyT, ValueT>::Find(const KeyT key) const
{
	unsigned int hash = HashKey(key);
	int index = hash % m_Size;
	
	HashPair_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( pCurrent->key == key )
			return &pCurrent->value;
	
		pCurrent = pCurrent->next;
	}
	
	return NULL;
}

template <typename KeyT, class ValueT>
inline bool CHashTable<KeyT, ValueT>::Insert(const KeyT key, const ValueT &value, void (*pfnOnInsertFailed)(ValueT *pFoundValue, ValueT *pInsertValue) /* = NULL */)
{
	unsigned int hash = HashKey(key);
	int index = hash % m_Size;
	
	HashPair_t *pPrev = NULL;
	HashPair_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( pCurrent->key == key )
		{
			if (pfnOnInsertFailed)
				pfnOnInsertFailed(const_cast<ValueT *>(&pCurrent->value), const_cast<ValueT *>(&value));
	
			return false;
		}
	
		pPrev = pCurrent;
		pCurrent = pCurrent->next;
	}
	
	HashPair_t *pPair = new HashPair_t;
	
	pPair->key = key;
	pPair->value = value;
	pPair->next = NULL;
	
	if (pPrev)
		pPrev->next = pPair;
	else
		m_Buckets[index] = pPair;

	return true;
}

template <typename KeyT, class ValueT>
inline bool CHashTable<KeyT, ValueT>::Remove(const KeyT key, bool (*pfnOnRemove)(ValueT *pRemoveValue, ValueT *pUserValue) /* = NULL */, ValueT *pUserValue /* = NULL */)
{
	unsigned int hash = HashKey(key);
	int index = hash % m_Size;
	
	HashPair_t *pPrev = NULL;
	HashPair_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( pCurrent->key == key )
		{
			if (pfnOnRemove && !pfnOnRemove(&pCurrent->value, pUserValue))
				return false;
	
			if (pCurrent->next)
			{
				if (pPrev)
					pPrev->next = pCurrent->next;
				else
					m_Buckets[index] = pCurrent->next;
			}
			else
			{
				if (pPrev)
					pPrev->next = NULL;
				else
					m_Buckets[index] = NULL;
			}
	
			delete pCurrent;
	
			return true;
		}
	
		pPrev = pCurrent;
		pCurrent = pCurrent->next;
	}
	
	return false;
}

template <typename KeyT, class ValueT>
inline void CHashTable<KeyT, ValueT>::RemoveAll()
{
	Purge();
}

template <typename KeyT, class ValueT>
inline void CHashTable<KeyT, ValueT>::Purge()
{
	if (m_Buckets)
	{
		for (unsigned int i = 0; i < m_Size; i++)
		{
			HashPair_t *pCurrent = m_Buckets[i];

			while (pCurrent)
			{
				HashPair_t *pRemovePair = pCurrent;

				pCurrent = pCurrent->next;

				delete pRemovePair;
			}

			m_Buckets[i] = NULL;
		}

		free((void *)m_Buckets);
	}
}

template <typename KeyT, class ValueT>
inline void CHashTable<KeyT, ValueT>::Clear()
{
	if (m_Buckets)
	{
		for (unsigned int i = 0; i < m_Size; i++)
		{
			HashPair_t *pCurrent = m_Buckets[i];

			while (pCurrent)
			{
				HashPair_t *pRemovePair = pCurrent;

				pCurrent = pCurrent->next;

				delete pRemovePair;
			}

			m_Buckets[i] = NULL;
		}
	}
}

template <typename KeyT, class ValueT>
inline void CHashTable<KeyT, ValueT>::IterateEntries(void (*pfnCallback)(const KeyT key, ValueT &value))
{
	if (m_Buckets)
	{
		for (unsigned int i = 0; i < m_Size; i++)
		{
			HashPair_t *pCurrent = m_Buckets[i];

			while (pCurrent)
			{
				pfnCallback(pCurrent->key, pCurrent->value);

				pCurrent = pCurrent->next;
			}
		}
	}
}

template <typename KeyT, class ValueT>
__forceinline unsigned int CHashTable<KeyT, ValueT>::HashKey(const KeyT key) const
{
	// Jenkins hash function
	unsigned int i = 0;
	unsigned int hash = 0;
	unsigned char *pKey = (unsigned char *)& key;

	while (i != sizeof(KeyT))
	{
		hash += pKey[i++];
		hash += hash << 10;
		hash ^= hash >> 6;
	}

	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;

	return hash;
}