#include <iostream>
#include <atomic>

/*
 * This clss used for safe reference counting.
 */
template<class T>
class SafeRefCount_t : public T
{
    private:
        mutable std::atomic<int> safeRefCount;
    protected:
        /*
         * Get the reference count
         *
         * @return
         *  Returns the reference count
         */
        virtual int RefCount(void) const
        {
            return safeRefCount;
        }
    public:
        SafeRefCount_t() : safeRefCount(0)
        { }
        /*
         * Destructor of the class
         */
        virtual ~SafeRefCount_t() {}
        /*
         * This function used to add the references
         */
        virtual void AddReference() const
        {
            safeRefCount.fetch_add(1, std::memory_order_relaxed);
            std::cout << safeRefCount << std::endl;
        }
        /*
         * This function used to remove the references, when the count reacheds zero
         * deallocate the pointer
         */
        virtual void RemoveReference() const
        {
            if (safeRefCount.fetch_sub(1, std::memory_order_release) == 1) {
                std::atomic_thread_fence(std::memory_order_acquire);
                std::cout << safeRefCount << std::endl;
                delete this;
            }
        }
};

/*
 * This class used to count the references.
 */
class IRefCount_t
{
    protected:
        virtual int RefCount(void) const = 0;
    public:
        virtual ~IRefCount_t() {};
        virtual void AddReference() const = 0;
        virtual void RemoveReference() const = 0;
};


/*
 * Add reference to the Intrusive pointer
 *
 * @param[in] p
 * An intrusive reference counting Boost-style smart pointer
 */
template<typename T> void IntrusivePtr_t_add_ref(T *p)
{
    if (p) {
        p->AddReference();
    }
}

/*
 * Remove reference of the Intrusive pointer
 *
 * @param[in] p
 * An intrusive reference counting Boost-style smart pointer
 */
template<typename T> void IntrusivePtr_t_release(T *p)
{
    if (p) {
        p->RemoveReference();
    }
}

/*
 * IntrusivePtr_t
 *
 * A smart pointer that uses intrusive reference counting.
 *
 * Relies on unqualified calls to
 *      void IntrusivePtr_t_add_ref(T * p);
 *      void IntrusivePtr_t_release(T * p);
 *
 * The object is responsible for destroying itself.
 */
template<class T> class IntrusivePtr_t
{
    private:
        typedef IntrusivePtr_t this_type;
    public:
        typedef T element_type;

        /*
         * Default Constructor of the class
         */
        IntrusivePtr_t() throw() : px(0)
        {
        }

        /*
         * Parameterized Constructor of the class
         * @param[in] p
         * Pointer to the template class object
         *
         * @param[in] add_ref
         * This is a default parameter set to true which enables reference counting
         */
        IntrusivePtr_t(T *p, bool add_ref = true) : px(p)
        {
            if (px != 0 && add_ref)
                IntrusivePtr_t_add_ref(px);
        }
        /*
         * Copy constructor of the class
         * @param[in] rhs(right hand argument, as lhs)
         * Object to be copied
         */
        IntrusivePtr_t(IntrusivePtr_t const & rhs) : px( rhs.px)
        {
            if (px != 0)
                IntrusivePtr_t_add_ref(px);
        }
        /*
         * Copy constructor of the class
         * @param[in] rhs
         * Object to be copied
         */
        template<class U> IntrusivePtr_t(IntrusivePtr_t<U> const & rhs): px(rhs.get())
        {
            if (px != 0)
                IntrusivePtr_t_add_ref(px);
        }
        /*
         * Destructor of the class
         */
        ~IntrusivePtr_t()
        {
            if (px != 0)
                IntrusivePtr_t_release(px);
        }

        /*
         * = operator overloading
         *
         * @param[in] rhs
         *  Object to be assigned
         */
        template<class U> IntrusivePtr_t & operator=(IntrusivePtr_t<U> const & rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }
        /*
         * = operator overloading
         *
         * @param[in] rhs
         *  Object to be assigned
         */
        IntrusivePtr_t & operator=(IntrusivePtr_t const & rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }
        /*
         * = operator overloading
         *
         * @param[in] rhs
         *  template class object's pointer to be assigned
         */
        IntrusivePtr_t & operator=(T * rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        /*
         * reset the pointer to NULL
         */
        void reset() throw()
        {
            this_type().swap(*this);
        }

        /*
         * reset the intrusive pointer with the pointer passed by the user
         *
         * @param[in] rhs
         *  template class object's pointer to by assigned
         */
        void reset(T *rhs)
        {
            this_type(rhs).swap(*this);
        }
        /*
         * Get the pointer
         * @return
         *  Returns the pointer to the template class object
         */
        T * get() const throw()
        {
            return px;
        }
        /*
         * Is the pointer is set to a valid object or not
         *
         * @return
         *  true - if the pointer is not NULL
         *  false - if the pointer is NULL
         */
        bool is_set() const throw()
        {
            return (0 != px);
        }

        /*
         * * operator overloading
         * @return
         *  Returns the object which intrusive pointer points to
         */
        T & operator*() const
        {
            return *px;
        }

        /*
         * -> operator overloading
         *
         *  @return
         *   Returns the intrusive pointer points to an object
         */
        T * operator->() const
        {
            return px;
        }

    private:
        /*
         * This method is used for copy and swap Idioms
         * @return
         *  Returns the intrusive pointer points to an object
         */
        void swap(IntrusivePtr_t & rhs)
        {
            T *tmp = px;
            px = rhs.px;
            rhs.px = tmp;
        }

        /*
         * @var T* px
         * Pointer to the template class object
         */
        T *px;
};

/*
 *  == operator overloading
 * @param[in] a
 *  Intrusive pointer object with template T
 * 
 * @param[in] b
 *  Intrusive pointer object with template U
 * 
 * @return
 *  True - if equal
 *  False - if not equal
 */
template<class T, class U> inline bool operator==(IntrusivePtr_t<T> const & a, IntrusivePtr_t<U> const & b)
{
    return a.get() == b.get();
}

/*
 *  != operator overloading
 * @param[in] a
 *  Intrusive pointer object with template T
 * 
 * @param[in] b
 *  Intrusive pointer object with template U
 * 
 * @return
 *  True - if not equal
 *  False - if equal
 */
template<class T, class U> inline bool operator!=(IntrusivePtr_t<T> const & a, IntrusivePtr_t<U> const & b)
{
    return a.get() != b.get();
}

/*
 *  == operator overloading
 * @param[in] a
 *  Intrusive pointer object with template T
 * 
 * @param[in] b
 *  Pointer to an object of template type U
 * 
 * @return
 *  True - if equal
 *  False - if not equal
 */
template<class T, class U> inline bool operator==(IntrusivePtr_t<T> const & a, U * b)
{
    return a.get() == b;
}

/*
 *  != operator overloading
 * @param[in] a
 *  Intrusive pointer object with template T
 * 
 * @param[in] b
 *  Pointer to an object of template type U
 * 
 * @return
 *  True - if not equal
 *  False - if equal
 */
template<class T, class U> inline bool operator!=(IntrusivePtr_t<T> const & a, U * b)
{
    return a.get() != b;
}

/*
 *  == operator overloading
 * @param[in] a
 *  Pointer to an object of template type T
 * 
 * @param[in] b
 *  Intrusive pointer object with template U
 * 
 * @return
 *  True - if equal
 *  False - if not equal
 */
template<class T, class U> inline bool operator==(T * a, IntrusivePtr_t<U> const & b)
{
    return a == b.get();
}

/*
 *  != operator overloading
 * @param[in] a
 *  Pointer to an object of template type T
 * 
 * @param[in] b
 *  Intrusive pointer object with template U
 * 
 * @return
 *  True - if not equal
 *  False - if equal
 */
template<class T, class U> inline bool operator!=(T * a, IntrusivePtr_t<U> const & b)
{
    return a != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96
// Resolve the ambiguity between our op != and the one in rel_ops
/*
 *  != operator overloading
 * @param[in] a
 *  Intrusive pointer object with template T
 * @param[in] b
 *  Intrusive pointer object with template U
 *
 * @return
 *  True - if no equal
 *  False - if equal
 */
template<class T> inline bool operator!=(IntrusivePtr_t<T> const & a, IntrusivePtr_t<T> const & b)
{
    return a.get() != b.get();
}
#endif

/*
 *  > operator overloading
 * @param[in] a
 *  Intrusive pointer object with template T
 * 
 * @param[in] b
 *  Intrusive pointer object with template T
 * 
 * @return
 *  True - if *a > *b
 *  False - if *a < *b
 */
template<class T> inline bool operator>(IntrusivePtr_t<T> const & a, IntrusivePtr_t<T> const & b)
{
    if (a.is_set() && b.is_set()) {
        return *a > *b;
    }
    return false;
}


/*
 *  swap the intrusive pointers
 * @param[in] lhs
 *  Intrusive pointer object with template T
 * 
 * @param[in] rhs
 *  Intrusive pointer object with template T
 * 
 */
template<class T> void swap(IntrusivePtr_t<T> const & lhs, IntrusivePtr_t<T> const & rhs)
{
    lhs.swap(rhs);
}

/*
 * Get the pointer to the object of template type T
 *
 * @param[in] p
 *  Intrusive pointer object with template T
 * @return
 *  Return the pointer to the object of template type T
 */
template<class T> T * get_pointer(IntrusivePtr_t<T> const & p)
{
    return p.get();
}

/*
 * wrapper to function to do the static cast of a pointer
 *
 * @param[in] p
 *  Intrusive pointer object with template U
 * @return
 *  Returns the pointer to the object of template type T
 */
template<class T, class U> IntrusivePtr_t<T> static_pointer_cast(IntrusivePtr_t<U> const &p)
{
    return static_cast<T *>(p.get());
}

/*
 * wrapper to function to do the const cast of a pointer
 *
 * @param[in] p
 *  Intrusive pointer object with template U
 * @return
 *  Returns the pointer to the object of template type T
 */
template<class T, class U> IntrusivePtr_t<T> const_pointer_cast(IntrusivePtr_t<U> const &p)
{
    return const_cast<T *>(p.get());
}

class A : public IRefCount_t{
};

int main()
{
    IntrusivePtr_t<A> aa = new SafeRefCount_t<A>();
    IntrusivePtr_t<A> bb(aa);
    return 0;
}