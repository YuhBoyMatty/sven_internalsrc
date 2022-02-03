// Hash Table

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Hash Dictionary: key (string) - value (any data)
//-----------------------------------------------------------------------------

template <class T, bool bDupeStrings = false>
class CHashDict
{
public:
	typedef struct HashPair_s
	{
		const char *key;
		T value;
		HashPair_s *next;

	} HashPair_t;

public:
	CHashDict(int iBucketsCount);
	~CHashDict();

	T *Find( const char *pszKey ) const;
	bool Insert( const char *pszKey, const T &value, void (*pfnOnInsertFailed)(T *pFoundValue, T *pInsertValue) = NULL );
	bool Remove( const char *pszKey, bool (*pfnOnRemove)(T *pRemoveValue, T *pUserValue) = NULL, T *pUserValue = NULL );

	void RemoveAll();
	void Purge();

	void Clear();

	void IterateEntries( void (*pfnCallback)(const char *pszKey, T &value) );

	unsigned int Count() const { return m_Size; }
	unsigned int Size() const { return m_Size; }

private:
	unsigned int HashKey(const char *pszKey) const;

protected:
	HashPair_t **m_Buckets = NULL;
	unsigned int m_Size = 0;
};

template <class T, bool bDupeStrings>
CHashDict<T, bDupeStrings>::CHashDict(int iBucketsCount)
{
	if ((iBucketsCount & (iBucketsCount - 1)) == 0) // power of two
		m_Size = iBucketsCount - 1;
	else
		m_Size = iBucketsCount;

	m_Buckets = (HashPair_t **)calloc(m_Size, sizeof(HashPair_t *));
}

template <class T, bool bDupeStrings>
CHashDict<T, bDupeStrings>::~CHashDict()
{
	Purge();
}

template <class T, bool bDupeStrings>
inline T *CHashDict<T, bDupeStrings>::Find(const char *pszKey) const
{
	unsigned int hash = HashKey(pszKey);
	int index = hash % m_Size;
	
	HashPair_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( !strcmp(pCurrent->key, pszKey) )
			return &pCurrent->value;
	
		pCurrent = pCurrent->next;
	}
	
	return NULL;
}

template <class T, bool bDupeStrings>
inline bool CHashDict<T, bDupeStrings>::Insert(const char *pszKey, const T &value, void (*pfnOnInsertFailed)(T *pFoundValue, T *pInsertValue) /* = NULL */)
{
	unsigned int hash = HashKey(pszKey);
	int index = hash % m_Size;
	
	HashPair_t *pPrev = NULL;
	HashPair_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( !strcmp(pCurrent->key, pszKey) )
		{
			if (pfnOnInsertFailed)
				pfnOnInsertFailed(const_cast<T *>(&pCurrent->value), const_cast<T *>(&value));
	
			return false;
		}
	
		pPrev = pCurrent;
		pCurrent = pCurrent->next;
	}
	
	HashPair_t *pPair = new HashPair_t;
	
	if (bDupeStrings)
		pPair->key = (const char *)strdup(pszKey);
	else
		pPair->key = pszKey;

	pPair->value = value;
	pPair->next = NULL;
	
	if (pPrev)
		pPrev->next = pPair;
	else
		m_Buckets[index] = pPair;

	return true;
}

template <class T, bool bDupeStrings>
inline bool CHashDict<T, bDupeStrings>::Remove(const char *pszKey, bool (*pfnOnRemove)(T *pRemoveValue, T *pUserValue) /* = NULL */, T *pUserValue /* = NULL */)
{
	unsigned int hash = HashKey(pszKey);
	int index = hash % m_Size;
	
	HashPair_t *pPrev = NULL;
	HashPair_t *pCurrent = m_Buckets[index];
	
	while (pCurrent)
	{
		if ( !strcmp(pCurrent->key, pszKey) )
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

			if (bDupeStrings)
				free((void *)pCurrent->key);
	
			delete pCurrent;
	
			return true;
		}
	
		pPrev = pCurrent;
		pCurrent = pCurrent->next;
	}
	
	return false;
}

template <class T, bool bDupeStrings>
inline void CHashDict<T, bDupeStrings>::RemoveAll()
{
	Purge();
}

template <class T, bool bDupeStrings>
inline void CHashDict<T, bDupeStrings>::Purge()
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

				if (bDupeStrings)
					free((void *)pRemovePair->key);

				delete pRemovePair;
			}

			m_Buckets[i] = NULL;
		}

		free((void *)m_Buckets);
	}
}

template <class T, bool bDupeStrings>
inline void CHashDict<T, bDupeStrings>::Clear()
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

				if (bDupeStrings)
					free((void *)pRemovePair->key);

				delete pRemovePair;
			}

			m_Buckets[i] = NULL;
		}
	}
}

template <class T, bool bDupeStrings>
inline void CHashDict<T, bDupeStrings>::IterateEntries(void (*pfnCallback)(const char *pszKey, T &value))
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

template <class T, bool bDupeStrings>
__forceinline unsigned int CHashDict<T, bDupeStrings>::HashKey(const char *pszKey) const
{
	// Jenkins hash function
	unsigned int hash = 0;

	while (*pszKey)
	{
		hash += *pszKey;
		hash += hash << 10;
		hash ^= hash >> 6;

		++pszKey;
	}

	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;

	return hash;
}