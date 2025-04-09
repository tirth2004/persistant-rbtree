#ifndef _VALUES_HPP_
#define _VALUES_HPP_

#include <vector>

template<typename Value>
class Values{
private:
    std :: vector<Value> values;
    int sz;
public:
    Values() : sz(0) {}
    Values(const std :: vector<Value>& initialValues) : values(initialValues), sz(initialValues.size()) {}

    int add(const Value value){
        values.push_back(value);
        return sz++;
    }

    int size() const{
        return sz;
    }

    Value& operator[](int index) {
        return values[index];
    }
};

#endif