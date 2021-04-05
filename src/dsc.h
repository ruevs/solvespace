//-----------------------------------------------------------------------------
// Data structures used frequently in the program, various kinds of vectors
// (of real numbers, not symbolic algebra stuff) and our templated lists.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#ifndef SOLVESPACE_DSC_H
#define SOLVESPACE_DSC_H

#include "solvespace.h"

#include <type_traits>
#include <vector>

/// Trait indicating which types are handle types and should get the associated operators.
/// Specialize for each handle type and inherit from std::true_type.
template<typename T>
struct IsHandleOracle : std::false_type {};

// Equality-compare any two instances of a handle type.
template<typename T>
static inline typename std::enable_if<IsHandleOracle<T>::value, bool>::type
operator==(T const &lhs, T const &rhs) {
    return lhs.v == rhs.v;
}

// Inequality-compare any two instances of a handle type.
template<typename T>
static inline typename std::enable_if<IsHandleOracle<T>::value, bool>::type
operator!=(T const &lhs, T const &rhs) {
    return !(lhs == rhs);
}

// Less-than-compare any two instances of a handle type.
template<typename T>
static inline typename std::enable_if<IsHandleOracle<T>::value, bool>::type
operator<(T const &lhs, T const &rhs) {
    return lhs.v < rhs.v;
}

class Vector;
class Vector4;
class Point2d;
class hEntity;
class hParam;

class Quaternion {
public:
    // a + (vx)*i + (vy)*j + (vz)*k
    double w, vx, vy, vz;

    static const Quaternion IDENTITY;

    static Quaternion From(double w, double vx, double vy, double vz);
    static Quaternion From(hParam w, hParam vx, hParam vy, hParam vz);
    static Quaternion From(Vector u, Vector v);
    static Quaternion From(Vector axis, double dtheta);

    Quaternion Plus(Quaternion b) const;
    Quaternion Minus(Quaternion b) const;
    Quaternion ScaledBy(double s) const;
    double Magnitude() const;
    Quaternion WithMagnitude(double s) const;

    // Call a rotation matrix [ u' v' n' ]'; this returns the first and
    // second rows, where that matrix is generated by this quaternion
    Vector RotationU() const;
    Vector RotationV() const;
    Vector RotationN() const;
    Vector Rotate(Vector p) const;

    Quaternion ToThe(double p) const;
    Quaternion Inverse() const;
    Quaternion Times(Quaternion b) const;
    Quaternion Mirror() const;
};

class Vector {
public:
    double x, y, z;

    static Vector From(double x, double y, double z);
    static Vector From(hParam x, hParam y, hParam z);
    static Vector AtIntersectionOfPlanes(Vector n1, double d1,
                                         Vector n2, double d2);
    static Vector AtIntersectionOfLines(Vector a0, Vector a1,
                                        Vector b0, Vector b1,
                                        bool *skew,
                                        double *pa=NULL, double *pb=NULL);
    static Vector AtIntersectionOfPlaneAndLine(Vector n, double d,
                                               Vector p0, Vector p1,
                                               bool *parallel);
    static Vector AtIntersectionOfPlanes(Vector na, double da,
                                         Vector nb, double db,
                                         Vector nc, double dc, bool *parallel);
    static void ClosestPointBetweenLines(Vector pa, Vector da,
                                         Vector pb, Vector db,
                                         double *ta, double *tb);

    double Element(int i) const;
    bool Equals(Vector v, double tol=LENGTH_EPS) const;
    bool EqualsExactly(Vector v) const;
    Vector Plus(Vector b) const;
    Vector Minus(Vector b) const;
    Vector Negated() const;
    Vector Cross(Vector b) const;
    double DirectionCosineWith(Vector b) const;
    double Dot(Vector b) const;
    Vector Normal(int which) const;
    Vector RotatedAbout(Vector orig, Vector axis, double theta) const;
    Vector RotatedAbout(Vector axis, double theta) const;
    Vector DotInToCsys(Vector u, Vector v, Vector n) const;
    Vector ScaleOutOfCsys(Vector u, Vector v, Vector n) const;
    double DistanceToLine(Vector p0, Vector dp) const;
    double DistanceToPlane(Vector normal, Vector origin) const;
    bool OnLineSegment(Vector a, Vector b, double tol=LENGTH_EPS) const;
    Vector ClosestPointOnLine(Vector p0, Vector deltal) const;
    double Magnitude() const;
    double MagSquared() const;
    Vector WithMagnitude(double s) const;
    Vector ScaledBy(double s) const;
    Vector ProjectInto(hEntity wrkpl) const;
    Vector ProjectVectorInto(hEntity wrkpl) const;
    double DivProjected(Vector delta) const;
    Vector ClosestOrtho() const;
    void MakeMaxMin(Vector *maxv, Vector *minv) const;
    Vector ClampWithin(double minv, double maxv) const;
    static bool BoundingBoxesDisjoint(Vector amax, Vector amin,
                                      Vector bmax, Vector bmin);
    static bool BoundingBoxIntersectsLine(Vector amax, Vector amin,
                                          Vector p0, Vector p1, bool asSegment);
    bool OutsideAndNotOn(Vector maxv, Vector minv) const;
    Vector InPerspective(Vector u, Vector v, Vector n,
                         Vector origin, double cameraTan) const;
    Point2d Project2d(Vector u, Vector v) const;
    Point2d ProjectXy() const;
    Vector4 Project4d() const;
};

inline double Vector::Element(int i) const {
    switch (i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: ssassert(false, "Unexpected vector element index");
    }
}

inline bool Vector::Equals(Vector v, double tol) const {
    // Quick axis-aligned tests before going further
    const Vector dv = this->Minus(v);
    if (fabs(dv.x) > tol) return false;
    if (fabs(dv.y) > tol) return false;
    if (fabs(dv.z) > tol) return false;

    return dv.MagSquared() < tol*tol;
}

inline Vector Vector::From(double x, double y, double z) {
    return {x, y, z};
}

inline Vector Vector::Plus(Vector b) const {
    return {x + b.x, y + b.y, z + b.z};
}

inline Vector Vector::Minus(Vector b) const {
    return {x - b.x, y - b.y, z - b.z};
}

inline Vector Vector::Negated() const {
    return {-x, -y, -z};
}

inline Vector Vector::Cross(Vector b) const {
    return {-(z * b.y) + (y * b.z), (z * b.x) - (x * b.z), -(y * b.x) + (x * b.y)};
}

inline double Vector::Dot(Vector b) const {
    return (x * b.x + y * b.y + z * b.z);
}

inline double Vector::MagSquared() const {
    return x * x + y * y + z * z;
}

inline double Vector::Magnitude() const {
    return sqrt(x * x + y * y + z * z);
}

inline Vector Vector::ScaledBy(const double v) const {
    return {x * v, y * v, z * v};
}

inline void Vector::MakeMaxMin(Vector *maxv, Vector *minv) const {
    maxv->x = max(maxv->x, x);
    maxv->y = max(maxv->y, y);
    maxv->z = max(maxv->z, z);

    minv->x = min(minv->x, x);
    minv->y = min(minv->y, y);
    minv->z = min(minv->z, z);
}

struct VectorHash {
    size_t operator()(const Vector &v) const;
};

struct VectorPred {
    bool operator()(Vector a, Vector b) const;
};

class Vector4 {
public:
    double w, x, y, z;

    static Vector4 From(double w, double x, double y, double z);
    static Vector4 From(double w, Vector v3);
    static Vector4 Blend(Vector4 a, Vector4 b, double t);

    Vector4 Plus(Vector4 b) const;
    Vector4 Minus(Vector4 b) const;
    Vector4 ScaledBy(double s) const;
    Vector PerspectiveProject() const;
};

class Point2d {
public:
    double x, y;

    static Point2d From(double x, double y);
    static Point2d FromPolar(double r, double a);

    Point2d Plus(const Point2d &b) const;
    Point2d Minus(const Point2d &b) const;
    Point2d ScaledBy(double s) const;
    double DivProjected(Point2d delta) const;
    double Dot(Point2d p) const;
    double DistanceTo(const Point2d &p) const;
    double DistanceToLine(const Point2d &p0, const Point2d &dp, bool asSegment) const;
    double DistanceToLineSigned(const Point2d &p0, const Point2d &dp, bool asSegment) const;
    double Angle() const;
    double AngleTo(const Point2d &p) const;
    double Magnitude() const;
    double MagSquared() const;
    Point2d WithMagnitude(double v) const;
    Point2d Normal() const;
    bool Equals(Point2d v, double tol=LENGTH_EPS) const;
};

// A simple list
template<class T>
class List {
    T *elem            = nullptr;
    int elemsAllocated = 0;

public:
    int  n = 0;

    bool IsEmpty() const { return n == 0; }

    void ReserveMore(int howMuch) {
        if(n + howMuch > elemsAllocated) {
            elemsAllocated = n + howMuch;
            T *newElem = (T *)::operator new[]((size_t)elemsAllocated*sizeof(T));
            for(int i = 0; i < n; i++) {
                new(&newElem[i]) T(std::move(elem[i]));
                elem[i].~T();
            }
            ::operator delete[](elem);
            elem = newElem;
        }
    }

    void AllocForOneMore() {
        if(n >= elemsAllocated) {
            ReserveMore((elemsAllocated + 32)*2 - n);
        }
    }

    void Add(const T *t) {
        AllocForOneMore();
        new(&elem[n++]) T(*t);
    }

    void AddToBeginning(const T *t) {
        AllocForOneMore();
        new(&elem[n]) T();
        std::move_backward(elem, elem + 1, elem + n + 1);
        elem[0] = *t;
        n++;
    }

    T *First() {
        return IsEmpty() ? nullptr : &(elem[0]);
    }
    const T *First() const {
        return IsEmpty() ? nullptr : &(elem[0]);
    }

    T *Last() { return IsEmpty() ? nullptr : &(elem[n - 1]); }
    const T *Last() const { return IsEmpty() ? nullptr : &(elem[n - 1]); }

    T *NextAfter(T *prev) {
        if(IsEmpty() || !prev) return NULL;
        if(prev - First() == (n - 1)) return NULL;
        return prev + 1;
    }
    const T *NextAfter(const T *prev) const {
        if(IsEmpty() || !prev) return NULL;
        if(prev - First() == (n - 1)) return NULL;
        return prev + 1;
    }

    T &Get(size_t i) { return elem[i]; }
    T const &Get(size_t i) const { return elem[i]; }
    T &operator[](size_t i) { return Get(i); }
    T const &operator[](size_t i) const { return Get(i); }

    T *begin() { return IsEmpty() ? nullptr : &elem[0]; }
    T *end() { return IsEmpty() ? nullptr : &elem[n]; }
    const T *begin() const { return IsEmpty() ? nullptr : &elem[0]; }
    const T *end() const { return IsEmpty() ? nullptr : &elem[n]; }
    const T *cbegin() const { return begin(); }
    const T *cend() const { return end(); }

    void ClearTags() {
        for(auto & elt : *this) {
            elt.tag = 0;
        }
    }

    void Clear() {
        for(int i = 0; i < n; i++)
            elem[i].~T();
        if(elem) ::operator delete[](elem);
        elem = NULL;
        n = elemsAllocated = 0;
    }

    void RemoveTagged() {
        auto newEnd = std::remove_if(this->begin(), this->end(), [](T &t) {
            if(t.tag) {
                return true;
            }
            return false;
        });
        auto oldEnd = this->end();
        n = newEnd - begin();
        if (newEnd != nullptr && oldEnd != nullptr) {
            while(newEnd != oldEnd) {
                newEnd->~T();
                ++newEnd;
            }
        }
        // and elemsAllocated is untouched, because we didn't resize
    }

    void RemoveLast(int cnt) {
        ssassert(n >= cnt, "Removing more elements than the list contains");
        for(int i = n - cnt; i < n; i++)
            elem[i].~T();
        n -= cnt;
        // and elemsAllocated is untouched, same as in RemoveTagged
    }

    void Reverse() {
        int i;
        for(i = 0; i < (n/2); i++) {
            swap(elem[i], elem[(n-1)-i]);
        }
    }
};

template<class T, class H> class IdList;

// Comparison functor used by IdList and related classes
template <class T, class H>
struct CompareId {

    CompareId(const IdList<T, H> *list) {
        idlist = list;
    }

    bool operator()(int lhs, T const& rhs) const {
        return idlist->elemstore[lhs].h.v < rhs.h.v;
    }
    bool operator()(int lhs, H rhs) const {
        return idlist->elemstore[lhs].h.v < rhs.v;
    }
    bool operator()(T *lhs, int rhs) const {
        return lhs->h.v < idlist->elemstore[rhs].h.v;
    }

private:
    const IdList<T, H> *idlist;
};

// A list, where each element has an integer identifier. The list is kept
// sorted by that identifier, and items can be looked up in log n time by
// id.
template <class T, class H>
class IdList {
    std::vector<T> elemstore;
    std::vector<int> elemidx;
    std::vector<int> freelist;
public:
    int n = 0;  // PAR@@@@@ make this private to see all interesting and suspicious places in SoveSpace ;-)

    friend struct CompareId<T, H>;
    using Compare = CompareId<T, H>;

    struct iterator {
        typedef std::random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef int difference_type;
        typedef T *pointer;
        typedef T &reference;

    public:
        T &operator*() const noexcept { return *elem; }
        const T *operator->() const noexcept { return elem; }

        T &operator=(const T &e) const noexcept {
            *elem = e;
            return *this;
        }
        T &operator=(const H h) const noexcept {
            elem->h = e;
            return *this;
        }

        bool operator==(const iterator &p) const { return p.position == position; }
        bool operator<(const iterator &p) const { return position < p.position; }
        bool operator!=(const iterator &p) const { return !operator==(p); }
        bool operator>(const iterator &p) const { return operator!=(p) && !operator<(p); }
        bool operator>=(const iterator &p) const { return !operator<(p); }
        bool operator<=(const iterator &p) const { return !operator>(p); }

        iterator &operator++() {
            ++position;
            if(position >= (int)list->elemidx.size()) {
                elem = nullptr; // PAR@@@@ Remove just debugging
            } else if(0 <= position) {
                elem = &(list->elemstore[list->elemidx[position]]);
            }
            return *this;
        }
        iterator &operator--() {
            --position;
            if(0 > position) {
                elem = nullptr; // PAR@@@@ Remove just debugging
            } else if(position < list->elemidx.size()) {
                elem = &(list->elemstore[list->elemidx[position]]);
            }
            return *this;
        }

        iterator(IdList<T, H> *l) : list(l), position(0) {
            if(list) {
                if(list->elemstore.size() && list->elemidx.size()) {
                    elem = &(list->elemstore[list->elemidx[position]]);
                }
            }
        };
        iterator(const iterator &iter)
            : list(iter.list), position(iter.position), elem(iter.elem){};
        iterator(IdList<T, H> *l, int pos) : list(l), position(pos) {
            if(position >= (int)list->elemidx.size()) {
                elem = nullptr;
            } else if(0 <= position) {
                elem = &((list->elemstore)[list->elemidx[position]]);
            }
        };

    private:
        int position;
        T *elem;
        IdList<T, H> *list;
    };


    bool IsEmpty() const {
        return n == 0;
    }

    uint32_t MaximumId() {
        if(IsEmpty()) {
            return 0;
        } else {
            return elemstore[elemidx.back()].h.v;
        }
    }

    H AddAndAssignId(T *t) {
        t->h.v = (MaximumId() + 1);

        // Add at the end of the list.
        elemstore.push_back(*t);
        elemidx.push_back(elemstore.size()-1);
        ++n;

        return t->h;
    }

    int LowerBoundIndex(T const& t) {
        if(IsEmpty()) {
            return 0;
        }
        auto it  = std::lower_bound(elemptr.begin(), elemptr.end(), t, Compare(this));
        auto idx = std::distance(elemidx.begin(), it);
        auto i = static_cast<int>(idx);
        return i;
    }

    void ReserveMore(int howMuch) {
        elemstore.reserve(elemstore.size() + howMuch);
        elemidx.reserve(elemidx.size() + howMuch);
        //        freelist.reserve(freelist.size() + howMuch);    // PAR@@@@ maybe we should - not much more RAM
    }

    void Add(T *t) {
        // Look to see if we already have something with the same handle value.
        ssassert(FindByIdNoOops(t->h) == nullptr, "Handle isn't unique");

        // Find out where the added element should be.
        auto pos = std::lower_bound(elemidx.begin(), elemidx.end(), *t, Compare(this));

        if(freelist.empty()) { // Add a new element to the store
            elemstore.push_back(*t);
            // Insert a pointer to the element at the correct position
            if(elemidx.empty()) {
                // The list is empty so pos, begin and end are all null.
                // insert does not work in this case.
                elemidx.push_back(elemstore.size()-1);
            } else {
                elemidx.insert(pos, elemstore.size() - 1);
            }
        } else { // Use the last element from the freelist
            // Insert an index to the element at the correct position
            elemidx.insert(pos, freelist.back());
            // Remove the element from the freelist
            freelist.pop_back();

            // Copy-construct to the element storage.
            elemstore[*pos] = T(*t);
            //            *elemptr[pos] = *t;   // PAR@@@@@@ maybe this?
        }

        ++n;
    }

    T *FindById(H h) {
        T *t = FindByIdNoOops(h);
        ssassert(t != nullptr, "Cannot find handle");
        return t;
    }

    T *FindByIdNoOops(H h) {
        if(IsEmpty()) {
            return nullptr;
        }
        auto it = std::lower_bound(elemidx.begin(), elemidx.end(), h, Compare(this));
        if(it == elemidx.end()) {
            return nullptr;
        } else {
            if(elemstore[*it].h.v != h.v) {
                return nullptr;
            }
            return &elemstore[*it];
        }
    }

    T &Get(size_t i) { return elemstore[elemidx[i]]; }
    T const &Get(size_t i) const { return elemstore[elemidx[i]]; }
    T &operator[](size_t i) { return Get(i); }
    T const &operator[](size_t i) const { return Get(i); }

    iterator begin() { return IsEmpty() ? nullptr : iterator(this); }
    iterator end() { return IsEmpty() ? nullptr : iterator(this, elemidx.size()); }
    const iterator begin() const { return IsEmpty() ? nullptr : iterator(this); }
    const iterator end() const { return IsEmpty() ? nullptr : iterator(this, elemidx.size()); }
    const iterator cbegin() const { return begin(); }
    const iterator cend() const { return end(); }

    void ClearTags() {
        for(auto &elt : *this) { elt.tag = 0; }
    }

    void Tag(H h, int tag) {
        auto it = FindByIdNoOops(h);
        if (it != nullptr) {
            it->tag = tag;
        }
    }

    void RemoveTagged() {
        int src, dest;
        dest = 0;
        for(src = 0; src < n; src++) {
            if(elemstore[elemidx[src]].tag) {
                // this item should be deleted
                elemstore[elemidx[src]].Clear();
//                elemstore[elemidx[src]].~T(); // Clear below calls the destructors
                freelist.push_back(elemidx[src]);
                elemidx[src] = 0xDEADBEEF; // PAR@@@@@ just for debugging, not needed, remove later
            } else {
                if(src != dest) {
                    elemidx[dest] = elemidx[src];
                }
                dest++;
            }
        }
        n = dest;
        elemidx.resize(n);  // Clear left over elements at the end.
    }
    void RemoveById(H h) {  // PAR@@@@@ this can be optimized
        ClearTags();
        FindById(h)->tag = 1;
        RemoveTagged();
    }

    void MoveSelfInto(IdList<T,H> *l) {
        l->Clear();
        std::swap(l->elemstore, elemstore);
        std::swap(l->elemidx, elemidx);
        std::swap(l->freelist, freelist);
        std::swap(l->n, n);
    }

    void DeepCopyInto(IdList<T,H> *l) {
        l->Clear();

        for(auto const &it : elemstore) {
            l->elemstore.push_back(it);
        }

        for(auto const &it : elemidx) {
            l->elemidx.push_back(it);
        }

        l->n = n;
    }

    void Clear() {
        for(auto &it : elemidx) {
            elemstore[it].Clear();
//            elemstore[it].~T(); // clear below calls the destructors
        }
        freelist.clear();
        elemidx.clear();
        elemstore.clear();
        n = 0;
    }

};

class BandedMatrix {
public:
    enum {
        MAX_UNKNOWNS   = 16,
        RIGHT_OF_DIAG  = 1,
        LEFT_OF_DIAG   = 2
    };

    double A[MAX_UNKNOWNS][MAX_UNKNOWNS];
    double B[MAX_UNKNOWNS];
    double X[MAX_UNKNOWNS];
    int n;

    void Solve();
};

#define RGBi(r, g, b) RgbaColor::From((r), (g), (b))
#define RGBf(r, g, b) RgbaColor::FromFloat((float)(r), (float)(g), (float)(b))

// Note: sizeof(class RgbaColor) should be exactly 4
//
class RgbaColor {
public:
    uint8_t red, green, blue, alpha;

    float redF()   const { return (float)red   / 255.0f; }
    float greenF() const { return (float)green / 255.0f; }
    float blueF()  const { return (float)blue  / 255.0f; }
    float alphaF() const { return (float)alpha / 255.0f; }

    bool IsEmpty() const { return alpha == 0; }

    bool Equals(RgbaColor c) const {
        return
            c.red   == red   &&
            c.green == green &&
            c.blue  == blue  &&
            c.alpha == alpha;
    }

    RgbaColor WithAlpha(uint8_t newAlpha) const {
        RgbaColor color = *this;
        color.alpha = newAlpha;
        return color;
    }

    uint32_t ToPackedIntBGRA() const {
        return
            blue |
            (uint32_t)(green << 8) |
            (uint32_t)(red << 16) |
            (uint32_t)((255 - alpha) << 24);
    }

    uint32_t ToPackedInt() const {
        return
            red |
            (uint32_t)(green << 8) |
            (uint32_t)(blue << 16) |
            (uint32_t)((255 - alpha) << 24);
    }

    uint32_t ToARGB32() const {
        return
            blue |
            (uint32_t)(green << 8) |
            (uint32_t)(red << 16) |
            (uint32_t)(alpha << 24);
    }

    static RgbaColor From(int r, int g, int b, int a = 255) {
        RgbaColor c;
        c.red   = (uint8_t)r;
        c.green = (uint8_t)g;
        c.blue  = (uint8_t)b;
        c.alpha = (uint8_t)a;
        return c;
    }

    static RgbaColor FromFloat(float r, float g, float b, float a = 1.0) {
        return From(
            (int)(255.1f * r),
            (int)(255.1f * g),
            (int)(255.1f * b),
            (int)(255.1f * a));
    }

    static RgbaColor FromPackedInt(uint32_t rgba) {
        return From(
            (int)((rgba)       & 0xff),
            (int)((rgba >> 8)  & 0xff),
            (int)((rgba >> 16) & 0xff),
            (int)(255 - ((rgba >> 24) & 0xff)));
    }

    static RgbaColor FromPackedIntBGRA(uint32_t bgra) {
        return From(
            (int)((bgra >> 16) & 0xff),
            (int)((bgra >> 8)  & 0xff),
            (int)((bgra)       & 0xff),
            (int)(255 - ((bgra >> 24) & 0xff)));
    }
};

struct RgbaColorCompare {
    bool operator()(RgbaColor a, RgbaColor b) const {
        return a.ToARGB32() < b.ToARGB32();
    }
};

class BBox {
public:
    Vector minp;
    Vector maxp;

    static BBox From(const Vector &p0, const Vector &p1);

    Vector GetOrigin() const;
    Vector GetExtents() const;

    void Include(const Vector &v, double r = 0.0);
    bool Overlaps(const BBox &b1) const;
    bool Contains(const Point2d &p, double r = 0.0) const;
};

#endif
