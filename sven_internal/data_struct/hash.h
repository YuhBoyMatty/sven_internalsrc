// Hash Map

#ifndef HASHMAP_H
#define HASHMAP_H

#ifdef _WIN32
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Hash Map: stores any data
//-----------------------------------------------------------------------------

template <class Data, typename C = bool (*)(const Data &, const Data &), typename H = unsigned int (*)(const Data &)>
class CHash
{
public:
	typedef C CompareFunc_t;
	typedef H HashFunc_t;

	typedef struct HashElement_s
	{
		Data data;
		HashElement_s *next;

	} HashElement_t;

public:
	explicit CHash(int iBucketsCount = 32, CompareFunc_t compareFunc = 0, HashFunc_t hashFunc = 0);
	~CHash();

	Data *Find( const Data &data ) const;
	bool Insert( const Data &data, void (*pfnOnInsertFailed)(Data *pFoundData, Data *pInsertData) = NULL );
	bool Remove( const Data &data, bool (*pfnOnRemove)(Data *pRemoveData, Data *pUserData) = NULL, Data *pUserData = NULL );

	void RemoveAll();
	void Purge();

	void Clear();

	void IterateEntries( void (*pfnCallback)(Data &data) );

	// Yes, it's idiotic
	void *operator[]( int element );

	unsigned int Count() const { return m_Size; }
	unsigned int Size() const { return m_Size; }

protected:
	HashElement_t **m_Buckets = NULL;
	unsigned int m_Size = 0;

	CompareFunc_t m_CompareFunc;
	HashFunc_t m_HashFunc;
};

template <class Data, typename C, typename H>
CHash<Data, C, H>::CHash(int iBucketsCount, CompareFunc_t compareFunc, HashFunc_t hashFunc) : m_CompareFunc(compareFunc), m_HashFunc(hashFunc)
{
	if ((iBucketsCount & (iBucketsCount - 1)) == 0) // power of two
		m_Size = iBucketsCount - 1;
	else
		m_Size = iBucketsCount;

	m_Buckets = (HashElement_t **)calloc(m_Size, sizeof(HashElement_t *));
}

template <class Data, typename C, typename H>
CHash<Data, C, H>::~CHash()
{
	Purge();
}

template <class Data, typename C, typename H>
inline Data *CHash<Data, C, H>::Find(const Data &data) const
{
	unsigned int hash = m_HashFunc(data);
	int index = hash % m_Size;
	
	HashElement_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( m_CompareFunc(pCurrent->data, data) )
			return &pCurrent->data;
	
		pCurrent = pCurrent->next;
	}
	
	return NULL;
}

template <class Data, typename C, typename H>
inline bool CHash<Data, C, H>::Insert(const Data &data, void (*pfnOnInsertFailed)(Data *pFoundData, Data *pInsertData) /* = NULL */)
{
	unsigned int hash = m_HashFunc(data);
	int index = hash % m_Size;
	
	HashElement_t *pPrev = NULL;
	HashElement_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( m_CompareFunc(pCurrent->data, data) )
		{
			if (pfnOnInsertFailed)
				pfnOnInsertFailed(const_cast<Data *>(&pCurrent->data), const_cast<Data *>(&data));
	
			return false;
		}
	
		pPrev = pCurrent;
		pCurrent = pCurrent->next;
	}
	
	HashElement_t *pElement = new HashElement_t;
	
	pElement->data = data;
	pElement->next = NULL;
	
	if (pPrev)
		pPrev->next = pElement;
	else
		m_Buckets[index] = pElement;

	return true;
}

template <class Data, typename C, typename H>
inline bool CHash<Data, C, H>::Remove(const Data &data, bool (*pfnOnRemove)(Data *pRemoveData, Data *pUserData) /* = NULL */, Data *pUserData /* = NULL */)
{
	unsigned int hash = m_HashFunc(data);
	int index = hash % m_Size;
	
	HashElement_t *pPrev = NULL;
	HashElement_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( m_CompareFunc(pCurrent->data, data) )
		{
			if (pfnOnRemove && !pfnOnRemove(&pCurrent->data, pUserData))
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

template <class Data, typename C, typename H>
inline void CHash<Data, C, H>::RemoveAll()
{
	Purge();
}

template <class Data, typename C, typename H>
inline void CHash<Data, C, H>::Purge()
{
	if (m_Buckets)
	{
		for (unsigned int i = 0; i < m_Size; i++)
		{
			HashElement_t *pCurrent = m_Buckets[i];

			while (pCurrent)
			{
				HashElement_t *pRemoveEntry = pCurrent;

				pCurrent = pCurrent->next;

				delete pRemoveEntry;
			}

			m_Buckets[i] = NULL;
		}

		free((void *)m_Buckets);
		m_Buckets = NULL;
	}
}

template <class Data, typename C, typename H>
inline void CHash<Data, C, H>::Clear()
{
	if (m_Buckets)
	{
		for (unsigned int i = 0; i < m_Size; i++)
		{
			HashElement_t *pCurrent = m_Buckets[i];

			while (pCurrent)
			{
				HashElement_t *pRemoveEntry = pCurrent;

				pCurrent = pCurrent->next;

				delete pRemoveEntry;
			}

			m_Buckets[i] = NULL;
		}
	}
}

template <class Data, typename C, typename H>
inline void CHash<Data, C, H>::IterateEntries(void (*pfnCallback)(Data &data))
{
	if (m_Buckets)
	{
		for (unsigned int i = 0; i < m_Size; i++)
		{
			HashElement_t *pCurrent = m_Buckets[i];

			while (pCurrent)
			{
				pfnCallback(pCurrent->data);

				pCurrent = pCurrent->next;
			}
		}
	}
}

template <class Data, typename C, typename H>
inline void *CHash<Data, C, H>::operator[](int element)
{
	return (void *)m_Buckets[element];
}

#endif // HASHMAP_H