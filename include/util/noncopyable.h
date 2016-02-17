#ifndef UTIL_NONCOPYABLE_H_
#define UTIL_NONCOPYABLE_H_

namespace util {

/**
 * Non copyable for all classes which don't want copy
 * constructor and assignment
*/
class Noncopyable {
private:
    Noncopyable(const Noncopyable&);
    Noncopyable& operator=(const Noncopyable&);

protected:
    Noncopyable() {}
    ~Noncopyable() {}
};

} // namespace util

#endif // UTIL_NONCOPYABLE_H_
