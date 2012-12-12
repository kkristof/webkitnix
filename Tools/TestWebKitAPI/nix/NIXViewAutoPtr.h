#ifndef NIXViewAutoPtr_h
#define NIXViewAutoPtr_h

#include <NIXView.h>

class NIXViewAutoPtr {
public:
    explicit NIXViewAutoPtr(NIXView view = 0) : m_view(view) { }
    ~NIXViewAutoPtr() { NIXViewRelease(m_view); }

    NIXView get() const { return m_view; }
    bool operator!() const { return !m_view; }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef NIXView NIXViewAutoPtr::*UnspecifiedBoolType;
    operator UnspecifiedBoolType() const { return m_view ? &NIXViewAutoPtr::m_view : 0; }

    void reset(NIXView view = 0) {
        NIXView old = m_view;
        m_view = view;
        if (old)
            NIXViewRelease(old);
    }

private:
    // Disable copy.
    NIXViewAutoPtr(NIXViewAutoPtr&);
    NIXViewAutoPtr& operator=(const NIXViewAutoPtr&);

    NIXView m_view;
};

#endif // NIXViewAutoPtr_h
