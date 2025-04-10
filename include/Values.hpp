#ifndef _VALUES_HPP_
#define _VALUES_HPP_

#include <vector>

template<typename Value>
class Values{
private:
    std :: vector<Value> values;
public:

    int add(const Value value){
        values.push_back(value);
        return (int)values.size() - 1;
    }

    int size() const{
        return values.size();
    }

    void clear(){
        values.clear();
    }

    Value& operator[](int index) {
        return values[index];
    }
};

#endif