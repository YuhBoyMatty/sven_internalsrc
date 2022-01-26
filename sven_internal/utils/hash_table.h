
// Hash Table

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

typedef void (*fnAddEntryFailed)(void *pEntry, void *pValue);
typedef bool (*fnOnRemoveEntry)(void *pEntry, void *pValue);
typedef void (*fnIterateEntries)(void *pEntry);

//-----------------------------------------------------------------------------
// Hash Entry
//-----------------------------------------------------------------------------

template <class hashData>
class CHashEntry64
{
public:
	CHashEntry64() : key(0), next(NULL)
	{
		memset(&value, 0, sizeof(hashData));
	}

	uint64_t key;
	hashData value;

	CHashEntry64 *next;
};

//-----------------------------------------------------------------------------
// Hash Table
//-----------------------------------------------------------------------------

template <uint32_t tableSize = 31, class hashData = uint32_t>
class CHashTable64
{
public:
	CHashTable64();
	~CHashTable64();

	CHashEntry64<hashData> *GetEntry(uint64_t key);

	bool AddEntry(uint64_t key, hashData value, fnAddEntryFailed pfnCallback = NULL);

	bool RemoveEntry(uint64_t key, hashData value = NULL, fnOnRemoveEntry pfnCallback = NULL);

	void IterateEntries(fnIterateEntries pfnCallback);

	void RemoveAll();

	int GetTableSize() const;

private:
	uint32_t Hash(uint8_t *key, size_t length) const;

private:
	CHashEntry64<hashData> *m_table[tableSize];
};

//-----------------------------------------------------------------------------
// Implementations
//-----------------------------------------------------------------------------

template <uint32_t tableSize, class hashData>
CHashTable64<tableSize, hashData>::CHashTable64() : m_table()
{
}

template <uint32_t tableSize, class hashData>
inline CHashTable64<tableSize, hashData>::~CHashTable64()
{
	RemoveAll();
}

template <uint32_t tableSize, class hashData>
CHashEntry64<hashData> *CHashTable64<tableSize, hashData>::GetEntry(uint64_t key)
{
	uint32_t hash = Hash((uint8_t *)&key, sizeof(uint64_t));
	int index = hash % tableSize;

	CHashEntry64<hashData> *entry = m_table[index];

	while (entry)
	{
		if (entry->key == key)
			return entry;

		entry = entry->next;
	}

	return NULL;
}

template <uint32_t tableSize, class hashData>
bool CHashTable64<tableSize, hashData>::AddEntry(uint64_t key, hashData value, fnAddEntryFailed pfnCallback /* = NULL */)
{
	uint32_t hash = Hash((uint8_t *)&key, sizeof(uint64_t));
	int index = hash % tableSize;

	CHashEntry64<hashData> *prev = NULL;
	CHashEntry64<hashData> *entry = m_table[index];

	while (entry)
	{
		if (entry->key == key)
		{
			if (pfnCallback)
				pfnCallback(reinterpret_cast<void *>(entry), reinterpret_cast<void *>(&value));

			return false;
		}

		prev = entry;
		entry = entry->next;
	}

	CHashEntry64<hashData> *newEntry = new CHashEntry64<hashData>;

	newEntry->key = key;
	newEntry->value = value;

	if (m_table[index])
		prev->next = newEntry;
	else
		m_table[index] = newEntry;

	return true;
}

template <uint32_t tableSize, class hashData>
bool CHashTable64<tableSize, hashData>::RemoveEntry(uint64_t key, hashData value /* = NULL */, fnOnRemoveEntry pfnCallback /* = NULL */)
{
	uint32_t hash = Hash((uint8_t *)&key, sizeof(uint64_t));
	int index = hash % tableSize;

	CHashEntry64<hashData> *prev = NULL;
	CHashEntry64<hashData> *entry = m_table[index];

	while (entry)
	{
		if (entry->key == key)
		{
			if (pfnCallback && !pfnCallback(reinterpret_cast<void *>(entry), reinterpret_cast<void *>(&value)))
				return false;

			if (entry->next)
			{
				if (prev)
					prev->next = entry->next;
				else
					m_table[index] = entry->next;
			}
			else
			{
				if (prev)
					prev->next = NULL;
				else
					m_table[index] = NULL;
			}

			delete entry;

			return true;
		}

		prev = entry;
		entry = entry->next;
	}

	return false;
}

template <uint32_t tableSize, class hashData>
void CHashTable64<tableSize, hashData>::IterateEntries(fnIterateEntries pfnCallback)
{
	for (int i = 0; i < tableSize; ++i)
	{
		CHashEntry64<hashData> *entry = m_table[i];

		while (entry)
		{
			pfnCallback(reinterpret_cast<void *>(entry));

			entry = entry->next;
		}
	}
}

template <uint32_t tableSize, class hashData>
void CHashTable64<tableSize, hashData>::RemoveAll()
{
	for (int i = 0; i < tableSize; ++i)
	{
		CHashEntry64<hashData> *entry = m_table[i];

		while (entry)
		{
			CHashEntry64<hashData> *remove_entry = entry;

			entry = entry->next;

			delete remove_entry;
		}

		m_table[i] = NULL;
	}
}

template <uint32_t tableSize, class hashData>
inline int CHashTable64<tableSize, hashData>::GetTableSize() const
{
	return tableSize;
}

template <uint32_t tableSize, class hashData>
__forceinline uint32_t CHashTable64<tableSize, hashData>::Hash(uint8_t *key, size_t length) const
{
	// Jenkins hash function

	size_t i = 0;
	uint32_t hash = 0;

	while (i != length)
	{
		hash += key[i++];
		hash += hash << 10;
		hash ^= hash >> 6;
	}

	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;

	return hash;
}

//-----------------------------------------------------------------------------
// Hash Entry
//-----------------------------------------------------------------------------

template <class hashData>
class CHashEntry
{
public:
	CHashEntry() : key(0),  next(NULL)
	{
		memset(&value, 0, sizeof(hashData));
	}

	uint32_t key;
	hashData value;

	CHashEntry *next;
};

//-----------------------------------------------------------------------------
// Hash Table
//-----------------------------------------------------------------------------

template <uint32_t tableSize = 31, class hashData = uint32_t>
class CHashTable
{
public:
	CHashTable();
	~CHashTable();

	CHashEntry<hashData> *GetEntry(uint32_t key);

	bool AddEntry(uint32_t key, hashData value, fnAddEntryFailed pfnCallback = NULL);

	bool RemoveEntry(uint32_t key, hashData value = NULL, fnOnRemoveEntry pfnCallback = NULL);

	void IterateEntries(fnIterateEntries pfnCallback);

	void RemoveAll();

	int GetTableSize() const;

private:
	uint32_t Hash(uint8_t *key, size_t length) const;

private:
	CHashEntry<hashData> *m_table[tableSize];
};

//-----------------------------------------------------------------------------
// Implementations
//-----------------------------------------------------------------------------

template <uint32_t tableSize, class hashData>
CHashTable<tableSize, hashData>::CHashTable() : m_table()
{
}

template <uint32_t tableSize, class hashData>
inline CHashTable<tableSize, hashData>::~CHashTable()
{
	RemoveAll();
}

template <uint32_t tableSize, class hashData>
CHashEntry<hashData> *CHashTable<tableSize, hashData>::GetEntry(uint32_t key)
{
	uint32_t hash = Hash((uint8_t *)&key, sizeof(uint32_t));
	int index = hash % tableSize;

	CHashEntry<hashData> *entry = m_table[index];

	while (entry)
	{
		if (entry->key == key)
			return entry;

		entry = entry->next;
	}

	return NULL;
}

template <uint32_t tableSize, class hashData>
bool CHashTable<tableSize, hashData>::AddEntry(uint32_t key, hashData value, fnAddEntryFailed pfnCallback /* = NULL */)
{
	uint32_t hash = Hash((uint8_t *)&key, sizeof(uint32_t));
	int index = hash % tableSize;

	CHashEntry<hashData> *prev = NULL;
	CHashEntry<hashData> *entry = m_table[index];

	while (entry)
	{
		if (entry->key == key)
		{
			if (pfnCallback)
				pfnCallback(reinterpret_cast<void *>(entry), reinterpret_cast<void *>(&value));

			return false;
		}

		prev = entry;
		entry = entry->next;
	}

	CHashEntry<hashData> *newEntry = new CHashEntry<hashData>;

	newEntry->key = key;
	newEntry->value = value;

	if (m_table[index])
		prev->next = newEntry;
	else
		m_table[index] = newEntry;

	return true;
}

template <uint32_t tableSize, class hashData>
bool CHashTable<tableSize, hashData>::RemoveEntry(uint32_t key, hashData value /* = NULL */, fnOnRemoveEntry pfnCallback /* = NULL */)
{
	uint32_t hash = Hash((uint8_t *)&key, sizeof(uint32_t));
	int index = hash % tableSize;

	CHashEntry<hashData> *prev = NULL;
	CHashEntry<hashData> *entry = m_table[index];

	while (entry)
	{
		if (entry->key == key)
		{
			if (pfnCallback && !pfnCallback(reinterpret_cast<void *>(entry), reinterpret_cast<void *>(&value)))
				return false;

			if (entry->next)
			{
				if (prev)
					prev->next = entry->next;
				else
					m_table[index] = entry->next;
			}
			else
			{
				if (prev)
					prev->next = NULL;
				else
					m_table[index] = NULL;
			}

			delete entry;

			return true;
		}

		prev = entry;
		entry = entry->next;
	}

	return false;
}

template <uint32_t tableSize, class hashData>
void CHashTable<tableSize, hashData>::IterateEntries(fnIterateEntries pfnCallback)
{
	for (int i = 0; i < tableSize; ++i)
	{
		CHashEntry<hashData> *entry = m_table[i];

		while (entry)
		{
			pfnCallback(reinterpret_cast<void *>(entry));

			entry = entry->next;
		}
	}
}

template <uint32_t tableSize, class hashData>
void CHashTable<tableSize, hashData>::RemoveAll()
{
	for (int i = 0; i < tableSize; ++i)
	{
		CHashEntry<hashData> *entry = m_table[i];

		while (entry)
		{
			CHashEntry<hashData> *remove_entry = entry;

			entry = entry->next;

			delete remove_entry;
		}

		m_table[i] = NULL;
	}
}

template <uint32_t tableSize, class hashData>
inline int CHashTable<tableSize, hashData>::GetTableSize() const
{
	return tableSize;
}

template <uint32_t tableSize, class hashData>
__forceinline uint32_t CHashTable<tableSize, hashData>::Hash(uint8_t *key, size_t length) const
{
	// Jenkins hash function

	size_t i = 0;
	uint32_t hash = 0;

	while (i != length)
	{
		hash += key[i++];
		hash += hash << 10;
		hash ^= hash >> 6;
	}

	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;

	return hash;
}

//-----------------------------------------------------------------------------
// CString Hash Entry
//-----------------------------------------------------------------------------

template <class hashData>
class CHashEntryString
{
public:
	CHashEntryString() : key(NULL), next(NULL)
	{
		memset(&value, 0, sizeof(hashData));
	}

	const char *key;
	hashData value;

	CHashEntryString *next;
};

//-----------------------------------------------------------------------------
// CString Hash Table
//-----------------------------------------------------------------------------

template <uint32_t tableSize = 31, class hashData = uint32_t, bool bAllocateKeys = true>
class CHashTableString
{
public:
	CHashTableString();
	~CHashTableString();

	inline CHashEntryString<hashData> *GetEntry(const char *key);

	inline bool AddEntry(const char *key, hashData value, fnAddEntryFailed pfnCallback = NULL);

	inline bool RemoveEntry(const char *key, hashData value = NULL, fnOnRemoveEntry pfnCallback = NULL);
	
	CHashEntryString<hashData> *GetEntry(const char *key, size_t length);

	bool AddEntry(const char *key, size_t length, hashData value, fnAddEntryFailed pfnCallback = NULL);

	bool RemoveEntry(const char *key, size_t length, hashData value = NULL, fnOnRemoveEntry pfnCallback = NULL);

	void IterateEntries(fnIterateEntries pfnCallback);

	void RemoveAll();

	int GetTableSize() const;

private:
	uint32_t Hash(const char *key, size_t length) const;

private:
	CHashEntryString<hashData> *m_table[tableSize];
	bool m_bAllocateKeys = bAllocateKeys;
};

//-----------------------------------------------------------------------------
// Implementations
//-----------------------------------------------------------------------------

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
CHashTableString<tableSize, hashData, bAllocateKeys>::CHashTableString() : m_table()
{
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
inline CHashTableString<tableSize, hashData, bAllocateKeys>::~CHashTableString()
{
	RemoveAll();
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
inline CHashEntryString<hashData> *CHashTableString<tableSize, hashData, bAllocateKeys>::GetEntry(const char *key)
{
	return GetEntry(key, strlen(key));
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
inline bool CHashTableString<tableSize, hashData, bAllocateKeys>::AddEntry(const char *key, hashData value, fnAddEntryFailed pfnCallback /* = NULL */)
{
	return AddEntry(key, strlen(key), value, pfnCallback);
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
inline bool CHashTableString<tableSize, hashData, bAllocateKeys>::RemoveEntry(const char *key, hashData value /* = NULL */, fnOnRemoveEntry pfnCallback /* = NULL */)
{
	return RemoveEntry(key, strlen(key), value, pfnCallback);
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
CHashEntryString<hashData> *CHashTableString<tableSize, hashData, bAllocateKeys>::GetEntry(const char *key, size_t length)
{
	uint32_t hash = Hash(key, length);
	int index = hash % tableSize;

	CHashEntryString<hashData> *entry = m_table[index];

	while (entry)
	{
		if (!strcmp(entry->key, key))
			return entry;

		entry = entry->next;
	}

	return NULL;
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
bool CHashTableString<tableSize, hashData, bAllocateKeys>::AddEntry(const char *key, size_t length, hashData value, fnAddEntryFailed pfnCallback /* = NULL */)
{
	uint32_t hash = Hash(key, length);
	int index = hash % tableSize;

	CHashEntryString<hashData> *prev = NULL;
	CHashEntryString<hashData> *entry = m_table[index];

	while (entry)
	{
		if (!strcmp(entry->key, key))
		{
			if (pfnCallback)
				pfnCallback(reinterpret_cast<void *>(entry), reinterpret_cast<void *>(&value));

			return false;
		}

		prev = entry;
		entry = entry->next;
	}

	CHashEntryString<hashData> *newEntry = new CHashEntryString<hashData>;

	newEntry->key = bAllocateKeys ? strdup(key) : key;
	newEntry->value = value;

	if (m_table[index])
		prev->next = newEntry;
	else
		m_table[index] = newEntry;

	return true;
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
bool CHashTableString<tableSize, hashData, bAllocateKeys>::RemoveEntry(const char *key, size_t length, hashData value /* = NULL */, fnOnRemoveEntry pfnCallback /* = NULL */)
{
	uint32_t hash = Hash(key, length);
	int index = hash % tableSize;

	CHashEntryString<hashData> *prev = NULL;
	CHashEntryString<hashData> *entry = m_table[index];

	while (entry)
	{
		if (!strcmp(entry->key, key))
		{
			if (pfnCallback && !pfnCallback(reinterpret_cast<void *>(entry), reinterpret_cast<void *>(&value)))
				return false;

			if (entry->next)
			{
				if (prev)
					prev->next = entry->next;
				else
					m_table[index] = entry->next;
			}
			else
			{
				if (prev)
					prev->next = NULL;
				else
					m_table[index] = NULL;
			}

			if (bAllocateKeys)
				free((void *)entry->key);

			delete entry;

			return true;
		}

		prev = entry;
		entry = entry->next;
	}

	return false;
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
void CHashTableString<tableSize, hashData, bAllocateKeys>::IterateEntries(fnIterateEntries pfnCallback)
{
	for (int i = 0; i < tableSize; ++i)
	{
		CHashEntryString<hashData> *entry = m_table[i];

		while (entry)
		{
			pfnCallback(reinterpret_cast<void *>(entry));

			entry = entry->next;
		}
	}
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
void CHashTableString<tableSize, hashData, bAllocateKeys>::RemoveAll()
{
	for (int i = 0; i < tableSize; ++i)
	{
		CHashEntryString<hashData> *entry = m_table[i];

		while (entry)
		{
			CHashEntryString<hashData> *remove_entry = entry;

			entry = entry->next;

			if (bAllocateKeys)
				free((void *)remove_entry->key);

			delete remove_entry;
		}

		m_table[i] = NULL;
	}
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
inline int CHashTableString<tableSize, hashData, bAllocateKeys>::GetTableSize() const
{
	return tableSize;
}

template <uint32_t tableSize, class hashData, bool bAllocateKeys>
__forceinline uint32_t CHashTableString<tableSize, hashData, bAllocateKeys>::Hash(const char *key, size_t length) const
{
	size_t i = 0;
	uint32_t hash = 0;

	while (i != length)
	{
		hash += key[i++];
		hash += hash << 10;
		hash ^= hash >> 6;
	}

	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;

	return hash;
}