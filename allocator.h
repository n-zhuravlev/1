#include <stdexcept>
#include <string>
#include <list>
using namespace std;

struct Memory_Element {
	// Element of list with pointer to allocated memory and size of allocated memory
	int8_t *ptr;
	size_t len;	
	Memory_Element(int8_t *_ptr, size_t _len):
		ptr(_ptr),
		len(_len)
	{}
};

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class Allocator;

class Pointer {
public:
	bool if_NULL;
	// iterator on corresponding element in list
	list<Memory_Element>::iterator iter;
	Pointer(list<Memory_Element>::iterator _iter):
		iter(_iter),
		if_NULL(false)
	{}
	Pointer(): if_NULL(true) {}
    void *get() const;
};

class Allocator {
private:
	int8_t *Start_Ptr;
	size_t Mem_size;
	list<Memory_Element> Managing_List;
	
public:
    Allocator(void *base, size_t size):
		Start_Ptr((int8_t *)base),
		Mem_size(size)
	{}
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);
    void defrag();
    std::string dump() { return ""; }
};

