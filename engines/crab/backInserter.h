
#ifndef CRAB_BACKINSERTER_H
#define CRAB_BACKINSERTER_H

#include "common/util.h"

namespace Crab {

template<class Container>
class backInserterIterator {
protected:
    Container *_container;

public:
    explicit backInserterIterator(Container& x) : _container(&x) {}

    backInserterIterator& operator=(const typename Container::value_type& value) {
        *_container += value;
        return *this;
    }

    backInserterIterator& operator=(const typename Container::value_type&& value) {
        *_container += Common::move(value);
        return *this;
    }

    backInserterIterator& operator*() { return *this; }
    backInserterIterator& operator++() { return *this; }
    backInserterIterator& operator++(int) { return *this; }
};

template<class Container>
backInserterIterator<Container>
backInserter(Container &x) {
    return backInserterIterator<Container>(x);
}

} // End of namespace Crab

#endif