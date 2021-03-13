#pragma once

#include "Things/Structures/Structure.h"
#include <NAS2D/Utility.h>

class StructureManager;

typedef Structure* SKey;

class StructureComponent
{
public:
	typedef int ComponentTypeID;

	// Every subclass of StructureComponent should have the following field:
	//static constexpr ComponentTypeID componentTypeID = ...

private:
	SKey mKey;

protected:
	StructureComponent(SKey key) : mKey(key) {}

public:
	virtual ~StructureComponent() {}
	Structure& structure() const { return *mKey; }
};

template<typename ComponentTy>
class ComponentRange
{
private:
	class Iterator
	{
	private:
		std::map<SKey, StructureComponent*>::iterator mIt;
	public:
		Iterator(std::map<SKey, StructureComponent*>::iterator it) : mIt(it) {}
		bool operator!= (const Iterator& rhs) const { return mIt != rhs.mIt; }
		Iterator& operator++() { ++mIt; return *this; }
		operator ComponentTy* () { return static_cast<ComponentTy*>(mIt->second); }
	};

	std::map<SKey, StructureComponent*>& mComponents;

public:
	ComponentRange(std::map<SKey, StructureComponent*>& components) : mComponents(components) {}

	Iterator begin() const { return Iterator(mComponents.begin()); }
	Iterator end() const { return Iterator(mComponents.end()); }
	size_t size() const { return mComponents.size(); }
};
template<>
class ComponentRange<Structure>
{
private:
	using Iterator = std::vector<Structure*>::iterator;

	std::vector<Structure*>& mComponents;

public:
	ComponentRange(std::vector<Structure*>& components) : mComponents(components) {}
	Iterator begin() const { return mComponents.begin(); }
	Iterator end() const { return mComponents.end(); }
	size_t size() const { return mComponents.size(); }
};

// Shorthand
template<typename T>
inline T& GetComponent(SKey s)
{
	return NAS2D::Utility<StructureManager>::get().get<T>(s);
}

// Special case if SKey is Structure*
template<>
inline Structure& GetComponent<Structure>(SKey s)
{
	return *s;
}

template<typename T>
inline T* TryGetComponent(SKey s)
{
	return NAS2D::Utility<StructureManager>::get().tryGet<T>(s);
}

// Special case if SKey is Structure*
template<>
inline Structure* TryGetComponent<Structure>(SKey s)
{
	return s;
}

template<typename T>
inline const ComponentRange<std::enable_if_t<!std::is_base_of_v<Structure,T>,T>> GetComponents()
{
	return NAS2D::Utility<StructureManager>::get().enumerate<T>();
}

// Compatibility layer. This allows updating code using a Structure subclass to the StructureComponent syntax without refactoring the structure subclass into a structure component.
template<typename StructSubclass>
class ComponentRangeEmulator
{
private:
	class Iterator
	{
	private:
		StructureList::const_iterator mIt;
	public:
		Iterator(StructureList::const_iterator it) : mIt(it) {}
		bool operator!= (const Iterator& rhs) const { return mIt != rhs.mIt; }
		Iterator& operator++() { ++mIt; return *this; }
		operator StructSubclass* () { return static_cast<StructSubclass*>(*mIt); }
	};

	const StructureList& mComponents;

public:
	ComponentRangeEmulator(const StructureList& components) : mComponents(components) {}

	Iterator begin() const { return Iterator(mComponents.begin()); }
	Iterator end() const { return Iterator(mComponents.end()); }
	size_t size() const { return mComponents.size(); }
};

template<typename T>
inline const ComponentRangeEmulator<std::enable_if_t<std::is_base_of_v<Structure, T>, T>> GetComponents()
{
	return ComponentRangeEmulator<T>(NAS2D::Utility<StructureManager>::get().structureList(T::typeStructureClass));
}
