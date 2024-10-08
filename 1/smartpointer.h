#include <unistd.h>
#include <utility>

template <typename T>
class SmartPointer {
public:
    T* ptr;           
	size_t* refCount;
    SmartPointer() : ptr(nullptr), refCount(new size_t(0)) {}

    SmartPointer(T* p) : ptr(p), refCount(new size_t(1)) {}

    SmartPointer(const SmartPointer& other) : ptr(other.ptr), refCount(other.refCount) {
        (*refCount)++;
    }

	template <typename U>
    SmartPointer(const SmartPointer<U>& other) 
        : ptr(static_cast<T*>(other.ptr)), refCount(other.refCount) {
        (*refCount)++;
    }

    SmartPointer& operator=(const SmartPointer& other) {
        if (this != &other) {
            if (--(*refCount) == 0 || ptr == nullptr) {
                delete ptr;
                delete refCount;
            }
            ptr = other.ptr;
            refCount = other.refCount;
            (*refCount)++;
        }
        return *this;
    }

    ~SmartPointer() {
        if (--(*refCount) == 0) {
            delete ptr;
			delete refCount;
        }
    }

    T& operator*() const {
        return *ptr;
    }

    T* operator->() const {
        return ptr;
    }

	size_t* get_count(){
		return refCount;
	}

    void reset(T* p = nullptr) {
        if (--(*refCount) == 0 || p == nullptr) {
            delete ptr;
            delete refCount;
        }
        ptr = p;
		if (p == nullptr) refCount = new size_t(0);
		else refCount = new size_t(1);
    }

    void swap(SmartPointer& other) {
        std::swap(ptr, other.ptr);
        std::swap(refCount, other.refCount);
    }

    T* get() const {
        return ptr;
    }

    bool operator==(const SmartPointer& other) const {
        return ptr == other.ptr;
    }

    bool operator!=(const SmartPointer& other) const {
        return ptr != other.ptr;
    }

};
